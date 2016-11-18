#include "StateSocket.h"

StateSocket::StateSocket(int socketFd) : Socket(socketFd)
{
	initState();
}

StateSocket::StateSocket()
{
	initState();
}

void StateSocket::initState()
{
	m_state = STATE_NON_AUTH;
}

StateMachineStep StateSocket::getState()
{
	return m_state;
}
void StateSocket::setState(StateMachineStep value)
{
	m_state = value;
}





