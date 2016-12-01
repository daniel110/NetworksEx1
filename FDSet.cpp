#include "FDSet.h"

void FDSet::add(const Socket& sock)
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
bool FDSet::check(const Socket& sock)
{
	if (sock.isValid())
	{
		return FD_ISSET(sock.m_socketfd, &m_fdset);
	}
	return false;

}
void FDSet::clear()
{
	FD_ZERO(&m_fdset);
	max_sd = 0;
}
bool FDSet::waitOnSockets()
{
	int activity = select(max_sd+1, &m_fdset, NULL, NULL, NULL);

	if ((activity < 0) && (errno!=EINTR))
	{
		return false;
	}
	return true;
}



