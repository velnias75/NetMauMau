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

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#include <cstdio>
#include <cstdlib>
#include <sstream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "sqliteimpl.h"

#include "iplayer.h"
#include "logger.h"

namespace {

int gamesCountCallback(void *arg, int cols, char **col_text, char **) {

	if(arg && cols == 1) {

		long long int *res = static_cast<long long int *>(arg);

		*res = std::strtoll(col_text[0], NULL, 10);

		return 0;
	}

	return -1;
}

const char *SCHEMA =
	"PRAGMA journal_mode=OFF;" \
	"PRAGMA synchronous=NORMAL;" \
	"CREATE TABLE IF NOT EXISTS \"meta\" (" \
	"dbver INTEGER UNIQUE," \
	"date INTEGER );"\
	"CREATE TABLE IF NOT EXISTS \"players\" (" \
	"id INTEGER PRIMARY KEY AUTOINCREMENT," \
	"name TEXT NOT NULL UNIQUE );" \
	"CREATE INDEX IF NOT EXISTS \"playername_index\" ON players (name ASC);" \
	"CREATE TABLE IF NOT EXISTS \"clients\" (" \
	"id INTEGER PRIMARY KEY AUTOINCREMENT," \
	"sock INTEGER NOT NULL," \
	"host TEXT NOT NULL," \
	"port INTEGER NOT NULL," \
	"version INTEGER NOT NULL," \
	"log_in INTEGER NOT NULL," \
	"log_out INTEGER," \
	"playerid INTEGER NOT NULL," \
	"gameid INTEGER );" \
	"CREATE INDEX IF NOT EXISTS \"player_index\" ON clients (playerid ASC);" \
	"CREATE INDEX IF NOT EXISTS \"game_index\" ON clients (gameid ASC);" \
	"CREATE INDEX IF NOT EXISTS \"player_game_index\" ON clients (playerid ASC, gameid ASC);" \
	"CREATE TABLE IF NOT EXISTS \"games\" (" \
	"id INTEGER PRIMARY KEY AUTOINCREMENT," \
	"server_start INTEGER NOT NULL," \
	"game_start INTEGER," \
	"end INTEGER," \
	"turns INTEGER," \
	"win_player INTEGER," \
	"lost_player INTEGER," \
	"lost_time INTEGER," \
	"score INTEGER DEFAULT 0 );" \
	"CREATE VIEW IF NOT EXISTS \"lost_scores\" AS " \
	"SELECT p.id, p.name, SUM(g.score) score, " \
	"ROUND(AVG(g.score)) avg, (SELECT COUNT(id) FROM clients cc WHERE cc.playerid = p.id) cnt " \
	"FROM clients c JOIN players p ON p.id = c.playerid JOIN games g ON g.id = c.gameid " \
	"WHERE g.lost_player = p.id GROUP by p.id ORDER BY AVG(g.score);" \
	"CREATE VIEW IF NOT EXISTS \"win_scores\" AS " \
	"SELECT p.id, p.name, SUM(g.score) score, " \
	"ROUND(AVG(g.score)) avg, (SELECT COUNT(id) FROM clients cc WHERE cc.playerid = p.id) cnt " \
	"FROM clients c JOIN players p ON p.id = c.playerid JOIN games g ON g.id = c.gameid " \
	"WHERE g.win_player = p.id GROUP by p.id ORDER BY AVG(g.score) DESC;"
	"CREATE VIEW IF NOT EXISTS \"total_scores\" AS " \
	"SELECT p.id, p.name, ((CASE WHEN ws.score IS NULL THEN 0 ELSE ws.score END) - " \
	"(CASE WHEN ls.score IS NULL THEN 0 ELSE ls.score END)) score FROM players p " \
	"LEFT OUTER JOIN win_scores ws ON ws.id = p.id LEFT OUTER JOIN lost_scores ls " \
	"ON ls.id = p.id ORDER BY score DESC;" \
	"CREATE VIEW IF NOT EXISTS \"total_scores_abs\" AS " \
	"SELECT p.id, p.name, ((SELECT ABS(MIN(sq.score)) FROM total_scores sq " \
	"WHERE sq.score < 0) + ((CASE WHEN ws.score IS NULL THEN 0 ELSE ws.score END) - " \
	"(CASE WHEN ls.score IS NULL THEN 0 ELSE ls.score END))) ascore FROM players p " \
	"LEFT OUTER JOIN win_scores ws ON ws.id = p.id LEFT OUTER JOIN lost_scores ls " \
	"ON ls.id = p.id /*WHERE ascore > 0*/ ORDER BY ascore DESC;";
}

using namespace NetMauMau::DB;

SQLiteImpl::SQLiteImpl() : m_db(0L), m_turnStmt(0L), m_winStmt(0L), m_scoreNormStmt(0L),
	m_scoreNormLimitStmt(0L), m_scoreAbsStmt(0L), m_scoreAbsLimitStmt(0L) {

	const std::string &db(getDBFilename());

	if(!db.empty() && !getenv("NMM_NO_SQLITE")) {

		logDebug("SQLite-DB is located at: " << db);

		if(sqlite3_open(db.c_str(), &m_db) != SQLITE_ERROR) {

			if(!(sqlite3_prepare_v2(m_db, "UPDATE games SET turns = @TURN " \
									"WHERE id = @ID;", -1, &m_turnStmt, NULL) == SQLITE_OK &&
					sqlite3_prepare_v2(m_db, "UPDATE games SET win_player = " \
									   "(SELECT id FROM players WHERE name = @NAME) " \
									   "WHERE id = @ID AND win_player IS NULL;",
									   -1, &m_winStmt, NULL) == SQLITE_OK &&
					sqlite3_prepare_v2(m_db, "SELECT * FROM total_scores;", -1,
									   &m_scoreNormStmt, NULL) == SQLITE_OK &&
					sqlite3_prepare_v2(m_db, "SELECT * FROM total_scores LIMIT @LIM;",
									   -1, &m_scoreNormLimitStmt, NULL) == SQLITE_OK &&
					sqlite3_prepare_v2(m_db, "SELECT * FROM total_scores_abs;", -1,
									   &m_scoreAbsStmt, NULL) == SQLITE_OK &&
					sqlite3_prepare_v2(m_db, "SELECT * FROM total_scores_abs LIMIT @LIM;",
									   -1, &m_scoreAbsLimitStmt, NULL) == SQLITE_OK)) {
				logDebug(sqlite3_errmsg(m_db));
			}

			exec(SCHEMA);

			std::ostringstream sql;

			sql << "INSERT OR IGNORE INTO meta (dbver, date) VALUES ("
				<< MAKE_VERSION(SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR)
				<< "," << std::time(0L) << ");";

			exec(sql.str());

		} else {
			logDebug(sqlite3_errmsg(m_db));
			sqlite3_close(m_db);
			m_db = 0L;
		}
	}
}

SQLiteImpl::~SQLiteImpl() {

	if(m_db) {

		std::ostringstream sql;

		sql << "DELETE FROM games WHERE game_start IS NULL AND turns IS NULL "
			<< "AND win_player IS NULL AND lost_player IS NULL AND lost_time IS NULL;";

		exec(sql.str());
		exec("VACUUM;");

		sqlite3_finalize(m_scoreNormStmt);
		sqlite3_finalize(m_scoreNormLimitStmt);
		sqlite3_finalize(m_scoreAbsStmt);
		sqlite3_finalize(m_scoreAbsLimitStmt);
		sqlite3_finalize(m_winStmt);
		sqlite3_finalize(m_turnStmt);

		sqlite3_close(m_db);
	}
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunreachable-code"
std::string SQLiteImpl::getDBFilename() {

#if !defined(_WIN32) && defined(HAVE_MKDIR)
#ifdef NDEBUG

	const char *home = getenv("HOME");

	if(getuid() && home) {

		struct stat st;
		std::string hp(std::string(home).append("/."PACKAGE_NAME));

		if(stat(hp.c_str(), &st) == -1) {
			if(mkdir(hp.c_str(), S_IRWXU) != -1) {}
		}
	}

	if(!getuid() || home) {
		return(getuid() ? std::string(home).append("/."PACKAGE_NAME"/")
			   .append(PACKAGE_NAME".db").c_str() : DBDIR "/" PACKAGE_NAME".db");
	}

#else
	return(BUILDDIR"/"PACKAGE_NAME"-dbg.db");
#endif
#endif

#ifndef _WIN32
	return std::string();
#else
	char buffer[MAX_PATH];

	strcpy(buffer, getenv("APPDATA"));

	if(strlen(buffer)) {

		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];

		_splitpath(buffer, drive, dir, fname, ext);
		_makepath(buffer, drive, dir, PACKAGE_NAME, "db");

		return std::string(buffer);
	} else {
		return std::string();
	}

#endif
}
#pragma GCC diagnostic pop

bool SQLiteImpl::exec(const char *sql) const {

	char *err = 0L;

	if(m_db && sqlite3_exec(m_db, sql, NULL, NULL, &err) == SQLITE_OK) {
		return true;
	}

	if(err) {
		logWarning("SQLite: " << err);
		sqlite3_free(err);
	}

	return false;
}

SQLite::SCORES SQLiteImpl::getScores(SQLite::SCORE_TYPE type, std::size_t limit) const {

	SQLite::SCORES res;

	if(m_db) {

		sqlite3_stmt *stmt = 0L;
		bool succ = false;

		switch(type) {
		case SQLite::NORM: {
			if(limit > 0) {
				stmt = m_scoreNormLimitStmt;
				succ = sqlite3_bind_int64(stmt, 1, limit) == SQLITE_OK;
			} else {
				stmt = m_scoreNormStmt;
				succ = true;
			}
		}
		break;

		case SQLite::ABS: {
			if(limit > 0) {
				stmt = m_scoreAbsLimitStmt;
				succ = sqlite3_bind_int64(stmt, 1, limit) == SQLITE_OK;
			} else {
				stmt = m_scoreAbsStmt;
				succ = true;
			}
		}
		break;
		}

		while(succ && sqlite3_step(stmt) == SQLITE_ROW) {

			SQLite::SCORE sc = {
				std::strtoll(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)),
				NULL, 10),
				reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)),
				std::strtoll(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)),
				NULL, 10),
			};

			res.push_back(sc);
		}

		if(!(succ &&
				sqlite3_clear_bindings(stmt) == SQLITE_OK &&
				sqlite3_reset(stmt) == SQLITE_OK)) {
			logDebug(sqlite3_errmsg(m_db));
		}
	}

	return res;
}

long long int SQLiteImpl::getServedGames() const {

	long long int res = 0LL;
	char *err = 0L;

	if(m_db && (sqlite3_exec(m_db, "SELECT count(*) FROM games WHERE end IS NOT NULL;",
							 gamesCountCallback, &res, &err) != SQLITE_OK)) {
		if(err) {
			logWarning("SQLite: " << err);
			sqlite3_free(err);
		}
	}

	return res;
}

bool SQLiteImpl::addAIPlayer(const NetMauMau::Player::IPlayer *ai) const {

	std::ostringstream sql;

	sql << "BEGIN; INSERT OR IGNORE INTO players (name) VALUES(\'" << ai->getName() << "\');"
		<< "INSERT INTO clients (sock, host, port, version, log_in, playerid) SELECT "
		<< INVALID_SOCKET << ", \'" << PACKAGE_STRING << "\', 0,"
		<< MAKE_VERSION(SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR) << "," << std::time(0L)
		<< ", id FROM players WHERE name = \'" << ai->getName()  << "\'; END TRANSACTION;";

	return exec(sql.str());
}

bool SQLiteImpl::addPlayer(const NetMauMau::Common::IConnection::INFO &info) const {

	std::ostringstream sql;

	sql << "BEGIN; INSERT OR IGNORE INTO players (name) VALUES(\'" << info.name << "\');"
		<< "INSERT INTO clients (sock, host, port, version, log_in, playerid) SELECT "
		<< info.sockfd << ",\'" << info.host << "\'," << info.port << ","
		<< MAKE_VERSION(info.maj, info.min) << "," << std::time(0L)
		<< ", id FROM players WHERE name = \'" << info.name  << "\'; END TRANSACTION;";

	return exec(sql.str());
}

bool SQLiteImpl::logOutPlayer(const NetMauMau::Common::IConnection::NAMESOCKFD &nsf) const {

	std::ostringstream sql;

	sql << "UPDATE clients SET log_out = " << std::time(0L) << " WHERE sock = " << nsf.sockfd
		<< " AND log_out IS NULL AND playerid IN (SELECT id FROM players WHERE name = \'"
		<< nsf.name << "\');";

	return exec(sql.str());
}

long long int SQLiteImpl::newGame() {

	std::ostringstream sql;

	sql << "INSERT INTO games (server_start) VALUES (" << std::time(0L) << ");";

	return exec(sql.str()) ? (m_db ? sqlite3_last_insert_rowid(m_db) : 0LL) : 0LL;
}

bool SQLiteImpl::gameEnded(long long int gameIndex) const {

	std::ostringstream sql;

	if(gameIndex >= 0) {
		sql << "UPDATE games SET end = " << std::time(0L) << " WHERE id = " << gameIndex << ";";
	} else {
		sql << "UPDATE games SET end = " << std::time(0L) << " WHERE end IS NULL;";
	}

	return exec(sql.str());
}

bool SQLiteImpl::addPlayerToGame(long long int gid,
								 const NetMauMau::Common::IConnection::NAMESOCKFD &nsf) const {

	std::ostringstream sql;

	sql << "UPDATE clients SET gameid = " << gid << " WHERE sock = " << nsf.sockfd
		<< " AND playerid IN (SELECT id FROM players WHERE name = \'" << nsf.name
		<< "\') AND log_out IS NULL;";

	return exec(sql.str());
}

bool SQLiteImpl::turn(long long int gameIndex, std::size_t t) const {

	const bool succ = m_db && sqlite3_bind_int64(m_turnStmt, 1, t) == SQLITE_OK &&
					  sqlite3_bind_int64(m_turnStmt, 2, gameIndex) == SQLITE_OK &&
					  sqlite3_step(m_turnStmt) == SQLITE_DONE &&
					  sqlite3_clear_bindings(m_turnStmt) == SQLITE_OK &&
					  sqlite3_reset(m_turnStmt) == SQLITE_OK;

	if(!succ && m_db) logWarning("SQLite: " << sqlite3_errmsg(m_db));

	return succ;
}

bool SQLiteImpl::gamePlayStarted(long long int gameIndex) const {

	std::ostringstream sql;

	sql << "UPDATE games SET game_start = " << std::time(0L) << " WHERE id = " << gameIndex << ";";

	return exec(sql.str());
}

bool SQLiteImpl::playerLost(long long int gameIndex,
							const NetMauMau::Common::IConnection::NAMESOCKFD &nsf,
							time_t time, std::size_t points) const {
	std::ostringstream sql;

	sql << "UPDATE games SET lost_time = " << time << ", score = " << points
		<< ", lost_player = (SELECT id FROM players WHERE name = \'" << nsf.name << "\')"
		<< " WHERE " << "id = " << gameIndex << ";";

	return exec(sql.str());
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
bool SQLiteImpl::playerWins(long long int gameIndex,
							const NetMauMau::Common::IConnection::NAMESOCKFD &nsf) const {

	const bool succ = m_db && sqlite3_bind_text(m_winStmt, 1, nsf.name.c_str(),
					  static_cast<int>(nsf.name.size()),
					  SQLITE_TRANSIENT) == SQLITE_OK &&
					  sqlite3_bind_int64(m_winStmt, 2, gameIndex) == SQLITE_OK &&
					  sqlite3_step(m_winStmt) == SQLITE_DONE &&
					  sqlite3_clear_bindings(m_winStmt) == SQLITE_OK &&
					  sqlite3_reset(m_winStmt) == SQLITE_OK;

	if(!succ && m_db) logWarning("SQLite: " << sqlite3_errmsg(m_db));

	return succ;
}
#pragma GCC diagnostic pop

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
