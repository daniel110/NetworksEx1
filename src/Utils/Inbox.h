#pragma once

#include <list>

#include "MailObj.h"
#include "User.h"

class Inbox
{
    private:
        std::list<MailObj*> mails;

        unsigned long idOfLastMail;
    public:
        User& user;

        Inbox(User& usr);

        bool addMail(MailObj * mil);
        bool removeMail(unsigned long id);
        Packet& getShowInboxAsPacket();
};
