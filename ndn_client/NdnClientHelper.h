//
// Created by mingj on 19-3-5.
//

#ifndef NDNIP_PROXY_NDNCLIENTHELPER_H
#define NDNIP_PROXY_NDNCLIENTHELPER_H

#include "NDNHelper.h"
#include <FileUtils.h>
#include "SequenceManager.h"
#include "ThreadPool.h"

using namespace std;

class NdnClientHelper {

public:
    static NdnClientHelper *getInstance();

    /**
     * 更新配置信息
     * @param configPath
     * @return
     */
    NdnClientHelper *updateConfig(const string &configPath, const string group_name);


    /**
     * 从一个IP主机获取文件
     * @return
     */
    NdnClientHelper *getFile(const string &ip, unsigned short port, const string &fileName);

    void start();

    ////////////////////////////////////////////
    /////// Callback
    ////////////////////////////////////////////
    void onNack(const Interest &interest, const lp::Nack &nack);

    void onTimeout(const Interest &interest);

    void onSliceTimeout(const Interest &interest, const string &basePrefix, int sequence, size_t chunkSize,
                        size_t totalCount,
                        boost::filesystem::path outputPath);

    void getFileInfoOnData(const Interest &interest,
                           const Data &data, const string &ip, unsigned short port, const string &fileName);

    void getFileOnData(const Interest &interest, const Data &data,
                       int position,
                       int totalCount,
                       const string &basePrefix, int chunkSize,
                       boost::filesystem::path outputPath);

    const string getBaseFileInfoPrefix(const string &ip, unsigned short port, const string &fileName);

    const string getBaseFileSlicePrefix(const string &ip, unsigned short port, const string &fileName);

private:

    NdnClientHelper(size_t windowSize = 5, size_t threadPoolSize = 10)
            : sequenceManager(0, windowSize), threadPool(threadPoolSize) {};

    static const string FILE_SLICE_PREFIX;
    static const string FILE_INFO_PREFIX;
    static const string IS_PREVIEW;
    NDNHelper ndnHelper;
    mingj::manager::SequenceManager sequenceManager;
    ThreadPool threadPool;
    boost::filesystem::fstream os;
    size_t receiveSliceCount = 0;
    //config
    string fileSlicePrefix;
    size_t fileSlicePrefixLen{};
    string fileInfoPrefix;
    size_t fileInfoPrefixLen{};
    bool isPreview{};
	string m_group_name ;
};


#endif //NDNIP_PROXY_NDNCLIENTHELPER_H
