#pragma once

#include "Socket.h"
#include "Inbox.h"

enum StateMachineStep
{
    STATE_NON_AUTH,
    STATE_LOGEDON,
    STATE_MAILREQ


};

class ServerSocketSession : public Socket
{
    public:
		ServerSocketSession(int socketFd) : Socket(socketFd)
    	{
			initState();
    	}

		ServerSocketSession()
        {
			initState();
        }

		void initState()
		{
			m_state = STATE_NON_AUTH;
			m_userInbox = nullptr;
		}

        StateMachineStep getState()
        {
            return m_state;
        }
        void setState(StateMachineStep value)
        {
            m_state = value;
        }

        void setInbox(Inbox * ibx);


    private:
        StateMachineStep m_state;
        Inbox * m_userInbox;
};


