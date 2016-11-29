/*
 * inbox_test.cpp
 *
 *  Created on: Nov 27, 2016
 *      Author: df
 */

#include <iostream>
#include "../../Inbox.h"


int maine()
{
	std::string name("Dan");
	std::string pass("Bay");

	User user(name, pass);
	Inbox in(user);

	MailObj obj;
	obj.m_from = "Dor";
	obj.m_to = "Dan,Ran";
	obj.m_subject = "Check1";
	obj.m_body = "Bla Bla";

	MailObj obj1;
	obj1.m_from = "Ben";
	obj1.m_to = "Dan";
	obj1.m_subject = "Check2";
	obj1.m_body = "Bla Bla";

	MailObj obj2;
	obj2.m_from = "Alina";
	obj2.m_to = "Alina,Dan,Ran";
	obj2.m_subject = "Check3";
	obj2.m_body = "Bla Bla";

	MailObj obj3;
	obj3.m_from = "Moshe";
	obj3.m_to = "Dan,Alina";
	obj3.m_subject = "Check4";
	obj3.m_body = "Bla Bla";

	in.addMail(&obj); /* id = 1 */
	in.addMail(&obj1); /* id = 2 */
	in.addMail(&obj2); /* id = 3 */
	in.addMail(&obj3); /* id = 4 */


	in.removeMail(2);

	MailObj obj4;
	obj4.m_from = "Moshe4";
	obj4.m_to = "Dan,Alina";
	obj4.m_subject = "Check5";
	obj4.m_body = "Bla Bla";

	in.addMail(&obj4); /* id = 5 */


	Packet pac;
	in.setShowInboxMails(pac);
	pac.jumptoStart();

	std::string field;

	pac.readForwardStringField(field);
	std::cout << field << std::endl;

	pac.readForwardStringField(field);
	std::cout << field << std::endl;

	pac.readForwardStringField(field);
	std::cout << field << std::endl;

	pac.readForwardStringField(field);
	std::cout << field << std::endl;

	return 0;
}


