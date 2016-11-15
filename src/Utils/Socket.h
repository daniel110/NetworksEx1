#pragma once

enum StateMachineStep
{
    STATE_NON_AUTH,
    STATE_LOGEDON,
    STATE_MAILREQ

    
}

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

class ServerSocketSession : public Socket
{
    public:
        ServerSocketState()
        {
            m_state = STATE_NON_AUTH;
            m_userInbox = nullptr;
        }

        StateMachineStep getState()
        {
            return m_state;
        }
        void setState(StateMachineStep value)
        {
            m_state = value;
        }

        void setInbox(Inbox * ibx);


    private:
        StateMachineStep m_state;
        Inbox * m_userInbox;
}