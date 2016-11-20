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


Client::Client(std::string& hostname, u_int16_t port,
		std::ostream& out, std::istream& in) : m_hostname(hostname),
												m_outStream(out),
											   m_inStream(in)
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


	bool keepGoing;
	std::string resultStr;

	/* login user */
	keepGoing = commandLogin(resultStr);


	std::string input;
	while (true && keepGoing)
	{
		if (false == recvStringFromUser(input))
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
		if (0 == commandTypeName.compare(COMMAND_SHOW_INBOX))
		{
			keepGoing = commandShowInbox(resultStr);
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
				keepGoing = commandGetMail(id, resultStr);
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
				keepGoing = commandDeleteMail(id, resultStr);
			}
		}
		else if (0 == commandTypeName.compare(COMMAND_COMPOSE))
		{
			keepGoing = commandCompose(resultStr);
		}
		else if (0 == commandTypeName.compare(COMMAND_QUIT))
		{
			keepGoing = commandQuit(resultStr);
		}
		else
		{
			resultStr = "Invalid command type name\n";
			keepGoing = false;
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

	std::string user;
	getStringFromInputWithPrefix(PREFIX_INPUT_USER, user);

	std::string pass;
	getStringFromInputWithPrefix(PREFIX_INPUT_PASSWORD, pass);

	/***************
	 * BuildPacket *
	 ***************/
	Packet loginPack;
	loginPack.writeForwardDWord(COMMANDTYPE_LOGIN_REQ);
	loginPack.writeForwardStringField(user);
	loginPack.writeForwardStringField(pass);

	/* TODO: check error */
	int res = m_sock.send(loginPack);

	Packet responde;
	/* TODO: check error */
	res = m_sock.recv(responde, MAX_MESSAGE_SIZE);

	std::string resMessage;
	if (false == parseGeneralResponse(resMessage))
	{

	}

	printStringToUser(resMessage.c_str());

	return true;
}
bool Client::commandShowInbox(std::string& result)
{

	return true;
}
bool Client::commandGetMail(unsigned int mailId, std::string& result)
{

	return true;
}
bool Client::commandDeleteMail(unsigned int mailId, std::string& result)
{

	return true;
}
bool Client::commandQuit(std::string& result)
{

	return true;
}
bool Client::commandCompose(std::string& result)
{
	return true;
}

bool Client::parseGeneralResponse(std::string& result)
{


	return true;
}


bool Client::printStringToUser(const char* output)
{
	m_outStream << output;
	if (m_outStream.fail())
	{
		return false;
	}

	return true;
}
bool Client::printIntToUser(int& output)
{
	m_outStream << output;
	if (m_outStream.fail())
	{
		return false;
	}

	return true;
}

bool Client::recvStringFromUser(std::string& input)
{
	m_inStream >> input;
	if (m_inStream.fail())
	{
		return false;
	}

	return true;
}


bool Client::getStringFromInputWithPrefix(const std::string& expectedPrefix,
											std::string& data)
{
	std::string prefix;
	m_inStream >> prefix;
	if (m_inStream.fail())
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

	m_inStream >> data;
	if (m_inStream.fail())
	{
		data = "Bad info format - failed to extract data\n";
		return false;
	}

	return true;
}
