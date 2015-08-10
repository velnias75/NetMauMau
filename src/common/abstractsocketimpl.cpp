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

#include <sys/types.h>

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>                      // for addrinfo, freeaddrinfo, etc
#endif

#ifdef _WIN32
#include <ws2tcpip.h>
#endif

#include <cstring>
#include <cstdio>
#include <cerrno>

#include "logger.h"
#include "errorstring.h"
#include "abstractsocket.h"
#include "abstractsocketimpl.h"

#define LINGER_TIMEOUT 2
#define RECVSNDTIMEO 0L
#define BUFMULT 2

using namespace NetMauMau::Common;

AbstractSocketImpl::AbstractSocketImpl(const char *server, uint16_t port, bool sockopt_env)
	: m_server(server ? server : ""), m_port(port), m_sfd(INVALID_SOCKET), m_wireError(),
	  m_sockoptEnv(sockopt_env), m_sopt(SOCKOPT_ALL) {}

AbstractSocketImpl::AbstractSocketImpl(const char *server, uint16_t port, unsigned char sockopts)
	: m_server(server ? server : ""), m_port(port), m_sfd(INVALID_SOCKET), m_wireError(),
	  m_sockoptEnv(false), m_sopt(sockopts) {}

AbstractSocketImpl::~AbstractSocketImpl() {
	if(m_sfd != INVALID_SOCKET) {
		AbstractSocket::shutdown(m_sfd);
	}
}

int AbstractSocketImpl::getAddrInfo(const char *server, uint16_t port, struct addrinfo **result,
									bool ipv4) {

	struct addrinfo hints;
	char portS[256];

	std::snprintf(portS, 255, "%u", port);
	std::memset(&hints, 0, sizeof(struct addrinfo));

#ifdef _WIN32
	hints.ai_family = AF_INET;
#else
	hints.ai_family = ipv4 ? AF_INET : AF_UNSPEC;
#endif
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

#if !defined(_WIN32) && defined(AI_V4MAPPED) && defined(AI_ADDRCONFIG)
	hints.ai_flags |= AI_V4MAPPED | AI_ADDRCONFIG;
#endif

	if(server) {
		hints.ai_flags |= AI_CANONNAME;
#if !defined(_WIN32) && defined(AI_CANONIDN)
		hints.ai_flags |= AI_CANONIDN;
#endif
	}

	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	return getaddrinfo(server, portS, &hints, result);
}

unsigned char AbstractSocketImpl::setSocketOptions(SOCKET fd, unsigned char what) {

	unsigned char ret = 0u;

	struct timeval timeout;
	socklen_t slen = sizeof(struct timeval);

	if((what & SOCKOPT_RCVTIMEO) &&  getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,
			reinterpret_cast<char *>(&timeout), &slen) != -1) {

		if(!(timeout.tv_sec != 0L && timeout.tv_usec == 0L) &&
				timeout.tv_sec != RECVSNDTIMEO) {

			logDebug("SO_RCVTIMEO: " << timeout.tv_sec << " sec; " << timeout.tv_usec
					 << " \u00b5sec, adjusting to " << RECVSNDTIMEO << " sec");

			timeout.tv_sec  = RECVSNDTIMEO;
			timeout.tv_usec = 0L;

			if(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,
						  reinterpret_cast<const char *>(&timeout),
						  sizeof(struct timeval)) == -1) {

				ret |= SOCKOPT_RCVTIMEO;

				logWarning("SOCKOPT_RCVTIMEO (" << static_cast<unsigned int>(SOCKOPT_RCVTIMEO)
						   << "): " << NetMauMau::Common::errorString(errno));
			}
		}
	}

	if((what & SOCKOPT_SNDTIMEO) && getsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,
			reinterpret_cast<char *>(&timeout), &slen) != -1) {

		if(!(timeout.tv_sec != 0L && timeout.tv_usec == 0L) &&
				timeout.tv_sec != RECVSNDTIMEO) {

			logDebug("SO_SNDTIMEO: " << timeout.tv_sec << " sec; " << timeout.tv_usec
					 << " \u00b5sec, adjusting to " << RECVSNDTIMEO << " sec");

			timeout.tv_sec  = RECVSNDTIMEO;
			timeout.tv_usec = 0L;

			if(setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,
						  reinterpret_cast<const char *>(&timeout),
						  sizeof(struct timeval)) == -1) {

				ret |= SOCKOPT_SNDTIMEO;

				logWarning("SOCKOPT_SNDTIMEO (" << static_cast<unsigned int>(SOCKOPT_SNDTIMEO)
						   << "): " << NetMauMau::Common::errorString(errno));
			}
		}
	}

	int bufSize = 0;
	slen = sizeof(int);

	if((what & SOCKOPT_SNDBUF) && getsockopt(fd, SOL_SOCKET, SO_SNDBUF,
			reinterpret_cast<char *>(&bufSize), &slen) != -1) {

		bufSize *= BUFMULT;

		if(setsockopt(fd, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char *>(&bufSize),
					  sizeof(int)) == -1) {

			ret |= SOCKOPT_SNDBUF;

			logWarning("SOCKOPT_SNDBUF (" << static_cast<unsigned int>(SOCKOPT_SNDBUF)
					   << "): " << NetMauMau::Common::errorString(errno));
		}
	}

	if((what & SOCKOPT_RCVBUF) && getsockopt(fd, SOL_SOCKET, SO_RCVBUF,
			reinterpret_cast<char *>(&bufSize), &slen) != -1) {

		bufSize *= BUFMULT;

		if(setsockopt(fd, SOL_SOCKET, SO_RCVBUF,
					  reinterpret_cast<const char *>(&bufSize), sizeof(int)) == -1) {

			ret |= SOCKOPT_RCVBUF;

			logWarning("SOCKOPT_RCVBUF (" << static_cast<unsigned int>(SOCKOPT_RCVBUF)
					   << "): " << NetMauMau::Common::errorString(errno));
		}
	}

	int keepAlive = 0;
	slen = sizeof(int);

	if((what & SOCKOPT_KEEPALIVE) &&  getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
			reinterpret_cast<char *>(&keepAlive), &slen) != -1) {

		if(!keepAlive) {

			keepAlive = 1;

			if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char *>(&keepAlive),
						  sizeof(int)) == -1) {

				ret |= SOCKOPT_KEEPALIVE;

				logWarning("SOCKOPT_KEEPALIVE (" << static_cast<unsigned int>(SOCKOPT_KEEPALIVE)
						   << "): " << NetMauMau::Common::errorString(errno));
			}
		}
	}

	struct linger linger;

	slen = sizeof(struct linger);

	if((what & SOCKOPT_LINGER) &&  getsockopt(fd, SOL_SOCKET, SO_LINGER,
			reinterpret_cast<char *>(&linger), &slen) != -1) {

		if(!linger.l_onoff) {

			linger.l_onoff  = 1;
			linger.l_linger = LINGER_TIMEOUT;

			if(setsockopt(fd, SOL_SOCKET, SO_LINGER,
						  reinterpret_cast<const char *>(&linger),
						  sizeof(struct linger)) == -1) {

				ret |= SOCKOPT_LINGER;

				logWarning("SOCKOPT_LINGER (" << static_cast<unsigned int>(SOCKOPT_LINGER)
						   << "): " << NetMauMau::Common::errorString(errno));
			}
		}
	}

#if defined(SO_REUSEPORT)

	int reuseport = 0;
	slen = sizeof(int);

	if((what & SOCKOPT_REUSEPORT) &&  getsockopt(fd, SOL_SOCKET, SO_REUSEPORT,
			reinterpret_cast<char *>(&reuseport), &slen) != -1) {

		if(!reuseport) {

			reuseport = 1;

			if(setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<const char *>(&reuseport),
						  sizeof(int)) == -1) {

				ret |= SOCKOPT_REUSEPORT;

				logWarning("SOCKOPT_REUSEPORT (" << static_cast<unsigned int>(SOCKOPT_REUSEPORT)
						   << "): " << NetMauMau::Common::errorString(errno));
			}
		}
	}

#endif

	return ret;
}

void AbstractSocketImpl::logErrSocketOptions(unsigned char what) {

	if(what & SOCKOPT_RCVTIMEO) logWarning("SOCKOPT_RCVTIMEO failed");

	if(what & SOCKOPT_SNDTIMEO) logWarning("SOCKOPT_SNDTIMEO failed");

	if(what & SOCKOPT_RCVBUF) logWarning("SOCKOPT_RCVBUF failed");

	if(what & SOCKOPT_SNDBUF) logWarning(" failed");

	if(what & SOCKOPT_KEEPALIVE) logWarning("SOCKOPT_KEEPALIVE failed");

	if(what & SOCKOPT_LINGER) logWarning("SOCKOPT_LINGER failed");

	if(what & SOCKOPT_REUSEPORT) logWarning("SOCKOPT_REUSEPORT failed");
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
