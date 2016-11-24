#include "User.h"


User::User(	std::string& userName,
			std::string& pass)
{
	this->init(m_userName, m_pass);
}

User::User(const User& user)
{
	this->init(user.m_userName, user.m_pass);
}


void User::init(const std::string& userName,
				const std::string& pass)
{
	m_userName = userName;
	m_pass = pass;
}

bool User::getUserAsPacket(Packet& pckt)
{
	bool res = pckt.writeForwardStringField(m_userName);
	if (res == false)
	{
		return false;
	}
	res = pckt.writeForwardStringField(m_pass);
	return res;
}

bool User::setUserFromPacket(Packet& pct)
{
	if (pct.readForwardStringField(m_userName) == false)
	{
		return false;
	}

	if (pct.readForwardStringField(m_pass) == false)
	{
		return false;
	}

	return true;
}


bool User::isPassCorrect(const std::string& pass) const
{
	return (pass == m_pass);
}

const std::string& User::getUserName() const
{
	return m_userName;
}
