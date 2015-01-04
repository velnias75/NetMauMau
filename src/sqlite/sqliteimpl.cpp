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

#include "logger.h"

namespace {
const char *SCHEMA = "CREATE TABLE IF NOT EXISTS players (" \
					 "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
					 "\"name\" TEXT NOT NULL," \
					 "\"host\" TEXT NOT NULL," \
					 "\"port\" INTEGER NOT NULL," \
					 "\"client_version\" INTEGER NOT NULL" \
					 ");";
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

	if(!getuid() || home) {

		std::string db(getuid() ? std::string(home).append("/."PACKAGE_NAME"/")
					   .append(PACKAGE_NAME".db").c_str() : DBDIR "/" PACKAGE_NAME".db");

		if(sqlite3_open(db.c_str(), &m_db) != SQLITE_ERROR) {
			sqlite3_exec(m_db, SCHEMA, NULL, NULL, NULL);
		} else {
			logDebug(sqlite3_errmsg(m_db));
			sqlite3_close(m_db);
			m_db = 0L;
		}
	}

#endif
}

SQLiteImpl::~SQLiteImpl() {
#ifndef _WIN32

	if(m_db) sqlite3_close(m_db);

#endif
}

bool SQLiteImpl::addPlayer(const NetMauMau::Common::AbstractConnection::INFO &info) const {

#ifndef _WIN32
	char *err;
	std::ostringstream sql;

	sql << "REPLACE INTO main.players (name, host, port, client_version) VALUES (\'"
		<< info.name << "\', \'" << info.host << "\', " << info.port << ", "
		<< MAKE_VERSION(info.maj, info.min) << ");";

	if(m_db && sqlite3_exec(m_db, sql.str().c_str(), NULL, NULL, &err) == SQLITE_OK) {
		return true;
	}

	if(err) {
		logWarning(err);
		sqlite3_free(err);
	}

#endif

	return false;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
