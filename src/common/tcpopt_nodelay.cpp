/*
 * Copyright 2015 by Heiko Schäfer <heiko@rangun.de>
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

#ifdef HAVE_NETINET_IN_H
#include <sys/types.h>
#include <netinet/tcp.h>
#elif defined(_WIN32)
#include <winsock2.h>
#endif

#include "tcpopt_nodelay.h"

using namespace NetMauMau::Common;

#if defined(HAVE_NETINET_IN_H) || defined(_WIN32)
TCPOptNodelay::TCPOptNodelay(SOCKET fd) throw() : TCPOptBase(fd, TCP_NODELAY, "TCP_NODELAY") {}
#else
TCPOptNodelay::TCPOptNodelay(SOCKET) throw() : TCPOptBase(INVALID_SOCKET, 0, "TCP_NODELAY") {}
#endif

TCPOptNodelay::~TCPOptNodelay() throw() {}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
