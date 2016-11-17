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
	Packet();
	unsigned long writeForwordStringField(std::string& str);
	unsigned long bytesLeft();
    unsigned long bytesLeftLine();
	bool allocateForward(unsigned long more);
	bool writeForword(const char * buf, unsigned len);
	unsigned long writeForword(std::string& str);
	bool writeForwordDWord(long value);
	bool readForwordDWord(long& output);
	bool readForword(char * dst, unsigned long len);
	unsigned long readLine(char * dst, unsigned long max);

	char * getData() const;
	unsigned long getSize() const;

};

