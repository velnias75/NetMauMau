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
#include "config.h"
#endif

#include <algorithm>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "abstractconnection.h"

#ifdef _WIN32
#include <mswsock.h>
#endif

namespace {

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct _isSocketFD :
	public std::binary_function < NetMauMau::Common::AbstractConnection::NAMESOCKFD, int, bool > {
	bool operator()(const NetMauMau::Common::AbstractConnection::NAMESOCKFD &nsd,
					int sockfd) const {
		return nsd.sockfd == sockfd;
	}
};

struct _isPlayer : public std::binary_function < NetMauMau::Common::AbstractConnection::NAMESOCKFD,
		std::string, bool > {
	bool operator()(const NetMauMau::Common::AbstractConnection::NAMESOCKFD &nsd,
					const std::string &player) const {
		return nsd.name == player;
	}
};
#pragma GCC diagnostic pop

}

using namespace NetMauMau::Common;

AbstractConnection::_info::_info() : sockfd(-1), name(), host(), port(0), maj(0), min(0) {}

AbstractConnection::_info::~_info() {}

AbstractConnection::AbstractConnection(const char *server, uint16_t port) :
	AbstractSocket(server, port), m_registeredPlayers() {}

AbstractConnection::~AbstractConnection() {}

void AbstractConnection::registerPlayer(const NAMESOCKFD &nfd) {
	m_registeredPlayers.push_back(nfd);
}

const AbstractConnection::PLAYERINFOS &AbstractConnection::getRegisteredPlayers() const {
	return m_registeredPlayers;
}

std::string AbstractConnection::getPlayerName(int sockfd) const {

	const PLAYERINFOS::const_iterator &f(std::find_if(m_registeredPlayers.begin(),
										 m_registeredPlayers.end(),
										 std::bind2nd(_isSocketFD(), sockfd)));

	return f != m_registeredPlayers.end() ? f->name : "";
}

void AbstractConnection::removePlayer(int sockfd) {

	const PLAYERINFOS::iterator &f(std::find_if(m_registeredPlayers.begin(),
								   m_registeredPlayers.end(),
								   std::bind2nd(_isSocketFD(), sockfd)));

	if(f != m_registeredPlayers.end()) m_registeredPlayers.erase(f);
}

void AbstractConnection::addAIPlayers(const std::vector<std::string> &aiPlayers) {
	m_aiPlayers.insert(m_aiPlayers.end(), aiPlayers.begin(), aiPlayers.end());
}

const std::vector<std::string> &AbstractConnection::getAIPlayers() const {
	return m_aiPlayers;
}

void AbstractConnection::reset() {

	for(PLAYERINFOS::const_iterator i(m_registeredPlayers.begin()); i != m_registeredPlayers.end();
			++i) {
		close(i->sockfd);
	}

	m_registeredPlayers.clear();
	m_aiPlayers.clear();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
