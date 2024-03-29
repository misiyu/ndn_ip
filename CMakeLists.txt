cmake_minimum_required(VERSION 3.5.1)
project(ndnip_proxy)

set(CMAKE_CXX_STANDARD 14)
include_directories(ip_server)
include_directories(ndn_client)
include_directories(ndn_ip_proxy)
include_directories(utils)
include_directories(protocol)
include_directories(common)
include_directories(manager)
include_directories(wordfilter)

#add_executable(ndnip_proxy main.cpp ip_server/ip_server.cpp ndn_ip_proxy/ndn_ip_proxy.cpp)

add_executable(nip wordfilter/wordfilter.cpp ndn_ip_proxy/ndn_ip_proxy.cpp ndn_ip_proxy/BoostTCPClientHelper.cpp utils/FileUtils.cpp
        utils/JSONCPPHelper.cpp protocol/ProtocolHelper.cpp protocol/BaseProtocolItem.cpp common/common.cpp
        protocol/protocol.cpp utils/NDNHelper.cpp ndn_ip_proxy/NdnIPProxyHelper.cpp utils/MutexHelper.cpp)
target_link_libraries(nip ndn-cxx boost_system boost_thread boost_filesystem jsoncpp pthread)

add_executable(is wordfilter/wordfilter.cpp ip_server/ip_server.cpp utils/FileUtils.cpp utils/JSONCPPHelper.cpp protocol/ProtocolHelper.cpp
        protocol/BaseProtocolItem.cpp common/common.cpp utils/MutexHelper.cpp manager/ResourceManager.cpp
        protocol/protocol.cpp ip_server/BoostTCPServerHelper.cpp )

target_link_libraries(is boost_system boost_thread boost_filesystem jsoncpp pthread)


add_executable(nc wordfilter/wordfilter.cpp ndn_client/ndn_client.cpp utils/NDNHelper.cpp utils/JSONCPPHelper.cpp protocol/BaseProtocolItem.cpp
        protocol/ProtocolHelper.cpp utils/FileUtils.cpp utils/Previewer.cpp utils/OnlinePreviewer.cpp
        ndn_client/NdnClientHelper.cpp utils/UUIDUtils.cpp manager/SequenceManager.cpp utils/TimeUtils.cpp )
target_link_libraries(nc ndn-cxx boost_system boost_thread boost_filesystem jsoncpp pthread)


###############################################
########## Test
###############################################
#add_executable(test_fcw test/test_fstram_cover_write.cpp utils/FileUtils.cpp ndn_ip_proxy/BoostTCPClientHelper.cpp
#        utils/JSONCPPHelper.cpp protocol/BaseProtocolItem.cpp protocol/ProtocolHelper.cpp common/common.cpp)
#target_link_libraries(test_fcw boost_system boost_filesystem jsoncpp pthread)
#target_link_libraries(ndnip_proxy pcap ndn-cxx boost_system boost_thread jsoncpp pthread)

#add_executable(test_boost_timer test/test_boost_timer.cpp )
#target_link_libraries(test_boost_timer boost_system)
#
#add_executable(test_rmf test/test_rmf.cpp manager/ResourceManager.cpp utils/FileUtils.cpp)
#target_link_libraries(test_rmf boost_system boost_filesystem)
#
#add_executable(test_slide_window test/test_slide_window.cpp manager/SequenceManager.cpp utils/MutexHelper.cpp)
#target_link_libraries(test_slide_window boost_system boost_filesystem boost_thread)