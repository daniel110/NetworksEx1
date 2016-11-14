#pragma once

class Packet
{
    private:
        char * m_buff;
        unsigned long m_len;
        unsigned long m_cur;
    public:
        Packet()
        {
            m_buff = nullptr;
            m_len = 0;
            m_cur = 0;
        }

        bool writeDWordForword(long value);
        bool readDWordForword(long& output);

        unsigned long writeForword(char * buf, unsigned len);
        unsigned long readForword(char * dst, unsigned long len);
        unsigned long readLine(char * dst, unsigned long max);
}