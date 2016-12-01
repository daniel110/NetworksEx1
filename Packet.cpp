#include <cstring>

#include "Packet.h"


Packet::Packet()
{
	m_buff = nullptr;
	m_len = 0;
	m_cur = 0;
}

Packet::~Packet()
{
	if (nullptr != m_buff)
	{
		delete[] m_buff;
	}
	m_len = 0;
	m_cur = 0;
}


bool Packet::writeForwardStringField(const std::string& str)
{
	bool res = writeForwardDWord(str.size() + 1);
	if (res == false)
	{
		return false;
	}
	res = writeForward(str);
	return res;
}

bool Packet::readForwardStringField(std::string& str)
{
	int32_t length = 0;

	if ((false == readForwardDWord(length)) || (length <= 0) || (length > MAX_FIELD_LENGTH))
	{
		return false;
	}

	return readForwardString(str, length);
}

unsigned long Packet::bytesLeft() const
{
	return (m_len - m_cur);
}

bool Packet::allocateForward(unsigned long more)
{
	unsigned long new_total = m_len + more - bytesLeft();
	if (new_total > m_len)
	{
		char * tmp = new char[new_total];
		if (tmp != nullptr)
		{
			if (m_buff != nullptr)
			{
				memcpy(tmp, m_buff, m_len);
			}
			memset(tmp + m_len, 0, new_total - m_len);
			m_len = new_total;
			delete[] m_buff;
			m_buff = tmp;
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool Packet::writeForward(const char * buf, unsigned len)
{
	if (true == allocateForward(len))
	{
		memcpy(m_buff + m_cur, buf, len);
		m_cur += len;
		return true;
	}
	return false;
}

bool Packet::writeForward(const std::string& str)
{
	/* Include the null-terminate */
	return writeForward(str.c_str(), str.size() + 1);
}

bool Packet::writeForwardDWord(int32_t value)
{
	if (true == allocateForward(sizeof(value)))
	{
		*reinterpret_cast<int32_t*>(m_buff + m_cur) = value;
		m_cur += sizeof(value);
		return true;
	}
	return false;
}


bool Packet::readForwardDWord(int32_t& output)
{
	if (bytesLeft() >= sizeof(output))
	{
		output = *reinterpret_cast<int32_t*>(m_buff + m_cur);
		m_cur += sizeof(output);
		return true;
	}
	else
	{
		return false;
	}
}

bool Packet::readForward(char * dst, unsigned long len)
{
	if (bytesLeft() >= len)
	{
		memcpy(dst, m_buff + m_cur, len);
		m_cur += len;
		return true;
	}
	else
	{
		return false;
	}
}

bool Packet::readForwardString(std::string& out, unsigned long length)
{
	if (bytesLeft() < length)
	{
		return false;
	}

	/* length contains null terminate. */
	out.assign(m_buff + m_cur, length - 1);

	m_cur += length;

	return true;
}

char* Packet::getData() const
{
	char* newBuf = new char[m_len];
	memcpy(newBuf, m_buff, m_len);

	return newBuf;
}

unsigned long Packet::getSize() const
{
	return m_len;
}

void Packet::jumptoStart()
{
	m_cur = 0;
}

void Packet::jumptoEnd()
{
	m_cur = m_len;
}
