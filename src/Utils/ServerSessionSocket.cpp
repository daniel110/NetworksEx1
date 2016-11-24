#include "ServerSessionSocket.h"

ServerSessionSocket::ServerSessionSocket(int socketFd) : Socket(socketFd)
{
	initState();
}

ServerSessionSocket::ServerSessionSocket(Socket& socket) : Socket(socket.m_socketfd)
{
	socket.m_socketfd = SOCKET_INVALID_DEFAULT;
	initState();
}

ServerSessionSocket::ServerSessionSocket()
{
	initState();
}

void ServerSessionSocket::initState()
{
	m_state = STATE_NON_AUTH;
	m_inbox = nullptr;
}

StateMachineStep ServerSessionSocket::getState()
{
	return m_state;
}
void ServerSessionSocket::setState(StateMachineStep value)
{
	m_state = value;
}


void ServerSessionSocket::setInbox(Inbox* inbox)
{
	m_inbox = inbox;
	if (inbox != nullptr)
	{
		inbox->loged_in = true;
	}
}

Inbox* ServerSessionSocket::getInbox()
{
	return m_inbox;
}

void ServerSessionSocket::close()
{
	if (m_inbox != nullptr)
	{
		m_inbox->loged_in = false;
		m_inbox = nullptr;
	}
	Socket::close();
}

ServerSessionSocket::~ServerSessionSocket()
{
	if (m_inbox != nullptr)
	{
		m_inbox->loged_in = false;
	}
}


