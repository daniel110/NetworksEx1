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
	static const long MAX_FIELD_LENGTH = 1024;

	Packet();
	virtual ~Packet();

	/* Read and write String field to the packet.
	 * a String field is 4 bytes that contain the length of the string (including the null-terminate)
	 * and then the full string with the null terminate.
	 * These functions use the DWord and String forward methods. */
	bool writeForwardStringField(const std::string& str);
	bool readForwardStringField(std::string& str);

	unsigned long bytesLeft() const;
	bool allocateForward(unsigned long more);

	/* Write a string into the packet. The null-terminate will be included */
	bool writeForward(const std::string& str);
	/* Read length bytes into a string. The length should contain the null-terminate */
	bool readForwardString(std::string& out, unsigned long length);

	/* Read and write 4 bytes that represent a 32 bit little endian number */
	bool writeForwardDWord(int32_t value);
	bool readForwardDWord(int32_t& output);

	/* Read and write specified number of bytes to and from a buffer */
	bool writeForward(const char * buf, unsigned len);
	bool readForward(char * dst, unsigned long len);

	/* Read lines. look for '\n' */
	unsigned long bytesLeftLine() const;
	unsigned long readLine(char * dst, unsigned long max);


	void jumptoStart();
	void jumptoEnd();

	char * getData() const;
	unsigned long getSize() const;

};

