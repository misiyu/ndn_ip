#include <iostream>
#include "bcHelper.h"

using namespace std;

int main()
{

	string str = "/NDN-IP/FILE_INFO/127.0.0.1/9758/hello/A~_T1_-_2381d35509a391925f92442ab640159d1771f02a933420ff2ad4d3358241f6c1_.~_T2_-_0cbc63a4744712453ae9acb60fe1bac003ae8ebe66c4b9a1b69b5f6b13ca7b01_.~_T3_-_17d5b25cccb4466440ffd69120cc45751d1b643720b06ca93ffa5f59ececd3f7_.~_c_-_04ae3bb194983a743e9684d93190de13_.~_ralpha_-_188f300218341bad3b7a7e8a440cc6dc_.~_rbeta_-_019427005626c47cd76dbff3d88ff5cd_.~_rdelta1_-_213080a28a9764097bebee4f10813fd4_.~_rdelta2_-_0d8200c6ae8ad54d03d83ddf4254abd0_.~_rx_-_088cc1023928b3fba55d54bc8c1ba856_~A~/ecb47fd7-b405-4c22-95e6-4565a1e2c674" ;

	sendlog2bc(str);

	return 0;
}
