#include <sys/types.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <new>
#include <string.h>

#include "Socket.h"
#include "Packet.h"

const std::string Socket::ANY_IP = "0.0.0.0";

Socket::Socket(socket_handle socketfd) : m_socketfd(socketfd)
{
	/* empty */
}

Socket::Socket() : m_socketfd(SOCKET_INVALID_DEFAULT)
{

}

Socket::~Socket()
{
	this->close();
}

int Socket::create()
{
	m_socketfd = socket(AF_INET, SOCK_STREAM, 0);

	if (0 > m_socketfd)
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	return RES_SUCCESS;
}

int Socket::bind(const std::string& ip, const u_int16_t port )
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	struct sockaddr_in localAddress;
	int ret = setSocketAddr(localAddress, ip, port);
	if (RES_INVALID_IP_ADDRESS == ret)
	{
		return RES_INVALID_ADDRESS;
	}

	int result = ::bind(m_socketfd, /* socket handle */
					(sockaddr*) &localAddress, /* address struct */
					sizeof(localAddress)); /* size of struct */

	return result;
}

int Socket::listen(const unsigned int maxConnections)
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	int result = ::listen(m_socketfd, maxConnections);

	return result;
}

int Socket::accept( Socket& clientSocket) const
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	struct sockaddr_in cliAddress;
	unsigned int cliAddLen = sizeof(cliAddress);

	socket_handle clientFd = ::accept(m_socketfd,
									(sockaddr * )&cliAddress,
									&cliAddLen);
	if (-1 == clientFd)
	{
		return RES_ACCEPT_INVALID_SOCKET_ERROR;
	}

	clientSocket.setSocket(clientFd);

	return RES_SUCCESS;
}

int Socket::connect( const std::string& host, const u_int16_t port )
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	struct sockaddr_in remoteAddress;

	int ret = setSocketAddr(remoteAddress, host, port);
	if (RES_SUCCESS != ret)
	{
		return ret;
	}

	return innerConnect(remoteAddress);
}


int Socket::innerConnect(struct sockaddr_in& remoteAddress)
{
	int result = ::connect(m_socketfd,
							(sockaddr *) &remoteAddress,
							sizeof(remoteAddress));

	return result;
}


int Socket::sendAll(const char* buf, long size) const
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	int total = 0;
	int bytesLeft = size;
	int readBytesCount;

	while(total < size)
	{
		readBytesCount = ::send(m_socketfd,
						(void*) (buf+total),
						(unsigned int) bytesLeft,
						0);

		if (-1 == readBytesCount)
		{
			return RES_SOCKET_GENERAL_FAILURE;
		}

		total += readBytesCount;
		bytesLeft -= readBytesCount;
	}

	return RES_SUCCESS;
}

int Socket::send(const Packet& packet) const
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	return sendAll(packet.getData(), packet.getSize());
}

int Socket::recv(Packet& packet, unsigned short size) const
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	char* buf = new (std::nothrow) char[size];
	if (0 == buf)
	{
		return RES_ALLOCATION_FAILED;
	}

	int readSize = ::recv(m_socketfd,
							(void *)buf,
							size,
							0);

	bool writeRes = packet.writeForward(buf, readSize);
	delete[] buf;

	if (false == writeRes)
	{
		return RES_ALLOCATION_FAILED;
	}

	packet.jumptoStart();
	return readSize;
}

int Socket::recvMessage(Packet& packet) const
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	length_field_t messageSize = 0;

	int readSize = ::recv(m_socketfd,
							(void *)&messageSize,
							sizeof(messageSize),
							0);

	if (readSize != sizeof(messageSize))
	{
		return RES_BAD_MESSAGE_FORMAT;
	}

	if (messageSize > MAX_PACKET_LENGTH)
	{
		return RES_PACKET_TOO_LONG;
	}

	long result = recv(packet, messageSize);

	if (result < 0)
	{
		/* result is an error */
		return result;
	}

	if (static_cast<length_field_t>(result) != messageSize)
	{
		return RES_FAILED_RECEIVING_ALL_DATA;
	}

	return RES_SUCCESS;
}

int Socket::sendMessage(const Packet& packet) const
{
	length_field_t packetSize = packet.getSize();

	if (packetSize > MAX_PACKET_LENGTH)
	{
		return RES_PACKET_TOO_LONG;
	}

	if (sizeof(packetSize) != sendAll((char*)&packetSize , sizeof(packetSize)))
	{
		return RES_FAILED_SENDING_ALL_DATA;
	}

	return send(packet);
}

bool Socket::isValid() const
{
	return (0 <= m_socketfd);
}

void Socket::close()
{
	if (this->isValid())
	{
		::close(m_socketfd);
		m_socketfd = SOCKET_INVALID_DEFAULT;
	}
}

int Socket::setSocketAddr(struct sockaddr_in& address,
					const std::string& host,
					const u_int16_t port)
{
	address.sin_family = AF_INET;
	/* htons has no error value */
	address.sin_port = htons(port);

	int ret = inet_aton(host.c_str(), &(address.sin_addr));
	if (1 != ret)
	{
		/* trying to convert host to ip address */
		struct hostent* hp = gethostbyname(host.c_str());
		if (NULL == hp)
		{
			return RES_INVALID_ADDRESS;
		}

		/* get only the first ip of the host */
		struct in_addr* ipAdd = (struct in_addr*) hp->h_addr_list[0];
		if (NULL == ipAdd)
		{
			return RES_INVALID_ADDRESS;
		}

		address.sin_addr = *ipAdd;
	}

	return RES_SUCCESS;
}


void Socket::setSocket(socket_handle socketfd)
{
	close();
	m_socketfd = socketfd;
}

void Socket::fromSocketResultToErrorString(int result,
										std::string& error)
{
	switch(result)
	{
		case RES_SUCCESS:
		{
			error.clear();
			break;
		}
		case RES_SOCKET_GENERAL_FAILURE:
		{
			error = strerror(errno);
			break;
		}

		case RES_BAD_MESSAGE_FORMAT:
		{
			error = "Failed to read packet size";
			break;
		}
		case RES_FAILED_RECEIVING_ALL_DATA:
		{
			error = "Failed to reading all data from socket";
			break;
		}
		case RES_FAILED_SENDING_ALL_DATA:
		{
			error = "Failed to sending all data";
			break;
		}
		case RES_ACCEPT_INVALID_SOCKET_ERROR:
		{
			error = "Failed to accept client connection";
			break;
		}
		case RES_INVALID_SOCKET_ERROR:
		{
			error = "The socket is invalid";
			break;
		}
		case RES_INVALID_ADDRESS:
		{
			error = "The address is neither hostname or ip";
			break;
		}
		case RES_INVALID_IP_ADDRESS:
		{
			error = "The address is not an ip";
			break;
		}
		case RES_ALLOCATION_FAILED:
		{
			error = "Failed to allocate necessary memory";
			break;
		}
		case RES_PACKET_TOO_LONG:
		{
			error = "Message size is too long";
			break;
		}
		default:
			error = "Unknown socket error";
	}
}
