#include "Common.h"


const std::string Common::GENERAL_RESPONSE_SUCCUSS_MESSAGE = "S";


bool Common::cmnPrintStringToUser(const char* output)
{
	std::cout << output << std::endl;
	if (std::cout.fail())
	{
		return false;
	}

	return true;
}


bool Common::cmnRecvLineFromUser(std::string& input)
{
	char buf[MAX_USER_INPUT_LINE];

	if (NULL == std::fgets(buf, MAX_USER_INPUT_LINE, stdin))
	{
		return false;
	}

	input = buf;
	return true;
}
