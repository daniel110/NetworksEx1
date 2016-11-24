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

class ServerSessionSocket : public Socket
{
    public:
		ServerSessionSocket(socket_handle socketFd);
		ServerSessionSocket(Socket& socket);
		ServerSessionSocket();

		~ServerSessionSocket();

        StateMachineStep getState();
        void setState(StateMachineStep value);

        void setInbox(Inbox* inbox);
        Inbox* getInbox();
        void close();

    private:
        StateMachineStep m_state;
        Inbox* m_inbox;

        void initState();
};


