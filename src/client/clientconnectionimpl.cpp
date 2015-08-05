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
#include "config.h"                     // for PACKAGE_NAME
#endif

#include "clientconnectionimpl.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/time.h>                   // for timeval
#include <cerrno>                       // for errno, EINTR

#include "select.h"
#include "logger.h"
#include "signalblocker.h"
#include "errorstring.h"                // for errorString
#include "shutdownexception.h"          // for ShutdownException
#include "timeoutexception.h"           // for TimeoutException

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY
#endif

using namespace NetMauMau::Client;

ConnectionImpl::ConnectionImpl(Connection *piface, const std::string &pName,
							   const timeval *timeout, uint32_t clientVersion) : _piface(piface),
	m_pName(pName), m_timeout(timeout), m_clientVersion(clientVersion), m_buf() {}

#ifndef __clang__
#pragma GCC diagnostic ignored "-Wunsafe-loop-optimizations"
#pragma GCC diagnostic push
#endif
ConnectionImpl::~ConnectionImpl() {}
#ifndef __clang__
#pragma GCC diagnostic pop
#endif

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic push
bool ConnectionImpl::hello(uint16_t *maj, uint16_t *min)
throw(NetMauMau::Common::Exception::SocketException) {

	NetMauMau::Common::SignalBlocker sb;
	_UNUSED(sb);

	_piface->NetMauMau::Common::AbstractConnection::connect();

	fd_set rfds;
	int pret = -1;

	struct timeval tv = { m_timeout ? m_timeout->tv_sec : 0, m_timeout ? m_timeout->tv_usec : 0 };

retry:

	FD_ZERO(&rfds);
	FD_SET(_piface->getSocketFD(), &rfds);

	if(!m_timeout || /*TEMP_FAILURE_RETRY*/
			(pret = NetMauMau::Common::Select::getInstance()->perform(_piface->getSocketFD() + 1,
					&rfds, NULL, NULL, &tv, true)) >  0) {

		const std::string rHello(_piface->read(_piface->getSocketFD()));
		const std::string::size_type spc = rHello.find(' ');
		const std::string::size_type dot = rHello.find('.');

		const bool isServerHello = _piface->isHello(dot, spc);

		if(isServerHello && maj) *maj = _piface->getMajorFromHello(rHello, dot, spc);

		if(isServerHello && min) *min = _piface->getMinorFromHello(rHello, dot);

		return _piface->isValidHello(dot, spc, rHello, PACKAGE_NAME);

	} else if(pret == -1 && errno != EINTR) {
		throw NetMauMau::Common::Exception::SocketException(NetMauMau::Common::errorString(),
				_piface->getSocketFD(), errno);
	} else if(pret == -1 && errno == EINTR) {
		logDebug(__PRETTY_FUNCTION__ << ": " << NetMauMau::Common::errorString(errno));
		goto retry;
	} else {
		throw Exception::TimeoutException("Timeout while connecting to server",
										  _piface->getSocketFD());
	}
}
#pragma GCC diagnostic pop

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
