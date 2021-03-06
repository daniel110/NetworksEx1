#include "Inbox.h"

const std::string Inbox::SHOW_INBOX_MAIL_INFO_DELIMITER = " ";
const std::string Inbox::SHOW_INBOX_MAIL_SUBJECT_WRAPPER = "\"";

Inbox::Inbox(User& usr) : m_user(usr),
							m_idOfLastMail(0) ,
							m_loggedIn(false) ,
							m_session(nullptr)
{
	// empty
}

Inbox::~Inbox()
{
	MailObj * cur_mail = nullptr;

	while (m_mails.empty() == false)
	{
		cur_mail = m_mails.back();

		delete cur_mail;

		m_mails.pop_back();
	}
}
void Inbox::addMail(MailObj& mail)
{
	MailObj* copyMail = new MailObj(mail);
	copyMail->m_id = ++m_idOfLastMail;

	m_mails.push_back(copyMail);
}

bool Inbox::removeMail(unsigned long id)
{
	bool isFound = false;

	for (std::list<MailObj*>::iterator it = m_mails.begin();
			it != m_mails.end();
			++it)
	{
		if (id == (*it)->m_id)
		{
			delete (*it); /* delete mail */

			m_mails.erase(it);
			isFound = true;
			break;
		}
	}

	return isFound;
}

bool Inbox::setShowInboxMails(Packet& showInboxPacket)
{
	for (std::list<MailObj*>::iterator it = m_mails.begin();
			it != m_mails.end();
			++it)
	{
		std::string mailRaw;
		if (false == fromMailToShowInboxRaw(*it , mailRaw))
		{
			return false;
		}

		if (false == showInboxPacket.writeForwardStringField(mailRaw))
		{
			return false;
		}
	}

	return true;
}

MailObj * Inbox::getMailByID(int32_t mail_id)
{
	for (std::list<MailObj*>::iterator it = m_mails.begin();
			it != m_mails.end();
			++it)
	{
		MailObj * mail = (*it);

		if (mail == nullptr)
		{
			return nullptr;
		}

		if (mail->m_id == (unsigned long) mail_id)
		{
			return mail;
		}
	}

	return nullptr;
}

bool Inbox::fillPacketWithMail(Packet& Packet, MailObj * mail)
{
	if (mail == nullptr)
	{
		return false;
	}

	if (false == Packet.writeForwardStringField(mail->m_from))
	{
		return false;
	}

	if (false == Packet.writeForwardStringField(mail->m_to))
	{
		return false;
	}

	if (false == Packet.writeForwardStringField(mail->m_subject))
	{
		return false;
	}

	if (false == Packet.writeForwardStringField(mail->m_body))
	{
		return false;
	}

	return true;
}

const User& Inbox::getUser()
{
	return m_user;
}


bool Inbox::fromMailToShowInboxRaw(const MailObj* mail,
									std::string& showInboxRaw)
{
	char idAsString[11] = {0}; /* max int has 10 digits */
	if ( 0 > sprintf(idAsString, "%d", mail->m_id))
	{
		return false;
	}

	showInboxRaw = idAsString;
	showInboxRaw += SHOW_INBOX_MAIL_INFO_DELIMITER +
					mail->m_from +
					SHOW_INBOX_MAIL_INFO_DELIMITER +

					SHOW_INBOX_MAIL_SUBJECT_WRAPPER +
					mail->m_subject +
					SHOW_INBOX_MAIL_SUBJECT_WRAPPER;


	return true;
}



bool Inbox::isLogged()
{
	return m_loggedIn;
}

ServerSessionSocket * Inbox::getSession()
{
	return m_session;
}
