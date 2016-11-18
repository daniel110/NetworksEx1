#pragma once

#include <string>
#include <stdlib.h>

class Packet
{
private:
	char * m_buff;
	unsigned long m_len;
	unsigned long m_cur;



public:
	const long MAX_FIELD_LENGTH = 1024;

	Packet();
	bool writeForwardStringField(const std::string& str);
	bool readForwardStringField(std::string& str);
	unsigned long bytesLeft() const;
    unsigned long bytesLeftLine() const;
	bool allocateForward(unsigned long more);
	bool writeForword(const char * buf, unsigned len);
	bool writeForword(const std::string& str);
	bool writeForwordDWord(long value);
	bool readForwordDWord(long& output);
	bool readForword(char * dst, unsigned long len);
	bool readForwordString(std::string& out, unsigned long length);

	unsigned long readLine(char * dst, unsigned long max);

	char * getData() const;
	unsigned long getSize() const;

};

