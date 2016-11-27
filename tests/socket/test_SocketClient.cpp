/*
 * test_SocketClient.cpp
 *
 *  Created on: Nov 19, 2016
 *      Author: df
 */

#include "../../src/Utils/Socket.h"
#include <stdio.h>

int mainc(int argc, char** argv)
{
	char* ip = argv[1];
	char* port = argv[2];
	int res;

	Socket client;
	res = client.create();
	res = client.connect(ip,atoi(port));
	printf("Connected\n");

	int i = 0;
	std::string s("Byeeeeeeeee ");
	while (i < 3)
	{
		Packet pack;
		res = client.recvMessage(pack);

		std::string out;
		pack.readForwardStringField(out);
		printf("RECV: %s\n", out.c_str());


		char strInt[3];
		sprintf(strInt, "%d " , i);
		s = s + strInt;


		Packet sendPack;
		sendPack.writeForwardStringField(s);

		res = client.sendMessage(sendPack);

		i++;
	}


	printf("BYYEEEE client\n");
	return 0;
}
