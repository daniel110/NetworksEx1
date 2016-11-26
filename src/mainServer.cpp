#include <stdlib.h>
#include <stdio.h>

#include "Server.h"


int main (int argc, char ** argv)
{
	uint16_t localPort = Server::DEFAULT_PORT;
	char * users_file_path = nullptr;

	if (argc == 1)
	{
		printf("Usage: Server <Users File> [Port]\n");
		return EXIT_FAILURE;
	}
	if (argc > 1)
	{
		users_file_path = argv[1];
	}
	if (argc > 2)
	{
		/* We assume that the user will enter a valid port number */
		localPort = atol(argv[2]);
	}
	/* do not care if we got extra parameters */

	/* create and run client  */
	Server server = Server(localPort);
	if (server.getState() != Server::SERVER_INITIATED)
	{
		return EXIT_FAILURE;
	}

	server.loadUsersFromFile(users_file_path);
	if (server.getState() != Server::SERVER_READY_TO_LISTEN)
	{
		return EXIT_FAILURE;
	}

	server.start();

	return EXIT_SUCCESS;
}
