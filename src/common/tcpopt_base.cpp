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

#include <cerrno>
#include <cstring>

#ifdef HAVE_SYS_SOCKET_H
#include <sys/types.h>
#include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#include "tcpopt_base.h"

#include "errorstring.h"
#include "logger.h"

using namespace NetMauMau::Common;

TCPOptBase::TCPOptBase(SOCKET fd, int optname, const char *optStr) throw() :
	m_optStr(strdup(optStr)), m_fd(fd), m_optname(optname), m_val(0), m_ok(false) {

#ifdef HAVE_NETINET_IN_H

	if(m_fd != INVALID_SOCKET) {

		socklen_t slen = sizeof(int);

		if(getsockopt(m_fd, IPPROTO_TCP, m_optname, reinterpret_cast<char *>(&m_val),
					  &slen) != -1) {

			if(!(m_ok = (setOpt(1) != -1)) &&
					errno != EBADF) {
				logWarning("Couldn't set " << m_optStr << ": " << errorString(errno));
			}

		} else if(errno != EBADF) {
			logWarning("Couldn't get initial value of " << m_optStr << ": " << errorString(errno));
		}
	}

#endif
}

TCPOptBase::~TCPOptBase() throw() {

#ifdef HAVE_NETINET_IN_H

	if(m_ok && m_fd != INVALID_SOCKET) {

		if(setOpt(m_val) == -1 && errno != EBADF) {
			logWarning("Couldn't reset " << m_optStr << ": " << errorString(errno));
		}
	}

#endif

	std::free(m_optStr);
}

int TCPOptBase::setOpt(const int val) const throw() {
	return setsockopt(m_fd, IPPROTO_TCP, m_optname, reinterpret_cast<const char *>(&val),
					  sizeof(int));
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
