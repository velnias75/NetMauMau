/*
 * Copyright 2015-2020 by Heiko Schäfer <heiko@rangun.de>
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

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>                     // for getuid, setegid, seteuid
#endif

#include <ctime>
#include <cerrno>                       // for errno
#include <climits>                      // for PATH_MAX
#include <cstring>                      // for strerror, strcmp, strlen, etc
#include <fstream>                      // for operator<<, basic_ostream, etc
#include <set>                          // for set, etc
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "logger.h"                     // for Logger
#include "servereventhandler.h"         // for EventHandler
#include "ttynamecheckdir.h"            // for ttynameCheckDir

#ifdef HAVE_LIBMICROHTTPD
#include "httpd.h"
#else
#include "eff_map.h"
#include "game.h"
#endif

#ifdef ENABLE_THREADS
#include "mutexlocker.h"
#endif

#ifndef DP_USER
#define DP_USER "nobody"
#endif

#ifndef DP_GROUP
#define DP_GROUP "tty"
#endif

#ifdef _WIN32
#define COPY "\270"
#define AUML "\204"
#else
#define COPY "\u00a9"
#define AUML "\u00e4"
#endif

namespace {
#ifdef ENABLE_THREADS
NetMauMau::Common::Mutex capsLock;
#endif

void unknownSignal(int sig) {
#if _XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L || GNU_SOURCE
	logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Received unhandled signal \""
			   << strsignal(sig) << "\" (" << sig << ")");
#endif
}

bool isValidIP(const char *candidate) {

#if HAVE_ARPA_INET_H
   char buf[INET6_ADDRSTRLEN];

   if(::inet_pton(AF_INET, candidate, buf) || 
	   ::inet_pton(AF_INET6, candidate, buf)) {
        return true;
    }
#endif

   return false;
}

int addr4proto(struct ifaddrs *const ifas, std::set<std::string> &ifaces, 
		bool listOnly, const char *iface, char *addr, std::size_t len,
		int family, std::size_t familyLen, const char *v) {
	
	for(struct ifaddrs *ifa = ifas; ifa != NULL; ifa = ifa->ifa_next) {

		if(ifa->ifa_addr && ifa->ifa_addr->sa_family == family) {

			ifaces.insert(ifa->ifa_name);

			if(!listOnly && !::strcmp(ifa->ifa_name, iface)) {
				::getnameinfo(ifa->ifa_addr, familyLen, 
								addr, len, NULL, 0, NI_NUMERICHOST);
				logDebug(v << " of " << iface << ": " << addr);
				::freeifaddrs(ifas);
				return 0;
			}
		}
	}
	
	return 1;
}

}

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
int port = SERVER_PORT;

const time_t startTime = std::time(0L);

#ifndef _WIN32
const char *interface;
#endif

#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic push
char *user = DP_USER;
char *grp = DP_GROUP;
char *arRank = "ACE";
char *dpErr = 0L;
#pragma GCC diagnostic pop

#ifdef HAVE_LIBMICROHTTPD
bool httpd = false;
int hport = HTTPD_PORT;
#endif

void updatePlayerCap(Server::Connection::CAPABILITIES &caps, std::size_t count,
					 Server::Connection &con) {

#ifdef ENABLE_THREADS
	MUTEXLOCKER(capsLock);
#endif

	std::ostringstream os;

	os << count;
	NetMauMau::Common::efficientAddOrUpdate(caps, "CUR_PLAYERS", os.str());

#ifdef HAVE_LIBMICROHTTPD

	const std::string &url(NetMauMau::Server::Httpd::getInstance()->getWebServerURL());

	if(!url.empty()) NetMauMau::Common::efficientAddOrUpdate(caps, "WEBSERVER_URL", url);

	NetMauMau::Server::Httpd::getInstancePtr()->setCapabilities(caps);

#endif

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
	}

	return str;
}

void sh_interrupt(int sig) {

	if(sig == SIGINT || sig == SIGTERM) {
		logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Server is about to shut down");
		Server::Game::setInterrupted();
		Server::EventHandler::setInterrupted();
		interrupt = true;
	} else {
		unknownSignal(sig);
	}
}

#ifndef _WIN32

void sh_dump(int sig, siginfo_t *info, void *) {

	if(sig == SIGUSR1) {

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

				std::free(sDummy);
#else
				char sDevice[20], sCmd[256];

				// cppcheck-suppress invalidscanf_libc
				// cppcheck-suppress invalidscanf
				if(std::fscanf(spf, "%255s %d %d %d %d %19s", sCmd, &iDummy, &iDummy, &iDummy,
							   &iDummy, sDevice)) {
					logDebug(NetMauMau::Common::Logger::time(TIMEFORMAT) << "BSD emitter: "
							 << sCmd); // why (swapper) and not (kill)?
					logDebug(NetMauMau::Common::Logger::time(TIMEFORMAT) << "BSD tty device: "
							 << sDevice); // why (-1,-1)?
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
			dump(out);
			out.flush();
		}
	} else {
		unknownSignal(sig);
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

int getIPForIF(bool ipv4, char *addr, size_t len, const char *iface) {

	const bool listOnly = !addr && !len && !iface;
	
	if(!listOnly && isValidIP(iface)) {
		::strcpy(addr, iface);
		return 0;
	}

	std::set<std::string> ifaces;

	struct ifaddrs *ifas;

	if(!::getifaddrs(&ifas)) {

 		if(addr4proto(ifas, ifaces, listOnly, iface, addr, len, 
 			!ipv4 ? AF_INET6 : AF_INET, 
 			!ipv4 ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in),
 			!ipv4 ? "IPv6" : "IPv4") == 0) return 0;

		if(listOnly) {
			for(std::set<std::string>::const_iterator iter(ifaces.begin());
					iter != ifaces.end(); ++iter) logger(*iter);
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

			if(chown(PIDFILE, uid, gid)) logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
														<< "Couldn't change ownership of "
														<< PIDFILE);

			if(chmod(PIDFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) {
				logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Couldn't change mode of "
						   << PIDFILE);
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

	if(unlink(PIDFILE)) logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Couldn't remove "
									   << PIDFILE << ": " << std::strerror(errno));

#endif

	logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Server shut down normally");
}
#endif

bool conLog(const Common::IConnection::INFO &info) {

	if(info.maj || info.min) {
		logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Connection from " << info.host
				<< ":" << info.port << (!info.name.empty() ? " as \"" : "")
				<< (!info.name.empty() ? info.name : "") << (!info.name.empty() ? "\" (" : " (")
				<< info.maj << "." << info.min << ") " << Common::Logger::nonl());

		return true;
	}

	return false;
}

void version(std::ostream &out, bool utf8) {

	out << PACKAGE_STRING << " " << BUILD_TARGET << "\n\n";

	std::ostringstream node;

	if(std::string("(none)") != BUILD_NODE) {
		node << " on " << BUILD_NODE;
	} else {
		node << "";
	}

	std::ostringstream cppversion;
#if defined(__GNUC__) && defined(__VERSION__)
	cppversion << " with g++ " << __VERSION__;
#else
	cppversion << "";
#endif

	char dateOut[1024];
	std::time_t t = BUILD_DATE;
	// cppcheck-suppress nonreentrantFunctionslocaltime
	std::strftime(dateOut, sizeof(dateOut), "%x", std::localtime(&t));

	out << "Built " << dateOut << node.str() << " (" << BUILD_HOST << ")"
		<< cppversion.str() << "\n\n";

	// cppcheck-suppress nonreentrantFunctionslocaltime
	std::strftime(dateOut, sizeof(dateOut), "%Y", std::localtime(&t));
	out << "Copyright " << (utf8 ? "\u00a9" : COPY) << " " << dateOut
		<< " Heiko Sch" << (utf8 ? "\u00e4" : AUML) << "fer <" << PACKAGE_BUGREPORT << ">\n";

#ifdef PACKAGE_URL

	if(*PACKAGE_URL) out << "\nWWW: " << PACKAGE_URL << "\n";

#endif
	out << "\nThere is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A "
		"PARTICULAR PURPOSE.";
}

void dump(std::ostream &out) {

	out << std::boolalpha << "== Options ==\n";
	out << "AI-delay: " << static_cast<float>(aiDelay) << " sec\n";
	out << "A/K/Q rounds: " << aceRound << "\n";

	std::string arRankStr;

	if(arRank) {
		const std::size_t al = std::strlen(arRank);
		arRankStr.reserve(al);
		std::transform(arRank, arRank + al, std::back_inserter(arRankStr), ::toupper);
	}

	if(aceRound) out << "A/K/Q rank: " << ((arRank != 0L) ? arRankStr.c_str() : "ACE") << "\n";

	out << "Decks: " << decks << "\n";
	out << "Direction change: " << dirChange << "\n";
	out << "Initial card count: " << initialCardCount << "\n";
	out << "Players: " << minPlayers << "\n";
	out << "Ultimate: " << ultimate << "\n";
	out << "Thread model: ";

	char sr[128];

#ifdef ENABLE_THREADS
	out << "per remote player";

#if defined(HAVE_CONFSTR) && defined(_CS_GNU_LIBPTHREAD_VERSION)

	confstr(_CS_GNU_LIBPTHREAD_VERSION, sr, 127);
	out << " (using " << sr << ")\n";

#elif defined(PTW32_VERSION_STRING)
	out << " (using Pthreads-win32 " << PTW32_VERSION_STRING << ")\n";
#else
	out << '\n';
#endif

#else
	out << "single\n";
#endif

#ifdef ENABLE_TREADS

	try {
#endif
		std::snprintf(sr, 127, "Total received %.2f kBytes; total sent %.2f kBytes",
					  static_cast<double>(Common::AbstractSocket::getTotalReceivedBytes()) / 1024.0,
					  static_cast<double>(Common::AbstractSocket::getTotalSentBytes()) / 1024.0);
#ifdef ENABLE_TREADS
	} catch(NetMauMau::Common::MutexException &) {}

#endif

	out << "== Network ==\n";
	out << "Host: " << (host && *host ? host : "localhost") << "\n";
	out << "Port: " << port << "\n";
	out << sr << "\n";

	char outstr[256];
	// cppcheck-suppress nonreentrantFunctionslocaltime
	struct tm *tmp = std::localtime(&startTime);

	if(tmp && std::strftime(outstr, sizeof(outstr), "Server start: %c", tmp)) {
		out << outstr << "\n";
	}

	out << "Served games since server start: " << Server::Game::getServedGames() << "\n";

	if(!DB::SQLite::getInstance()->getDBFilename().empty()) {
		out << "Total served games on this server: "
			<< DB::SQLite::getInstance()->getServedGames() << "\n";
	}
}

#if 0
#ifdef HAVE_LIBRT
void armIdleTimer(timer_t timerid, struct itimerspec &its) {

	its.it_value.tv_sec = 900;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if(timer_settime(timerid, 0, &its, NULL) == -1) {
		logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Could not arm idle timer");
	} else {

		struct itimerspec itc;
		char *sdStr = 0L;

		if(!timer_gettime(timerid, &itc) &&
				!(itc.it_value.tv_sec == 0 &&
				  itc.it_value.tv_nsec == 0)) {

			time_t now;
			time(&now);
			now += itc.it_value.tv_sec;
			sdStr = std::asctime(std::localtime(&now));

		}

		if(sdStr) logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Idle timer armed to "
							  << sdStr);
	}
}

void disarmIdleTimer(timer_t timerid, struct itimerspec &its) {

	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if(timer_settime(timerid, 0, &its, NULL) == -1) {
		logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Could not disarm idle timer");
	} else {
		logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Idle timer disarmed");
	}
}
#endif
#endif

}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; remove-trailing-space: true;
