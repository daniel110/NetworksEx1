#include "User.h"


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

