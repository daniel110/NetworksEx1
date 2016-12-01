#pragma once

#include "Socket.h"
#include "Inbox.h"
#include "Common.h"

enum StateMachineStep
{
    STATE_NON_AUTH,
    STATE_LOGEDON,
};


/* ServerSessionSocket Class
 *
 * Session represents a connected socket with a client.
 * The session has a state (StateMachineStep) that represent the state
 * of the client with the server.
 *
 * If the client is logged in, The session also has a pointer to an Inbox
 * that represent the logged in user Inbox. The Inbox is marked as logged
 * in during this time.
 *
 */
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

        /****
         * Send a general response to the client.
         */
        void sendGeneralRespond(GeneralRespondStatuses res);

        /****
         * get session unique id. This value should not have any meaning except
         * the ID of the session.
         */
        int getID() const;

        /****
         * Set inbox object and update that the inbox is now logged on.
         * This only saves the pointer. Could be nullptr.
         */
        void setInbox(Inbox* inbox);

        /****
         * return inbox object.
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

        /***
         * Initiate the internal state of the session.
         */
        void initState();
};


