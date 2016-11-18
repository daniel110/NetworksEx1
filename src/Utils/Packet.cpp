#include <cstring>

#include "Packet.h"


Packet::Packet()
{
	m_buff = nullptr;
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
	long length = 0;

	if ((false == readForwardDWord(length)) || (length <= 0) || (length > MAX_FIELD_LENGTH))
	{
		return false;
	}
	/* length contains null terminate. assign don't need it */
	return readForwardString(str, length - 1);
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
	return writeForward(str.c_str(), str.size() + 1);
}

bool Packet::writeForwardDWord(long value)
{
	if (true == allocateForward(sizeof(value)))
	{
		*reinterpret_cast<long*>(m_buff + m_cur) = value;
		m_cur += sizeof(value);
		return true;
	}
	return false;
}


bool Packet::readForwardDWord(long& output)
{
	if (bytesLeft() >= sizeof(long))
	{
		output = *reinterpret_cast<long*>(m_buff + m_cur);
		m_cur += sizeof(long);
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

unsigned long Packet::bytesLeftLine() const
{
	unsigned long cur = m_cur;
	unsigned long old_cur = m_cur;
	unsigned long left = bytesLeft();

	while (left > 0)
	{
		char cur_char = *(m_buff + cur);
		cur ++;
		if (cur_char == '\n')
		{
			break;
		}
		left -= 1;
	}

	return (cur - old_cur);
}

bool Packet::readForwardString(std::string& out, unsigned long length)
{
	if (bytesLeft() < length)
	{
		return false;
	}

	out.assign(m_buff + m_cur, length);
	return true;
}

unsigned long Packet::readLine(char * dst, unsigned long max)
{
	unsigned long old_cur = m_cur;
	unsigned long left = bytesLeft();

	while (left > 0)
	{
		char cur_char = *(m_buff + m_cur);
		*dst = cur_char;
		m_cur ++;
		if (cur_char == '\n')
		{
			break;
		}
		left -= 1;
		dst += 1;
	}

	return (m_cur - old_cur);
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
