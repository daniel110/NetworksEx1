#pragma once

#include <string>

#include "Packet.h"

class MailObj
{
public:
    /*  We assume all fields contain info */
    unsigned int m_id;
    std::string m_subject;
    std::string m_from;
    std::string m_to;
    std::string m_body;

    MailObj(MailObj& other);
    MailObj();


    bool getMailasPacket(Packet& pct) const;
    bool setMailAsPacket(Packet& pct);
};
