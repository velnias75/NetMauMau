/*
 * Copyright 2014-2015 by Heiko Schäfer <heiko@rangun.de>
 *
 * This file is part of NetMauMau.
 *
 * NetMauMau is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * NetMauMau is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with NetMauMau.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @author Heiko Schäfer <heiko@rangun.de>
 */

#ifndef NETMAUMAU_ABSTRACTSOCKET_H
#define NETMAUMAU_ABSTRACTSOCKET_H

#include <stdint.h>

#ifdef _WIN32
#include <ws2tcpip.h>
#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif

#include "socketexception.h"

#define SOCKOPT_NONE      0x00
#define SOCKOPT_RCVTIMEO  0x01
#define SOCKOPT_SNDTIMEO  0x02
#define SOCKOPT_RCVBUF    0x04
#define SOCKOPT_SNDBUF    0x08
#define SOCKOPT_KEEPALIVE 0x10
#define SOCKOPT_LINGER    0x20
#define SOCKOPT_REUSEPORT 0x40
#define SOCKOPT_ALL       SOCKOPT_RCVTIMEO|SOCKOPT_SNDTIMEO|SOCKOPT_RCVBUF|SOCKOPT_SNDBUF| \
	SOCKOPT_KEEPALIVE|SOCKOPT_LINGER|SOCKOPT_REUSEPORT

namespace NetMauMau {

namespace Common {

class AbstractSocketImpl;

/**
 * @brief tbw
 */
class _EXPORT AbstractSocket {
	DISALLOW_COPY_AND_ASSIGN(AbstractSocket)
	friend class AbstractSocketImpl;
public:
	virtual ~AbstractSocket();

	virtual void connect(bool inetd = false) throw(Exception::SocketException);

	std::string read(SOCKET fd, std::size_t len = 1024) throw(Exception::SocketException);
	static void write(SOCKET fd, const std::string &msg) throw(Exception::SocketException);
	static void write(SOCKET *fds, std::size_t numfd,
					  const std::string &msg) throw(Exception::SocketException);

	_EXPORT static void setInterrupted(bool b = true);
	void setInterrupted(bool b, bool shut) const;

	_EXPORT static void checkSocket(SOCKET fd) throw(Exception::SocketException);

	static unsigned long getTotalReceivedBytes() _PURE;
	static unsigned long getTotalSentBytes() _PURE;

	static unsigned long getReceivedBytes() _PURE;
	static void resetReceivedBytes();

	static unsigned long getSentBytes() _PURE;
	static void resetSentBytes();

	static void shutdown(SOCKET sfd);

protected:
	explicit AbstractSocket(const char *server, uint16_t port);
	explicit AbstractSocket(const char *server, uint16_t port, unsigned char sockopts);
	explicit AbstractSocket(const char *server, uint16_t port, bool sockenv);

	virtual bool wire(SOCKET sockfd, const sockaddr *addr, socklen_t addrlen) const = 0;
	virtual std::string wireError(const std::string &err) const = 0;
	virtual void intercept() _CONST;

	std::size_t recv(void *buf, std::size_t len,
					 SOCKET fd) throw(Exception::SocketException) _NOUNUSED;
	static void send(const void *buf, std::size_t len, SOCKET fd) throw(Exception::SocketException);

	SOCKET getSocketFD() const _PURE;

private:
	AbstractSocketImpl *const _pimpl;
	volatile static bool m_interrupt;

	static unsigned long m_recvTotal;
	static unsigned long m_sentTotal;
	static unsigned long m_recv;
	static unsigned long m_sent;
};

}

}

#endif /* NETMAUMAU_ABSTRACTSOCKET_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
