/* client implementation */

#include "Client.h"
#include "Utils/Common.h"
#include "Utils/Packet.h"


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

const unsigned int Client::MAX_USER_INPUT_LINE = 2000;


const std::string Client::USER_MESSAGE_LOGIN_SUCCESS = "Connected to server\n";
const std::string Client::USER_MESSAGE_COMPOSE_SUCCESS = "Mail sent\n";



Client::Client(std::string& hostname, u_int16_t port) : m_hostname(hostname)
{
	m_port = port;
}

void Client::start()
{
	int res = 0;
	res = m_sock.connect(m_hostname, m_port);
	if (RES_INVALID_ADDRESS == res)
	{
		printStringToUser("Invalid Hostname Address\n");
		return;
	}

	/* TODO: add recv welcome message */


	bool keepGoing = true;;
	std::string resultStr;

	/* First and must operation: login user to server*/
	keepGoing = commandLogin(resultStr);
	if (false == keepGoing)
	{
		printStringToUser(resultStr.c_str());
		return;
	}

	/******************************************
	 * Mail Loop: receive and execute command *
	 ******************************************/
	std::string input;
	while (keepGoing)
	{
		if (false == recvLineFromUser(input))
		{
			printStringToUser("Unable to read command - Going down\n");
			break;
		}
		std::basic_stringstream<char> inputStream(input);

		/* Get only first word (should be a command string) */
		std::string commandTypeName;
		inputStream >> commandTypeName;
		if (inputStream.fail() )
		{
			printStringToUser("Unable to extract command type\n");
			continue;
		}

		/********************************
		 * Call correct command handler *
		 ********************************/
		resultStr = "Got Extra arguments. Try command again.\n";

		if (0 == commandTypeName.compare(COMMAND_SHOW_INBOX))
		{
			/* before calling each command - we check that no extra argument
			 * was passed by the user */
			if (!inputStream.eof())
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
				resultStr = "Unable to extract mail id.\n";
			}
			else
			{
				if (!inputStream.eof())
				{
					resultStr = "The second and last parameter of GET_MAIL should be int\n";
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
				resultStr = "Unable to extract mail id.\n";
			}
			else
			{
				if (!inputStream.eof())
				{
					resultStr = "The second and last parameter of DELETE_MAIL should be int\n";
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
			resultStr = "Invalid command type name\n";
		}


		/* log handler result */
		if ( false == printStringToUser(resultStr.c_str()))
		{
			/* nothing to do, just exit */
			keepGoing = false;
		}
	}

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

	long commandType;
	if (false == resPacket.readForwardDWord(commandType))
	{
		result = "Unable to read command type\n";
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
	long commandType;
	if (false == resPacket.readForwardDWord(commandType))
	{
		result = "Unable to read command type\n";
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
			result += "Failed to read next mail from packet\n";
			return true;
		}

		if (false == printStringToUser(mailInfo.c_str()))
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
	long commandType;
	if (false == resPacket.readForwardDWord(commandType))
	{
		result = "Unable to read command type\n";
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
		result += "Failed to read 'From' field\n";
		return true;
	}

	mailInfo = PREFIX_MAIL_DATA_ON_GET_MAIL_FROM + mailInfo;
	if (false == printStringToUser(mailInfo.c_str()))
	{
		/* nothing to do, just exit */
		result.clear();
		return false;
	}

	if (false == resPacket.readForwardStringField(mailInfo))
	{
		result += "Failed to read 'To' field\n";
		return true;
	}

	mailInfo = PREFIX_MAIL_DATA_ON_GET_MAIL_TO + mailInfo;
	if (false == printStringToUser(mailInfo.c_str()))
	{
		/* nothing to do, just exit */
		result.clear();
		return false;
	}


	if (false == resPacket.readForwardStringField(mailInfo))
	{
		result += "Failed to read 'Subject' field\n";
		return true;
	}

	mailInfo = PREFIX_MAIL_DATA_ON_GET_MAIL_SUBJECT + mailInfo;
	if (false == printStringToUser(mailInfo.c_str()))
	{
		/* nothing to do, just exit */
		result.clear();
		return false;
	}


	if (false == resPacket.readForwardStringField(mailInfo))
	{
		result += "Failed to read 'Text' field\n";
		return true;
	}

	mailInfo = PREFIX_MAIL_DATA_ON_GET_MAIL_TEXT + mailInfo;
	if (false == printStringToUser(mailInfo.c_str()))
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
	deleteMailPack.writeForwardDWord(COMMANDTYPE_GET_MAIL_REQ);
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
	long commandType;
	if (false == resPacket.readForwardDWord(commandType))
	{
		result = "Unable to read command type\n";
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
	long commandType;
	if (false == resPacket.readForwardDWord(commandType))
	{
		result = "Unable to read command type\n";
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
		result += "Got wrong type response\n";
		return GENERAL_RESPOND_UNKNOWN_STATUS;
	}

	long int messageIdGeneralRes;
	if (false == pack.readForwardDWord(messageIdGeneralRes))
	{
		result += "Failed reading respond\n";
		return GENERAL_RESPOND_UNKNOWN_STATUS;
	}


	return (GeneralRespondStatuses)messageIdGeneralRes;
}

bool Client::convertFromGeneralResMessageIdToString(GeneralRespondStatuses messageIdGeneralRes,
															std::string& result)
{
	switch(messageIdGeneralRes)
	{

	/*TODO: added message */

	default:
		result += "Unknown general message id\n";
		return false;

	}


	return true;
}


bool Client::printStringToUser(const char* output)
{
	std::cout << output << std::endl;
	if (std::cout.fail())
	{
		return false;
	}

	return true;
}


bool Client::recvLineFromUser(std::string& input)
{
	char buf[MAX_USER_INPUT_LINE];

	if (NULL == std::fgets(buf, MAX_USER_INPUT_LINE, stdin))
	{
		return false;
	}

	input = buf;
	return true;
}


bool Client::receiveUserCommandArg(const std::string& expectedPrefix,
									std::string& data)
{
	std::string input;
	if (false == recvLineFromUser(input))
	{
		data += "Error reading user data\n";
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
		data = "Bad info Format\n";
		return false;
	}

	if (0 != expectedPrefix.compare(prefix))
	{
		data = "Unmatched Prefix, Expected: " + expectedPrefix;
		 	 	 " ,Got: " + prefix;

		return false;
	}

	inputStream >> data;
	if (inputStream.fail())
	{
		data = "Bad info format - failed to extract data\n";
		return false;
	}

	if (!inputStream.eof())
	{
		data = "Bad info format - got extra parameter\n";
		return false;
	}

	return true;
}

bool Client::sendCommandAndLogSocketError(Packet& pack,
											std::string& error)
{
	/* send packet */
	int res = m_sock.sendMessage(pack);
	if (RES_SUCCESS != res)
	{
		std::string socketErr;
		Socket::fromSocketResultToErrorString(res, socketErr);

		error += "Error on sending command: " +
					socketErr + "\n";

		return false;
	}

	return true;
}

bool Client::receiveRespondAndLogSocketError(Packet& pack,
											std::string& error)
{
	/* send packet */
	int res = m_sock.recvMessage(pack);
	if (RES_SUCCESS != res)
	{
		std::string socketErr;
		Socket::fromSocketResultToErrorString(res, socketErr);

		error += "Error on receiving response: " +
					socketErr + "\n";

		return false;
	}

	return true;
}
