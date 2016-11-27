/* client implementation */

#include "Client.h"
#include "Utils/Common.h"
#include "Utils/Packet.h"


const std::string Client::DEFAULT_HOST = "localhost";

const std::string Client::COMMAND_SHOW_INBOX = "SHOW_INBOX";
const std::string Client::COMMAND_GET_MAIL = "GET_MAIL";
const std::string Client::COMMAND_DELETE_MAIL = "DELETE_MAIL" ;
const std::string Client::COMMAND_QUIT = "QUIT";
const std::string Client::COMMAND_COMPOSE = "COMPOSE";


const std::string Client::PREFIX_INPUT_USER = "User:";
const std::string Client::PREFIX_INPUT_PASSWORD= "Password:";

const std::string Client::PREFIX_INPUT_TO = "To:";
const std::string Client::PREFIX_INPUT_SUBJECT = "Subject:";
const std::string Client::PREFIX_INPUT_TEXT = "Text:";


const std::string Client::PREFIX_MAIL_DATA_ON_GET_MAIL_FROM= "From: ";
const std::string Client::PREFIX_MAIL_DATA_ON_GET_MAIL_TO = "To: ";
const std::string Client::PREFIX_MAIL_DATA_ON_GET_MAIL_SUBJECT = "Subject: ";
const std::string Client::PREFIX_MAIL_DATA_ON_GET_MAIL_TEXT = "Text: ";


const std::string Client::USER_MESSAGE_LOGIN_SUCCESS = "Connected to server";
const std::string Client::USER_MESSAGE_COMPOSE_SUCCESS = "Mail sent";


const std::string Client::ERROR_FAILED_TO_READ_USER_INPUT = "Unable to read command - Going down";
const std::string Client::ERROR_FAILED_TO_EXTRACT_COMMAND_FROM_USER_INPUT = "Unable to extract command type";



Client::Client(std::string& hostname, u_int16_t port) : m_hostname(hostname)
{
	m_port = port;
}

void Client::start()
{
	std::string resultStr;

	/**
	 * initialize connection to server
	 */
	if (false == initServerConnection(resultStr))
	{
		printStringToUserLine(resultStr);
		return;
	}

	/* Print that we are connected */
	printStringToUserLine(resultStr);

	/******************************************
	 * Mail Loop: receive and execute command *
	 ******************************************/
	bool keepGoing = true;
	std::string input;
	while (keepGoing)
	{
		if (false == recvLineFromUser(input))
		{
			printStringToUserLine(ERROR_FAILED_TO_READ_USER_INPUT);
			break;
		}
		std::basic_stringstream<char> inputStream(input);

		/* Get only first word (should be a command string) */
		std::string commandTypeName;
		inputStream >> commandTypeName;
		if (inputStream.fail() )
		{
			printStringToUserLine(ERROR_FAILED_TO_EXTRACT_COMMAND_FROM_USER_INPUT);
			continue;
		}

		/********************************
		 * Call correct command handler *
		 ********************************/
		resultStr = "Got Extra arguments. Try command again.";

		if (0 == commandTypeName.compare(COMMAND_SHOW_INBOX))
		{
			/* before calling each command - we check that no extra argument
			 * was passed by the user */
			if (inputStream.eof())
			{
				keepGoing = commandShowInbox(resultStr);
			}
		}
		else if (0 == commandTypeName.compare(COMMAND_GET_MAIL))
		{
			int id;
			inputStream >> id;
			/* check that the argument was really an int */
			if (inputStream.fail() )
			{
				resultStr = "Unable to extract mail id.";
			}
			else
			{
				if (!inputStream.eof())
				{
					resultStr = "The second and last parameter of GET_MAIL should be int";
				}
				else
				{
					keepGoing = commandGetMail(id, resultStr);
				}
			}
		}
		else if (0 == commandTypeName.compare(COMMAND_DELETE_MAIL))
		{
			int id;
			inputStream >> id;
			if (inputStream.fail() )
			{
				resultStr = "Unable to extract mail id.";
			}
			else
			{
				if (!inputStream.eof())
				{
					resultStr = "The second and last parameter of DELETE_MAIL should be int";
				}
				else
				{
					keepGoing = commandDeleteMail(id, resultStr);
				}
			}
		}
		else if (0 == commandTypeName.compare(COMMAND_COMPOSE))
		{
			if (inputStream.eof())
			{
				keepGoing = commandCompose(resultStr);
			}
		}
		else if (0 == commandTypeName.compare(COMMAND_QUIT))
		{
			if (inputStream.eof())
			{
				keepGoing = commandQuit(resultStr);
			}
		}
		else
		{
			resultStr = "Invalid command type name";
		}


		/* log handler result */
		if ( false == printStringToUserLine(resultStr))
		{
			/* nothing to do, just exit */
			keepGoing = false;
		}
	}

}

bool Client::initServerConnection(std::string& result)
{
	/* create socket */
	if (Socket::RES_INVALID_SOCKET_ERROR == m_sock.create())
	{
		result += "Failed to create socket";
		return false;
	}

	/* connect to server */
	int res = m_sock.connect(m_hostname, m_port);
	if (Socket::RES_SUCCESS != res)
	{
		std::string socketErr;
		Socket::fromSocketResultToErrorString(res, socketErr);

		result += "Connection error: " + socketErr;

		return false;
	}


	/* receive welcome message from server - and print it to user */
	if (false == recvWelcomeMessage(result))
	{
		result += "Failed receiving welcome message: " + result;
		return false;
	}

	printStringToUserLine(result);


	result.clear();
	/*login user to server*/
	if (false == commandLogin(result))
	{
		return false;
	}

	return true;
}

bool Client::recvWelcomeMessage(std::string& result)
{
	/* receive response packet */
	Packet resPacket;
	if (false == receiveRespondAndLogSocketError(resPacket,
														result))
	{
		return false;
	}


	/**********************
	 * Check command type *
	 **********************/
	int32_t commandType;
	if (false == resPacket.readForwardDWord(commandType))
	{
		result = "Unable to read command type";
		return false;
	}

	if (COMMANDTYPE_WELCOME_RES != commandType)
	{
		std::string resMessage;

		GeneralRespondStatuses stat = parseGeneralResponse(commandType,
																	resPacket,
																	resMessage);
		if (GENERAL_RESPOND_UNKNOWN_STATUS == stat)
		{
			result += "Error on parsing response " + resMessage;
			return false;
		}

		if (false == convertFromGeneralResMessageIdToString(stat,
												resMessage))
		{
			result += resMessage;
			return false;
		}

		result += resMessage;
		return false;
	}


	if (false == resPacket.readForwardStringField(result))
	{
		result += "Failed to read welcome message";
		return false;
	}

	return true;
}



bool Client::commandLogin(std::string& result)
{
	result = "Failed on Login: ";

	/* parse user name from input */
	std::string user;
	if (false == receiveUserCommandArg(PREFIX_INPUT_USER,
											user))
	{
		result += user;
		return false;
	}

	/* parse password name from input */
	std::string pass;
	if (false == receiveUserCommandArg(PREFIX_INPUT_PASSWORD,
											pass))
	{
		result += pass;
		return false;
	}

	/***************
	 * BuildPacket *
	 ***************/
	Packet loginPack;
	loginPack.writeForwardDWord(COMMANDTYPE_LOGIN_REQ);
	loginPack.writeForwardStringField(user);
	loginPack.writeForwardStringField(pass);

	if (false == sendCommandAndLogSocketError(loginPack,result))
	{
		return false;
	}

	/* receive response packet */
	Packet resPacket;
	if (false == receiveRespondAndLogSocketError(resPacket,result))
	{
		return false;
	}

	int32_t commandType;
	if (false == resPacket.readForwardDWord(commandType))
	{
		result += "Unable to read command type";
		return false;
	}

	/* parse respond */
	std::string resMessage;

	GeneralRespondStatuses stat = parseGeneralResponse(commandType,
																resPacket,
																resMessage);
	if (GENERAL_RESPOND_UNKNOWN_STATUS == stat)
	{
		result += "Error on parsing response " + resMessage;
		return false;
	}

	if (GENERAL_RESPOND_STATUS_SUCCESS != stat)
	{
		/* return value is not relevant  */
		convertFromGeneralResMessageIdToString(stat,
												resMessage);

		result += resMessage;
		return false;
	}

	result = USER_MESSAGE_LOGIN_SUCCESS;
	return true;
}
bool Client::commandShowInbox(std::string& result)
{
	result = "Failed on Show Inbox: ";

	/***************
	 * BuildPacket *
	 ***************/
	Packet showInboxPack;
	showInboxPack.writeForwardDWord(COMMANDTYPE_SHOW_INBOX_REQ);

	if (false == sendCommandAndLogSocketError(showInboxPack,result))
	{
		return false;
	}

	/* receive response packet */
	Packet resPacket;
	if (false == receiveRespondAndLogSocketError(resPacket,result))
	{
		return false;
	}

	/**********************
	 * Check command type *
	 **********************/
	int32_t commandType;
	if (false == resPacket.readForwardDWord(commandType))
	{
		result += "Unable to read command type";
		return false;
	}

	if (COMMANDTYPE_SHOW_INBOX_RES != commandType)
	{
		std::string resMessage;

		GeneralRespondStatuses stat = parseGeneralResponse(commandType,
																	resPacket,
																	resMessage);
		if (GENERAL_RESPOND_UNKNOWN_STATUS == stat)
		{
			result += "Error on parsing response " + resMessage;
			return false;
		}

		if (false == convertFromGeneralResMessageIdToString(stat,
												resMessage))
		{
			result += resMessage;
			return false;
		}

		result += resMessage;
		return true;
	}

	/********************************************
	 * Parse command - read and print all mails *
	 ********************************************/
	std::string mailInfo;
	while (0 != resPacket.bytesLeft())
	{
		if (false == resPacket.readForwardStringField(mailInfo))
		{
			result += "Failed to read next mail from packet";
			return true;
		}

		if (false == printStringToUserLine(mailInfo))
		{
			/* nothing to do, just exit */
			result.clear();
			return false;
		}
	}

	result.clear();
	return true;
}
bool Client::commandGetMail(unsigned int mailId, std::string& result)
{
	result = "Failed on Get Mail: ";

	/***************
	 * BuildPacket *
	 ***************/
	Packet getMailPack;
	getMailPack.writeForwardDWord(COMMANDTYPE_GET_MAIL_REQ);
	getMailPack.writeForwardDWord(mailId);

	if (false == sendCommandAndLogSocketError(getMailPack,result))
	{
		return false;
	}

	/* receive response packet */
	Packet resPacket;
	if (false == receiveRespondAndLogSocketError(resPacket,result))
	{
		return false;
	}

	/**********************
	 * Check command type *
	 **********************/
	int32_t commandType;
	if (false == resPacket.readForwardDWord(commandType))
	{
		result += "Unable to read command type";
		return false;
	}

	if (COMMANDTYPE_GET_MAIL_RES != commandType)
	{
		std::string resMessage;

		GeneralRespondStatuses stat = parseGeneralResponse(commandType,
																	resPacket,
																	resMessage);
		if (GENERAL_RESPOND_UNKNOWN_STATUS == stat)
		{
			result += "Error on parsing response " + resMessage;
			return false;
		}


		if (false == convertFromGeneralResMessageIdToString(stat,
												resMessage))
		{
			result += resMessage;
			return false;
		}

		result += resMessage;
		return true;
	}

	/********************************************
	 * Parse command - read and print mail data *
	 ********************************************/
	std::string mailInfo;
	if (false == resPacket.readForwardStringField(mailInfo))
	{
		result += "Failed to read 'From' field";
		return true;
	}

	mailInfo = PREFIX_MAIL_DATA_ON_GET_MAIL_FROM + mailInfo;
	if (false == printStringToUserLine(mailInfo))
	{
		/* nothing to do, just exit */
		result.clear();
		return false;
	}

	if (false == resPacket.readForwardStringField(mailInfo))
	{
		result += "Failed to read 'To' field";
		return true;
	}

	mailInfo = PREFIX_MAIL_DATA_ON_GET_MAIL_TO + mailInfo;
	if (false == printStringToUserLine(mailInfo))
	{
		/* nothing to do, just exit */
		result.clear();
		return false;
	}


	if (false == resPacket.readForwardStringField(mailInfo))
	{
		result += "Failed to read 'Subject' field";
		return true;
	}

	mailInfo = PREFIX_MAIL_DATA_ON_GET_MAIL_SUBJECT + mailInfo;
	if (false == printStringToUserLine(mailInfo))
	{
		/* nothing to do, just exit */
		result.clear();
		return false;
	}


	if (false == resPacket.readForwardStringField(mailInfo))
	{
		result += "Failed to read 'Text' field";
		return true;
	}

	mailInfo = PREFIX_MAIL_DATA_ON_GET_MAIL_TEXT + mailInfo;
	if (false == printStringToUserLine(mailInfo))
	{
		/* nothing to do, just exit */
		result.clear();
		return false;
	}


	result.clear();
	return true;
}
bool Client::commandDeleteMail(unsigned int mailId, std::string& result)
{
	result = "Failed on delete mail: ";

	/***************
	 * BuildPacket *
	 ***************/
	Packet deleteMailPack;
	deleteMailPack.writeForwardDWord(COMMANDTYPE_DELETE_MAIL_REQ);
	deleteMailPack.writeForwardDWord(mailId);

	/* send packet */
	if (false == sendCommandAndLogSocketError(deleteMailPack,result))
	{
		return false;
	}

	/* receive response packet */
	Packet resPacket;
	if (false == receiveRespondAndLogSocketError(resPacket,result))
	{
		return false;
	}

	/**********************
	 * Check command type *
	 **********************/
	int32_t commandType;
	if (false == resPacket.readForwardDWord(commandType))
	{
		result += "Unable to read command type";
		return false;
	}

	/* parse respond */
	std::string resMessage;

	GeneralRespondStatuses stat = parseGeneralResponse(commandType,
																resPacket,
																resMessage);
	if (GENERAL_RESPOND_UNKNOWN_STATUS == stat)
	{
		result += "Error on parsing response " + resMessage;
		return false;
	}

	if (GENERAL_RESPOND_STATUS_SUCCESS != stat)
	{
		if (false == convertFromGeneralResMessageIdToString(stat,
												resMessage))
		{
			result += resMessage;
			return false;
		}

		result += resMessage;
		return true;
	}

	result.clear();
	return true;
}
bool Client::commandQuit(std::string& result)
{
	result = "Error on quit: ";
	/***************
	 * BuildPacket *
	 ***************/
	Packet quitPack;
	quitPack.writeForwardDWord(COMMANDTYPE_QUIT_REQ);

	/* send packet */
	if (false == sendCommandAndLogSocketError(quitPack,result))
	{
		return false;
	}

	/* just break the loop - and exit - not message to show */
	result.clear();
	return false;
}
bool Client::commandCompose(std::string& result)
{
	result = "Failed on compose mail: ";

	std::string toFeild;
	if (false == receiveUserCommandArg(PREFIX_INPUT_TO,
											toFeild))
	{
		result += toFeild;
		return true;
	}
	/* parse subject from input */
	std::string subjectFeild;
	if (false == receiveUserCommandArg(PREFIX_INPUT_SUBJECT,
											subjectFeild))
	{
		result += subjectFeild;
		return true;
	}
	/* parse password name from input */
	std::string textFeild;
	if (false == receiveUserCommandArg(PREFIX_INPUT_TEXT,
											textFeild))
	{
		result += textFeild;
		return true;
	}

	/***************
	 * BuildPacket *
	 ***************/
	Packet composePack;
	composePack.writeForwardDWord(COMMANDTYPE_COMPOSE_REQ);
	composePack.writeForwardStringField(toFeild);
	composePack.writeForwardStringField(subjectFeild);
	composePack.writeForwardStringField(textFeild);

	/* send packet */
	if (false == sendCommandAndLogSocketError(composePack,result))
	{
		return false;
	}

	/* receive response packet */
	Packet resPacket;
	if (false == receiveRespondAndLogSocketError(resPacket,result))
	{
		return false;
	}


	/**********************
	 * Check command type *
	 **********************/
	int32_t commandType;
	if (false == resPacket.readForwardDWord(commandType))
	{
		result += "Unable to read command type";
		return false;
	}

	/* parse respond */
	std::string resMessage;

	GeneralRespondStatuses stat = parseGeneralResponse(commandType,
																resPacket,
																resMessage);
	if (GENERAL_RESPOND_UNKNOWN_STATUS == stat)
	{
		result += "Error on parsing response " + resMessage;
		return false;
	}

	if (GENERAL_RESPOND_STATUS_SUCCESS != stat)
	{
		if (false == convertFromGeneralResMessageIdToString(stat,
												resMessage))
		{
			result += resMessage;
			return false;
		}

		result += resMessage;
		return true;
	}

	result = USER_MESSAGE_COMPOSE_SUCCESS;
	return true;
}

GeneralRespondStatuses Client::parseGeneralResponse( long commandType,
									Packet& pack,
									std::string& result)
{
	if (COMMANDTYPE_GENERAL_MESSAGE != commandType)
	{
		result += "Got wrong type response";
		return GENERAL_RESPOND_UNKNOWN_STATUS;
	}

	int32_t messageIdGeneralRes;
	if (false == pack.readForwardDWord(messageIdGeneralRes))
	{
		result += "Failed reading respond";
		return GENERAL_RESPOND_UNKNOWN_STATUS;
	}


	return (GeneralRespondStatuses)messageIdGeneralRes;
}

bool Client::convertFromGeneralResMessageIdToString(GeneralRespondStatuses messageIdGeneralRes,
															std::string& result)
{
	switch(messageIdGeneralRes)
	{
		case GENERAL_RESPOND_STATUS_SUCCESS:
		{
			result += "Command executed successfully.";
			break;
		}
		case GENERAL_RESPOND_STATUS_UNKNOWN_USER:
		{
			result += "Unknown user name.";
			break;
		}
		case GENERAL_RESPOND_STATUS_NOT_LOGGED_IN:
		{
			result += "User is not logged in.";
			break;
		}
		case GENERAL_RESPOND_STATUS_UNKNOWN_MAIL_ID:
		{
			result += "Unknown mail id.";
			break;
		}
		case GENERAL_RESPOND_STATUS_USER_ALREADY_LOGGEDON:
		{
			result += "User is already logged on.";
			break;
		}
		case GENERAL_RESPOND_STATUS_INTERNAL_FAILURE:
		{
			result += "Server internal error - try command again.";
			break;
		}
		case GENERAL_RESPOND_STATUS_FATAL_ERROR:
		{
			result += "Server had a fatal error and it had to go down - going down.";
			break;
		}
		case GENERAL_RESPOND_STATUS_UNVALID_MESSAGE:
		{
			result += "Server got unknown command - try different command.";
			break;
		}
		case GENERAL_RESPOND_STATUS_SESSION_FAILURE:
		{
			result += "Server connection error - going down.";
			break;
		}

		default:
		{
			result += "Unknown general message id.";
			return false;
		}
	}


	return true;
}


bool Client::printStringToUserLine(const std::string& output) const
{
	if (output.empty())
	{
		/* nothing to print */
		return true;
	}

	return Common::cmnPrintStringToUser(output.c_str());
}


bool Client::recvLineFromUser(std::string& input) const
{
	return Common::cmnRecvLineFromUser(input);
}


bool Client::receiveUserCommandArg(const std::string& expectedPrefix,
									std::string& data)
{
	std::string input;
	if (false == recvLineFromUser(input))
	{
		data += "Error reading user data";
		return false;
	}

	/* parse data from input */
	if (false == getStringFromInputWithPrefix(input,
											expectedPrefix,
											data))
	{
		return false;
	}

	return true;
}


bool Client::getStringFromInputWithPrefix(std::string& orgString,
											const std::string& expectedPrefix,
											std::string& data)
{
	std::basic_stringstream<char> inputStream(orgString);

	std::string prefix;
	inputStream >> prefix;
	if (inputStream.fail())
	{
		data += "Bad info Format";
		return false;
	}

	if (0 != expectedPrefix.compare(prefix))
	{
		data += "Unmatched Prefix, Expected: " + expectedPrefix;
		 	 	 " ,Got: " + prefix;

		return false;
	}

	inputStream >> data;
	if (inputStream.fail())
	{
		data += "Bad info format - failed to extract data";
		return false;
	}

	if (!inputStream.eof())
	{
		data += "Bad info format - got extra parameter";
		return false;
	}

	return true;
}

bool Client::sendCommandAndLogSocketError(Packet& pack,
											std::string& error)
{
	/* send packet */
	int res = m_sock.sendMessage(pack);
	if (Socket::RES_SUCCESS != res)
	{
		std::string socketErr;
		Socket::fromSocketResultToErrorString(res, socketErr);

		error += "Error on sending command: " +
					socketErr;

		return false;
	}

	return true;
}

bool Client::receiveRespondAndLogSocketError(Packet& pack,
											std::string& error)
{
	/* send packet */
	int res = m_sock.recvMessage(pack);
	if (Socket::RES_SUCCESS != res)
	{
		std::string socketErr;
		Socket::fromSocketResultToErrorString(res, socketErr);

		error += "Error on receiving response: " +
					socketErr;

		return false;
	}

	return true;
}
