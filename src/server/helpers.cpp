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
#include "config.h"                     // for HAVE_ATEXIT, HAVE_CHOWN, etc
#endif

#include "helpers.h"

#ifndef _WIN32
#include <ifaddrs.h>                    // for ifaddrs, freeifaddrs, etc
#include <grp.h>                        // for getgrnam, gid_t, group
#include <pwd.h>                        // for getpwnam, passwd
#else
#define HOST_NAME_MAX 64
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>                      // for getnameinfo, NI_NUMERICHOST
#endif

#include <cstdlib>                      // for free

#ifdef HAVE_UNISTD_H
#include <unistd.h>                     // for getuid, setegid, seteuid
#endif

#include <cerrno>                       // for errno
#include <climits>                      // for PATH_MAX
#include <cstdio>                       // for snprintf, NULL, fclose, etc
#include <cstring>                      // for strerror, strcmp, strlen, etc
#include <fstream>                      // for operator<<, basic_ostream, etc
#include <set>                          // for set, etc

#include "game.h"                       // for TIMEFORMAT, Game
#include "logger.h"                     // for Logger
#include "servereventhandler.h"         // for EventHandler
#include "sqlite.h"                     // for SQLite
#include "ttynamecheckdir.h"            // for ttynameCheckDir

#ifndef DP_USER
#define DP_USER "nobody"
#endif

#ifndef DP_GROUP
#define DP_GROUP "tty"
#endif

namespace NetMauMau {

volatile bool interrupt = false;

double aiDelay = 1.0;
bool aceRound = false;
int decks = 1;
bool dirChange = false;
int initialCardCount = 5;
std::size_t minPlayers = 1;
bool ultimate = false;
char bind[HOST_NAME_MAX] = { 0 };
char *host = bind;
uint16_t port = SERVER_PORT;

#ifndef _WIN32
const time_t startTime = std::time(0L);
const char *interface;
#endif

#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic push
char *user = DP_USER;
char *grp = DP_GROUP;
char *arRank = "ACE";
char *dpErr = 0L;
#pragma GCC diagnostic pop

void updatePlayerCap(Server::Connection::CAPABILITIES &caps, std::size_t count,
					 Server::Connection &con, bool aiOpponent) {

	std::ostringstream os;

	os << count - (aiOpponent ? 1 : 0);
	caps["CUR_PLAYERS"] = os.str();
	con.setCapabilities(caps);
}

char *inetdParsedString(char *str) {

	if(str) {

		char *ptr = str;

		while(*ptr) {
			if(*ptr == '%' && (*(ptr + 1) && *(ptr + 1) != '%')) {
				*ptr = ' ';
			} else if(*ptr == '%' && (*(ptr + 1) && *(ptr + 1) == '%')) {
				std::strncpy(ptr, ptr + 1, std::strlen(ptr + 1) + 1);
			}

			++ptr;
		}

		return str;

	} else {
		return 0L;
	}
}

void sh_interrupt(int) {

	logWarning(Common::Logger::time(TIMEFORMAT) << "Server is about to shut down");
	Server::EventHandler::setInterrupted();
	interrupt = true;
}

#ifndef _WIN32

void sh_dump(int, siginfo_t *info, void *) {

	char *p = NULL;

	if(info) {

		char sp[PATH_MAX] = "";

#ifndef __OpenBSD__
		std::snprintf(sp, PATH_MAX, "/proc/%d/stat", info->si_pid);
#else
		std::snprintf(sp, PATH_MAX, "/proc/%d/status", info->si_pid);
#endif

		int tty_nr = 0;

		FILE *spf;

		if((spf = std::fopen(sp, "r"))) {

			int  iDummy;

#ifndef __OpenBSD__

			char cDummy, *sDummy;

			// cppcheck-suppress invalidscanf_libc
			// cppcheck-suppress invalidscanf
			if(std::fscanf(spf, "%d %ms %c %d %d %d %d", &iDummy, &sDummy, &cDummy, &iDummy,
						   &iDummy, &iDummy, &tty_nr)) {}

			free(sDummy);
#else
			char sDevice[20], sCmd[256];

			// cppcheck-suppress invalidscanf_libc
			// cppcheck-suppress invalidscanf
			if(std::fscanf(spf, "%255s %d %d %d %d %19s", sCmd, &iDummy, &iDummy, &iDummy,
						   &iDummy, sDevice)) {
				logDebug("BSD emitter: " << sCmd); // why (swapper) and not (kill)?
				logDebug("BSD tty device: " << sDevice); // why (-1,-1)?
			}

#endif

			std::fclose(spf);
		}

		if(!(p = Server::ttynameCheckDir(static_cast<dev_t>(tty_nr), "/dev/pts"))) {
			p = Server::ttynameCheckDir(static_cast<dev_t>(tty_nr), "/dev");
		}
	}

	std::ofstream out(p ? p : "/dev/null");

	free(p);

	if(out.is_open()) {

		out << std::boolalpha << "== Options ==" << std::endl;
		out << "AI-delay: " << static_cast<float>(aiDelay) << " sec" << std::endl;
		out << "A/K/Q rounds: " << aceRound << std::endl;

		if(aceRound) out << "A/K/Q rank: " << ((arRank != 0L) ? arRank : "ACE") << std::endl;

		out << "Decks: " << decks << std::endl;
		out << "Direction change: " << dirChange << std::endl;
		out << "Initial card count: " << initialCardCount << std::endl;
		out << "Players: " << minPlayers << std::endl;
		out << "Ultimate: " << ultimate << std::endl;

		char sr[128];
		std::snprintf(sr, 127, "Total received %.2f kb; total sent %.2f kb",
					  static_cast<double>(Common::AbstractSocket::getTotalReceivedBytes()) / 1024.0,
					  static_cast<double>(Common::AbstractSocket::getTotalSentBytes()) / 1024.0);

		out << "== Network ==" << std::endl;
		out << "Host: " << (host && *host ? host : "localhost") << std::endl;
		out << "Port: " << port << std::endl;
		out << sr << std::endl;

		char outstr[256];
		// cppcheck-suppress nonreentrantFunctionslocaltime
		struct tm *tmp = std::localtime(&startTime);

		if(tmp && std::strftime(outstr, sizeof(outstr), "Server start: %c", tmp)) {
			out << outstr << std::endl;
		}

		out << "Served games since server start: " << Server::Game::getServedGames() << std::endl;

		if(!DB::SQLite::getInstance()->getDBFilename().empty()) {
			out << "Total served games on this server: "
				<< DB::SQLite::getInstance()->getServedGames() << std::endl;
		}
	}
}

int getGroup(gid_t *gid, const char *group) {

	errno = 0;

	// cppcheck-suppress nonreentrantFunctionsgetgrnam
	struct group *g = getgrnam(group);

	if(g) {
		*gid = g->gr_gid;
		return 0;
	} else if(errno) {
		dpErr = std::strerror(errno);
	} else {
		dpErr = "unknown group";
	}

	return -1;
}


int getIPForIF(char *addr, size_t len, const char *iface) {

	bool listOnly = !addr && !len && !iface;

	std::set<std::string> ifaces;

	struct ifaddrs *ifas;

	if(!::getifaddrs(&ifas)) {

		for(struct ifaddrs *ifa = ifas; ifa != NULL; ifa = ifa->ifa_next) {

			if(ifa->ifa_addr && (ifa->ifa_addr->sa_family == AF_INET ||
								 ifa->ifa_addr->sa_family == AF_INET6)) {

				ifaces.insert(ifa->ifa_name);

				if(!listOnly && !::strcmp(ifa->ifa_name, iface)) {
					::getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), addr, len, NULL, 0,
								  NI_NUMERICHOST);
					::freeifaddrs(ifas);
					return 0;
				}
			}
		}

		if(listOnly) {
			for(std::set<std::string>::const_iterator iter(ifaces.begin());
					iter != ifaces.end(); ++iter) {
				logger(*iter);
			}
		}

		::freeifaddrs(ifas);
	}

	return -1;
}

int getUser(uid_t *uid, const char *usr) {

	errno = 0;

	// cppcheck-suppress nonreentrantFunctionsgetpwnam
	struct passwd *u = getpwnam(usr);

	if(u) {
		*uid = u->pw_uid;
		return 0;
	} else if(errno) {
		dpErr = std::strerror(errno);
	} else {
		dpErr = "unknown user";
	}

	return -1;
}

int dropPrivileges(const char *usr, const char *group) {

	if(!getuid()) {

		uid_t uid;
		gid_t gid;

		if(!getUser(&uid, usr) && !getGroup(&gid, group)) {

#if !defined(_WIN32) && defined(PIDFILE) && defined(HAVE_ATEXIT) && defined(HAVE_CHOWN)

			if(chown(PIDFILE, uid, gid)) logWarning("Couldn't change ownership of " << PIDFILE);

			if(chmod(PIDFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) {
				logWarning("Couldn't change mode of " << PIDFILE);
			}

#endif

			if(!setegid(gid) && !seteuid(uid)) {
				return 0;
			} else {
				dpErr = std::strerror(errno);
			}
		}

	} else {
		return 0;
	}

	return -1;
}
#endif

#ifdef HAVE_ATEXIT
void exit_hdlr() {
#if !defined(_WIN32) && defined(PIDFILE) && defined(HAVE_CHOWN)

	if(unlink(PIDFILE)) logWarning("Couldn't remove " << PIDFILE << ": " << std::strerror(errno));

#endif

	logInfo(Common::Logger::time(TIMEFORMAT) << "Server shut down normally");
}
#endif

void conLog(const Common::IConnection::INFO &info) {
	logInfo(Common::Logger::time(TIMEFORMAT) << "Connection from " << info.host << ":" << info.port
			<< " as \"" << info.name << "\" (" << info.maj << "." << info.min << ") "
			<< Common::Logger::nonl());
}

}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
