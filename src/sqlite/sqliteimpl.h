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

#include "abstractconnection.h"

namespace NetMauMau {

namespace DB {

class SQLiteImpl {
	DISALLOW_COPY_AND_ASSIGN(SQLiteImpl)
public:
	SQLiteImpl();
	~SQLiteImpl();

	bool addPlayer(const Common::AbstractConnection::INFO &info) const;
	bool logOutPlayer(const Common::AbstractConnection::NAMESOCKFD &nsf) const;
	// cppcheck-suppress functionStatic
	long long int newGame() const;
	bool gameEnded(long long int gameIndex) const;
	bool addPlayerToGame(long long int gid,
						 const Common::AbstractConnection::NAMESOCKFD &nsf) const;
	bool turn(long long int gameIndex, std::size_t turn) const;
	bool gamePlayStarted(long long int gameIndex) const;
	bool playerLost(long long int gameIndex, const Common::AbstractConnection::NAMESOCKFD &nsf,
					time_t time, std::size_t points) const;
	bool playerWins(long long int gameIndex, const Common::AbstractConnection::NAMESOCKFD &nsf)
	const;

private:
	// cppcheck-suppress functionStatic
	bool exec(const std::string &sql) const;

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
