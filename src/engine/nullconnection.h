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

#ifndef NETMAUMAU_NULLCONNECTION_H
#define NETMAUMAU_NULLCONNECTION_H

#include "iconnection.h"

namespace NetMauMau {

class NullConnection : public Common::IConnection {
	DISALLOW_COPY_AND_ASSIGN(NullConnection)
public:
	virtual ~NullConnection();

	static NullConnection &getInstance();

	virtual NAMESOCKFD getPlayerInfo(SOCKET sockfd) const;
	virtual std::string getPlayerName(SOCKET sockfd) const;
	virtual void removePlayer(SOCKET sockfd) _CONST;
	virtual void addAIPlayers(const std::vector<std::string> &aiPlayers) _CONST;

	virtual bool hasHumanPlayers() const _CONST;

	virtual void wait(long ms) throw(Common::Exception::SocketException) _CONST;

private:
	explicit NullConnection();
};

}

#endif /* NETMAUMAU_ENGINE_NULLCONNECTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
