#pragma once

class Inbox
{
    private:
        std::list<mails*> mails;
        unsigned long idOfLastMail;
    public:
        User& user;

        Inbox(user& usr);
        bool addMail(MailObj * mil);
        bool removeMail(unsigned long id);
        Packet& getShowInboxAsPacket();
}