/*
 * test_SocketServer.cpp
 *
 *  Created on: Nov 19, 2016
 *      Author: df
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>

#include "../../Socket.h"

int maind(int argc, char** argv)
{
	char* ip = argv[1];
	char* port = argv[2];
	int res;

	Socket listener;
	res = listener.create();
	res = listener.bind(ip,atoi(port));

	res = listener.listen(3);

	Socket client; /* no need to call create */
	res = listener.accept(client);
	printf("Got request");

	int i = 0;
	std::string s("Hiii ");
	while (i < 3)
	{
		char strInt[3];
		sprintf(strInt, "%d " , i);
		s = s + strInt;

		Packet sendPack;
		sendPack.writeForwardStringField(s);

		res = client.sendMessage(sendPack);

		Packet pack;
		res = client.recvMessage(pack);

		std::string out;
		pack.readForwardStringField(out);
		printf("RECV: %s\n", out.c_str());

		i++;
	}


	printf("BYYEEEE Server\n");
	return 0;
}

