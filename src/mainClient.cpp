#include <iostream>

#include "Client.h"


#define DEFAULT_HOST ("localhost")
#define DEFAULT_PORT (6423)


int mainb (int argc, char ** argv)
{
	std::string remoteHost(DEFAULT_HOST);
	u_int16_t remotePort = DEFAULT_PORT;


	if (argc > 1)
	{
		remoteHost = argv[1];
	}
	if (argc > 2)
	{
		remotePort = atoi(argv[2]);
	}
	/* do not care if we got extra parameters */


	/* create and run client  */
	Client client(remoteHost, remotePort);
	client.start();


	return EXIT_SUCCESS;
}


