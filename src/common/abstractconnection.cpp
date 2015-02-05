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

#include <algorithm>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <cstring>
#include <cerrno>

#include "abstractconnection.h"
#include "abstractconnectionimpl.h"

#include "errorstring.h"

#ifdef _WIN32
#include <mswsock.h>
#endif

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY
#endif

namespace {

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct _isSocketFD :
	public std::binary_function<NetMauMau::Common::AbstractConnection::NAMESOCKFD, SOCKET, bool> {
	bool operator()(const NetMauMau::Common::AbstractConnection::NAMESOCKFD &nsd,
					SOCKET sockfd) const {
		return nsd.sockfd == sockfd;
	}
};
#pragma GCC diagnostic pop

}

using namespace NetMauMau::Common;

AbstractConnection::_info::_info() : sockfd(-1), name(), host(), port(0), maj(0), min(0) {}

AbstractConnection::_info::~_info() {}

AbstractConnection::_nameSockFD::_nameSockFD(const std::string &n, const std::string &pp,
		SOCKET sfd, uint32_t cv) : name(n), playerPic(pp), sockfd(sfd), clientVersion(cv) {}

AbstractConnection::_nameSockFD::_nameSockFD() : name(), playerPic(), sockfd(INVALID_SOCKET),
	clientVersion(0) {}

AbstractConnection::_nameSockFD::~_nameSockFD() {}

AbstractConnection::AbstractConnection(const char *server, uint16_t port) :
	AbstractSocket(server, port), _pimpl(new AbstractConnectionImpl()) {}

AbstractConnection::~AbstractConnection() {
	delete _pimpl;
}

void AbstractConnection::registerPlayer(const NAMESOCKFD &nfd) {
	_pimpl->m_registeredPlayers.push_back(nfd);
}

const AbstractConnection::PLAYERINFOS &AbstractConnection::getRegisteredPlayers() const {
	return _pimpl->m_registeredPlayers;
}

AbstractConnection::NAMESOCKFD AbstractConnection::getPlayerInfo(SOCKET sockfd) const {

	const PLAYERINFOS::const_iterator &f(std::find_if(_pimpl->m_registeredPlayers.begin(),
										 _pimpl->m_registeredPlayers.end(),
										 std::bind2nd(_isSocketFD(), sockfd)));

	return f != _pimpl->m_registeredPlayers.end() ? *f : NAMESOCKFD();
}

std::string AbstractConnection::getPlayerName(SOCKET sockfd) const {
	return getPlayerInfo(sockfd).name;
}

void AbstractConnection::removePlayer(SOCKET sockfd) {

	const PLAYERINFOS::iterator &f(std::find_if(_pimpl->m_registeredPlayers.begin(),
								   _pimpl->m_registeredPlayers.end(),
								   std::bind2nd(_isSocketFD(), sockfd)));

	if(f != _pimpl->m_registeredPlayers.end()) _pimpl->m_registeredPlayers.erase(f);
}

void AbstractConnection::addAIPlayers(const std::vector<std::string> &aiPlayers) {
	_pimpl->m_aiPlayers.insert(_pimpl->m_aiPlayers.end(), aiPlayers.begin(), aiPlayers.end());
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic push
void AbstractConnection::wait(long ms) throw(Exception::SocketException) {

	fd_set rfds;
	int sret = 1;
	timeval tv = { ms / 1000L, ms % 1000L };

	while(sret > 0) {

		FD_ZERO(&rfds);
		FD_SET(getSocketFD(), &rfds);

		if((sret = TEMP_FAILURE_RETRY(::select(getSocketFD() + 1, &rfds, NULL, NULL, &tv))) < 0) {
			throw Exception::SocketException(NetMauMau::Common::errorString(), getSocketFD(),
											 errno);
		} else if(sret > 0) {
			intercept();
#if _POSIX_C_SOURCE >= 200112L && defined(__linux)

			if(!(ms = tv.tv_sec * 1000L + tv.tv_usec)) sret = 0;

#endif
		}
	}
}
#pragma GCC diagnostic pop

const std::vector<std::string> &AbstractConnection::getAIPlayers() const {
	return _pimpl->m_aiPlayers;
}

void AbstractConnection::reset() throw() {

	for(PLAYERINFOS::const_iterator i(_pimpl->m_registeredPlayers.begin());
			i != _pimpl-> m_registeredPlayers.end(); ++i) {
		close(i->sockfd);
	}

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

	for(PLAYERINFOS::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) {
		if(i->sockfd != INVALID_SOCKET) return true;
	}

	return false;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
