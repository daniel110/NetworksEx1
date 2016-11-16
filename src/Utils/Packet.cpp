

	Packet::Packet()
	{
		m_buff = nullptr;
		m_len = 0;
		m_cur = 0;
	}


	unsigned long Packet::writeForwordStringField(std::string& str)
	{
		unsigned long sum = 0;
		sum += writeForwordDWord(str.size() + 1);
		sum += writeForword(str);
		return sum;
	}

	unsigned long Packet::bytesLeft()
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

	bool Packet::writeForword(const char * buf, unsigned len)
	{
		if (true == allocateForward(len))
		{
			memcpy(m_buff + m_cur, buf, len);
			m_cur += len;
			return true;
		}
		return false;
	}

	unsigned long Packet::writeForword(std::string& str)
	{
		return writeForword(str.c_str(), str.size() + 1);
	}
	
	bool Packet::writeForwordDWord(long value)
	{
		if (true == allocateForward(sizeof(value)))
		{
			*reinterpret_cast<long*>(m_buff + m_cur) = value;
			m_cur += sizeof(value);
			return true;
		}
		return false;
	}


	bool Packet::readForwordDWord(long& output)
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

	bool Packet::readForword(char * dst, unsigned long len)
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

    unsigned long Packet::bytesLeftLine()
    {
        
    }

	unsigned long Packet::readLine(char * dst, unsigned long max)
	{
		unsigned long old_cur = m_cur;
		unsigned long left = bytesLeft();

		while (left > 0)
		{
				left -= 1;
				m_cur += 1;
				dst += 1;
				*dst = *(m_buff + m_cur);
				if (*dst == '\n')
				{
					break;
				}
		}

		return (m_cur - old_cur);
	}
