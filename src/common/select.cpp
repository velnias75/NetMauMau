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

#ifdef _WIN32
#include <ws2tcpip.h>
#endif

#include <cerrno>
#include <csignal>

#ifdef HAVE_UNISTD_H
#include <unistd.h>                     // for TEMP_FAILURE_RETRY
#endif

#include "select.h"
#include "errorstring.h"

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY
#endif

using namespace NetMauMau::Common;

Select::Select() throw(Exception::SocketException) : SmartSingleton<Select>(), m_sigSet() {

#ifdef HAVE_PSELECT

	if(sigemptyset(&m_sigSet) && sigfillset(&m_sigSet) && (sigdelset(&m_sigSet, SIGINT) ||
			sigdelset(&m_sigSet, SIGTERM))) {
		throw Exception::SocketException(std::string("Couldn't prepare signal mask: ")
										 + NetMauMau::Common::errorString(errno));
	}

#endif
}

Select::~Select() throw() {}

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic push
int Select::perform(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
					timeval *timeout, bool blockall) const throw() {

#ifdef HAVE_PSELECT

	if(blockall) {
		sigaddset(&m_sigSet, SIGINT);
		sigaddset(&m_sigSet, SIGTERM);
	}

	struct timespec ts = {
		timeout ? timeout->tv_sec : 0L,
		timeout ? timeout->tv_usec * 1000L : 0L
	};

	const int ret = ::pselect(nfds, readfds, writefds, exceptfds, timeout ? &ts : NULL, &m_sigSet);

	if(blockall) {
		sigdelset(&m_sigSet, SIGINT);
		sigdelset(&m_sigSet, SIGTERM);
	}

	return ret;

#else
	_UNUSED(blockall);
	return ::select(nfds, readfds, writefds, exceptfds, timeout);
#endif
}
#pragma GCC diagnostic pop

template class NetMauMau::Common::SmartSingleton<NetMauMau::Common::Select>;

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
