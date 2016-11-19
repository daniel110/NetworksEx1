#pragma once

#include <list>

#include "MailObj.h"
#include "User.h"

class Inbox
{
    public:
        Inbox(User& usr);
        ~Inbox();

        void addMail(MailObj* mail);
        bool removeMail(unsigned long id);
        int setShowInboxMails(Packet& showInboxPacket);

        const User& getUser();

    private:
        User& m_user;
        std::list<MailObj*> m_mails;
        unsigned long m_idOfLastMail;

        std::string fromMailToShowInboxRaw(const MailObj* mail);

};
