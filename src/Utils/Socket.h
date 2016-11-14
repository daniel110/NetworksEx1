#pragma once

class Socket
{
    public:
        Socket();
        virtual ~Socket();
        bool create();

        bool bind (const std::string& host, const int port );
        bool listen();
        bool accept ( Socket& ) const;

        bool connect ( const std::string& host, const int port );

        bool send ( const std::string& ) const;
        bool send ( const Packet& ) const;
        int recv ( std::string& ) const;

        bool validity() const { return m_sock != -1; }

        void close();

    private:

        int m_sock;

        
        // sockaddr_in m_addr; Should be only for bind and connect ??
}