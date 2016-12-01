#include "User.h"


User::User(	std::string& userName,
			std::string& pass)
{
	this->init(userName, pass);
}

void User::init(const std::string& userName,
				const std::string& pass)
{
	m_userName = userName;
	m_pass = pass;
}

bool User::isPassCorrect(const std::string& pass) const
{
	return (pass == m_pass);
}

const std::string& User::getUserName() const
{
	return m_userName;
}
