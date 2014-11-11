/*
 * Copyright 2014 by Heiko Schäfer <heiko@rangun.de>
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

#include <set>
#include <ctime>
#include <cerrno>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <iostream>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef _WIN32
#include <ifaddrs.h>
#include <pwd.h>
#include <grp.h>
#else
#define HOST_NAME_MAX 64
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include <signal.h>

#include <popt.h>

#include "game.h"
#include "serverplayer.h"
#include "serverconnection.h"
#include "servereventhandler.h"
#include "logger.h"

#ifndef DP_USER
#define DP_USER "nobody"
#endif

#ifndef DP_GROUP
#define DP_GROUP "nogroup"
#endif

namespace {

bool ultimate = false;
std::size_t minPlayers = 1;
uint16_t port = SERVER_PORT;

volatile bool refuse = false;

#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic push
char bind[HOST_NAME_MAX] = { 0 };
char *host = bind;
char *aiName = "Computer";
#ifndef _WIN32
char *user  = DP_USER;
char *grp = DP_GROUP;
char *dpErr = 0L;
const char *interface;
#endif
#pragma GCC diagnostic pop

poptOption poptOptions[] = {
	{
		"players", 'p', POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &minPlayers,
		'p', "Set amount of players", "AMOUNT"
	},
	{ "ultimate", 'u', POPT_ARG_NONE, NULL, 'u', "Play until last player wins", NULL },
	{
		"ai-name", 'A', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &aiName, 0,
		"Set the name of the AI player", "NAME"
	},
	{ "bind", 'b', POPT_ARG_STRING, &host, 0, "Bind to HOST", "HOST" },
#ifndef _WIN32
	{ "iface", 'I', POPT_ARG_STRING, &interface, 'I', "Bind to INTERFACE", "INTERFACE" },
#endif
	{
		"port", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &port, 0,
		"Set the port to listen to", "PORT"
	},
#ifndef _WIN32
	{
		"user", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &user, 0,
		"Set the user to run as (only if started as root)", "USER"
	},
	{
		"group", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &grp, 0,
		"Set the group to run as (only if started as root)", "GROUP"
	},
#endif
	{ "version", 'V', POPT_ARG_NONE, NULL, 'V', "Display version information", NULL },
	POPT_AUTOHELP
	POPT_TABLEEND
};

void updatePlayerCap(NetMauMau::Server::Connection::CAPABILITIES &caps, std::size_t count,
					 NetMauMau::Server::Connection &con, bool aiOpponent) {

	std::ostringstream os;

	os << count - (aiOpponent ? 1 : 0);
	caps["CUR_PLAYERS"] = os.str();
	con.setCapabilities(caps);
}

volatile bool interrupt = false;

void sh_interrupt(int) {
	logWarning("Server is about to shut down");
	NetMauMau::Server::EventHandler::setInterrupted();
	interrupt = true;
}

#ifndef _WIN32
int getGroup(gid_t *gid, const char *group) {

	errno = 0;

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


int getIPForIF(char *addr = NULL, size_t len = 0, const char *iface = NULL) {

	bool listOnly = !addr && !len && !iface;

	std::set<std::string> ifaces;

	struct ifaddrs *ifas;

	if(!::getifaddrs(&ifas)) {

		for(struct ifaddrs *ifa = ifas; ifa != NULL; ifa = ifa->ifa_next) {

			if(ifa->ifa_addr && (ifa->ifa_addr->sa_family == AF_INET ||
								 ifa->ifa_addr->sa_family == AF_INET6)) {

				ifaces.insert(ifa->ifa_name);

				if(!listOnly && !::strcmp(ifa->ifa_name, iface)) {
					::getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
								  addr, len, NULL, 0, NI_NUMERICHOST);
					::freeifaddrs(ifas);
					return 0;
				}
			}
		}

		if(listOnly) {
			for(std::set<std::string>::const_iterator iter(ifaces.begin()); iter != ifaces.end();
					++iter) {
				logger(*iter);
			}
		}

		::freeifaddrs(ifas);
	}

	return -1;
}

int getUser(uid_t *uid, const char *usr) {

	errno = 0;

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

}

using namespace NetMauMau;

int main(int argc, const char **argv) {

#ifndef HAVE_ARC4RANDOM_UNIFORM
	std::srand(std::time(0L));
#endif

	poptContext pctx = poptGetContext(NULL, argc, argv, poptOptions, 0);
	int c;

	while((c = poptGetNextOpt(pctx)) >= 0) {
		switch(c) {
		case 'V': {

			logger(PACKAGE_STRING << " " << BUILD_TARGET);
			logger("");

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
			std::strftime(dateOut, sizeof(dateOut), "%x", std::localtime(&t));

			logger("Built " << dateOut << node.str() << " (" << BUILD_HOST << ")"
				   << cppversion.str());

			logger("");
			std::strftime(dateOut, sizeof(dateOut), "%Y", std::localtime(&t));
			logger("Copyright \u00a9 " << dateOut << " Heiko Sch\u00e4fer <" << PACKAGE_BUGREPORT
				   << ">");

#ifdef PACKAGE_URL

			if(*PACKAGE_URL) {
				logger("");
				logger("WWW: " << PACKAGE_URL);
			}

#endif
			logger("");
			logger("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A "
				   "PARTICULAR PURPOSE.");
		}

		poptFreeContext(pctx);
		return EXIT_SUCCESS;

		case 'p':
			minPlayers = std::min<std::size_t>(minPlayers, 5);
			break;

		case 'u':
			ultimate = true;
			break;

		case 'I':
#if !WIN32
			if(interface[0] == '?') {
				getIPForIF();
				poptFreeContext(pctx);
				return EXIT_SUCCESS;
			} else if(getIPForIF(host, HOST_NAME_MAX, interface)) {
				if(getIPForIF(host, HOST_NAME_MAX, interface)) {
					logError("Couldn't bind to interface" << interface)
					poptFreeContext(pctx);
					return EXIT_FAILURE;
				}
			}

#endif
			break;
		}
	}

	if(c < -1) {
		std::cerr << poptBadOption(pctx, POPT_BADOPTION_NOALIAS) << ": " << poptStrerror(c)
				  << std::endl;;
		return EXIT_FAILURE;
	}

	poptFreeContext(pctx);

	logInfo("Welcome to " << PACKAGE_STRING);

#ifndef _WIN32
	struct sigaction sa;

	std::memset(&sa, 0, sizeof(struct sigaction));

	sa.sa_handler = sh_interrupt;
	sa.sa_flags = SA_RESETHAND;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	if(!dropPrivileges(user, grp)) {

#else
	signal(SIGINT, sh_interrupt);

#endif

		const bool aiOpponent = minPlayers <= 1;
		Server::Connection con(port, *host ? host : NULL);
	
		ultimate = minPlayers > 2 ? ultimate : false;

		if(ultimate && !aiOpponent) logInfo("Running in ultimate mode");

		try {

			con.connect();

			Server::EventHandler evtHdlr(con);
			Server::Game game(evtHdlr, aiOpponent, aiName);

			Server::Connection::CAPABILITIES caps;
			caps.insert(std::make_pair("SERVER_VERSION", PACKAGE_VERSION));
			caps.insert(std::make_pair("AI_OPPONENT", aiOpponent ? "true" : "false"));
			caps.insert(std::make_pair("ULTIMATE", ultimate ? "true" : "false"));

			if(aiOpponent) caps.insert(std::make_pair("AI_NAME", aiName));

			std::ostringstream mvos;
			mvos << MIN_MAJOR << '.' << MIN_MINOR;
			caps.insert(std::make_pair("MIN_VERSION", mvos.str()));

			std::ostringstream mpos;
			mpos << (aiOpponent ? 1 : minPlayers);
			caps.insert(std::make_pair("MAX_PLAYERS", mpos.str()));

			std::ostringstream cpos;
			cpos << game.getPlayerCount() - (aiOpponent ? 1 : 0);
			caps.insert(std::make_pair("CUR_PLAYERS", cpos.str()));

			con.setCapabilities(caps);

			while(!interrupt) {

				if(con.wait() > 0) {

					Server::Connection::INFO info;
					Server::Connection::ACCEPT_STATE state;

					if(!interrupt && (state = con.accept(info, refuse)) ==
							Server::Connection::PLAY) {

						logInfo("Connection from " << info.host << ":" << info.port << " as \""
								<< info.name << "\" (" << info.maj << "." << info.min << ") "
								<< NetMauMau::Common::Logger::nonl());

						Server::Game::COLLECT_STATE cs =
							game.collectPlayers(minPlayers, new Server::Player(info.name,
												info.sockfd, con));

						if(cs == Server::Game::ACCEPTED || cs == Server::Game::ACCEPTED_READY) {

							logger("accepted");
							updatePlayerCap(caps, game.getPlayerCount(), con, aiOpponent);

							if(cs == Server::Game::ACCEPTED_READY) {

								refuse = true;
								game.start(ultimate);
								updatePlayerCap(caps, game.getPlayerCount(), con, aiOpponent);
								refuse = false;
							}

						} else {
							logger("refused");
						}
					}
				}
			}

		} catch(const Common::Exception::SocketException &e) {
			logError(e.what());
			return EXIT_FAILURE;
		}

#ifndef _WIN32
	} else {
		logError("Changing user/group failed" << (dpErr ? ": " : "") << (dpErr ? dpErr : ""));
	}

#endif

	return EXIT_SUCCESS;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
