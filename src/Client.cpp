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


const unsigned int Client::MAX_USER_INPUT_LINE = 2000;


const std::string Client::USER_MESSAGE_LOGIN_SUCCESS = "Connected to server\n";


Client::Client(std::string& hostname, u_int16_t port) : m_hostname(hostname)
{
	m_port = port;
}

void Client::start()
{
	int res = 0;
	res = m_sock.connect(m_hostname, m_port);
	if (Socket::RES_INVALID_ADDRESS == res)
	{
		printStringToUser("Invalid Hostname Address\n");
		return;
	}


	bool keepGoing = true;;
	std::string resultStr;

	/* login user */
	keepGoing = commandLogin(resultStr);
	if (false == keepGoing)
	{
		printStringToUser(resultStr.c_str());
	}

	std::string input;
	while (true && keepGoing)
	{
		if (false == recvLineFromUser(input))
		{
			printStringToUser("Unable to read command - Going down\n");
			break;
		}
		std::basic_stringstream<char> inputStream(input);
		/*Get only first word*/
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
		resultStr = "Wrong arguments. Try command again\n";

		if (0 == commandTypeName.compare(COMMAND_SHOW_INBOX))
		{
			if (!inputStream.eof())
			{
				keepGoing = commandShowInbox(resultStr);
			}
		}
		else if (0 == commandTypeName.compare(COMMAND_GET_MAIL))
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


		if ( false == printStringToUser(resultStr.c_str()))
		{
			/* nothing to do, just exit */
			break;
		}
	}

}

bool Client::commandLogin(std::string& result)
{
	result = "Failed on Login: ";

	/* get user name from user */
	std::string input;
	if (false == recvLineFromUser(input))
	{
		result += "Error reading user data\n";
		return false;
	}

	/* parse user name from input */
	std::string user;
	if (false == getStringFromInputWithPrefix(input,
											PREFIX_INPUT_USER,
											user))
	{
		result += user;
		return false;
	}

	/* get password from user */
	if (false == recvLineFromUser(input))
	{
		result += "Error reading user data\n";
		return false;
	}

	/* parse password name from input */
	std::string pass;
	if (false == getStringFromInputWithPrefix(input,
											PREFIX_INPUT_PASSWORD,
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

	/* send packet */
	int res = m_sock.sendMessage(loginPack);
	if (Socket::RES_SUCCESS != res)
	{
		result += "Error on sending response\n";
		return false;
	}

	/* receive response packet */
	Packet resPacket;
	res = m_sock.recvMessage(resPacket);
	if (Socket::RES_SUCCESS != res)
	{
		result += "Error on receiving response\n";
		return false;
	}

	/* parse respond */
	std::string resMessage;
	if (false == parseGeneralResponse(resPacket, resMessage))
	{
		result += "Error on parsing response " + resMessage;
		return false;
	}

	if (0 != resMessage.compare(Common::GENERAL_RESPONSE_SUCCUSS_MESSAGE))
	{
		result += resMessage;
		return false;
	}

	result = USER_MESSAGE_LOGIN_SUCCESS;
	return true;
}
bool Client::commandShowInbox(std::string& result)
{
	result = "Failed on Show Inbox: ";

	return true;
}
bool Client::commandGetMail(unsigned int mailId, std::string& result)
{
	result = "Failed on get mail: ";

	return true;
}
bool Client::commandDeleteMail(unsigned int mailId, std::string& result)
{
	result = "Failed on delete mail: ";

	return true;
}
bool Client::commandQuit(std::string& result)
{
	result = "Failed on quit: ";

	return true;
}
bool Client::commandCompose(std::string& result)
{
	result = "Failed on compose mail: ";

	return true;
}

bool Client::parseGeneralResponse(Packet& pack, std::string& result)
{
	long commandType;
	if (false == pack.readForwardDWord(commandType))
	{
		result = "Unable to read command type\n";
		return false;
	}

	if (COMMANDTYPE_GENERAL_MESSAGE != commandType)
	{
		result = "Got wrong type response\n";
		return false;
	}

	if (false == pack.readForwardStringField(result))
	{
		result = "Failed reading respond\n";
		return false;
	}

	return true;
}


bool Client::printStringToUser(const char* output)
{
	std::cout << output;
	if (std::cout.fail())
	{
		return false;
	}

	return true;
}
bool Client::printIntToUser(int& output)
{
	std::cout << output;
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
