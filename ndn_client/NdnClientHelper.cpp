//
// Created by mingj on 19-3-5.
//

#include "JSONCPPHelper.h"
#include <boost/thread/pthread/shared_mutex.hpp>
#include <boost/algorithm/string.hpp>
#include <OnlinePreviewer.h>
#include "protocol.h"
#include "UUIDUtils.h"
#include "NdnClientHelper.h"
#include <boost/thread.hpp>
#include "TimeUtils.h"

const string NdnClientHelper::FILE_SLICE_PREFIX = "fileSlicePrefix";
const string NdnClientHelper::FILE_INFO_PREFIX = "fileInfoPrefix";
const string NdnClientHelper::IS_PREVIEW = "isPreview";

NdnClientHelper *NdnClientHelper::getInstance() {
    static NdnClientHelper *instance = nullptr;
    if (instance == nullptr) {
        static boost::shared_mutex mutex;
        boost::shared_lock<boost::shared_mutex> m(mutex);
        if (instance == nullptr) {
            instance = new NdnClientHelper();
        }
    }
    return instance;
}

NdnClientHelper *NdnClientHelper::updateConfig(const string &configPath , const string group_name) {
	this->m_group_name = group_name ;
    JSONCPPHelper jsoncppHelper(configPath, 1);

    this->fileSlicePrefix = jsoncppHelper.getString(FILE_SLICE_PREFIX);
    vector<string> fileds;
    boost::split(fileds, this->fileSlicePrefix, boost::is_any_of("/"));
    this->fileSlicePrefixLen = fileds.size();


    this->fileInfoPrefix = jsoncppHelper.getString(FILE_INFO_PREFIX);
    fileds.clear();
    boost::split(fileds, this->fileInfoPrefix, boost::is_any_of("/"));
    this->fileInfoPrefixLen = fileds.size();

    this->isPreview = jsoncppHelper.getBool(IS_PREVIEW);
    return this;
}

NdnClientHelper *NdnClientHelper::getFile(const string &ip, unsigned short port, const string &fileName) {
    cout << "start: " << TimeUtils::getCurTime() << endl;
	cout << "filename = " << fileName << endl ;
    ndnHelper.expressInterest(
            this->getBaseFileInfoPrefix(ip, port, fileName) + "/" + UUIDUtils::generateUUid(),
            std::bind(&NdnClientHelper::getFileInfoOnData, this, std::placeholders::_1, std::placeholders::_2,
                      ip, port, fileName),
            std::bind(&NdnClientHelper::onNack, this, std::placeholders::_1, std::placeholders::_2),
            std::bind(&NdnClientHelper::onTimeout, this, std::placeholders::_1));
    return this;
}

void NdnClientHelper::onNack(const Interest &interest, const lp::Nack &nack) {
    cout << "NACK: " << interest.getName().toUri() << endl;
}

void NdnClientHelper::onTimeout(const Interest &interest) {
    cout << interest.getName().toUri() << " timeout" << endl;
}

void NdnClientHelper::onSliceTimeout(const Interest &interest, const string &basePrefix, int sequence, size_t chunkSize,
                                     size_t totalCount,
                                     boost::filesystem::path outputPath) {
    cout << interest.getName().toUri() << " timeout and retrans" << endl;
    ndnHelper.expressInterest(basePrefix + "/" + to_string(sequence),
                              std::bind(&NdnClientHelper::getFileOnData, this, std::placeholders::_1,
                                        std::placeholders::_2, sequence, totalCount, basePrefix,
                                        chunkSize,
                                        outputPath),
                              std::bind(&NdnClientHelper::onNack, this, std::placeholders::_1,
                                        std::placeholders::_2),
                              std::bind(&NdnClientHelper::onSliceTimeout, this, std::placeholders::_1, basePrefix,
                                        sequence, chunkSize, totalCount, outputPath));
}

void NdnClientHelper::getFileInfoOnData(const Interest &interest, const Data &data, const string &ip,
                                        unsigned short port, const string &fileName) {
    getInstance()->threadPool.enqueue([=]{
        const auto &content = data.getContent();
        string json((char *) content.value(), content.value_size());
        ResponseBody responseBody = ProtocolHelper::jsonToResponseBody(json);
        cout << responseBody.toJson() << endl;

        //////////////////////////////////////////
        /////// 下面获取所有的文件块，并写到文件当中
        //////////////////////////////////////////

        auto outputPath = FileUtils::getOutputPath();

        outputPath.append(fileName);

        //如果不存在就创建文件
        FileUtils::createFileIfNotExist(outputPath);


        this->os.open(outputPath, std::ios_base::binary | std::ios_base::out | std::ios_base::in);
        int count = (int) (responseBody.fileSize / responseBody.chunkSize) + 1;
        string basePrefix = this->getBaseFileSlicePrefix(ip, port, fileName);
        ssize_t sequence;
        while ((sequence = this->sequenceManager.getNextSequence()) != -1) {
            ndnHelper.expressInterest(basePrefix + "/" + to_string(sequence),
                                      std::bind(&NdnClientHelper::getFileOnData, this, std::placeholders::_1,
                                                std::placeholders::_2, sequence, count, basePrefix, responseBody.chunkSize,
                                                outputPath),
                                      std::bind(&NdnClientHelper::onNack, this, std::placeholders::_1,
                                                std::placeholders::_2),
                                      std::bind(&NdnClientHelper::onSliceTimeout, this, std::placeholders::_1, basePrefix,
                                                sequence, responseBody.chunkSize, count, outputPath));
        }
    });
}

void NdnClientHelper::getFileOnData(const Interest &interest, const Data &data, int position, int totalCount,
                                    const string &basePrefix, int chunkSize, boost::filesystem::path outputPath) {
    receiveSliceCount++;
    getInstance()->threadPool.enqueue([=]{
        sequenceManager.ackSequence(static_cast<size_t>(position));

        static bool preview = false;
        // 偏移到指定位置
        os.seekp(position * chunkSize, std::ios::beg);
        // 将数据写入到文件当中
        os.write((char *) data.getContent().value(),
                 data.getContent().value_size());
        if (isPreview && !preview && (position > 1000 || 2 * position > totalCount)) {
            preview = true;
            os.close();
            cout << outputPath.string() << endl;
            boost::thread t([=]() {
                OnlinePreviewer().preview(outputPath.string());
            });
            os.open(outputPath, std::ios_base::binary | std::ios_base::out |
                                std::ios_base::in);

        }
        if (receiveSliceCount == totalCount) {
            os.flush();
            os.close();

            cout << "end: " << TimeUtils::getCurTime() << endl;

            //手动退出
            exit(0);
        } else {
            ssize_t sequence;
            while ((sequence = this->sequenceManager.getNextSequence()) != -1 && sequence < totalCount) {
                ndnHelper.expressInterest(basePrefix + "/" + to_string(sequence),
                                          std::bind(&NdnClientHelper::getFileOnData, this, std::placeholders::_1,
                                                    std::placeholders::_2, sequence, totalCount, basePrefix,
                                                    chunkSize,
                                                    outputPath),
                                          std::bind(&NdnClientHelper::onNack, this, std::placeholders::_1,
                                                    std::placeholders::_2),
                                          std::bind(&NdnClientHelper::onSliceTimeout, this, std::placeholders::_1, basePrefix,
                                                    sequence, chunkSize, totalCount, outputPath));
            }
        }
    });

}

string file2string(string filepath){
	std::fstream fs(filepath.data());
	stringstream ss ;
	ss << fs.rdbuf();
	return ss.str();
}
string encode_key(string str){
	char buff[2000] ;
	int str_len = str.length();
	int buff_len = 0;
	for (int i = 0; i < str_len; i++) {
		if(str[i]=='{' || str[i] == '}') buff[buff_len++]='A' ;
		else if(str[i] == '\n') buff[buff_len++] = '~';
		else if(str[i] == '"') buff[buff_len++] = '_';
		else if(str[i] == ':') buff[buff_len++] = '-';
		else if(str[i] == ',') buff[buff_len++] = '.';
		else if(str[i] == ' ') continue ;
		else buff[buff_len++] = str[i] ;
	}
	buff[buff_len++] = '\0';
	string result = buff ;
	return result ;
}

string get_sig(string group_name , string msg){
	string cmd = "./gsc -s "+group_name+" "+msg + " > log" ;
	cout << "cmd = " << cmd << endl ; 
	system(cmd.data());
	return file2string("./pipefile");
}
const string NdnClientHelper::getBaseFileInfoPrefix(const string &ip, unsigned short port, const string &fileName) {
    string interest_name =  this->fileInfoPrefix + "/" + ip + "/" + to_string(port) + "/" + fileName;
	string sig = get_sig(this->m_group_name ,interest_name) ;
	sig += this->m_group_name;
	cout << "sig =  \n" << sig << endl ;
	cout <<"sig =  \n" << encode_key(sig) << endl ;
	interest_name += "/"+encode_key(sig);
	cout << interest_name << endl ;
	//exit(0);
	return interest_name ;
}



const string NdnClientHelper::getBaseFileSlicePrefix(const string &ip, unsigned short port, const string &fileName) {
	string interest_name = this->fileSlicePrefix + "/" + ip + 
		"/" + to_string(port) + "/" + fileName;
	//interest_name += "/"+encode_key(get_sig(this->m_group_name ,interest_name));
    return interest_name ;
}

void NdnClientHelper::start() {
    ndnHelper.processEvent();
}




