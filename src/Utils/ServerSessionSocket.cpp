#include "ServerSessionSocket.h"

ServerSessionSocket::ServerSessionSocket(int socketFd) : Socket(socketFd)
{
	initState();
}

ServerSessionSocket::ServerSessionSocket()
{
	initState();
}

void ServerSessionSocket::initState()
{
	m_state = STATE_NON_AUTH;
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
}

Inbox* ServerSessionSocket::getInbox()
{
	return m_inbox;
}



