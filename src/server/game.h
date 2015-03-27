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

#ifndef NETMAUMAU_SERVER_GAME_H
#define NETMAUMAU_SERVER_GAME_H

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#include "engine.h"
#include "stdplayer.h"

#ifndef _WIN32
#define TIMEFORMAT "%T - "
#else
#define TIMEFORMAT "%H:%M:%S - "
#endif

namespace NetMauMau {

namespace Server {

class GameConfig;

class Game {
	DISALLOW_COPY_AND_ASSIGN(Game)
public:

	typedef enum { ACCEPTED, REFUSED, ACCEPTED_READY, REFUSED_FULL } COLLECT_STATE;

	Game(GameConfig &gameConfig) throw(Common::Exception::SocketException);
	~Game();

	COLLECT_STATE collectPlayers(std::size_t minPlayers, Player::IPlayer *player);

	inline std::size_t getPlayerCount() const {
		return m_engine.getPlayerCount();
	}

	void removePlayer(const std::string &player);

	void start(bool ultimate = false) throw(Common::Exception::SocketException);
	void reset(bool playerLost) throw();
	void shutdown(const std::string &reason = std::string()) const throw();

	inline static long getServedGames() {
		return m_gameServed;
	}

private:
	bool addPlayer(Player::IPlayer *player);
	void gameReady();

private:
	static long m_gameServed;

	const GameConfig &m_cfg;
	Engine m_engine;
	std::vector<Player::StdPlayer *> m_aiPlayers;
	std::vector<Player::IPlayer *> m_players;
	long long int m_gameIndex;
};

}

}

#endif /* NETMAUMAU_SERVER_GAME_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
