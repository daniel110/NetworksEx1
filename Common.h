#pragma once

#include <string>
#include <sstream>
#include <iostream>

/* Valid message types */
enum CommandTypes
{
	COMMANDTYPE_UNVALID_REQ = 0,
	COMMANDTYPE_LOGIN_REQ = 1,
	COMMANDTYPE_SHOW_INBOX_REQ = 2,
	COMMANDTYPE_SHOW_INBOX_RES = 3,
	COMMANDTYPE_GET_MAIL_REQ = 4,
	COMMANDTYPE_GET_MAIL_RES = 5,
	COMMANDTYPE_DELETE_MAIL_REQ = 6,
	COMMANDTYPE_COMPOSE_REQ = 7,
	COMMANDTYPE_GENERAL_MESSAGE = 8,
	COMMANDTYPE_WELCOME_RES = 9,
	COMMANDTYPE_QUIT_REQ = 10,

};

/* Valid response statuses from the server */
enum GeneralRespondStatuses
{
	/* this message do not sent by the server - it for client only */
	GENERAL_RESPOND_UNKNOWN_STATUS = -1,

	GENERAL_RESPOND_STATUS_SUCCESS = 0,
	GENERAL_RESPOND_STATUS_UNKNOWN_USER = 1,

	/* We do not allow the client to distinguish between user name not found
	 * and password is incorrect for security reasons */
	//GENERAL_RESPOND_STATUS_WRONG_PASS = 2,

	GENERAL_RESPOND_STATUS_NOT_LOGGED_IN = 3,
	GENERAL_RESPOND_STATUS_UNKNOWN_MAIL_ID = 4,
	GENERAL_RESPOND_STATUS_USER_ALREADY_LOGGEDON = 5,
	GENERAL_RESPOND_STATUS_INTERNAL_FAILURE = 10,
	GENERAL_RESPOND_STATUS_FATAL_ERROR = 11,
	GENERAL_RESPOND_STATUS_UNVALID_MESSAGE = 12,
	GENERAL_RESPOND_STATUS_SESSION_FAILURE = 13,
};

/* General common methods and constatns used by the client and the server */
class Common
{

public:
	static const std::string GENERAL_RESPONSE_SUCCUSS_MESSAGE;

	static const unsigned int MAX_USER_INPUT_LINE = 2000;

	static bool cmnPrintStringToUser(const char* output);
	static bool cmnRecvLineFromUser(std::string& input);

};
