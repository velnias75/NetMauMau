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

#include "engine.h"                     // for Engine
#include "observable.h"

#ifndef _WIN32
#define TIMEFORMAT "%T - "
#else
#define TIMEFORMAT "%H:%M:%S - "
#endif

namespace NetMauMau {

namespace Player {
class AbstractPlayer;
}

namespace Server {

class GameContext;

typedef enum { PLAYERADDED, PLAYERREMOVED, GAMESTARTED, GAMEENDED, READY } NOTIFYWHAT;

class Game : public Common::Observable<Game, NOTIFYWHAT> {
	DISALLOW_COPY_AND_ASSIGN(Game)
public:
	typedef enum { ACCEPTED, REFUSED, ACCEPTED_READY, REFUSED_FULL } COLLECT_STATE;

	explicit Game(GameContext &gameCtx) throw(Common::Exception::SocketException);
	~Game();

	COLLECT_STATE collectPlayers(std::size_t minPlayers, Player::IPlayer *player);

	inline std::size_t getPlayerCount() const {
		return m_engine.getPlayerCount();
	}

	void removePlayer(const std::string &player);

	void start(bool ultimate = false) throw(Common::Exception::SocketException);
	void reset(bool playerLost) throw();
	void shutdown(const std::string &reason = std::string()) const throw();

	static void setInterrupted() {
		m_interrupted = true;
	}

	inline static long getServedGames() {
		return m_gameServed;
	}

	inline bool isRunning() const {
		return m_running;
	}

	inline DB::GAMEIDX getGameIndex() const {
		return m_gameIndex;
	}

	inline Engine &getEngine() {
		return m_engine;
	}

private:
	bool addPlayer(Player::IPlayer *player);
	void gameReady();

private:
	static long m_gameServed;
	static bool m_interrupted;

	const GameContext &m_ctx;
	Engine m_engine;
	const DB::SQLite::SQLitePtr m_db;

	std::vector<Player::AbstractPlayer *> m_aiPlayers;
	std::vector<Player::IPlayer *> m_players;
	DB::GAMEIDX m_gameIndex;

	bool m_running;
};

}

}

#endif /* NETMAUMAU_SERVER_GAME_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
