class Packet
{
    private:
        std::string m_buff;
        unsigned long m_len;
        unsigned long m_cur;
    public:
        Packet()
        {
            m_len = 0;
            m_cur = 0;
        }

        unsigned long writeForword(std::string& buf);
        std::string& readForword(unsigned long);
        std::string& readLine();
}

class MailObj
{
    unsigned long m_id;
    std::string m_subject;
    std::string m_from;
    std::string m_to;
    std::string m_body;

    MailObj(MailObj& other);

    Packet& getMailasPacket();
    bool setMailAsPacket(Packet& pct);
}

class PacketCreator
{
    /*  Server
    ************* */
    static Packet& getWelcomePacket();
    static Packet& getShowInboxResponseAsPacket(Inbox& box);
    /*  null is no mail exists */
    static Packet& getGetMailResponseAsPacket(MailObj * mail);
    static Packet& getDeleteResponseAsPacket(int state);
    static Packet& getLogingStatePacket(User& user, int state);
    static Packet& getComposeResponseAsPacket(int state);


    /*  Client
    ************* */
    static Packet& getShowInboxRequestAsPacket();
    static Packet& getGetMailRequestAsPacket(unsigned long id);
    static Packet& getDeleteRequestAsPacket(unsigned long id);
    static Packet& getLoginRequestAsPacket(unsigned long id);
    static Packet& getComposeRequestAsPacket(MailObj& mail);
}


class Socket
{
    public:
        Socket();
        virtual ~Socket();
        bool create();

        bool bind (const std::string host, const int port );
        bool listen();
        bool accept ( Socket& ) const;

        bool connect ( const std::string host, const int port );

        bool send ( const std::string ) const;
        bool send ( const Packet& ) const;
        int recv ( std::string& ) const;

        bool validity() const { return m_sock != -1; }

        void close();

    private:

        int m_sock;

        
        // sockaddr_in m_addr; Should be only for bind and connect ??
}

class Inbox
{
    private:
        std::list<mails*> mails;
        unsigned long idOfLastMail;
    public:
        User& user;

        Inbox(user& usr);
        bool addMail(MailObj * mil);
        bool removeMail(unsigned long id);
        Packet& getShowInboxAsPacket();
}


class User
{
    private:
        std::string m_userName;
        std::string m_pass;

    public:
        User(std::string& userName, std::string& pass);

        Packet& getUserAsPacket();
        bool setUserFromPacket(Packet& pct);

        bool isPassCorrect(std::string& pass);

        std::string& getUserName();
}
