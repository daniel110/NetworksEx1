class server
{
    private:
        vector<ServerSocketSession*> m_sessions;

        Socket listener;
        std::list<Inbox*> m_all_inbox;

    public:
        vector<MailObj>& getMailsBySender(char * name, unsigned long len);
        vector<MailObj>& getMailsByReceiver(char * name, unsigned long len);

        std::list<User&> getUserFromFile(str::String path);
        SetSessionsFromUsers(std::list<User&> usrs);

        void ListenLoop();

        /*  Creates and adds to the session list */
        ServerSocketSession * createNewSession(SOCKET sck);

        ServerSocketSession * getSessionFromSocket(SOCKET sck);

        void ProcessSocket(ServerSocketSession * socket);

}