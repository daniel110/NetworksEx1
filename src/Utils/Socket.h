#pragma once

#include <string>

#include "Packet.h"

typedef int socket_handle;

/***
 * Socket class is a warpper for socket linux api
 */
class Socket
{
    public:
		const int RECEIVE_MAX_BUF_SIZE = 1024;

		const int RES_INVALID_SOCKET_ERROR = -555;
		const int RES_INVALID_IP_ADDRESS = -500;
		const int RES_INVALID_ARG_SIZE = -505;

		Socket(socket_handle socketfd);

        Socket();
        virtual ~Socket();
        int create();

        int bind (const std::string& ip, const u_int16_t port);
        int listen(const unsigned int maxConnections);
        Socket* accept ( Socket& ) const;

        int connect(const std::string& ip, const u_int16_t port);
        int connect(struct in_addr& sinAddress, const u_int16_t port);

        int send ( const std::string& str) const;
        int send ( const Packet& packet) const;
        int recv ( Packet& packet, unsigned short size) const;

        bool isValid() const;

        void close();

    private:

    socket_handle m_socketfd;


    int innerConnect(struct sockaddr_in& remoteAddress);

    bool isSocketOpWorked(int result) const;

    /**
     *
     *
     * @return
     *  on success: 0
     *  on error: INVALID_IP_ADDRESS -
     *  			ip does not contain a character string
     *  			representing a valid network address
     */
    int setSocketAddr(struct sockaddr_in& address,
    					const std::string& ip,
    					const u_int16_t port) ;
};


