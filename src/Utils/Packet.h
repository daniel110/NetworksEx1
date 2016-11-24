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
	virtual ~Packet();
	bool writeForwardStringField(const std::string& str);
	bool readForwardStringField(std::string& str);
	unsigned long bytesLeft() const;
    unsigned long bytesLeftLine() const;
	bool allocateForward(unsigned long more);
	bool writeForward(const char * buf, unsigned len);
	bool writeForward(const std::string& str);
	bool writeForwardDWord(int32_t value);
	bool readForwardDWord(int32_t& output);
	bool readForward(char * dst, unsigned long len);
	bool readForwardString(std::string& out, unsigned long length);

	unsigned long readLine(char * dst, unsigned long max);

	void jumptoStart();
	void jumptoEnd();


	char * getData() const;
	unsigned long getSize() const;

};

