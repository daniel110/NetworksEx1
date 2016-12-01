#pragma once

#include <sys/select.h>
#include <errno.h>
#include "Socket.h"


/* FDSet class
 *
 * a Wrapper for the fd_set struct and methods that allow more readable
 * code. This enable us to use our own Socket Class directly with the add, check operations.
 * We also wrap the select system call in waitOnSockets().
 *
 * Instructions:
 * -First call clear() to clean the fd_set struct.
 * -use add() to add any socket you want to use in select.
 * -call waitOnSockets() to call for select system call. This is a blocking method.
 * -After waitOnSockets() returned, use check() to check any socket you wish for new data
 * 	or new connections.
 *
 * -Before you call waitOnSockets() again, use clean() and add() again,
 */
class FDSet
{

public:

	/* Add a socket to the socket we will select from */
	void add(const Socket& sock);
	/* Check if the given socket has new data or new connection.
	 * Returns true if so.
	 */
	bool check(const Socket& sock);
	/* Clear all sockets from the list */
	void clear();
	/* This method calls select system call with all the socket you added usiging add().
	 * This method is blocking
	 */
	bool waitOnSockets();

private:
	fd_set m_fdset;
	/* needed for select system call */
	int max_sd;
};

