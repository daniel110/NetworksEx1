#pragma once

#include <list>
#include <vector>
#include <string>

#include "Utils/User.h"
#include "Utils/Inbox.h"
#include "Utils/MailObj.h"
#include "Utils/ServerSessionSocket.h"


class server
{
    public:
        std::list<MailObj*>& getMailsBySender(std::string& userName);
        std::list<MailObj*>& getMailsByReceiver(std::string& userName);


    private:
        Socket listener;
        std::list<Inbox*> m_all_inbox;
		std::list<ServerSessionSocket*> m_sessions;


        std::list<User*>& getUsersFromFile(std::string path);
        void createInboxes(std::list<User*>& users);

        /* default listening port 6423 */
        void listenLoop();
        void processRequset(ServerSessionSocket * session);

        /*  Creates and adds to the session list */
        ServerSessionSocket* createNewSessionSocket(socket_handle socketfd);
};

