#include <iostream>

#include "Client.h"


#define DEFAULT_HOST ("localhost")


int main (int argc, char ** argv)
{
	std::string remoteHost(DEFAULT_HOST);
	u_int16_t remotePort = Client::DEFAULT_PORT;


	if (argc > 1)
	{
		remoteHost = argv[1];
	}
	if (argc > 2)
	{
		/* We assume that the user will enter a valid port number */
		remotePort = atoi(argv[2]);
	}
	/* do not care if we got extra parameters */


	/* create and run client  */
	Client client(remoteHost, remotePort);
	client.start();


	return EXIT_SUCCESS;
}


