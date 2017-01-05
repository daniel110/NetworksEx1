#pragma once

#include <list>
#include "MailObj.h"
#include "User.h"

/*	This declaration is necessary so that
	we could go to the ServerSession if we got the Inbox.
	This is useful for the server to get the session
	after getting the inbox by user string.
	*/
class ServerSessionSocket;

/* Inbox class
 *
 * Inbox represent a server user with his stored emails.
 * For every user registered on the server, we will use an Inbox.
 */
class Inbox
{
	friend class ServerSessionSocket;

    public:
		/* Create an Inbox for that user.
		 * The inbox will have an internal copy of the User object.
		 */
        Inbox(User& usr);

        /****
         * Dtor - clear mail list
         */
        virtual ~Inbox();

        /****
         * Create a copy of *mail*, and add it the inbox.
         */
        void addMail(MailObj& mail);

        /****
         * Remove mail from inbox by *id*
         *
         * @return	On success true, On Failure false
         */
        bool removeMail(unsigned long id);

        /****
         * Write to the packet *showInboxPacket* the Show_Inbox response message.
         * This packet is sent to the client and printed as strings to the client constole.
         *
         * @return	On success true, On Failure false
         */
        bool setShowInboxMails(Packet& showInboxPacket);

        /****
         * Get an email pointer (not a copy) by ID.
         * @return	Pointer of the email. If not mail found return nullptr.
         */
        MailObj * getMailByID(int32_t mail_id);

        /****
         * Write to the packet *Packet* the get_mail response message
         * using the *mail* email object.
         *
         * @return	On success true, On Failure false
         */
        bool fillPacketWithMail(Packet& Packet, MailObj * mail);

        /***
         * return a copy of the user object that connected to the inbox
         */
        const User& getUser();

        /***
		 * return true if the user is currently logged, else false
		 */
        bool isLogged();

        ServerSessionSocket * getSession();

    private:
        User m_user;
        std::list<MailObj*> m_mails;
        unsigned long m_idOfLastMail;

        bool m_loggedIn;
        ServerSessionSocket * m_session;

        /***
         * Get *mail* object and set *showInboxRaw* with a string which represent
         * a mail line for the SHOW_INBOX console output.
         * Used by setShowInboxMails()
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
