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
#endif

#include "tcpopt_cork.h"

#if defined(TCP_NOPUSH)
#define ONAME TCP_NOPUSH
#define ONSTR "TCP_NOPUSH"
#elif defined(TCP_CORK)
#define ONAME TCP_CORK
#define ONSTR "TCP_CORK"
#else
#undef ONAME
#undef ONSTR
#endif

#if !(defined(ONAME) && defined(ONSTR)) && defined(HAVE_NETINET_IN_H)
#warning "Neither TCP_NOPUSH nor TCP_CORK is available on this platform"
#endif

using namespace NetMauMau::Common;

#if defined(ONAME) && defined(ONSTR)
TCPOptCork::TCPOptCork(SOCKET fd) throw() : TCPOptBase(fd, ONAME, ONSTR) {}
#else
TCPOptCork::TCPOptCork(SOCKET) throw() : TCPOptBase(INVALID_SOCKET, 0, "") {}
#endif

TCPOptCork::~TCPOptCork() throw() {}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
