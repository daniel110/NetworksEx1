#pragma once

#include <string>
#include <stdlib.h>

/* Packet Class
 *
 * This represent a buffer of a packet and let us read or write fields as needed.
 */
class Packet
{
private:
	/* Pointer to an allocated buffer that is the packet data */
	char * m_buff;
	/* The size of the buffer */
	unsigned long m_len;
	/* Current index within the buffer to read/write to */
	unsigned long m_cur;



public:
	static const long MAX_FIELD_LENGTH = 1024;

	/* Create an empty packet. */
	Packet();
	/* Release resource. */
	virtual ~Packet();

	/* Write a String field to the packet.
	 * a String field is 4 bytes that contain the length of the string (including the null-terminate)
	 * and then the full string with the null terminate.
	 * Using the DWord and String forward methods.
	 * @return True on success. If false, the position of the index of the packet is undefined. */
	bool writeForwardStringField(const std::string& str);


	/* Read a String field from the packet.
	 * a String field is 4 bytes that contain the length of the string (including the null-terminate)
	 * and then the full string with the null terminate.
	 * Using the DWord and String forward methods.
	 * @return True on success. If false, the position of the index of the packet is undefined. */
	bool readForwardStringField(std::string& str);


	/* @return The number of bytes available to read forward */
	unsigned long bytesLeft() const;

	/* Check if there are *more* bytes available to write forward. If not,
	 * Try to allocate a new buffer with the right size.
	 * On success, the state of the packet will stay the same, except it's size.
	 * The data of the buffer in the new area (new size - old size) is undefined.
	 * @return Allocation succeeded. If false. the packet state stayed the same. */
	bool allocateForward(unsigned long more);

	/* Write a string into the packet. The null-terminate will be included.
	 * Use allocateForward() if needed.
	 * @return True on success. If false, the state of the packet is saved. */
	bool writeForward(const std::string& str);
	/* Read length bytes into a string. The length should contain the null-terminate.
	 * @return True on success. If false, Not enough byte forward available. */
	bool readForwardString(std::string& out, unsigned long length);

	/* Write a DWord (32 bit little endian number) into the packet.
	 * Use allocateForward() if needed.
	 * @return True on success. If false, the state of the packet is saved. */
	bool writeForwardDWord(int32_t value);
	/* Read a DWord (32 bit little endian number) from the packet.
	 * @return True on success. If false, Not enough byte forward available. */
	bool readForwardDWord(int32_t& output);

	/* Read and write specified number of bytes to and from a buffer */

	/* Write a buffer into the packet.
	 * Use allocateForward() if needed.
	 * @return True on success. If false, the state of the packet is saved. */
	bool writeForward(const char * buf, unsigned len);
	/* Read a buffer from the packet.
	 * @return True on success. If false, Not enough byte forward available. */
	bool readForward(char * dst, unsigned long len);

	/* Set the position of the index to be the start of the packet */
	void jumptoStart();
	/* Set the position of the index to be the end of the packet */
	void jumptoEnd();

	/* Get a new allocated buffer with the copied data from the packet.
	 * The buffer will contain ALL packet data, no matter what the position of the index is.
	 * Don't forget to delete the new buffer. */
	char * getData() const;
	/* Get the size of the packet. */
	unsigned long getSize() const;

};

