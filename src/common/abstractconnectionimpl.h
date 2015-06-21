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

#ifndef NETMAUMAU_ABSTRACTCONNECTIONIMPL_H
#define NETMAUMAU_ABSTRACTCONNECTIONIMPL_H

#include "abstractconnection.h"         // for AbstractConnection

namespace NetMauMau {

namespace Common {

class AbstractConnectionImpl {
	DISALLOW_COPY_AND_ASSIGN(AbstractConnectionImpl)
public:
	explicit AbstractConnectionImpl();
	~AbstractConnectionImpl();

	IConnection::NAMESOCKFD getPlayerInfo(SOCKET sockfd) const;

	inline IConnection::PLAYERNAMES::value_type getPlayerName(SOCKET sockfd) const {
		return getPlayerInfo(sockfd).name;
	}

	bool registerPlayer(const IConnection::NAMESOCKFD &nfd, const IConnection::PLAYERNAMES &ai);

	inline void removePlayer(SOCKET sockfd) {
		removePlayer(m_registeredPlayers, findBySocket(sockfd));
	}

	void removePlayer(const IConnection::INFO &info);

private:
	IConnection::PLAYERINFOS::iterator findBySocket(SOCKET sockfd);
	IConnection::PLAYERINFOS::const_iterator findBySocket(SOCKET sockfd) const;

	inline void removePlayer(IConnection::PLAYERINFOS &pi,
							 const IConnection::PLAYERINFOS::iterator &i) {
		if(i != pi.end()) pi.erase(i);
	}

public:
	AbstractConnection::PLAYERINFOS m_registeredPlayers;
	AbstractConnection::PLAYERNAMES m_aiPlayers;
};

}

}

#endif /* NETMAUMAU_ABSTRACTCONNECTIONIMPL_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
