#pragma once

#include "Packet.h"

/* User class
 *
 * Represent a user info.
 */
class User
{
    private:
        std::string m_userName;
        std::string m_pass;

        void init(const std::string& userName,
        		  const std::string& pass);

    public:
        /* This is just for Debug print user list */
        friend class Server;

        /* Initiate a user object */
        User(std::string& userName, std::string& pass);

        /* Check if a password is the user password.
         * @return True is the password is correct.
         */
        bool isPassCorrect(const std::string& pass) const;

        /* @return a Reference of the user name string.
         */
        const std::string& getUserName() const;
};
