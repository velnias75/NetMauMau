/*
 * Copyright 2014 by Heiko Schäfer <heiko@rangun.de>
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

#ifndef NETMAUMAU_SOCKET_H
#define NETMAUMAU_SOCKET_H

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

namespace NetMauMau {

namespace Common {

/**
 * @brief tbw
 */
class _EXPORT AbstractSocket {
	DISALLOW_COPY_AND_ASSIGN(AbstractSocket)
public:
	virtual ~AbstractSocket();

	virtual void connect() throw(Exception::SocketException);

	std::string read(SOCKET fd, std::size_t len = 1024) throw(Exception::SocketException);
	static void write(SOCKET fd, const std::string &msg) throw(Exception::SocketException);
	static void write(SOCKET *fds, std::size_t numfd,
					  const std::string &msg) throw(Exception::SocketException);

	_EXPORT static void setInterrupted(bool b = true);
	void setInterrupted(bool b, bool shut);

	_EXPORT static void checkSocket(SOCKET fd) throw(Exception::SocketException);

protected:
	AbstractSocket(const char *server, uint16_t port);

	virtual bool wire(SOCKET sockfd, const sockaddr *addr, socklen_t addrlen) const = 0;
	virtual std::string wireError(const std::string &err) const = 0;
	virtual void intercept() _CONST;

	std::size_t recv(void *buf, std::size_t len, SOCKET fd) throw(Exception::SocketException);
	static void send(const void *buf, std::size_t len, SOCKET fd) throw(Exception::SocketException);

	inline SOCKET getSocketFD() const {
		return m_sfd;
	}

private:
	volatile static bool m_interrupt;
	const std::string m_server;
	const uint16_t m_port;
	SOCKET m_sfd;
	std::string m_wireError;
};

}

}

#endif /* NETMAUMAU_SOCKET_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
