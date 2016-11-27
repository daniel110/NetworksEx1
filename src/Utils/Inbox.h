#pragma once

#include <list>

#include "MailObj.h"
#include "User.h"

class Inbox
{
	friend class ServerSessionSocket;

    public:
        Inbox(User& usr);

        /****
         * Dtor - clear mail list
         */
        virtual ~Inbox();

        /****
         * Add *mail* to the inbox
         */
        void addMail(MailObj* mail);

        /****
         * Remove mail from inbox by *id*
         *
         * @return	On success true, On Failure false
         */
        bool removeMail(unsigned long id);

        /****
         * Set *showInboxPacket* with all the inbox's mails in correct format
         *
         * @return	On success true, On Failure false
         */
        bool setShowInboxMails(Packet& showInboxPacket);

        MailObj * getMailByID(int32_t mail_id);
        bool fillPacketWithMail(Packet& Packet, MailObj * mail);

        /***
         * return the user that connected to the inbox
         */
        const User& getUser();

        /***
		 * return true if the user is currently logged, else false
		 */
        bool isLogged();

    private:
        User& m_user;
        std::list<MailObj*> m_mails;
        unsigned long m_idOfLastMail;

        bool m_loggedIn;

        /***
         * Get *mail* object and set *showInboxRaw* with a string which represent
         * the mail as SHOW_INBOX
         *
         * @return	On success true, On Failure false
         */
        bool fromMailToShowInboxRaw(const MailObj* mail,
        							std::string& showInboxRaw);

        /*********************************************
         * Constants for creating mail at SHOW_INBOX *
         *********************************************/
        static const std::string SHOW_INBOX_MAIL_INFO_DELIMITER;
        static const std::string SHOW_INBOX_MAIL_SUBJECT_WRAPPER;

};
