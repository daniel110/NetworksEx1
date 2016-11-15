#pragma once

class MailObj
{
    /*  We assume all fields contain info */
    unsigned long m_id;
    std::string m_subject;
    std::string m_from;
    std::string m_to;
    std::string m_body;

    MailObj(MailObj& other);

    Packet& getMailasPacket();
    bool setMailAsPacket(Packet& pct);
}