#pragma once

#include <string>

#include "Packet.h"

/* MailObj Class
 *
 * Represent an email with all of its fields.
 */
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

    /* Write to the packet the emails fields for the get_email response on
     * the server side.
     * @return True on success. */
    bool getMailasPacket(Packet& pct) const;
    /* Read from the packet the fields and put them into this mail.
     * The From field is set to the the user argument.
     * This method used in the compose request on the server side.
     * @return True on success. */
    bool setMailAsPacket(const std::string& user, Packet& pct);
};
