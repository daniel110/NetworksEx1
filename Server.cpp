/* server implementation */
#include "Server.h"
#include <errno.h>
#include <fcntl.h>          // open
#include <unistd.h>         // close / read / write / lseek
#include <stdio.h>

Server::Server(uint16_t port)
{
	int res = listener.create();

	if (Socket::RES_SUCCESS == res)
	{
		res = listener.bind(Socket::ANY_IP, port);
		if (Socket::RES_SUCCESS == res)
		{
			printDebugLog("Listener initiated successfully.", DEBUGLEVEL::EVENTS);
			m_state = SERVER_INITIATED;
			return;
		}
		printDebugLog("Listener failed to bind.", DEBUGLEVEL::EVENTS);
	}

	printDebugLog("Listener failed get a socket.", DEBUGLEVEL::EVENTS);

	printSocketError(res);
	m_state = SERVER_LISTEN_SOCKET_ERROR;
}

Server::~Server()
{
	clearSessionList();
	clearInboxList();
}

void Server::start()
{

	if (SERVER_READY_TO_LISTEN != getState())
	{
		return;
	}

	FDSet sockets;
	Socket new_conection;
	int socket_res = 0;

	std::list<ServerSessionSocket*>::iterator it_begin;
	std::list<ServerSessionSocket*>::iterator it_end;
	std::list<ServerSessionSocket*>::iterator it;

	/* Start ti listen */
	socket_res = listener.listen(MAX_CONNECTIONS_QUEUE);
	if (Socket::RES_SUCCESS != socket_res)
	{
		printSocketError(socket_res);
		m_state = SERVER_LISTEN_SOCKET_ERROR;
		return;
	}

	printDebugLog("Server is listening...", DEBUGLEVEL::EVENTS);

	/* Check for new connections and updated on the current connections */
	while (true)
	{

		/* Fill the sockets class with the listener socket
		 * and all current sessions sockets.
		 */
		sockets.clear();
		sockets.add(listener);

		it_begin = m_sessions.begin();
		it_end = m_sessions.end();
		for (it = it_begin;
				it != it_end;
				++it)
		{
			ServerSessionSocket * session = *it;
			sockets.add(*session);
		}

		/* Wait until new data or connection arrives.
		 * This is a blocking method.
		 */
		sockets.waitOnSockets();

		printDebugLog("Out of select.", DEBUGLEVEL::INFO);

		/* Check if we got new connection. If so, accept it, create new session
		 * and send a welcome message to the client.
		 */
		if (true == sockets.check(listener))
		{
			printDebugLog("Got new client.", DEBUGLEVEL::EVENTS);

			socket_res = listener.accept(new_conection);
			if (Socket::RES_SUCCESS != socket_res)
			{
				printSocketError(socket_res);
				m_state = SERVER_LISTEN_SOCKET_ERROR;
				return;
			}

			ServerSessionSocket * new_session = new ServerSessionSocket(new_conection);
			m_sessions.push_back(new_session);

			sessionWelcome(*new_session);
		}

		/* Check for every session in the list, if it got new data */
		it_begin = m_sessions.begin();
		it_end = m_sessions.end();
		for (it = it_begin;
				it != it_end;
				)
		{
			ServerSessionSocket * session = *it;

			if (true == sockets.check(*session))
			{
				/* We got new data, so process the session.
				 * Check if the session is closed. if so, remove it from the list.
				 */
				processRequset(*session);
				if (session->isValid() == false)
				{
					delete session;
					it = m_sessions.erase(it);
					continue;
				}
			}
			++it;
		} /* session iterator */
	} /* while true */
}

void Server::sessionWelcome(ServerSessionSocket& session)
{
	Packet welcom;
	welcom.writeForwardDWord(COMMANDTYPE_WELCOME_RES);
	std::string message = "Welcome! I am simple-mail-server.";
	welcom.writeForwardStringField(message);
	session.sendMessage(welcom);
}

void Server::sessionLogin(ServerSessionSocket& session)
{
	Packet message;
	std::string user_field;
	std::string pass_field;
	Inbox * inbox = nullptr;
	bool read_res = false;

	/* Check type of message to be login request.
	 * All sessions that are not logged in must send login message first.
	 */
	if (COMMANDTYPE_LOGIN_REQ != getMessageAndType(session, message))
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_NOT_LOGGED_IN);
		return;
	}

	/* Read user name field */
	read_res = message.readForwardStringField(user_field);
	if (read_res != true)
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_UNVALID_MESSAGE);
		return;
	}

	/* Read password field */
	read_res = message.readForwardStringField(pass_field);
	if (read_res != true)
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_UNVALID_MESSAGE);
		return;
	}

	/* Try to find the inbox with this user name and password */
	inbox = getInboxFromUserString(user_field);
	if (inbox == nullptr)
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_UNKNOWN_USER);
		return;
	}

	/* Check the password of the user */
	if (inbox->getUser().isPassCorrect(pass_field) == false)
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_WRONG_PASS);
		return;
	}

	/* Check if the user already checked in */
	if (inbox->isLogged() == true)
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_USER_ALREADY_LOGGEDON);
		return;
	}

	/* Pair the inbox with the session and set the session to be logged in */
	session.setInbox(inbox);
	session.setState(StateMachineStep::STATE_LOGEDON);

	printDebugLog("User logged in.", session, DEBUGLEVEL::INFO);

	/* Send success response */
	session.sendGeneralRespond(GENERAL_RESPOND_STATUS_SUCCESS);
}

void Server::processRequset(ServerSessionSocket& session)
{
	printDebugLog("got message.", session, DEBUGLEVEL::EVENTS);

	switch (session.getState())
	{
	case StateMachineStep::STATE_NON_AUTH:
		sessionLogin(session);
		break;
	case StateMachineStep::STATE_LOGEDON:
		sessionCommandRequest(session);
		break;
	}
}

void Server::sessionCommandRequest(ServerSessionSocket& session)
{
	Packet message;
	int32_t type;

	type = getMessageAndType(session, message);

	switch(type)
	{
	case COMMANDTYPE_SHOW_INBOX_REQ:
		sessionRequestShowInbox(session, message);
		break;
	case COMMANDTYPE_GET_MAIL_REQ:
		sessionRequestGetMail(session, message);
		break;
	case COMMANDTYPE_DELETE_MAIL_REQ:
		sessionRequestDeleteMail(session, message);
		break;
	case COMMANDTYPE_COMPOSE_REQ:
		sessionRequestCompose(session, message);
		break;
	case COMMANDTYPE_QUIT_REQ:
		session.close();
		break;
	case COMMANDTYPE_SHOW_ONLINE_USERS_REQ:
		//session.close();
		break;
	case COMMANDTYPE_SEND_CHAT_MESSAGE_REQ:
		//session.close();
		break;
	default:
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_UNVALID_MESSAGE);
		break;
	}
}

void Server::sessionRequestShowInbox(ServerSessionSocket& session, Packet& message)
{
	Packet response;
	Inbox * inbox = session.getInbox();

	printDebugLog("sessionRequestShowInbox", session, DEBUGLEVEL::EVENTS);

	if (false == response.writeForwardDWord(COMMANDTYPE_SHOW_INBOX_RES))
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_INTERNAL_FAILURE);
		return;
	}

	if (false == inbox->setShowInboxMails(response))
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_INTERNAL_FAILURE);
		return;
	}

	session.sendMessage(response);
}

void Server::sessionRequestShowOnlineUsers(ServerSessionSocket& session, Packet& message)
{
	Packet response;

	printDebugLog("sessionRequestShowOnlineUsers", session, DEBUGLEVEL::EVENTS);

	if (false == response.writeForwardDWord(COMMANDTYPE_SHOW_ONLINE_USERS_RES))
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_INTERNAL_FAILURE);
		return;
	}

	if (false == this->sessionWriteOnlineUsers(response))
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_INTERNAL_FAILURE);
		return;
	}

	session.sendMessage(response);
}

void Server::sessionRequestSendChatMessage(ServerSessionSocket& session, Packet& message)
{
	Packet response;

	printDebugLog("sessionRequestSendChatMessage", session, DEBUGLEVEL::EVENTS);

	std::string recv_user;
	std::string chat_message;
	Inbox * rcv_inbox;
	Packet chat_packet;

	if (false == message.readForwardStringField(recv_user))
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_UNVALID_MESSAGE);
		return;
	}

	if (false == message.readForwardStringField(chat_message))
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_UNVALID_MESSAGE);
		return;
	}

	rcv_inbox = getInboxFromUserString(recv_user);
	if (rcv_inbox == nullptr)
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_UNKNOWN_USER_CHAT);
	}



	if ((rcv_inbox->isLogged() == true) && (rcv_inbox->getSession() != nullptr))
	{
		/* The user is online so we build a string of the visible message to the client */

		ServerSessionSocket * rcv_session = rcv_inbox->getSession();
		std::string forward_msg = "New message from ";
		forward_msg += session.getInbox()->getUser().getUserName() + ": ";
		forward_msg += chat_message;

		/* Build the response packet, type with a string */
		if (false == chat_packet.writeForwardDWord(COMMANDTYPE_FORWARD_CHAT_MESSAGE))
		{
			session.sendGeneralRespond(GENERAL_RESPOND_STATUS_INTERNAL_FAILURE);
			return;
		}

		if (false == chat_packet.writeForwardStringField(forward_msg))
		{
			session.sendGeneralRespond(GENERAL_RESPOND_STATUS_INTERNAL_FAILURE);
			return;
		}

		if (Socket::RES_SUCCESS != rcv_session->sendMessage(chat_packet))
		{
			session.sendGeneralRespond(GENERAL_RESPOND_STATUS_INTERNAL_FAILURE);
			return;
		}

	}
	else
	{

		/* Here we build a Mail obj with fields as a real mail packet:
		 * To field */
		if (false == chat_packet.writeForwardStringField(recv_user))
		{
			session.sendGeneralRespond(GENERAL_RESPOND_STATUS_INTERNAL_FAILURE);
			return;
		}

		/* Subject field */
		if (false == chat_packet.writeForwardStringField("Message received offline"))
		{
			session.sendGeneralRespond(GENERAL_RESPOND_STATUS_INTERNAL_FAILURE);
			return;
		}

		/* Text field */
		if (false == chat_packet.writeForwardStringField(chat_message))
		{
			session.sendGeneralRespond(GENERAL_RESPOND_STATUS_INTERNAL_FAILURE);
			return;
		}

		/* Create mail object from packet and add it to the inbox. */
		MailObj mail;
		if (true != mail.setMailAsPacket(session.getInbox()->getUser().getUserName(), chat_packet))
		{
			session.sendGeneralRespond(GENERAL_RESPOND_STATUS_INTERNAL_FAILURE);
			return;
		}

		rcv_inbox->addMail(mail);
	}

	session.sendGeneralRespond(GENERAL_RESPOND_STATUS_SUCCESS);

}

bool Server::sessionWriteOnlineUsers(Packet& response)
{
	std::list<ServerSessionSocket*>::iterator it_begin = m_sessions.begin();
	std::list<ServerSessionSocket*>::iterator it_end = m_sessions.end();
	std::list<ServerSessionSocket*>::iterator it;

	std::string users = "Online users: ";

	for (std::list<ServerSessionSocket*>::iterator it = it_begin;
			it != it_end;
			++it)
	{
		ServerSessionSocket * session = *it;
		if (session == nullptr)
		{
			return false;
		}
		Inbox * inb = session->getInbox();
		if (inb == nullptr)
		{
			return false;
		}

		users += inb->getUser().getUserName() + ",";
	}

	users = users.substr(0, users.length()-1);
	response.writeForwardStringField(users);

	return true;
}

void Server::sessionRequestGetMail(ServerSessionSocket& session, Packet& message)
{
	Packet response;
	int32_t mail_id = 0;
	Inbox * inbox = session.getInbox();
	MailObj * mail = nullptr;

	printDebugLog("sessionRequestGetMail", session, DEBUGLEVEL::EVENTS);

	if (false == message.readForwardDWord(mail_id))
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_UNVALID_MESSAGE);
		return;
	}

	mail = inbox->getMailByID(mail_id);
	if (mail == nullptr)
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_UNKNOWN_MAIL_ID);
		return;
	}

	if (false == response.writeForwardDWord(COMMANDTYPE_GET_MAIL_RES))
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_INTERNAL_FAILURE);
		return;
	}

	if (false == inbox->fillPacketWithMail(response, mail))
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_INTERNAL_FAILURE);
		return;
	}

	session.sendMessage(response);
}

void Server::sessionRequestDeleteMail(ServerSessionSocket& session, Packet& message)
{
	Packet response;
	int32_t mail_id = 0;
	Inbox * inbox = session.getInbox();

	printDebugLog("sessionRequestDeleteMail", session, DEBUGLEVEL::EVENTS);

	if (false == message.readForwardDWord(mail_id))
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_UNVALID_MESSAGE);
		return;
	}

	if (false == inbox->removeMail(mail_id))
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_UNKNOWN_MAIL_ID);
	}
	else
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_SUCCESS);
	}

}

void Server::sessionRequestCompose(ServerSessionSocket& session, Packet& message)
{
	Packet response;
	Inbox * inbox = session.getInbox();
	MailObj mail;

	printDebugLog("sessionRequestCompose", session, DEBUGLEVEL::EVENTS);

	if (false == mail.setMailAsPacket(inbox->getUser().getUserName(), message))
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_UNVALID_MESSAGE);
		return;
	}

	std::istringstream stream(mail.m_to);
	std::string to_user;

	while (std::getline(stream, to_user, ','))
	{

		inbox = getInboxFromUserString(to_user);
		if (inbox != nullptr)
		{
			inbox->addMail(mail);
		}

	}

	session.sendGeneralRespond(GENERAL_RESPOND_STATUS_SUCCESS);
}

int32_t Server::getMessageAndType(ServerSessionSocket& session, Packet& message_result)
{
	int32_t type;

	/* Get a message from the client */
	if (Socket::RES_SUCCESS != session.recvMessage(message_result))
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_SESSION_FAILURE);
		session.close();
		return COMMANDTYPE_UNVALID_REQ;
	}

	/* Read the type of the message */
	if (message_result.readForwardDWord(type) != true)
	{
		session.sendGeneralRespond(GENERAL_RESPOND_STATUS_UNVALID_MESSAGE);
		return COMMANDTYPE_UNVALID_REQ;
	}

	return type;
}

void * Server::debugPrintUserList(std::list<User*>& list)
{
	std::list<User*>::iterator it_begin;
	std::list<User*>::iterator it_end;
	std::list<User*>::iterator it;
	char buflog[1024];

	sprintf(buflog, "%d Users loaded.", static_cast<int>(list.size()));
	printDebugLog(buflog, DEBUGLEVEL::INFO);

	it_begin = list.begin();
	it_end = list.end();
	for (it = it_begin;
			it != it_end;
			++it)
	{
		User * user = *it;


		sprintf(buflog, "%.10s - %.10s", user->m_userName.c_str(), user->m_pass.c_str());
		printDebugLog(buflog, DEBUGLEVEL::INFO_MORE);


	}

	return nullptr;
}


Inbox * Server::getInboxFromUserString(const std::string& user)
{
	std::list<Inbox*>::iterator it_begin;
	std::list<Inbox*>::iterator it_end;
	std::list<Inbox*>::iterator it;


	it_begin = m_all_inbox.begin();
	it_end = m_all_inbox.end();
	for (it = it_begin;
			it != it_end;
			++it)
	{
		Inbox * inbox = *it;

		if (inbox->getUser().getUserName() == user)
		{
			return inbox;
		}

	}

	return nullptr;
}




bool Server::printSocketError(int result)
{
	std::string error;
	Socket::fromSocketResultToErrorString(result, error);
	return printStringToUser(error.c_str());
}

bool Server::printStringToUser(const char* output)
{
	return Common::cmnPrintStringToUser(output);
}

bool Server::loadUsersFromFile(char * filePatch)
{
	if (m_state != SERVER_INITIATED)
	{
		return false;
	}

	std::list<User*> * user_list = getUsersFromFile(filePatch);
	if (user_list == nullptr)
	{
		m_state = SERVER_USERFILE_ERROR;
		return false;
	}

	debugPrintUserList(*user_list);

	createInboxList(*user_list);

	delete user_list;

	clearSessionList();

	m_state = SERVER_READY_TO_LISTEN;
	return true;
}

User * Server::getUserFromLine(char * buf, int size)
{
	std::string suser;
	std::string spass;
	char * ptr_start = buf;
	char * ptr_cur = buf;

	if (buf == nullptr)
	{
		return nullptr;
	}

	while ((*ptr_cur != '\0') && (*ptr_cur != '\r') && (*ptr_cur != '\n') && (*ptr_cur != '\t') && ((ptr_cur - ptr_start) < size))
	{
		ptr_cur++;
	}

	if ((ptr_cur - ptr_start) == 0)
	{
		return nullptr;
	}

	suser.assign(ptr_start, ptr_cur - ptr_start);

	ptr_cur++;

	size -= (ptr_cur - ptr_start);
	ptr_start = ptr_cur;

	while ((*ptr_cur != '\0') && (*ptr_cur != '\r') && (*ptr_cur != '\n') && ((ptr_cur - ptr_start) < size))
	{
		ptr_cur++;
	}

	if ((ptr_cur - ptr_start) == 0)
	{
		return nullptr;
	}

	spass.assign(ptr_start, ptr_cur - ptr_start);

	return new User(suser, spass);
}

std::list<User*> * Server::getUsersFromFile(char * filePatch)
{
	FILE * hdl_input = 0;
	char * buf = NULL;
	size_t buf_len = 0;
	ssize_t read_count = 0;
	std::list<User*> * user_list = new std::list<User*>();

    /*  Get key file descriptor */
    hdl_input = fopen(filePatch, "r");
    if (hdl_input == NULL)
    {
		printf("Error opening user files: %s\n%s\n", strerror(errno), filePatch);
		return nullptr;
    }

    /* getline gets a char** and sets a newly allocated buffer.
     * If the pointer contains not NULL, it reallocs if necessary */
    read_count = getline(&buf, &buf_len, hdl_input);
    while (read_count > 0)
    {

    	User * nuser = getUserFromLine(buf, buf_len);
    	if (nuser == nullptr)
    	{
    		break;
    	}

    	user_list->push_back(nuser);


    	read_count = getline(&buf, &buf_len, hdl_input);
    }

    fclose(hdl_input);
    free(buf);

    return user_list;
}

void Server::clearInboxList()
{
	Inbox * cur_inbox = nullptr;

	while (m_all_inbox.empty() == false)
	{
		cur_inbox = m_all_inbox.back();

		delete cur_inbox;

		m_all_inbox.pop_back();
	}

}

void Server::clearSessionList()
{
	ServerSessionSocket * cur_session = nullptr;

	while (m_sessions.empty() == false)
	{
		cur_session = m_sessions.back();

		delete cur_session;

		m_sessions.pop_back();
	}
}

void Server::createInboxList(std::list<User*>& users)
{
	User * cur_user = nullptr;
	Inbox * nInbox = nullptr;

	clearInboxList();

	while (users.empty() == false)
	{
		cur_user = users.back();
		nInbox = new Inbox(*cur_user);
		m_all_inbox.push_back(nInbox);

		/* delete user and pop out of the list */
		delete cur_user;
		users.pop_back();
	}

}


void Server::printDebugLog(const char * buf, ServerSessionSocket& session, DEBUGLEVEL type)
{
	if (type <= debug_log)
	{

		char buflog[1024];

		if (session.getInbox() == nullptr)
		{
			sprintf(buflog, "session %d: ", session.getID());
		}
		else
		{
			sprintf(buflog, "user %.10s: ", session.getInbox()->getUser().getUserName().c_str());
		}

		strcat(buflog, buf);
		printStringToUser(buflog);

	}
}
void Server::printDebugLog(const char * buf, DEBUGLEVEL type)
{
	if (type <= debug_log)
	{
		printStringToUser(buf);
	}
}

