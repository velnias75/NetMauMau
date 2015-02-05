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
#endif

/**
 * @defgroup util Utilities
 *
 * @def MAKE_VERSION(maj, min)
 * @hideinitializer
 * @ingroup util
 *
 * Computes the corresponding version number integer
 *
 * @param maj the major of the version number
 * @param min the minor of the version number
 *
 * @since 0.8
 */
#define MAKE_VERSION(maj, min) static_cast<uint32_t>((static_cast<uint16_t>(maj) << 16u) | \
		static_cast<uint16_t>(min))

namespace NetMauMau {

namespace Common {

class AbstractConnectionImpl;

/**
 * @brief Abstract connection class
 *
 * Handles all registred users.
 */
class _EXPORT AbstractConnection : public AbstractSocket {
	DISALLOW_COPY_AND_ASSIGN(AbstractConnection)
	friend class AbstractConnectionImpl;
public:
	typedef struct _EXPORT _info {
		_info();
		~_info();
		SOCKET sockfd;
		std::string  name;
		std::string  host;
		uint16_t     port;
		uint16_t maj, min;
	} INFO;

	typedef struct _nameSockFD {
		_nameSockFD();
		_nameSockFD(const std::string &name, const std::string &playerPic, SOCKET sockfd,
					uint32_t clientVersion);
		~_nameSockFD();
		std::string name;
		mutable std::string playerPic;
		SOCKET sockfd;
		uint32_t clientVersion;
	} NAMESOCKFD;

	typedef std::vector<NAMESOCKFD> PLAYERINFOS;

	/**
	 * @brief Key/value map of the server capabilities
	 *
	 * Key            | Value
	 * -------------- | -----
	 * ACEROUND       | `false` if no *ace rounds* are enabled, else `A`, `Q` or `K` for the *rank*
	 * AI_NAME        | name of the first AI player
	 * AI_OPPONENT    | `true` if the AI opponent is enabled
	 * CUR_PLAYERS    | amount of current players joined
	 * DIRCHANGE      | `true` if direction changes are allowed, `false` otherwise
	 * HAVE_SCORES    | `true` if the server can provide scores, `false` otherwise
	 * MAX_PLAYERS    | amount of players needed to start the game
	 * MIN_VERSION    | minimum version of client that can connect
	 * SERVER_VERSION | version of the server
	 * ULTIMATE       | `true` if running in ultimate mode, `false` otherwise
	 *
	 */
	typedef std::map<std::string, std::string> CAPABILITIES;

	virtual ~AbstractConnection();

	NAMESOCKFD  getPlayerInfo(SOCKET sockfd) const;
	std::string getPlayerName(SOCKET sockfd) const;
	virtual void removePlayer(SOCKET sockfd);

	void addAIPlayers(const std::vector<std::string> &aiPlayers);
	void wait(long ms) throw(Exception::SocketException);

	bool hasHumanPlayers() const _PURE;

	virtual void reset() throw();

protected:
	AbstractConnection(const char *server, uint16_t port);

	void registerPlayer(const NAMESOCKFD &nfd);
	const PLAYERINFOS &getRegisteredPlayers() const _PURE;
	const std::vector<std::string> &getAIPlayers() const _PURE;

	static bool isHello(std::string::size_type dot, std::string::size_type spc) _CONST;
	static bool isValidHello(std::string::size_type dot, std::string::size_type spc,
							 const std::string &rHello, const std::string &expHello);

	static uint16_t getMajorFromHello(const std::string &hello, std::string::size_type dot,
									  std::string::size_type spc);
	static uint16_t getMinorFromHello(const std::string &hello, std::string::size_type dot);

private:
	AbstractConnectionImpl *const _pimpl;
};

}

}

#endif /* NETMAUMAU_ABSTRACTCONNECTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
