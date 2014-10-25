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

#ifndef NETMAUMAU_ABSTRACTCONNECTION_H
#define NETMAUMAU_ABSTRACTCONNECTION_H

#include <map>
#include <vector>

#include "abstractsocket.h"

#ifdef _WIN32
#include <mswsock.h>
#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif
#endif

namespace NetMauMau {

namespace Common {

class _EXPORT AbstractConnection : public AbstractSocket {
	DISALLOW_COPY_AND_ASSIGN(AbstractConnection)
public:
	typedef struct _EXPORT _info {
		_info();
		int sockfd;
		std::string  name;
		std::string  host;
		uint16_t     port;
		uint16_t maj, min;
	} INFO;

	typedef struct {
		std::string name;
		int sockfd;
	} NAMESOCKFD;

	typedef std::vector<NAMESOCKFD> PLAYERINFOS;
	typedef std::map<std::string, std::string> CAPABILITIES;

	virtual ~AbstractConnection();

	std::string getPlayerName(int sockfd) const;
	void removePlayer(int sockfd);

	void reset();


protected:
	AbstractConnection(const char *server, uint16_t port);

	void registerPlayer(const NAMESOCKFD &nfd);
	const PLAYERINFOS &getRegisteredPlayers() const _CONST;

private:
	PLAYERINFOS m_registeredPlayers;
};

}

}

#endif /* NETMAUMAU_ABSTRACTCONNECTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
