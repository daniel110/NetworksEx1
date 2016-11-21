#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "Utils/Socket.h"
#include "Utils/Packet.h"


class Client
{
	public:
		Client(std::string& hostname, u_int16_t port);

		void start();


	/************
	 * commands *
	 ************/
	private:
		Socket m_sock;
		std::string& m_hostname;
		u_int16_t m_port;

		bool commandLogin(std::string& result);
		bool commandShowInbox(std::string& result);
		bool commandGetMail(unsigned int mailId, std::string& result);
		bool commandDeleteMail(unsigned int mailId, std::string& result);
		bool commandQuit(std::string& result);
		bool commandCompose(std::string& result);


		bool parseGeneralResponse(long commandType,
										Packet& pack,
										std::string& result);


		bool printStringToUser(const char* output);
		bool recvLineFromUser(std::string& input);

		bool getStringFromInputWithPrefix(std::string& orgString,
											const std::string& expectedPrefix,
											std::string& data);

		/******************
		 * Commands Names *
		 ******************/
		static const std::string COMMAND_SHOW_INBOX;
		static const std::string COMMAND_GET_MAIL;
		static const std::string COMMAND_DELETE_MAIL;
		static const std::string COMMAND_QUIT;
		static const std::string COMMAND_COMPOSE;


		/********************************
		 * Prefixes for user input data *
		 ********************************/
		static const std::string PREFIX_INPUT_USER;
		static const std::string PREFIX_INPUT_PASSWORD;

		static const std::string PREFIX_INPUT_TO;
		static const std::string PREFIX_INPUT_SUBJECT;
		static const std::string PREFIX_INPUT_TEXT;


		static const unsigned int MAX_USER_INPUT_LINE;

		/*****************
		 * User messages *
		 *****************/
		static const std::string USER_MESSAGE_LOGIN_SUCCESS;
		static const std::string USER_MESSAGE_COMPOSE_SUCCESS;


		static const std::string PREFIX_MAIL_DATA_ON_GET_MAIL_FROM;
		static const std::string PREFIX_MAIL_DATA_ON_GET_MAIL_TO;
		static const std::string PREFIX_MAIL_DATA_ON_GET_MAIL_SUBJECT;
		static const std::string PREFIX_MAIL_DATA_ON_GET_MAIL_TEXT;

};



