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

/**
 * @file
 * @author Heiko Schäfer <heiko@rangun.de>
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

/**
 * @brief Abstract connection class
 *
 * Handles all registred users.
 */
class _EXPORT AbstractConnection : public AbstractSocket {
	DISALLOW_COPY_AND_ASSIGN(AbstractConnection)
public:
	typedef struct _EXPORT _info {
		_info();
		~_info();
		int sockfd;
		std::string  name;
		std::string  host;
		uint16_t     port;
		uint16_t maj, min;
	} INFO;

	typedef struct {
		std::string name;
		std::string playerPic;
		int sockfd;
		uint32_t clientVersion;
	} NAMESOCKFD;

	typedef std::vector<NAMESOCKFD> PLAYERINFOS;

	/**
	 * @brief Key/value map of the server capabilities
	 */
	typedef std::map<std::string, std::string> CAPABILITIES;

	virtual ~AbstractConnection();

	std::string getPlayerName(int sockfd) const;
	void removePlayer(int sockfd);

	void addAIPlayers(const std::vector<std::string> &aiPlayers);
	void wait(long ms) throw(Exception::SocketException);

	void reset() throw();

protected:
	AbstractConnection(const char *server, uint16_t port);

	void registerPlayer(const NAMESOCKFD &nfd);
	const PLAYERINFOS &getRegisteredPlayers() const _CONST;
	const std::vector<std::string> &getAIPlayers() const _CONST;

	static bool isHello(std::string::size_type dot, std::string::size_type spc) _CONST;
	static bool isValidHello(std::string::size_type dot, std::string::size_type spc,
							 const std::string &rHello, const std::string &expHello);

	static uint16_t getMajorFromHello(const std::string &hello, std::string::size_type dot,
									  std::string::size_type spc);
	static uint16_t getMinorFromHello(const std::string &hello, std::string::size_type dot);

private:
	PLAYERINFOS m_registeredPlayers;
	std::vector<std::string> m_aiPlayers;
};

}

}

#endif /* NETMAUMAU_ABSTRACTCONNECTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
