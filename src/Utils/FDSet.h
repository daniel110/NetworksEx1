#pragma once

#include <sys/select.h>
#include "Socket.h"

class FDSet
{

public:

	void add(const Socket& sock)
	{
		if (sock.isValid())
		{
			FD_SET(sock.m_socketfd, &m_fdset);
			if (sock.m_socketfd > max_sd)
			{
				max_sd = sock.m_socketfd;
			}
		}
	}
	bool check(const Socket& sock)
	{
		if (sock.isValid())
		{
			return FD_ISSET(sock.m_socketfd, &m_fdset);
		}
		return false;

	}
	void clear()
	{
		FD_ZERO(&m_fdset);
		max_sd = 0;
	}
	bool waitOnSockets()
	{
		int activity = select(max_sd+1, &m_fdset, NULL, NULL, NULL);

        if ((activity < 0) && (errno!=EINTR))
        {
            return false;
        }
        return true;
	}

private:
	fd_set m_fdset;
	int max_sd;
};

