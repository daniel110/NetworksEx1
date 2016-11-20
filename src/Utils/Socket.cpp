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

const std::string Socket::ANY_IP= "0.0.0.0";


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
		/* handle host name to ip */

		return RES_INVALID_ADDRESS;
	}

	int result =  ::bind(m_socketfd, /* socket handle */
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

int Socket::connect( const std::string& ip, const u_int16_t port )
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	struct sockaddr_in remoteAddress;
	int ret = setSocketAddr(remoteAddress, ip, port);

	if (RES_INVALID_IP_ADDRESS == ret)
	{
		/* handle host name to ip */

		return RES_INVALID_ADDRESS;
	}

	return innerConnect(remoteAddress);
}

int Socket::connect(struct in_addr& sinAddress, const u_int16_t port )
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	struct sockaddr_in remoteAddress;
	remoteAddress.sin_family = AF_INET;
	remoteAddress.sin_port = htons(port);
	remoteAddress.sin_addr = sinAddress;

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
						(void*) buf+total,
						(unsigned int) bytesLeft,
						0);

		if(-1 == readBytesCount)
		{
			break;
		}

		total += readBytesCount;
		bytesLeft -= readBytesCount;
	}

	return readBytesCount;
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

	if (size != readSize)
	{
		delete[] buf;
		return RES_FAILED_RECEIVING_ALL_DATA;

	}

	bool writeRes = packet.writeForward(buf, readSize);
	delete[] buf;

	if (false == writeRes)
	{
		return RES_ALLOCATION_FAILED;
	}

	packet.jumptoStart();
	return RES_SUCCESS;
}

int Socket::recvMessage(Packet& packet) const
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	int sizeOfPacketLen = sizeof(long);

	char* buf = new (std::nothrow) char[sizeOfPacketLen];
	if (0 == buf)
	{
		return RES_ALLOCATION_FAILED;
	}

	int readSize = ::recv(m_socketfd,
							(void *)buf,
							sizeOfPacketLen,
							0);

	if (readSize != sizeOfPacketLen)
	{
		delete[] buf;
		return RES_BAD_MESSAGE_FORMAT;
	}

	long messageSize;
	memcpy(&messageSize, buf, sizeOfPacketLen);

	delete[] buf;

	return recv(packet, messageSize);
}

int Socket::sendMessage(const Packet& packet) const
{
	long packetSize = packet.getSize();
	if (packetSize != sendAll((char*)&packetSize , sizeof(long)))
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
					const std::string& ip,
					const u_int16_t port)
{
	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	int ret = inet_aton(ip.c_str(), &(address.sin_addr));
	if (1 != ret)
	{
		return RES_INVALID_IP_ADDRESS;
	}

	return RES_SUCCESS;
}


void Socket::setSocket(socket_handle socketfd)
{
	close();
	m_socketfd = socketfd;
}
