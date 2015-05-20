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

#ifndef NETMAUMAU_HELPERS_H
#define NETMAUMAU_HELPERS_H

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"                     // for HAVE_ATEXIT
#endif

#include <csignal>                      // for siginfo_t, uid_t
#include <cstddef>                      // for size_t, NULL

#include "abstractconnection.h"
#include "serverconnection.h"           // for Connection

namespace NetMauMau _LOCAL {

extern volatile bool interrupt;
extern std::size_t minPlayers;
extern char *arRank;
extern int decks;
extern int initialCardCount;
extern double aiDelay;
extern char bind[];
extern char *host;
extern uint16_t port;
extern char *user;
extern char *grp;
extern bool ultimate;
extern bool dirChange;
extern bool aceRound;
extern char *dpErr;

#ifndef _WIN32
extern const char *interface;
#endif

void updatePlayerCap(Server::Connection::CAPABILITIES &caps, std::size_t count,
					 Server::Connection &con, bool aiOpponent);

char *inetdParsedString(char *str);
void sh_interrupt(int);

#ifndef _WIN32
void sh_dump(int, siginfo_t *info, void *);
int getGroup(gid_t *gid, const char *group);
int getIPForIF(char *addr = NULL, size_t len = 0, const char *iface = NULL);
int getUser(uid_t *uid, const char *usr);
int dropPrivileges(const char *usr, const char *group);
#endif

#ifdef HAVE_ATEXIT
void exit_hdlr();
#endif

void conLog(const NetMauMau::Common::IConnection::INFO &info);

}

#endif /* NETMAUMAU_HELPERS_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
