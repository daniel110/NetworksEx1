#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "Socket.h"
#include "Packet.h"
#include "Common.h"

/*	Client class
 *
 * 	Client side of the mail service.
 * 	Instructions:
 * 	- Construct the client with the hostname and port.
 * 	  (Default port is DEFAULT_PORT)
 *	- Call start()
 *
 */
class Client
{
	public:

		/*	default values */
		static const u_int16_t DEFAULT_PORT = 6423;
		static const std::string DEFAULT_HOST;

		/***
		 * Ctor - get hostname and port of the server
		 */
		Client(std::string& hostname, u_int16_t port);

		/**
		 * start client
		 */
		void start();

	private:
		Socket m_sock;
		std::string& m_hostname;
		u_int16_t m_port;

		/****
		 * Handle the initialization of the client-server communication
		 * (calls recvWelcomeMessage - and print the welcome message to the user)
		 *
		 * @return: On success true, On failure false (result holds the error)
		 */
		bool initServerConnection(std::string& result);

		/****
		 * Set *result* with the welcome message sent by the server
		 *
		 * @return: On success true, On failure false (result holds the error)
		 */
		bool recvWelcomeMessage(std::string& result);


		void handleUserRequest(bool& keepGoing, std::string& resultStr);
		void handleServerRequest(bool& keepGoing, std::string& resultStr);

		/************
		 * commands *
		 ************/
		/****
		 * All command get string:result as arg and set it
		 * with a message to the user (errors or info)
		 ****/
		bool commandLogin(std::string& result);
		bool commandShowInbox(std::string& result);
		bool commandGetMail(unsigned int mailId, std::string& result);
		bool commandDeleteMail(unsigned int mailId, std::string& result);
		bool commandQuit(std::string& result);
		bool commandCompose(std::string& result);
		bool commandShowOnlineUsers(std::string& result);
		bool commandSendChatMessage(std::string& to,
									std::string& message,
									std::string& result);
		bool handleChatMessageReceive(Packet& serverPacket, std::string& result);


		/****
		 * Check whether the *commandType* is equal to general respond type
		 * and set *result* with the responded data from the server.
		 *
		 * @return: On success true, On failure false (result holds the error)
		 */
		GeneralRespondStatuses parseGeneralResponse(long commandType,
										Packet& pack,
										std::string& result);


		/****
		 * Convert from GeneralRespondStatus(enum) to textual status in *result*
		 *
		 * @return: On success true, On failure false (result holds the error)
		 */
		bool convertFromGeneralResMessageIdToString(GeneralRespondStatuses messageIdGeneralRes,
				std::string& result);

		/****
		 * Print *output* to the user console (adds "\n")
		 *
		 * @return: On success true, On failure false (output holds the error)
		 */
		bool printStringToUserLine(const std::string& output) const;

		/****
		 * set *input* with string from the console
		 *
		 * @return: On success true, On failure false (input holds the error)
		 */
		bool recvLineFromUser(std::string& input) const;

		/****
		 * 1) Uses recvLineFromUser to get user's input.
		 * 2) Uses getStringFromInputWithPrefix to check whether
		 * 		the first arg is equal to *expectedPrefix*.
		 * 3) if it is - set *data* to the next users' arg
		 *
		 * @return: On success true, On failure false (data holds the error)
		 */
		bool receiveUserCommandArg(const std::string& expectedPrefix,
										std::string& data);

		/****
		 * Check whether the first word in *orgString* is equal to *expectedPrefix*.
		 * If it is - set *data* to the next word in *orgString*
		 *
		 * @return: On success true, On failure false (data holds the error)
		 */
		bool getStringFromInputWithPrefix(std::string& orgString,
											const std::string& expectedPrefix,
											std::string& data);


		/****
		 * send *pack* using m_sock. if an error occurs, log it in *error*
		 *
		 * @return: On success true, On failure false (error holds the error)
		 */
		bool sendCommandAndLogSocketError(Packet& pack,
												std::string& error);

		/****
		 * receive data from m_sock to *pack*. if an error occurs, log it in *error*
		 *
		 * @return: On success true, On failure false (error holds the error)
		 */
		bool receiveRespondAndLogSocketError(Packet& pack,
													std::string& error);


		/******************
		 * Commands Names *
		 ******************/
		static const std::string COMMAND_SHOW_INBOX;
		static const std::string COMMAND_GET_MAIL;
		static const std::string COMMAND_DELETE_MAIL;
		static const std::string COMMAND_QUIT;
		static const std::string COMMAND_COMPOSE;
		static const std::string COMMAND_SHOW_USERS;
		static const std::string COMMAND_SEND_CHAT;


		/********************************************
		 * Prefixes for user input data (to remove) *
		 ********************************************/
		static const std::string PREFIX_INPUT_USER;
		static const std::string PREFIX_INPUT_PASSWORD;

		static const std::string PREFIX_INPUT_TO;
		static const std::string PREFIX_INPUT_SUBJECT;
		static const std::string PREFIX_INPUT_TEXT;


		/*****************
		 * User messages *
		 *****************/
		static const std::string USER_MESSAGE_LOGIN_SUCCESS;
		static const std::string USER_MESSAGE_COMPOSE_SUCCESS;

		/***********************************************
		 * Prefix of data in GET_MAIL command (to add) *
		 ***********************************************/
		static const std::string PREFIX_MAIL_DATA_ON_GET_MAIL_FROM;
		static const std::string PREFIX_MAIL_DATA_ON_GET_MAIL_TO;
		static const std::string PREFIX_MAIL_DATA_ON_GET_MAIL_SUBJECT;
		static const std::string PREFIX_MAIL_DATA_ON_GET_MAIL_TEXT;


		/**********
		 * Errors *
		 **********/
		static const std::string ERROR_FAILED_TO_READ_USER_INPUT;
		static const std::string ERROR_FAILED_TO_EXTRACT_COMMAND_FROM_USER_INPUT;

		/****************
		 * Chat Message *
		 ****************/
		static const std::string SEND_CHAT_DEST_SUFFIX;

};



