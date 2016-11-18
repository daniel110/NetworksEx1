#include "MailObj.h"



MailObj::MailObj(MailObj& other)
{
	m_body = other.m_body;
	m_from = other.m_from;
	m_to = other.m_to;
	m_subject = other.m_subject;
	m_id = other.m_id; /* TODO: should copy id? */
}
