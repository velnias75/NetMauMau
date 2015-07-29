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
#include "config.h"                     // for HAVE_UNISTD_H, PACKAGE_NAME
#endif

#ifndef _WIN32
#include <sys/select.h>                 // for select, FD_SET, FD_ZERO, etc
#endif

#include <sys/time.h>                   // for timeval

#ifdef HAVE_UNISTD_H
#include <unistd.h>                     // for TEMP_FAILURE_RETRY
#endif

#include <algorithm>                    // for find_if, find, for_each
#include <cerrno>                       // for errno
#include <cstring>                      // for NULL, strlen

#include "abstractconnectionimpl.h"     // for AbstractConnectionImpl
#include "errorstring.h"                // for errorString

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY
#endif

namespace {
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct socketCloser : std::unary_function<NetMauMau::Common::IConnection::NAMESOCKFD, void> {
	inline result_type operator()(const argument_type &nsf) const {
		NetMauMau::Common::AbstractSocket::shutdown(nsf.sockfd);
	}
};
#pragma GCC diagnostic pop
}

using namespace NetMauMau::Common;

IConnection::_info::_info() : sockfd(INVALID_SOCKET), name(), host(), port(0), maj(0), min(0) {}

IConnection::_info::~_info() {}

IConnection::_nameSockFD::_nameSockFD(const std::string &n, const std::string &pp, SOCKET sfd,
									  uint32_t cv) : name(n), playerPic(pp), sockfd(sfd),
	clientVersion(cv) {}

IConnection::_nameSockFD::_nameSockFD() : name(), playerPic(), sockfd(INVALID_SOCKET),
	clientVersion(0) {}

IConnection::_nameSockFD::~_nameSockFD() {}

AbstractConnection::AbstractConnection(const char *server, uint16_t port) :
	AbstractSocket(server, port), _pimpl(new AbstractConnectionImpl()) {}

AbstractConnection::~AbstractConnection() {
	delete _pimpl;
}

bool AbstractConnection::isNull() const throw() {
	return false;
}

bool AbstractConnection::registerPlayer(const NAMESOCKFD &nfd, const PLAYERNAMES &ai) {
	return _pimpl->registerPlayer(nfd, ai);
}

const IConnection::PLAYERINFOS &AbstractConnection::getRegisteredPlayers() const {
	return _pimpl->m_registeredPlayers;
}

IConnection::NAMESOCKFD AbstractConnection::getPlayerInfo(SOCKET sockfd) const {
	return _pimpl->getPlayerInfo(sockfd);
}

IConnection::PLAYERNAMES::value_type AbstractConnection::getPlayerName(SOCKET sockfd) const {
	return _pimpl->getPlayerName(sockfd);
}

void AbstractConnection::removePlayer(const IConnection::INFO &info) {
	_pimpl->removePlayer(info);
}

void AbstractConnection::removePlayer(SOCKET sockfd) {
	_pimpl->removePlayer(sockfd);
}

void AbstractConnection::addAIPlayers(const PLAYERNAMES &aiPlayers) {
	_pimpl->m_aiPlayers.insert(_pimpl->m_aiPlayers.end(), aiPlayers.begin(), aiPlayers.end());
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic push
void AbstractConnection::wait(long ms) throw(Exception::SocketException) {

	if(ms == 0L) return;

	fd_set rfds;
	int sret = 1;
	timeval tv = { ms / 0xF4240L, ms % 0xF4240L };

	while(sret > 0) {

		FD_ZERO(&rfds);
		FD_SET(getSocketFD(), &rfds);

		if((sret = TEMP_FAILURE_RETRY(::select(getSocketFD() + 1, &rfds, NULL, NULL, &tv))) < 0) {
			throw Exception::SocketException(NetMauMau::Common::errorString(), getSocketFD(),
											 errno);
		} else if(sret > 0) {
			intercept();
#if _POSIX_C_SOURCE >= 200112L && defined(__linux)

			if(!(ms = tv.tv_sec * 0xF4240L + tv.tv_usec)) sret = 0;

#endif
		}
	}
}
#pragma GCC diagnostic pop

const IConnection::PLAYERNAMES &AbstractConnection::getAIPlayers() const {
	return _pimpl->m_aiPlayers;
}

void AbstractConnection::reset() throw() {

	std::for_each(_pimpl->m_registeredPlayers.begin(), _pimpl->m_registeredPlayers.end(),
				  socketCloser());

	_pimpl->m_registeredPlayers.clear();
	_pimpl->m_aiPlayers.clear();
}

bool AbstractConnection::isHello(std::string::size_type dot, std::string::size_type spc) {
	return spc != std::string::npos && dot != std::string::npos && spc < dot;
}

bool AbstractConnection::isValidHello(std::string::size_type dot, std::string::size_type spc,
									  const std::string &rHello, const std::string &expHello) {
	return isHello(dot, spc) && rHello.substr(0, std::strlen(PACKAGE_NAME)) == expHello;
}

uint16_t AbstractConnection::getMajorFromHello(const std::string &hello,
		std::string::size_type dot, std::string::size_type spc) {
	return (uint16_t)std::strtoul(hello.substr(spc + 1, dot).c_str(), NULL, 10);
}

uint16_t AbstractConnection::getMinorFromHello(const std::string &hello,
		std::string::size_type dot) {
	return (uint16_t)std::strtoul(hello.substr(dot + 1).c_str(), NULL, 10);
}

bool AbstractConnection::hasHumanPlayers() const {

	const PLAYERINFOS &pi(getRegisteredPlayers());

	return std::find_if(pi.begin(), pi.end(), std::bind2nd(std::not_equal_to<SOCKET>(),
						INVALID_SOCKET)) != pi.end();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
