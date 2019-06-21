#ifndef _BCHELPER_H_
#define _BCHELPER_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <jsoncpp/json/json.h>
#include <sys/time.h>
#include <boost/array.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio.hpp>

using std::string ;
using std::cout ;
using std::endl ;
using std::stringstream ;
using namespace boost::asio;

#define BC_SERV_PORT 8010
#define BUFF_SIZE 4000

static string file2string(string filepath){
	std::fstream fs(filepath.data());
	stringstream ss ;
	ss << fs.rdbuf();
	return ss.str();
}

static int string2file(string str , string filepath){
	std::ofstream fs(filepath.data());
	fs << str ;
	fs.close();
	return 0 ;
}


string wgh_sendMessage(string msg , string ip)
{
	io_service iosev;
	// socket对象
	ip::tcp::socket socket(iosev);
	// 连接端点，这里使用了本机连接，可以修改IP地址测试远程连接
	cout << ip << endl ;
	//ip = "121.15.171.86";
	ip::tcp::endpoint ep(ip::address_v4::from_string(ip.data()), 8010);
	// 连接服务器
	//cout << "hello" << endl ;
	//exit(0);
	boost::system::error_code ec;
	socket.connect(ep,ec);
	// 如果出错，打印出错信息
	if(ec)
	{
		std::cout << boost::system::system_error(ec).what() << std::endl;
		throw "";
	}
	int len=msg.size();
	std::string len_str((char*)(&len),sizeof(int));
	socket.send(buffer(len_str)) ;
	socket.send(buffer(msg));
	// 接收数据
	boost::asio::streambuf sb(sizeof(int));
	//boost::system::error_code ec;
	boost::asio::read(socket, sb, ec);
	boost::asio::streambuf::const_buffers_type bufs = sb.data();
	std::string line(boost::asio::buffers_begin(bufs),boost::asio::buffers_begin(bufs) + sizeof(int));
	const char *ch=line.data();
	int size=*((int*)ch);
	//std::cout << "received: '" << size << "'\n";
	if (ec) {
		std::cout << "status: " << ec.message() << "\n";
		//return;
	}
	boost::asio::streambuf sb1(size);
	boost::system::error_code ec1;
	boost::asio::read(socket, sb1, ec1);
	//std::cout << "received: '" << &sb1 << "'\n";
	if (ec1) {
		std::cout << "status: " << ec1.message() << "\n";
		//return;
	}
	boost::asio::streambuf::const_buffers_type buf1 = sb1.data();
	std::string received_msg(boost::asio::buffers_begin(buf1),boost::asio::buffers_begin(buf1) + size);
	//std::cout<<received_msg<<"\n";
	socket.close();
	return received_msg;
}

string wgh_get_timestamp(){
	timeval start;  
	gettimeofday(&start, NULL); 
	//return std::to_string(start.tv_sec);
	return ctime(&(start.tv_sec));
}
// wgh
static int sendlog2bc(string message){
	int tmp1 = message.find_first_of('~');
	if(tmp1 == string::npos) return 1;
	tmp1 -= 1;
	int tmp2 = message.find_last_of('/');
	int tmp3 = tmp2 ;
	while(message.at(tmp2) != '~') tmp2 -- ;
	tmp2 ++ ;

	string source_name = message.substr(0 , tmp1-1);
	string sig = message.substr(tmp1,tmp2-tmp1);
	string real_msg = message.substr(tmp2 , tmp3 - tmp2);

	Json::Value root ;
	root["type"]="NDN-IP";
	root["data"]["command"]="Log";
	root["data"]["QueryCode"]= 21;
	root["data"]["name"]=source_name;
	root["data"]["sig"] = sig;
	root["data"]["real_msg"] = real_msg;
	root["data"]["timestamp"] = wgh_get_timestamp();
	string log = root.toStyledString();

	string ip = file2string("bcserverip.conf") ;
    size_t n = ip.find_last_not_of(" \r\n\t");
    if (n != string::npos){
        ip.erase(n + 1, ip.size() - n);
    }
    n = ip.find_first_not_of(" \r\n\t");
    if (n != string::npos){
        ip.erase(0, n);
    }
	if(ip == ""){
		std::cout << "err ip : " << ip << std::endl ;
		return 0 ;
	}
	int port = BC_SERV_PORT ;
	cout << log << endl ;
	cout << ip << endl ;
	
	wgh_sendMessage(log , ip);
	return 0 ;

}

#endif 
