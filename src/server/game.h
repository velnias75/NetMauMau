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

#ifndef NETMAUMAU_GAME_H
#define NETMAUMAU_GAME_H

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#include "engine.h"
#include "stdplayer.h"

namespace NetMauMau {

namespace Event {
class IEventHandler;
}

namespace Server {

class Game {
	DISALLOW_COPY_AND_ASSIGN(Game)
public:

	typedef enum { ACCEPTED, REFUSED, ACCEPTED_READY, REFUSED_FULL } COLLECT_STATE;

	Game(Event::IEventHandler &evtHdlr, bool aiPlayer = false,
		 const std::string &aiName = "Computer");
	~Game();

	COLLECT_STATE collectPlayers(std::size_t minPlayers, Player::IPlayer *player);

	inline std::size_t getPlayerCount() const {
		return m_engine.getPlayerCount();
	}

	void start() throw(Common::Exception::SocketException);

private:
	bool addPlayer(Player::IPlayer *player);
	void reset() throw(Common::Exception::SocketException);

private:
	Engine m_engine;
	bool m_aiOpponent;
	Player::StdPlayer m_aiPlayer;
	std::vector<Player::IPlayer *> m_players;
};

}

}

#endif /* NETMAUMAU_GAME_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
