#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#include "Socket.h"
#include "Packet.h"


Socket::Socket(socket_handle socketfd) : m_socketfd(socketfd)
{
	/* empty */
}

Socket::Socket() : m_socketfd(-1)
{
	/* empty */
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
		return -1;
	}

	return 0;
}

int Socket::bind(const std::string& ip, const u_int16_t port )
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	struct sockaddr_in localAddress;
	int ret = setSocketAddr(localAddress, ip, port);
	if (0 != ret)
	{
		return ret;
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

Socket* Socket::accept( Socket& ) const
{
	if (!isValid())
	{
		return NULL;
	}

	struct sockaddr_in cliAddress;
	unsigned int cliAddLen = sizeof(cliAddress);

	socket_handle clientFd = ::accept(m_socketfd,
									(sockaddr * )&cliAddress,
									&cliAddLen);
	if (-1 == clientFd)
	{
		return NULL;
	}

	return new Socket(clientFd);
}

int Socket::connect( const std::string& ip, const u_int16_t port )
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	struct sockaddr_in remoteAddress;
	int ret = setSocketAddr(remoteAddress, ip, port);
	if (0 != ret)
	{
		return ret;
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


int Socket::send( const std::string& str) const
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	int result = ::write(m_socketfd,
						(void*) str.c_str(),
						str.size());

	return result;
}

int Socket::send(const Packet& packet) const
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	int result = ::send(m_socketfd,
						(void*) packet.getData(),
						(unsigned int) packet.getSize(),
						0);

	return result;
}

int Socket::recv(Packet& packet, unsigned short size) const
{
	if (!isValid())
	{
		return RES_INVALID_SOCKET_ERROR;
	}
	if (size > RECEIVE_MAX_BUF_SIZE)
	{
		return RES_INVALID_SOCKET_ERROR;
	}

	char* buf = new char[size];
	int readSize = ::recv(m_socketfd,
							(void *)buf,
							size,
							0);

	if (0 <= size)
	{
		packet.writeForward(buf, readSize);
	}

	delete[] buf;

	return size;
}

bool Socket::isValid() const
{
	return (0 > m_socketfd);
}

void Socket::close()
{
	if (this->isValid())
	{
		::close(m_socketfd);
		m_socketfd = -1;
	}
}

int Socket::setSocketAddr(struct sockaddr_in& address,
					const std::string& ip,
					const u_int16_t port)
{
	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	int ret =  inet_aton(ip.c_str(), &(address.sin_addr));

	if (1 != ret)
	{
		return RES_INVALID_IP_ADDRESS;
	}

	return RES_SUCCESS;
}
