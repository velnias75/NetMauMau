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

#ifndef NETMAUMAU_DB_SQLITEIMPL_H
#define NETMAUMAU_DB_SQLITEIMPL_H

#include <sqlite3.h>                    // for sqlite3_stmt, sqlite3

#include <ctime>                        // for time_t

#include "sqlite.h"                     // for SQLite, SQLite::SCORES, etc

namespace NetMauMau {

namespace Player {
class IPlayer;
}

namespace DB {

class SQLiteImpl {
	DISALLOW_COPY_AND_ASSIGN(SQLiteImpl)
public:
	explicit SQLiteImpl();
	~SQLiteImpl();

	static std::string getDBFilename();

	SQLite::SCORES getScores(SQLite::SCORE_TYPE type, std::size_t limit) const;

	long long int getServedGames() const;

	bool addAIPlayer(const NetMauMau::Player::IPlayer *ai) const;
	bool addPlayer(const Common::IConnection::INFO &info) const;
	bool logOutPlayer(const Common::IConnection::NAMESOCKFD &nsf) const;

	GAMEIDX newGame();

	bool gameEnded(GAMEIDX gameIndex) const;
	bool addPlayerToGame(GAMEIDX gid, const Common::IConnection::NAMESOCKFD &nsf) const;
	bool turn(GAMEIDX gameIndex, std::size_t turn) const;
	bool gamePlayStarted(GAMEIDX gameIndex) const;
	bool playerLost(GAMEIDX gameIndex, const Common::IConnection::NAMESOCKFD &nsf, time_t time,
					std::size_t points) const;
	bool playerWins(GAMEIDX gameIndex, const Common::IConnection::NAMESOCKFD &nsf) const;

private:

	bool prepareStatements();

	bool exec(const char *sql) const;

	inline bool exec(const std::string &sql) const {
		return exec(sql.c_str());
	}

private:
	sqlite3 *m_db;

	sqlite3_stmt *m_turnStmt;
	sqlite3_stmt *m_winStmt;

	sqlite3_stmt *m_scoreNormStmt;
	sqlite3_stmt *m_scoreNormLimitStmt;
	sqlite3_stmt *m_scoreAbsStmt;
	sqlite3_stmt *m_scoreAbsLimitStmt;
};

}

}

#endif /* NETMAUMAU_DB_SQLITEIMPL_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
