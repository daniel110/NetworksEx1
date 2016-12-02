#pragma once

#include <string>

#include "Packet.h"


typedef int socket_handle;

/***
 * Socket class
 * a Wrapper for socket linux api
 *
 *	Usage:
 *		1) server side init flow:
 *			create -> bind -> listen -> accept
 *
 *		2) client side init flow:
 *			create -> connect
 */
class Socket
{

    public:

		/* Allow it to access the private socket descriptor */
		friend class FDSet;
		friend class ServerSessionSocket;

		/* Possible return values for each public method. */
		enum socketError
		{
			RES_BAD_MESSAGE_FORMAT = -700,
			RES_FAILED_RECEIVING_ALL_DATA = -601,
			RES_ACCEPT_INVALID_SOCKET_ERROR = -556,
			RES_INVALID_SOCKET_ERROR = -555,
			RES_INVALID_ADDRESS = -501,
			RES_INVALID_IP_ADDRESS = -500,
			RES_ALLOCATION_FAILED = -100,
			RES_SOCKET_GENERAL_FAILURE = -1,
			RES_PACKET_TOO_LONG = -502,

			RES_SUCCESS = 0,
		};

		/* Length field type */
		typedef int32_t length_field_t;
		static const length_field_t MAX_PACKET_LENGTH = 0xFFFF;

		/* ANY_IP constant */
		static const std::string ANY_IP;

		/****
		 * Ctors: empty or gets socket handle
		 */
		Socket(socket_handle socketfd);
        Socket();

        /****
         * Dtor - close m_socketfd
         */
        virtual ~Socket();

        /****
         * Create socket for IP + TCP connection type
         *
		 * @return:
		 *  on success: RES_SUCCESS
		 *  on Failure: RES_INVALID_SOCKET_ERROR - if socket not valid
		 *  										(closed or not created)
         */
        int create();

        /*************************
         * Servers functionality *
         *************************/
        /***
         * bind socket to *ip* and *port*
         *
		 * @return:
		 *  on success: RES_SUCCESS
		 *  on Failure: RES_INVALID_SOCKET_ERROR - if socket not valid
		 *  										(closed or not created)
		 *  			RES_INVALID_ADDRESS - *ip* not valid ip string
         */
        int bind (const std::string& ip, const u_int16_t port);

        /***
	     * start listen on socket, allow *maxConnections* simultaneously
	     *
	     *
		 * @return:
		 *  on success: RES_SUCCESS
		 *  on Failure: RES_INVALID_SOCKET_ERROR - if socket not valid
		 *  										(closed or not created)
	     */
        int listen(const unsigned int maxConnections);

        /****
         * Waiting for client connection - set *clientSocket* to the new connection
         *
		 * @return:
		 *  on success: RES_SUCCESS
		 *  on Failure: RES_INVALID_SOCKET_ERROR - if socket not valid
		 *  										(closed or not created)
		 *  			RES_ACCEPT_INVALID_SOCKET_ERROR - failed on accepting client
         */
        int accept( Socket& clientSocket) const;
        /************************/

        /**********************
         * User functionality *
         **********************/

        /****
         * connect user to server by *host* and *port*
         * 		if the host is not an ip address, try to convert it to host,
         * 		(note: even if multiply ip address is available check only the first)
         *
         *
		 * @return:
		 *  on success: RES_SUCCESS
		 *  on Failure: RES_INVALID_SOCKET_ERROR - if socket not valid
		 *  										(closed or not created)
		 *  			RES_SOCKET_GENERAL_FAILURE
         */
        int connect(const std::string& host, const u_int16_t port);
        /**********************/


        /****
		 *	send *buf* data of size *size
		 *
		 * @return:
		 *  on success: RES_SUCCESS
		 *  on Failure: RES_INVALID_SOCKET_ERROR - if socket not valid
		 *  										(closed or not created)
		 *  			RES_SOCKET_GENERAL_FAILURE
		 */
        int sendAll(const char* buf, long size) const;

        /****
		 * send *packet* data (using Socket::sendAll)
		 *
		 * @return:
		 *  on success: RES_SUCCESS
		 *  on Failure: RES_INVALID_SOCKET_ERROR - if socket not valid
		 *  										(closed or not created)
		 *  			RES_SOCKET_GENERAL_FAILURE
		 */
        int send ( const Packet& packet) const;

        /****
		 * set *packet* with received data of size *size*
		 *
		 * @return:
		 *  on success: RES_SUCCESS
		 *  on Failure: RES_INVALID_SOCKET_ERROR - if socket not valid
		 *  										(closed or not created)
		 *  			RES_ALLOCATION_FAILED - failed to allocate buffer to receive the data
		 *  			RES_FAILED_RECEIVING_ALL_DATA - got different then expected
		 */
        int recv ( Packet& packet, unsigned short size) const;

        /****
		 * receive packet data by reading the size field,
		 * check the packet size and then the data (calls recv).
		 * The size field is a 4-byte little endian number.
		 *
		 * @return:
		 *  on success: RES_SUCCESS
		 *  on Failure: RES_INVALID_SOCKET_ERROR - if socket not valid
		 *  										(closed or not created)
		 *  			RES_FAILED_RECEIVING_ALL_DATA - got different size then expected
		 */
        int recvMessage(Packet& packet) const;
        /****
		 * Send packet data, and before that the packet size field.
		 * The size field is a 4-byte little endian number.
		 * (using Socket::sendAll and Socket::send)
		 *
		 * @return:
		 *  on success: RES_SUCCESS
		 *  on Failure: RES_INVALID_SOCKET_ERROR - if socket not valid
		 *  										(closed or not created)
		 */
        int sendMessage(const Packet& packet) const;

        /****
         * Check whether the socket is valid
         *
         * @return: if valid true, else false
         */
        bool isValid() const;

        /****
         * close socket
         */
        void close();

        /****
         * translate socket *result* to error string
         * (set *error* with the error string)
         */
        static void fromSocketResultToErrorString(int result,
        										std::string& error);

    protected:
        socket_handle m_socketfd;

    private:
        static const int SOCKET_INVALID_DEFAULT = -1;

        /* return values of apis */
        static const int GETADDRINFO_SUCCESS_RET = 0;
        static const int INET_ATON_FAILED_RET = 0;


		/****
		 *	connecting to the server
		 *
		 * @return:
		 *  on success: RES_SUCCESS
		 */
		int innerConnect(struct sockaddr_in& remoteAddress);

		/**
		 * set *sockaddr_in* with *host* and *port* arguments
		 * *host* could be hostname or ip.
		 *
		 * @return:
		 *  on success: RES_SUCCESS
		 *  on error: RES_INVALID_ADDRESS -
		 *  			ip does not contain a character string
		 *  			representing a valid network address,
		 *  			Or we failed to convert the hostname to ip
		 *  			(depends on the *ip* input)
		 */
		int setSocketAddr(struct sockaddr_in& address,
							const std::string& host,
							const u_int16_t port) ;


		/****
		 * close current socket handle and set *socketfd* to be the new one
		 */
		void setSocket(socket_handle socketfd);

};


