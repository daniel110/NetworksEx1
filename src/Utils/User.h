#pragma once

#include "Packet.h"

class User
{
    private:
        std::string m_userName;
        std::string m_pass;

        void init(const std::string& userName,
        		  const std::string& pass);

    public:
        User(std::string& userName, std::string& pass);
        User(const User& user);

        Packet& getUserAsPacket();
        bool setUserFromPacket(Packet& pct);

        bool isPassCorrect(std::string& pass);

        std::string& getUserName();
};
