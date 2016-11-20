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
		static const int RES_SUCCESS = 0;
		static const int RES_ALLOCATION_FAILED = -100;
		static const int RES_INVALID_IP_ADDRESS = -500;
		static const int RES_INVALID_ADDRESS = -501;
		static const int RES_INVALID_SOCKET_ERROR = -555;
		static const int RES_ACCEPT_INVALID_SOCKET_ERROR = -556;

		static const std::string ANY_IP;


		Socket(socket_handle socketfd);

        Socket();
        virtual ~Socket();
        int create();

        int bind (const std::string& ip, const u_int16_t port);
        int listen(const unsigned int maxConnections);
        int accept( Socket& clientSocket) const;

        int connect(const std::string& ip, const u_int16_t port);
        int connect(struct in_addr& sinAddress, const u_int16_t port);

        int send ( const std::string& str) const;
        int send ( const Packet& packet) const;
        int recv ( Packet& packet, unsigned short size) const;

        bool isValid() const;

        void close();

    private:
        static const int SOCKET_INVALID_DEFAULT = -1;

		socket_handle m_socketfd;


		int innerConnect(struct sockaddr_in& remoteAddress);

		bool isSocketOpWorked(int result) const;

		/**
		 *
		 *
		 * @return
		 *  on success: RES_SUCCESS
		 *  on error: RES_INVALID_IP_ADDRESS -
		 *  			ip does not contain a character string
		 *  			representing a valid network address
		 */
		int setSocketAddr(struct sockaddr_in& address,
							const std::string& ip,
							const u_int16_t port) ;


		void setSocket(socket_handle socketfd);

};


