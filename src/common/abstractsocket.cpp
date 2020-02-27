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
#include "config.h"                     // for HAVE_NETDB_H, HAVE_UNISTD_H
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>                      // for addrinfo, freeaddrinfo, etc
#endif

#include <arpa/inet.h>

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include <sys/time.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>                     // for close, socklen_t, ssize_t
#endif

#ifdef ENABLE_THREADS
#include "mutex.h"
#else
#include "errorstring.h"
#endif

#include <cerrno>                       // for errno, ENOMEM
#include <cstdio>                       // for NULL, fileno, snprintf, etc
#include <vector>                       // for vector
#include <stdbool.h>

#include "abstractsocket.h"             // for AbstractSocket
#include "abstractsocketimpl.h"         // for AbstractSocketImpl
#include "logger.h"                     // for logWarning
#include "select.h"

#ifdef ENABLE_THREADS
#include "mutexlocker.h"
#endif

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY
#endif

namespace {

#ifdef ENABLE_THREADS
NetMauMau::Common::RWLock recvBytesLock;
NetMauMau::Common::RWLock sentBytesLock;
#endif

#ifdef _WIN32
#define MSG_NOSIGNAL 0x0000000
#define MSG_DONTWAIT 0x0000000

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
#endif
}

using namespace NetMauMau::Common;

volatile bool AbstractSocket::m_interrupt = false;

unsigned long AbstractSocket::m_recvTotal = 0L;
unsigned long AbstractSocket::m_sentTotal = 0L;
unsigned long AbstractSocket::m_recv = 0L;
unsigned long AbstractSocket::m_sent = 0L;

AbstractSocket::AbstractSocket(const char *server, uint16_t port)
	: _pimpl(new AbstractSocketImpl(server, port)) {}

AbstractSocket::AbstractSocket(const char *server, uint16_t port, unsigned char sockopt)
	: _pimpl(new AbstractSocketImpl(server, port, sockopt)) {

	logDebug("Socket options set via constructor to "
			 << NetMauMau::Common::Logger::hex << static_cast<unsigned int>(sockopt) << ":");
}

AbstractSocket::AbstractSocket(const char *server, uint16_t port, bool sockenv)
	: _pimpl(new AbstractSocketImpl(server, port, sockenv)) {}

AbstractSocket::~AbstractSocket() {
	delete _pimpl;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic push
void AbstractSocket::connect(bool inetd) throw(Exception::SocketException) {

#ifdef _WIN32

	if(wsaErr != 0) throw Exception::SocketException("WSAStartup failed");

#endif

	unsigned char sopt = _pimpl->getSockOpts();
	char *nmm_sockopts = std::getenv("NMM_SOCKOPTS");

	if(_pimpl->getSockoptEnv() && nmm_sockopts && *nmm_sockopts) {

		sopt = static_cast<unsigned char>(std::atoi(nmm_sockopts));

		logInfo("Socket options set via env variable \"NMM_SOCKOPTS\" to "
				<< NetMauMau::Common::Logger::hex << static_cast<unsigned int>(sopt) << ":");

		logInfo("SOCKOPT_RCVTIMEO:  " << (sopt & SOCKOPT_RCVTIMEO  ? "enabled" : "disabled"));
		logInfo("SOCKOPT_SNDTIMEO:  " << (sopt & SOCKOPT_SNDTIMEO  ? "enabled" : "disabled"));
		logInfo("SOCKOPT_RCVBUF:    " << (sopt & SOCKOPT_RCVBUF    ? "enabled" : "disabled"));
		logInfo("SOCKOPT_SNDBUF:    " << (sopt & SOCKOPT_SNDBUF    ? "enabled" : "disabled"));
		logInfo("SOCKOPT_KEEPALIVE: " << (sopt & SOCKOPT_KEEPALIVE ? "enabled" : "disabled"));
		logInfo("SOCKOPT_LINGER:    " << (sopt & SOCKOPT_LINGER    ? "enabled" : "disabled"));
		logInfo("SOCKOPT_REUSEPORT: " << (sopt & SOCKOPT_REUSEPORT ? "enabled" : "disabled"));
	}

	if(!inetd) {

		struct addrinfo *result, *rp = NULL;
		int s;

		
		if((s = _pimpl->getAddrInfo(_pimpl->m_server.empty() ? 0L : _pimpl->m_server.c_str(),
									_pimpl->m_port, &result)) != 0) {
			throw Exception::SocketException(NetMauMau::Common::errorString(s, true),
											_pimpl->m_sfd, errno);
		}

		for(rp = result; rp != NULL; rp = rp->ai_next) {

			
			if(rp->ai_family == AF_INET6) {
				
				char dst[INET6_ADDRSTRLEN + 1];
				
				struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)rp->ai_addr;
				void *addr = &(ipv6->sin6_addr);
			
				inet_ntop(AF_INET6, addr, dst, INET6_ADDRSTRLEN);
				logDebug("trying addr: " << dst);
				
			} else {
				char dst[INET_ADDRSTRLEN + 1];
			
				struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
				void *addr = &(ipv4->sin_addr);
			
				inet_ntop(AF_INET, addr, dst, INET_ADDRSTRLEN);
				logDebug("trying addr: " << dst);
			}
			
			_pimpl->m_sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

			if(_pimpl->m_sfd == INVALID_SOCKET) continue;
			
			// listen on any IPv6/IPv4
			if(_pimpl->m_server.empty() && rp->ai_family == AF_INET6) {
				int no = 0;     
				::setsockopt(_pimpl->m_sfd, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no)); 
				sockaddr_in6 *sad = (sockaddr_in6 *)rp->ai_addr;
				sad->sin6_addr = in6addr_any;
			}

			if(wire(_pimpl->m_sfd, rp->ai_addr, rp->ai_addrlen)) {
				break;
			} else {
				_pimpl->m_wireError = NetMauMau::Common::errorString();
			}

#ifndef _WIN32
			TEMP_FAILURE_RETRY(::close(_pimpl->m_sfd));
#else
			closesocket(_pimpl->m_sfd);
#endif
		}

		freeaddrinfo(result);

		unsigned char soErr = 0u;

		if(rp == NULL) {

			_pimpl->m_sfd = INVALID_SOCKET;
			throw Exception::SocketException(wireError(_pimpl->m_wireError));

		} else if((soErr = _pimpl->setSocketOptions(_pimpl->m_sfd, sopt))) {
			logWarning("Failed to set some socket options");
			_pimpl->logErrSocketOptions(soErr);
		}

	} else {

		_pimpl->m_sfd = fileno(stdin);

		unsigned char soErr = 0u;

		if((soErr = _pimpl->setSocketOptions(_pimpl->m_sfd, sopt))) {
			logWarning("Failed to set some socket options [on (x)inetd socket]");
			_pimpl->logErrSocketOptions(soErr);
		}
	}
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic push
std::size_t AbstractSocket::recv(void *buf, std::size_t len,
								 SOCKET fd) throw(Exception::SocketException) {

	if(!len) throw Exception::SocketException("Attempt to read zero length data", fd);

	checkSocket(fd);

	std::size_t total = 0;
	fd_set rfds;
	int sret;

again:

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	FD_SET(getSocketFD(), &rfds);

	if(!m_interrupt && (sret = NetMauMau::Common::Select::getInstance()->perform(std::max(fd,
							   getSocketFD()) + 1, &rfds, NULL, NULL, NULL)) > 0) {

		if(FD_ISSET(getSocketFD(), &rfds)) {

			intercept();

			if(!FD_ISSET(fd, &rfds)) goto again;

		}

		unsigned char *ptr = static_cast<unsigned char *>(buf);

		std::size_t origLen = len;

		bool peerClose = false;

		while(len > 0) {

			ssize_t i = TEMP_FAILURE_RETRY(::recv(fd, reinterpret_cast<char *>(ptr), len, 0));

			if(i < 0) throw Exception::SocketException(NetMauMau::Common::errorString(), fd, errno);

			if(i > 0) {

				ptr += i;

				if(static_cast<std::size_t>(i) < len) break;

				len -= static_cast<std::size_t>(i);

				if(len > origLen) throw Exception::SocketException("BUG: internal recv overflow",
							fd, errno);
			} else {
				peerClose = true;
				break;
			}
		}

		total = !peerClose ? static_cast<std::size_t>(ptr - static_cast<unsigned char *>(buf)) : 0u;
	}

#ifdef ENABLE_THREADS

	try {
		NetMauMau::Common::WriteLock l(recvBytesLock);
#endif
		m_recvTotal += total;
		m_recv += total;
#ifdef ENABLE_THREADS
	} catch(const NetMauMau::Common::MutexException &e) {
		throw NetMauMau::Common::Exception::SocketException(e.what());
	}

#endif
	return total;
}
#pragma GCC diagnostic pop

std::string AbstractSocket::read(SOCKET fd, std::size_t len) throw(Exception::SocketException) {

	std::string ret;

	try {

		std::vector<char> rbuf = std::vector<char>(len);
		const std::size_t rlen = recv(rbuf.data(), len, fd);

		if(rlen > 0 && rlen <= ret.max_size()) {

			try {
				ret.reserve(rlen);
			} catch(const std::bad_alloc &) {
				throw Exception::SocketException(NetMauMau::Common::errorString(ENOMEM), fd,
												 ENOMEM);
			}

		} else if(rlen == 0) {
#if defined(ESHUTDOWN)
			throw Exception::SocketException(NetMauMau::Common::errorString(ESHUTDOWN), fd,
											 ESHUTDOWN);
#else
			throw Exception::SocketException("Connection closed by peer", fd);
#endif
		}

		ret.append(rbuf.data(), rlen);

	} catch(const std::bad_alloc &) {
		throw Exception::SocketException(NetMauMau::Common::errorString(ENOMEM), fd, ENOMEM);
	}

	return ret;
}

void AbstractSocket::send(const void *buf, std::size_t len,
						  SOCKET fd) throw(Exception::SocketException) {

	if(!len) throw Exception::SocketException("Attempt to send zero length data", fd);

	checkSocket(fd);

	const char *ptr = static_cast<const char *>(buf);

	std::size_t origLen = len;

	while(len > 0) {

		ssize_t i = TEMP_FAILURE_RETRY(::send(fd, ptr, len, MSG_NOSIGNAL));

#ifdef _WIN32

		if(i == SOCKET_ERROR /*|| i == 0*/)
#else
		if(i < 0)
#endif
			throw Exception::SocketException(NetMauMau::Common::errorString(), fd, errno);

		if(i != 0) {

			ptr += i;
			len -= static_cast<std::size_t>(i);

			if(len > origLen) throw Exception::SocketException("BUG: internal send overflow", fd,
						errno);
		} else {
			logDebugN(NetMauMau::Common::nextLogBuf(), "::send return 0, check behaviour!");
		}
	}

#ifdef ENABLE_THREADS

	try {
		NetMauMau::Common::WriteLock l(sentBytesLock);
		m_sentTotal += origLen;
		m_sent += origLen;
	} catch(const NetMauMau::Common::MutexException &e) {
		throw NetMauMau::Common::Exception::SocketException(e.what());
	}

#endif
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

				if(partLen <= v.max_size()) v.reserve(partLen);

				v = msg.substr(start, start + partLen);

				for(std::size_t f = 0; f < numfd; ++f) {
					write(fds[f], v);
				}
			}

			const std::string::size_type start = p * partLen;

			if(partRem <= v.max_size()) v.reserve(partRem);

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

	if(b && shut) shutdown(getSocketFD());
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

void AbstractSocket::shutdown(SOCKET cfd) {
	::shutdown(cfd, SHUT_RDWR);
#ifndef _WIN32
	TEMP_FAILURE_RETRY(::close(cfd));
#else
	closesocket(cfd);
#endif
}

unsigned long AbstractSocket::getReceivedBytes() {
#ifdef ENABLE_THREADS
	NetMauMau::Common::ReadLock l(recvBytesLock);
#endif

	return m_recv;
}

void AbstractSocket::resetReceivedBytes() {
#ifdef ENABLE_THREADS
	NetMauMau::Common::WriteLock l(recvBytesLock);
#endif

	m_recv = 0L;
}

unsigned long AbstractSocket::getSentBytes() {
#ifdef ENABLE_THREADS
	NetMauMau::Common::ReadLock l(sentBytesLock);
#endif
	return m_sent;
}

void AbstractSocket::resetSentBytes() {
#ifdef ENABLE_THREADS
	NetMauMau::Common::WriteLock l(sentBytesLock);
#endif

	m_sent = 0L;
}

unsigned long AbstractSocket::getTotalReceivedBytes() {
#ifdef ENABLE_THREADS
	NetMauMau::Common::ReadLock l(recvBytesLock);
#endif

	return m_recvTotal;
}

unsigned long AbstractSocket::getTotalSentBytes() {
#ifdef ENABLE_THREADS
	NetMauMau::Common::ReadLock l(sentBytesLock);
#endif

	return m_sentTotal;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
