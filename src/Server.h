#pragma once

#include <list>
#include <string.h>

#include "Utils/User.h"
#include "Utils/Inbox.h"
#include "Utils/MailObj.h"
#include "Utils/ServerSessionSocket.h"


class server
{
    public:


		/*	From instructions */
        const long DEFAULT_LISTENING_PORT = 6423;

		std::list<MailObj*>& getMailsBySender(std::string& userName);
        std::list<MailObj*>& getMailsByReceiver(std::string& userName);

        server();

        bool loadUsersFromFile(char * filePath);
        void startListenLoop(const u_int16_t port);



    private:
        Socket listener;
        std::list<Inbox*> m_all_inbox;
		std::list<ServerSessionSocket*> m_sessions;

		User * getUserFromLine(char * buf, int size);
		std::list<User*> * getUsersFromFile(char * filePatch);
        void createInboxList(std::list<User*>& users);

        void processRequset(ServerSessionSocket * session);

        void clearInboxList();
        void clearSessionList();

        /*  Creates and adds to the session list */
        ServerSessionSocket* createNewSessionSocket(socket_handle socketfd);
};

