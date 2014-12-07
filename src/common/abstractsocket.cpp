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

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#include <cerrno>
#include <cstdio>
#include <cstring>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include "abstractsocket.h"

#ifdef _WIN32
#define MSG_NOSIGNAL 0x0000000
#define MSG_DONTWAIT 0x0000000

namespace {

int wsaErr;

void initialize() __attribute__((constructor));
void shut_down() __attribute__((destructor));

void initialize() {
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	wsaErr = WSAStartup(wVersionRequested, &wsaData);
}

void shut_down() {
	WSACleanup();
}

}
#endif

using namespace NetMauMau::Common;

bool AbstractSocket::m_interrupt = false;

AbstractSocket::AbstractSocket(const char *server, uint16_t port) : m_server(server ? server : ""),
	m_port(port), m_sfd(-1), m_wireError() {}

AbstractSocket::~AbstractSocket() {
	if(m_sfd != -1) {
		shutdown(m_sfd, SHUT_RDWR);
		close(m_sfd);
	}
}

void AbstractSocket::connect() throw(Exception::SocketException) {

#ifdef _WIN32

	if(wsaErr != 0) throw Exception::SocketException("WSAStartup failed");

#endif

	struct addrinfo hints;
	struct addrinfo *result, *rp = NULL;
	char portS[256];
	int s;

	std::snprintf(portS, 255, "%u", m_port);
	std::memset(&hints, 0, sizeof(struct addrinfo));

#ifdef _WIN32
	hints.ai_family = AF_INET;
#else
	hints.ai_family = AF_UNSPEC;
#endif
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	if((s = getaddrinfo(m_server.empty() ? 0L : m_server.c_str(), portS, &hints, &result)) != 0) {
		throw Exception::SocketException(gai_strerror(s), m_sfd, errno);
	}

	for(rp = result; rp != NULL; rp = rp->ai_next) {

		m_sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if(m_sfd == -1) continue;

		if(wire(m_sfd, rp->ai_addr, rp->ai_addrlen)) {
			break;
		} else {
			m_wireError = std::strerror(errno);
		}

		close(m_sfd);
	}

	freeaddrinfo(result);

	if(rp == NULL) {
		m_sfd = -1;
		throw Exception::SocketException(wireError(m_wireError));
	}

}

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic push
/**
 *
 * @param buf
 * @param len
 * @param fd
 * @param check
 * @return
 */
std::size_t AbstractSocket::recv(void *buf, std::size_t len,
								 int fd) throw(Exception::SocketException) {

	checkSocket(fd);

	std::size_t total = 0;
	fd_set rfds;
	int sret;

again:

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	FD_SET(getSocketFD(), &rfds);

	if(!m_interrupt && (sret = ::select(std::max(fd, getSocketFD()) + 1, &rfds, NULL, NULL,
										NULL)) > 0) {

		if(FD_ISSET(getSocketFD(), &rfds)) {

			intercept();

			if(!FD_ISSET(fd, &rfds)) goto again;

		}

		char *ptr = static_cast<char *>(buf);

		while(len > 0) {

			ssize_t i = ::recv(fd, ptr, len, 0);

			if(i < 0) throw Exception::SocketException(std::strerror(errno), fd, errno);

			ptr += i;

			if(i < static_cast<ssize_t>(len)) break;

			len -= i;
		}

		total = (ptr - static_cast<char *>(buf));
	}

	return total;
}
#pragma GCC diagnostic pop

std::string AbstractSocket::read(int fd, std::size_t len) throw(Exception::SocketException) {

	std::string ret;
	char *rbuf = new(std::nothrow) char[len];

	if(!rbuf) throw Exception::SocketException(std::strerror(ENOMEM), fd, ENOMEM);

	const std::size_t rlen = recv(rbuf, len, fd);

	try {
		ret.reserve(rlen);
	} catch(const std::bad_alloc &) {
		delete [] rbuf;
		throw Exception::SocketException(std::strerror(ENOMEM), fd, ENOMEM);
	}

	ret.append(rbuf, rlen);

	delete [] rbuf;

	return ret;
}

void AbstractSocket::send(const void *buf, std::size_t len,
						  int fd) throw(Exception::SocketException) {

	checkSocket(fd);

	const char *ptr = static_cast<const char *>(buf);

	while(len > 0) {

		ssize_t i = ::send(fd, ptr, len, MSG_NOSIGNAL);

		if(i < 0) throw Exception::SocketException(std::strerror(errno), fd, errno);

		ptr += i;
		len -= i;
	}
}

void AbstractSocket::write(int fd, const std::string &msg) throw(Exception::SocketException) {
	std::string v(msg);
	v.append(1, 0);
	send(v.c_str(), v.length(), fd);
}

void AbstractSocket::intercept() {}

void AbstractSocket::setInterrupted(bool b) {
	m_interrupt = b;
}

void AbstractSocket::checkSocket(int fd) throw(Exception::SocketException) {

	int ret = 0, error_code = 0;
	socklen_t slen = sizeof(error_code);

	if(getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&error_code), &slen) == -1 ||
			error_code) {
		throw Exception::SocketException(std::strerror(ret == -1 ? errno : error_code), fd,
										 ret == -1 ? errno : error_code);
	}
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
