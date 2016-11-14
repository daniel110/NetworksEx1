#pragma once

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