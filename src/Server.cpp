class server
{
    private:
        vector<std::pair<Inbox&, Socket&>> m_sessions;

    public:
        vector<MailObj>& getMailsBySender(char * name, unsigned long len);
        vector<MailObj>& getMailsByReceiver(char * name, unsigned long len);

        std::list<User&> getUserFromFile(str::String path);
        SetSessionsFromUsers(std::list<User&> usrs);


}