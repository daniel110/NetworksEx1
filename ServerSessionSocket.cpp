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

int ServerSessionSocket::getID() const
{
	return m_socketfd;
}

void ServerSessionSocket::setInbox(Inbox* inbox)
{
	m_inbox = inbox;
	if (inbox != nullptr)
	{
		inbox->m_loggedIn = true;
		inbox->m_session = this;
	}
}

void ServerSessionSocket::sendGeneralRespond(GeneralRespondStatuses res)
{
	Packet response;
	response.writeForwardDWord(COMMANDTYPE_GENERAL_MESSAGE);
	response.writeForwardDWord(res);

	/* TODO: check response val */
	sendMessage(response);
}

Inbox* ServerSessionSocket::getInbox()
{
	return m_inbox;
}

void ServerSessionSocket::close()
{
	if (m_inbox != nullptr)
	{
		m_inbox->m_loggedIn = false;
		m_inbox->m_session = nullptr;
		m_inbox = nullptr;
	}
	Socket::close();
}

ServerSessionSocket::~ServerSessionSocket()
{
	close();
}


