#include "MailObj.h"

MailObj::MailObj()
{
	m_body = "";
	m_from = "";
	m_to = "";
	m_subject = "";
	m_id = 0;
}

MailObj::MailObj(MailObj& other)
{
	m_body = other.m_body;
	m_from = other.m_from;
	m_to = other.m_to;
	m_subject = other.m_subject;
	m_id = other.m_id;
}


bool MailObj::getMailasPacket(Packet& pckt) const
{
	if (pckt.writeForwardStringField(m_from) == false)
	{
		return false;
	}
	if (pckt.writeForwardStringField(m_to) == false)
	{
		return false;
	}
	if (pckt.writeForwardStringField(m_subject) == false)
	{
		return false;
	}
	if (pckt.writeForwardStringField(m_body) == false)
	{
		return false;
	}
	return true;
}


bool MailObj::setMailAsPacket(Packet& pct)
{
	if (pct.readForwardStringField(m_from) == false)
	{
		return false;
	}

	if (pct.readForwardStringField(m_to) == false)
	{
		return false;
	}

	if (pct.readForwardStringField(m_subject) == false)
	{
		return false;
	}

	if (pct.readForwardStringField(m_body) == false)
	{
		return false;
	}

	return true;
}
