#pragma once

#include <list>
#include <string>

#include <string.h>

#include "User.h"
#include "Inbox.h"
#include "MailObj.h"
#include "ServerSessionSocket.h"
#include "Common.h"
#include "FDSet.h"

/*	Server class
 *
 * 	Server side of the mail service.
 * 	Instructions:
 * 	- Construct the client with the port.
 * 	  (Default port is DEFAULT_PORT)
 *	- Call loadUsersFromFile() to load the user files.
 *	- Call start()
 *
 */
class Server
{
    public:

		enum SERVER_STATE
		{
			SERVER_LISTEN_SOCKET_ERROR,
			SERVER_SOCKET_ERROR,
			SERVER_USERFILE_ERROR,


			SERVER_INITIATED,
			SERVER_READY_TO_LISTEN,
			SERVER_RUNNING,
			SERVER_FINISHED,
		};

		/*	Default values */
		static const long DEFAULT_PORT = 6423;
		static const long MAX_CONNECTIONS_QUEUE = 50;

		/* Create a server and bind a socket with the port and ANY_IP.
		 *
		 * On success, getState() will return SERVER_INITIATED.
		 * else, it will return SERVER_LISTEN_SOCKET_ERROR and the linux socket error string
		 * will be printed to the user.
		 *
		 */
        Server(uint16_t port);

        /* Release session and inbox lists. */
        virtual ~Server();

        /* Load users list and Initiate the Inbox list.
         * Inbox is created and added to the Inbox list for every user in the user file.
         *
         * File format:
         * - Each line is a user entry with the format: User name, a tab, his password.
         * - No spaces is allowed.
         *
         * In order for the method to run, getState() must be SERVER_INITIATED.
         *
         * On success, getState() will return SERVER_READY_TO_LISTEN.
         * else, it will return SERVER_USERFILE_ERROR and the reason the file failed
         * to open will be printed to the user.
         *
         * @return: True if success.
         */
        bool loadUsersFromFile(char * filePath);

        /* Start the server, start listen on the given port.
         *
         * In order for the method to run, getState() must be SERVER_READY_TO_LISTEN.
         *
         * We listen to the port with MAX_CONNECTIONS at the time.
         */
        void start();

        /* Get the current state of the server */
        SERVER_STATE getState()
        {
        	return m_state;
        }

        /* Debug prints level */
        enum DEBUGLEVEL
		{
        	NO_LOG,
			EVENTS,
			INFO,
			INFO_MORE,
		};
        DEBUGLEVEL debug_log = INFO_MORE;

    private:
        /* The internal state of the server */
        SERVER_STATE m_state;
        /* Main listiner for new connections */
        Socket listener;
        /* Inbox list, every inbox is a user with its mails */
        std::list<Inbox*> m_all_inbox;
        /* Session list, every session is a socket.
         * If a user logged in, an inbox is attached to the session.
         * Every session has an internal state which controls what operations can
         * the client do.
         */
		std::list<ServerSessionSocket*> m_sessions;

		/* Load user file and create Inbox list.
		 * These are internal helper functions. */
		User * getUserFromLine(char * buf, int size);
		std::list<User*> * getUsersFromFile(char * filePatch);
        void createInboxList(std::list<User*>& users);

        /* Dectrutor methods */
        void clearInboxList();
        void clearSessionList();

        /* Get an inbox, from the inbox list, that represent the given user.
         * This function does not change the Inbox list.
         *
         * @return: a pointer to an Inbox.
         * 			If Inbox does not found, nullptr is returnd.
         */
        Inbox * getInboxFromUserString(const std::string& user);

        /* Get a session and call the right session handler according to the
         * session state.
         */
        void processRequset(ServerSessionSocket& session);

        /* Get the message from the session and return the type of the message.
         * The message is returned in the message_result argument.
         *
         * If COMMANDTYPE_UNVALID_REQ is returned as the message type, the message did not
         * returned and message_result content is undefined.
         * The appropriate general respond has been sent to the client and the session may end as a result.
         *
         * @return: The type of the message. */
        int32_t getMessageAndType(ServerSessionSocket& session, Packet& message_result);


        /* *************************
         * Session Handlers by state
         * ************************
         */
        /* Client needs to log in, check login message */
        void sessionLogin(ServerSessionSocket& session);
        /* Find what the client is requesting and call the right request handler. */
        void sessionCommandRequest(ServerSessionSocket& session);
        /* Session just created, send a welcome message to the client.
         * This method is not called by the processRequset() method but by the ListenLoop in start() */
        void sessionWelcome(ServerSessionSocket& session);


        /* *************************
         * Session Request Handlers by message type.
         * sessionCommandRequest() calls these methods.
         * ************************
         */
        void sessionRequestShowInbox(ServerSessionSocket& session, Packet& message);
        void sessionRequestGetMail(ServerSessionSocket& session, Packet& message);
        void sessionRequestDeleteMail(ServerSessionSocket& session, Packet& message);
        void sessionRequestCompose(ServerSessionSocket& session, Packet& message);
        void sessionRequestShowOnlineUsers(ServerSessionSocket& session, Packet& message);
        void sessionRequestSendChatMessage(ServerSessionSocket& session, Packet& message);

        /* Print strings to user */
        bool printSocketError(int result);
        bool printStringToUser(const char* output);

        /* *************************
         * Session Requests Helpers
         * ************************
         */
        bool sessionWriteOnlineUsers(Packet& response);

        /* *************************
         * Debug methods
         * ************************
         */
        /* Print the user list to the console */
        void * debugPrintUserList(std::list<User*>& list);
        /* Print a message with the relevant session info to the console.
         * Print only if debug level is right. */
		void printDebugLog(const char * buf, ServerSessionSocket& session, DEBUGLEVEL type);
        /* Print a message to the console.
         * Print only if debug level is right. */
        void printDebugLog(const char * buf, DEBUGLEVEL type);

};

