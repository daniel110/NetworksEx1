#pragma once

#include <list>
#include <string>

#include <string.h>

#include "Utils/User.h"
#include "Utils/Inbox.h"
#include "Utils/MailObj.h"
#include "Utils/ServerSessionSocket.h"
#include "Utils/Common.h"
#include "Utils/FDSet.h"

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

		/*	From instructions */
		static const long DEFAULT_PORT = 6423;
		static const long MAX_CONNECTIONS = 1;

        Server(uint16_t port);
        virtual ~Server();
        bool loadUsersFromFile(char * filePath);
        void start();

        SERVER_STATE getState()
        {
        	return m_state;
        }

        enum DEBUGLEVEL
		{
        	NO_LOG,
			EVENTS,
			INFO,
			INFO_MORE,
		};

        DEBUGLEVEL debug_log = INFO_MORE;

    private:
        SERVER_STATE m_state;
        Socket listener;
        std::list<Inbox*> m_all_inbox;
		std::list<ServerSessionSocket*> m_sessions;

		//std::list<MailObj*>& getMailsBySender(std::string& userName);
        //std::list<MailObj*>& getMailsByReceiver(std::string& userName);



		User * getUserFromLine(char * buf, int size);
		std::list<User*> * getUsersFromFile(char * filePatch);
        void createInboxList(std::list<User*>& users);

        void clearInboxList();
        void clearSessionList();

        Inbox * getInboxFromUserString(const std::string& user, const std::string& pass);


        void processRequset(ServerSessionSocket& session);
        void sessionWelcome(ServerSessionSocket& session);
        void sessionLogin(ServerSessionSocket& session);


        bool printSocketError(int result);
        bool printStringToUser(const char* output);
        bool recvLineFromUser(std::string& input);


        void * debugPrintUserList(std::list<User*>& list);
		void printDebugLog(const char * buf, ServerSessionSocket& session, DEBUGLEVEL type)
		{
			if (type <= debug_log)
			{

				char buflog[1024];

				if (session.getInbox() == nullptr)
				{
					sprintf(buflog, "session %d: ", session.getID());
				}
				else
				{
					sprintf(buflog, "user %s: ", session.getInbox()->getUser().getUserName().c_str());
				}

				strcat(buflog, buf);
				printStringToUser(buflog);

			}
		}
        void printDebugLog(const char * buf, DEBUGLEVEL type)
		{
			if (type <= debug_log)
			{
				printStringToUser(buf);
			}
		}

        /*  Creates and adds to the session list */
        ServerSessionSocket* createNewSessionSocket(socket_handle socketfd);
};

