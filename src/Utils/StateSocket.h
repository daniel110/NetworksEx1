#pragma once

#include "Socket.h"
#include "Inbox.h"

enum StateMachineStep
{
    STATE_NON_AUTH,
    STATE_LOGEDON,
	STATE_SHOWINBOX,
    STATE_GETMAIL,
	STATE_DELETEMAIL,
	STATE_COMPOSE,

	/* At the current mail-server configuration, no need to use this state:
	 * since after quit request - we destroy the session */
	STATE_QUIT
};

class StateSocket : public Socket
{
    public:
		StateSocket(socket_handle socketFd);
		StateSocket();

        StateMachineStep getState();
        void setState(StateMachineStep value);


    private:
        StateMachineStep m_state;

        void initState();
};


