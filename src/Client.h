#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "Utils/Socket.h"
#include "Utils/Packet.h"


class Client
{
	public:
		Client(std::string& hostname, u_int16_t port,
				std::ostream& out, std::istream& in);

		void start();


	/************
	 * commands *
	 ************/
	private:
		Socket m_sock;
		std::string& m_hostname;
		u_int16_t m_port;

		std::ostream& m_outStream;
		std::istream& m_inStream;

		bool commandLogin(std::string& result);
		bool commandShowInbox(std::string& result);
		bool commandGetMail(unsigned int mailId, std::string& result);
		bool commandDeleteMail(unsigned int mailId, std::string& result);
		bool commandQuit(std::string& result);
		bool commandCompose(std::string& result);


		bool parseGeneralResponse(std::string& result);

		bool printStringToUser(const char* output);
		bool printIntToUser(int& output);

		bool recvStringFromUser(std::string& input);

		bool getStringFromInputWithPrefix(const std::string& expectedPrefix,
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

};



