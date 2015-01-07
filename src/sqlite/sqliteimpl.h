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

#ifndef NETMAUMAU_SQLITEIMPL_H
#define NETMAUMAU_SQLITEIMPL_H

#include <string>

#ifndef _WIN32
#include <sqlite3.h>
#endif

#include "sqlite.h"

#if _WIN32
#define WSTATIC_S static
#define WSTATIC_E
#else
#define WSTATIC_S
#define WSTATIC_E const
#endif

#ifndef _WCONST
#if _WIN32
#define _WCONST _CONST
#else
#define _WCONST
#endif
#endif

namespace NetMauMau {

namespace Player {
class IPlayer;
}

namespace DB {

class SQLiteImpl {
	DISALLOW_COPY_AND_ASSIGN(SQLiteImpl)
public:
	SQLiteImpl();
	~SQLiteImpl() _WCONST;

	static std::string getDBFilename();

	SQLite::SCORES getScores(SQLite::SCORE_TYPE type, std::size_t limit) const;

	WSTATIC_S bool addAIPlayer(const NetMauMau::Player::IPlayer *ai) WSTATIC_E;
	bool addPlayer(const Common::AbstractConnection::INFO &info) const;
	bool logOutPlayer(const Common::AbstractConnection::NAMESOCKFD &nsf) const;

	WSTATIC_S long long int newGame() WSTATIC_E _WCONST;

	WSTATIC_S bool gameEnded(long long int gameIndex) WSTATIC_E;
	bool addPlayerToGame(long long int gid,
						 const Common::AbstractConnection::NAMESOCKFD &nsf) const;
	WSTATIC_S bool turn(long long int gameIndex, std::size_t turn) WSTATIC_E;
	WSTATIC_S bool gamePlayStarted(long long int gameIndex) WSTATIC_E;
	bool playerLost(long long int gameIndex, const Common::AbstractConnection::NAMESOCKFD &nsf,
					time_t time, std::size_t points) const;
	bool playerWins(long long int gameIndex, const Common::AbstractConnection::NAMESOCKFD &nsf)
	const;

private:
	WSTATIC_S bool exec(const std::string &sql) WSTATIC_E _WCONST;

private:
#ifndef _WIN32
	sqlite3 *m_db;
#else
	char *m_db;
#endif
};

}

}

#endif /* NETMAUMAU_SQLITEIMPL_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
