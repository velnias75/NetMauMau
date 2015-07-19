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

#include "abstractsocketimpl.h"

using namespace NetMauMau::Common;

AbstractSocketImpl::AbstractSocketImpl(const char *server, uint16_t port)
	: m_server(server ? server : ""), m_port(port), m_sfd(INVALID_SOCKET), m_wireError() {}

AbstractSocketImpl::~AbstractSocketImpl() {}

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

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
