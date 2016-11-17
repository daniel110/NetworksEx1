#pragma once

#include <list>
#include <vector>
#include <string>

#include "Utils/ServerSocketSession.h"
#include "Utils/User.h"
#include "Utils/Inbox.h"
#include "Utils/MailObj.h"


class server
{
    private:
		std::list<ServerSocketSession*> m_sessions;

        Socket listener;
        std::list<Inbox*> m_all_inbox;

    public:
        std::list<MailObj>& getMailsBySender(char * name, unsigned long len);
        std::list<MailObj>& getMailsByReceiver(char * name, unsigned long len);

        std::list<User&> getUserFromFile(std::string path);

        void ListenLoop();

        /*  Creates and adds to the session list */
        ServerSocketSession * createNewSession(socket_handle socketfd);

        ServerSocketSession * getSessionFromSocket(int socketfd);

        void ProcessSocket(ServerSocketSession * socket);

};

