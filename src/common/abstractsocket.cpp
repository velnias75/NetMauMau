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
#include "abstractsocketimpl.h"

#include "errorstring.h"
#include "logger.h"

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

volatile bool AbstractSocket::m_interrupt = false;

AbstractSocket::AbstractSocket(const char *server, uint16_t port)
	: _pimpl(new AbstractSocketImpl(server, port)) {}

AbstractSocket::~AbstractSocket() {
	if(_pimpl->m_sfd != INVALID_SOCKET) {
		shutdown(_pimpl->m_sfd, SHUT_RDWR);
#ifndef _WIN32
		close(_pimpl->m_sfd);
#else
		closesocket(_pimpl->m_sfd);
#endif
	}

	delete _pimpl;
}

void AbstractSocket::connect(bool inetd) throw(Exception::SocketException) {

#ifdef _WIN32

	if(wsaErr != 0) throw Exception::SocketException("WSAStartup failed");

#endif

	if(!inetd) {

		struct addrinfo hints;
		struct addrinfo *result, *rp = NULL;
		char portS[256];
		int s;

		std::snprintf(portS, 255, "%u", _pimpl->m_port);
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

		if((s = getaddrinfo(_pimpl->m_server.empty() ? 0L : _pimpl->m_server.c_str(), portS, &hints,
							&result)) != 0) {
			throw Exception::SocketException(gai_strerror(s), _pimpl->m_sfd, errno);
		}

		for(rp = result; rp != NULL; rp = rp->ai_next) {

			_pimpl->m_sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

			if(_pimpl->m_sfd == INVALID_SOCKET) continue;

			if(wire(_pimpl->m_sfd, rp->ai_addr, rp->ai_addrlen)) {
				break;
			} else {
				_pimpl->m_wireError = NetMauMau::Common::errorString();
			}

#ifndef _WIN32
			close(_pimpl->m_sfd);
#else
			closesocket(_pimpl->m_sfd);
#endif
		}

		freeaddrinfo(result);

		if(rp == NULL) {
			_pimpl->m_sfd = INVALID_SOCKET;
			throw Exception::SocketException(wireError(_pimpl->m_wireError));
		}
	} else {
		_pimpl->m_sfd = fileno(stdin);
	}
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic push
std::size_t AbstractSocket::recv(void *buf, std::size_t len,
								 SOCKET fd) throw(Exception::SocketException) {

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

		unsigned char *ptr = static_cast<unsigned char *>(buf);

		while(len > 0) {

			ssize_t i = ::recv(fd, reinterpret_cast<char *>(ptr), len, 0);

			if(i < 0) throw Exception::SocketException(NetMauMau::Common::errorString(), fd, errno);

			ptr += i;

			if(static_cast<std::size_t>(i) < len) break;

			len -= static_cast<std::size_t>(i);
		}

		total = static_cast<std::size_t>(ptr - static_cast<unsigned char *>(buf));
	}

	return total;
}
#pragma GCC diagnostic pop

std::string AbstractSocket::read(SOCKET fd, std::size_t len) throw(Exception::SocketException) {

	std::string ret;
	char *rbuf = new(std::nothrow) char[len];

	if(!rbuf) throw Exception::SocketException(NetMauMau::Common::errorString(ENOMEM), fd, ENOMEM);

	const std::size_t rlen = recv(rbuf, len, fd);

	try {
		ret.reserve(rlen);
	} catch(const std::bad_alloc &) {
		delete [] rbuf;
		throw Exception::SocketException(NetMauMau::Common::errorString(ENOMEM), fd, ENOMEM);
	}

	ret.append(rbuf, rlen);

	delete [] rbuf;

	return ret;
}

void AbstractSocket::send(const void *buf, std::size_t len,
						  SOCKET fd) throw(Exception::SocketException) {

	checkSocket(fd);

	const char *ptr = static_cast<const char *>(buf);

	while(len > 0) {

		ssize_t i = ::send(fd, ptr, len, MSG_NOSIGNAL);

		if(i < 0) throw Exception::SocketException(NetMauMau::Common::errorString(), fd, errno);

		ptr += i;
		len -= static_cast<std::size_t>(i);
	}
}

void AbstractSocket::write(SOCKET *fds, std::size_t numfd,
						   const std::string &msg) throw(Exception::SocketException) {
	if(fds) {
		if(numfd > 1) {

			std::string v;

			const std::size_t partLen = msg.size() / numfd;
			const std::size_t partRem = msg.size() % numfd;

			std::size_t p = 0;

			for(; p < numfd; ++p) {

				const std::string::size_type start = p * partLen;

				v.reserve(partLen);
				v = msg.substr(start, start + partLen);

				for(std::size_t f = 0; f < numfd; ++f) {
					write(fds[f], v);
				}
			}

			const std::string::size_type start = p * partLen;

			v.reserve(partRem);
			v = msg.substr(start, start + partRem).append(1, 0);

			for(std::size_t f = 0; f < numfd; ++f) {
				write(fds[f], v);
			}

		} else if(numfd == 1) {
			write(fds[0], msg);
		}
	}
}

void AbstractSocket::write(SOCKET fd, const std::string &msg) throw(Exception::SocketException) {
	std::string v(msg);
	v.append(1, 0);
	send(v.c_str(), v.length(), fd);
}

void AbstractSocket::intercept() {}

void AbstractSocket::setInterrupted(bool b) {
	m_interrupt = b;
}

void AbstractSocket::setInterrupted(bool b, bool shut) const {

	setInterrupted(b);

	if(b && shut) {
		shutdown(getSocketFD(), SHUT_RDWR);
#ifndef _WIN32
		close(getSocketFD());
#else
		closesocket(getSocketFD());
#endif
	}
}

void AbstractSocket::checkSocket(SOCKET fd) throw(Exception::SocketException) {

	int ret = 0, error_code = 0;
	socklen_t slen = sizeof(error_code);

	if((ret = getsockopt(fd, SOL_SOCKET, SO_ERROR,
						 reinterpret_cast<char *>(&error_code), &slen)) == -1 || error_code) {
		throw Exception::SocketException(NetMauMau::Common::errorString(ret == -1 ? errno :
										 error_code), fd, ret == -1 ? errno : error_code);
	}
}

SOCKET AbstractSocket::getSocketFD() const {
	return _pimpl->m_sfd;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
