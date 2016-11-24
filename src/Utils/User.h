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

        bool getUserAsPacket(Packet& pckt);
        bool setUserFromPacket(Packet& pct);

        bool isPassCorrect(const std::string& pass) const;

        const std::string& getUserName() const;
};
