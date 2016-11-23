#pragma once

#include <string>

enum CommandTypes
{
	COMMANDTYPE_LOGIN_REQ = 1,
	COMMANDTYPE_SHOW_INBOX_REQ = 2,
	COMMANDTYPE_SHOW_INBOX_RES = 3,
	COMMANDTYPE_GET_MAIL_REQ = 4,
	COMMANDTYPE_GET_MAIL_RES = 5,
	COMMANDTYPE_DELETE_MAIL_REQ = 6,
	COMMANDTYPE_COMPOSE_REQ = 7,
	COMMANDTYPE_GENERAL_MESSAGE = 8,
	COMMANDTYPE_WELCOME_RES = 9,

};

enum GeneralCommenadStatuses
{
	GENERAL_COMMADN_STATUS_SUCCESS = 0,
	GENERAL_COMMADN_STATUS_UNKNOWN_USER = 1,
	GENERAL_COMMADN_STATUS_WRONG_PASS = 2,
	GENERAL_COMMADN_STATUS_NOT_LOGOED_IN = 3,
	GENERAL_COMMADN_STATUS_UNKNOWN_MAIL_ID = 4,

	GENERAL_COMMADN_STATUS_INTERNAL_FAILURE = 10,
	GENERAL_COMMADN_STATUS_FATAL_ERROR = 11,
};

class Common
{

public:
	static const std::string GENERAL_RESPONSE_SUCCUSS_MESSAGE;

};
