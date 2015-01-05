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

#include <sstream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "sqliteimpl.h"

#include "iplayer.h"
#include "logger.h"

namespace {
const char *SCHEMA =
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
	"ON ls.id = p.id ORDER BY score DESC;";
}

using namespace NetMauMau::DB;

SQLiteImpl::SQLiteImpl() : m_db(0L) {
#ifndef _WIN32
	const char *home = getenv("HOME");

	if(getuid() && home) {

		struct stat st;
		std::string hp(std::string(home).append("/."PACKAGE_NAME));

		if(stat(hp.c_str(), &st) == -1) {
			if(mkdir(hp.c_str(), S_IRWXU) != -1) {}
		}
	}

#ifdef NDEBUG

	if(!getuid() || home) {

		std::string db(getuid() ? std::string(home).append("/."PACKAGE_NAME"/")
					   .append(PACKAGE_NAME".db").c_str() : DBDIR "/" PACKAGE_NAME".db");
#else
	std::string db(BUILDDIR"/"PACKAGE_NAME"-dbg.db");
	logDebug("SQLite-DB is located at: " << db);
#endif

		if(sqlite3_open(db.c_str(), &m_db) != SQLITE_ERROR) {

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

#ifdef NDEBUG
	}

#endif

#endif
}

SQLiteImpl::~SQLiteImpl() {
#ifndef _WIN32

	if(m_db) {

		std::ostringstream sql;

		sql << "DELETE FROM games WHERE game_start IS NULL AND turns IS NULL "
			<< "AND win_player IS NULL AND lost_player IS NULL AND lost_time IS NULL;";

		exec(sql.str());
		exec("VACUUM;");

		sqlite3_close(m_db);
	}

#endif
}

bool SQLiteImpl::exec(const std::string &sql) const {
#ifndef _WIN32
	char *err;

	if(m_db && sqlite3_exec(m_db, sql.c_str(), NULL, NULL, &err) == SQLITE_OK) {
		return true;
	}

	if(err) {
		logWarning(err);
		sqlite3_free(err);
	}

#endif

	return false;
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

bool SQLiteImpl::addPlayer(const NetMauMau::Common::AbstractConnection::INFO &info) const {

	std::ostringstream sql;

	sql << "BEGIN; INSERT OR IGNORE INTO players (name) VALUES(\'" << info.name << "\');"
		<< "INSERT INTO clients (sock, host, port, version, log_in, playerid) SELECT "
		<< info.sockfd << ",\'" << info.host << "\'," << info.port << ","
		<< MAKE_VERSION(info.maj, info.min) << "," << std::time(0L)
		<< ", id FROM players WHERE name = \'" << info.name  << "\'; END TRANSACTION;";

	return exec(sql.str());
}

bool SQLiteImpl::logOutPlayer(const NetMauMau::Common::AbstractConnection::NAMESOCKFD &nsf) const {

	std::ostringstream sql;

	sql << "UPDATE clients SET log_out = " << std::time(0L) << " WHERE sock = " << nsf.sockfd
		<< " AND log_out IS NULL AND playerid IN (SELECT id FROM players WHERE name = \'"
		<< nsf.name << "\');";

	return exec(sql.str());
}

long long int SQLiteImpl::newGame() const {

	std::ostringstream sql;

	sql << "INSERT INTO games (server_start) VALUES (" << std::time(0L) << ");";

#ifndef _WIN32
	return exec(sql.str()) ? (m_db ? sqlite3_last_insert_rowid(m_db) : 0LL) : 0LL;
#else
	return 0LL;
#endif

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
								 const NetMauMau::Common::AbstractConnection::NAMESOCKFD &nsf)
const {

	std::ostringstream sql;

	sql << "UPDATE clients SET gameid = " << gid << " WHERE sock = " << nsf.sockfd
		<< " AND playerid IN (SELECT id FROM players WHERE name = \'" << nsf.name
		<< "\') AND log_out IS NULL;";

	return exec(sql.str());
}

bool SQLiteImpl::turn(long long int gameIndex, std::size_t t) const {

	std::ostringstream sql;
	sql << "UPDATE games SET turns = " << t << " WHERE id = " << gameIndex << ";";
	return exec(sql.str());
}

bool SQLiteImpl::gamePlayStarted(long long int gameIndex) const {

	std::ostringstream sql;
	sql << "UPDATE games SET game_start = " << std::time(0L) << " WHERE id = " << gameIndex << ";";
	return exec(sql.str());
}

bool SQLiteImpl::playerLost(long long int gameIndex,
							const NetMauMau::Common::AbstractConnection::NAMESOCKFD &nsf,
							time_t time, std::size_t points) const {
	std::ostringstream sql;
	sql << "UPDATE games SET lost_time = " << time << ", score = " << points
		<< ", lost_player = (SELECT id FROM players WHERE name = \'" << nsf.name << "\')"
		<< " WHERE " << "id = " << gameIndex << ";";
	return exec(sql.str());
}

bool SQLiteImpl::playerWins(long long int gameIndex,
							const NetMauMau::Common::AbstractConnection::NAMESOCKFD &nsf) const {
	std::ostringstream sql;
	sql << "UPDATE games SET win_player = (SELECT id FROM players WHERE name = \'"
		<< nsf.name << "\')" << " WHERE " << "id = " << gameIndex << " AND win_player IS NULL;";
	return exec(sql.str());
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
