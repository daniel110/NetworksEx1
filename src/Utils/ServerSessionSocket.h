#pragma once

#include "Socket.h"
#include "Inbox.h"
#include "Common.h"

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
		/****
		 * Ctor - Initialize Socket with *socketFd*
		 */
		ServerSessionSocket(socket_handle socketFd);

		/****
		 * Ctor - Initialize Socket with *socket*, and
		 * 			make *socket* to be invalid
		 */
		ServerSessionSocket(Socket& socket);

		/****
		 * Ctor - default
		 */
		ServerSessionSocket();

		/****
		 * Close socket - call close;
		 */
		virtual ~ServerSessionSocket();

		/****
		 * return current state
		 */
        StateMachineStep getState();

        /****
         * Update state to *value* state.
         */
        void setState(StateMachineStep value);

        void sendGeneralRespond(GeneralRespondStatuses res);

        /****
         * get session unique id
         */
        int getID() const;

        /****
         * Set inbox object and update that the inbox is now logged on
         */
        void setInbox(Inbox* inbox);

        /****
         * return inbox object
         */
        Inbox* getInbox();

        /***
         * Close socket and disassociate inbox from the session.
         * From now on the object is not usable
         */
        void close();

    private:
        StateMachineStep m_state;
        Inbox* m_inbox;

        void initState();
};


