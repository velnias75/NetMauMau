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

#include "clientconnectionimpl.h"

#include "shutdownexception.h"
#include "timeoutexception.h"
#include "errorstring.h"

namespace {
NetMauMau::Client::Connection::BASE64RAII DEFAULTBASE64;
}

using namespace NetMauMau::Client;

ConnectionImpl::ConnectionImpl(Connection *piface, const std::string &pName,
							   const timeval *timeout, uint32_t clientVersion) : _piface(piface),
	m_pName(pName), m_timeout(timeout), m_clientVersion(clientVersion), m_base64(DEFAULTBASE64) {}

ConnectionImpl::ConnectionImpl(Connection *piface, const std::string &pName,
							   const timeval *timeout, uint32_t clientVersion,
							   Connection::BASE64RAII &base64) : _piface(piface),
	m_pName(pName), m_timeout(timeout), m_clientVersion(clientVersion), m_base64(base64) {}

ConnectionImpl::~ConnectionImpl() {}

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic push
bool ConnectionImpl::hello(uint16_t *maj, uint16_t *min)
throw(NetMauMau::Common::Exception::SocketException) {

	_piface->NetMauMau::Common::AbstractConnection::connect();

	fd_set rfds;
	int pret = -1;

	FD_ZERO(&rfds);
	FD_SET(_piface->getSocketFD(), &rfds);

	timeval tv = { m_timeout ? m_timeout->tv_sec : 0, m_timeout ? m_timeout->tv_usec : 0 };

	if(!m_timeout || (pret = ::select(_piface->getSocketFD() + 1, &rfds, NULL, NULL, &tv)) > 0) {

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
		throw Exception::ShutdownException("Server is shutting down", _piface->getSocketFD());
	} else {
		throw Exception::TimeoutException("Timeout while connecting to server",
										  _piface->getSocketFD());
	}
}
#pragma GCC diagnostic pop

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
