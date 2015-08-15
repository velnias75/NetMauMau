/*
 * Copyright 2014-2015 by Heiko Schäfer <heiko@rangun.de>
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

#define NMM_VERSION_STRING(maj,min) NMMSTR(maj) "." NMMSTR(min)
#define NMMSTR(s) #s

#include <popt.h>                       // for poptFreeContext, etc

#include <cerrno>
#include <climits>                      // IWYU pragma: keep
#include <cstring>                      // for memset
#include <cstdio>                       // for snprintf
#include <iostream>                     // for operator<<, ostringstream, etc
#include <stdbool.h>

#ifdef PIDFILE
#include <fstream>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/types.h>

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_LIBRT
#include <sys/time.h>
#include <csignal>
#endif

#include "logger.h"                     // for BasicLogger, logger, etc
#include "gamecontext.h"                // for GameConfig
#include "helpers.h"                    // for arRank, minPlayers, decks, etc
#include "servereventhandler.h"         // for EventHandler
#include "serverplayer.h"               // for Player
#include "iruleset.h"

#ifdef HAVE_LIBMICROHTTPD
#include "httpd.h"
#else
#include "game.h"
#include "eff_map.h"
#include "ci_string.h"
#endif

namespace {

const char *REFUSED = "refused";

bool inetd = false;

volatile bool refuse = false;

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct _AINameCmp : std::binary_function<std::string, std::string, bool> {
	inline result_type operator()(const first_argument_type &x,
								  const second_argument_type &y) const {
		return std::equal_to<NetMauMau::Common::ci_string>()(first_argument_type(x).substr(0,
				first_argument_type(x).rfind(':')).c_str(), second_argument_type(y).substr(0,
						second_argument_type(y).rfind(':')).c_str());
	}
};
#pragma GCC diagnostic pop

#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic push
char *aiName = AI_NAME;
#pragma GCC diagnostic pop

NetMauMau::Server::GameContext::AINAMES aiNames;

poptOption poptOptions[] = {
	{
		"players", 'p', POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &NetMauMau::minPlayers,
		0, "Set amount of players", "AMOUNT"
	},
	{ "ultimate", 'u', POPT_ARG_NONE, NULL, 'u', "Play until last player wins", NULL },
	{ "direction-change", 'd', POPT_ARG_NONE, NULL, 'd', "Allow direction changes", NULL },
	{
		"ace-round", 'a', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT | POPT_ARGFLAG_OPTIONAL,
		&NetMauMau::arRank, 'a', "Enable ace rounds (requires all clients to be at " \
		"least of version 0.7)", "ACE|QUEEN|KING"
	},
	{
		"decks", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &NetMauMau::decks,
		0, "Amount of card decks to use", "AMOUNT"
	},
	{
		"initial-card-count", 'c', POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT,
		&NetMauMau::initialCardCount, 0, "Amount of cards each player gets at game start", "AMOUNT"
	},
	{
		"ai-name", 'A', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &aiName, 'A',
		"Set the name of one AI player. Can be given multiple times. " \
		"Optionally append :E[asy] or :H[ard] to the name to set the strength. " \
		"Whitespaces can get substituted by \'%\', \'%\' itself by \"%%\"", "NAME[:E|H]"
	},
	{
		"ai-delay", 'D', POPT_ARG_DOUBLE | POPT_ARGFLAG_SHOW_DEFAULT, &NetMauMau::aiDelay,
		0, "Delay after AI turns", "SECONDS"
	},
#ifndef _WIN32
	{
		"inetd", 0,  POPT_ARG_NONE,  NULL, 'i',
		"Put the server in a mode suitable for (x)inetd", NULL
	},
#endif
	{ "bind", 'b', POPT_ARG_STRING, &NetMauMau::host, 0, "Bind to HOST", "HOST" },
#ifndef _WIN32
	{ "iface", 'I', POPT_ARG_STRING, &NetMauMau::interface, 'I', "Bind to INTERFACE", "INTERFACE" },
#endif
	{
		"port", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &NetMauMau::port, 0,
		"Set the port the to listen to", "PORT"
	},
#ifdef HAVE_LIBMICROHTTPD
	{
		"webserver", 'W', POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT | POPT_ARGFLAG_OPTIONAL,
		&NetMauMau::hport, 'W', "Enable the webserver at PORT", "PORT"
	},
#endif
#ifndef _WIN32
	{
		"user", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &NetMauMau::user, 0,
		"Set the user to run as (only if started as root)", "USER"
	},
	{
		"group", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &NetMauMau::grp, 0,
		"Set the group to run as (only if started as root)", "GROUP"
	},
#endif
	{ "version", 'V', POPT_ARG_NONE, NULL, 'V', "Display version information", NULL },
	POPT_AUTOHELP
	POPT_TABLEEND
};

}

using namespace NetMauMau;

int main(int argc, const char **argv) {

	poptContext pctx = poptGetContext(NULL, argc, argv, poptOptions, 0);
	int c;

	while((c = poptGetNextOpt(pctx)) >= 0) {
		switch(c) {
		case 'A': {

			const std::string typeStripped(std::string(inetdParsedString(aiName)).substr(0,
										   std::string(aiName).rfind(':')));

			if(std::count_if(aiNames.begin(), aiNames.end(), std::bind2nd(_AINameCmp(),
							 inetdParsedString(aiName)))) {
				logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
						   << "Duplicate AI player name: \"" << typeStripped << "\"");
			} else {
				aiNames.push_back(aiName);
			}
		}
		break;

#ifdef HAVE_LIBMICROHTTPD

		case 'W':
			NetMauMau::httpd = true;

			if(NetMauMau::hport == 0) NetMauMau::hport = HTTPD_PORT;

			break;
#endif

		case 'V': {

#if !(defined(_WIN32) || defined(NOH2M))

			if(!getenv("HELP2MAN_OUTPUT")) {
#endif
				std::ostringstream os;
				NetMauMau::version(os);

				logger(os.str());

#if !(defined(_WIN32) || defined(NOH2M))

			} else {

				char dateOut[1024];
				std::time_t t = BUILD_DATE;
				// cppcheck-suppress nonreentrantFunctionslocaltime
				std::strftime(dateOut, sizeof(dateOut), "%Y", std::localtime(&t));

				logger(PACKAGE_NAME << " - Copyright (c) " << dateOut
					   << ", Heiko Schaefer <heiko@rangun.de>");
			}

#endif
		}

		poptFreeContext(pctx);
		return EXIT_SUCCESS;

		case 'u':
			ultimate = true;
			break;

		case 'd':
			dirChange = true;
			break;

		case 'i':
			inetd = true;
			break;

		case 'a':
			aceRound = true;

			if(arRank && !(Common::ci_char_traits::eq(arRank[0], 'A') ||
						   Common::ci_char_traits::eq(arRank[0], 'Q') ||
						   Common::ci_char_traits::eq(arRank[0], 'K'))) {

				logError(NetMauMau::Common::Logger::time(TIMEFORMAT) << "\'" << arRank
						 << "\' is not a valid ace round rank. " \
						 "Valid ranks are ACE, KING, or QUEEN.");
				poptFreeContext(pctx);
				return EXIT_FAILURE;
			}

			break;

		case 'I':
#ifndef WIN32
			{
				int r;

				if(interface[0] == '?') {
					getIPForIF();
					poptFreeContext(pctx);
					return EXIT_SUCCESS;
				} else if((r = getIPForIF(host, HOST_NAME_MAX, interface))) {

					if(r) {
						logError(NetMauMau::Common::Logger::time(TIMEFORMAT)
								 << "Couldn't bind to interface" << interface)
						poptFreeContext(pctx);
						return EXIT_FAILURE;
					}
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

#ifndef _WIN32

	if(inetd) {
		NetMauMau::Common::Logger::_time::enabled = false;
		NetMauMau::Common::Logger::setSilentMask(0xFF);
		NetMauMau::Common::Logger::writeSyslog(true);
	} else {
		NetMauMau::Common::Logger::writeSyslog(false);
	}

#endif

#ifndef HAVE_ARC4RANDOM_UNIFORM
#if HAVE_INITSTATE
	char istate[256];
	initstate(static_cast<unsigned int>(std::time(0L)), istate, 256);
#else
	std::srand(std::time(0L));
#endif
#endif

#ifdef HAVE_ATEXIT
#if !defined(_WIN32) && defined(PIDFILE) && defined(HAVE_CHOWN)
	std::ofstream pidFile(PIDFILE);

	if(pidFile.is_open()) {
		pidFile << getpid();
		pidFile.close();
	} else {
		logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Couldn't create " << PIDFILE);
	}

#endif

	if(std::atexit(exit_hdlr)) logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
											  << "Couldn't register atexit function");

#endif

	logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Welcome to " << PACKAGE_STRING);

#ifndef _WIN32
	static struct sigaction sa_pipe;
	std::memset(&sa_pipe, 0, sizeof(struct sigaction));
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic push
	sa_pipe.sa_handler = SIG_IGN;
#pragma GCC diagnostic pop
	sa_pipe.sa_flags = static_cast<int>(SA_RESETHAND | SA_RESTART);
	sigaction(SIGPIPE, &sa_pipe, NULL);

	static struct sigaction sa_intr;
	std::memset(&sa_intr, 0, sizeof(struct sigaction));
	sa_intr.sa_handler = sh_interrupt;
	sa_intr.sa_flags = static_cast<int>(SA_RESETHAND | SA_RESTART);
	sigaction(SIGINT, &sa_intr, NULL);
	sigaction(SIGTERM, &sa_intr, NULL);

#if 0
#ifdef HAVE_LIBRT

	timer_t timerid = 0;
	struct sigevent sev;
	struct itimerspec its;

	if(inetd) {

		sev.sigev_notify = SIGEV_SIGNAL;
		sev.sigev_signo = SIGRTMIN;
		sev.sigev_value.sival_ptr = &timerid;

		if(timer_create(CLOCK_REALTIME, &sev, &timerid) != -1) {

			static struct sigaction sa_idle;
			sigaction(SIGRTMIN, &sa_idle, NULL);
			NetMauMau::armIdleTimer(timerid, its);

		} else {
			logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
					   << "Could not create timer for idle shutdown");
		}
	}

#endif
#endif

	static struct sigaction sa_dump;
	std::memset(&sa_dump, 0, sizeof(struct sigaction));
	sa_dump.sa_sigaction = sh_dump;
	sa_dump.sa_flags = static_cast<int>(SA_SIGINFO | SA_RESTART);
	sigaction(SIGUSR1, &sa_dump, NULL);

	NetMauMau::DB::SQLite::getInstance();

	if(!dropPrivileges(user, grp)) {

#else
	NetMauMau::DB::SQLite::getInstance();

	signal(SIGINT, sh_interrupt);
#endif

		const bool aiOpponent = minPlayers <= 1;

		if(aiOpponent) {

			if(aiNames.empty()) aiNames.push_back(aiName);

			minPlayers = std::max<std::size_t>(2, aiNames.size() + minPlayers);
		}

		Server::Connection con(aceRound ? std::max(7u, MAKE_VERSION(MIN_MAJOR, MIN_MINOR)) :
							   MAKE_VERSION(MIN_MAJOR, MIN_MINOR), false,
							   static_cast<uint16_t>(port), *host ? host : NULL);

		ultimate = (!aiOpponent && minPlayers > 2) ? ultimate : true;

		if(ultimate) logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT)
								 << "Running in ultimate mode");

		logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Server accepts clients >= "
				<< static_cast<uint16_t>(con.getMinClientVersion() >> 16)
				<< "." << static_cast<uint16_t>(con.getMinClientVersion()));

		try {

			con.connect(inetd);

			Common::CARDCONFIG cconf(NetMauMau::Common::getCardConfig(minPlayers,
									 static_cast<std::size_t>(initialCardCount),
									 std::min(std::numeric_limits<std::size_t>::max() >> 5,
											  static_cast<std::size_t>(decks))));

			Server::EventHandler evtHdlr(con);
			Server::GameContext ctx(evtHdlr, static_cast<long>(::fabs(aiDelay * 1e06)),
									dirChange, cconf, aiOpponent, aiNames,
									static_cast<char>(aceRound ? ::toupper(arRank ?
											arRank[0] : 'A') : 0));
			Server::Game game(ctx);

#ifdef HAVE_LIBMICROHTTPD
			con.addObserver(NetMauMau::Server::Httpd::getInstance());
			game.addObserver(NetMauMau::Server::Httpd::getInstance());
			game.getEngine().addObserver(NetMauMau::Server::Httpd::getInstance());
#endif

			if(cconf.decks != static_cast<std::size_t>(decks)) {
				logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
						   << "Adjusted amount of card decks from " << decks << " to "
						   << cconf.decks);
				decks = static_cast<int>(cconf.decks);
			}

			if(cconf.initialCards != static_cast<std::size_t>(initialCardCount)) {
				logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
						   << "Adjusted amount of initial cards from " << initialCardCount
						   << " to " << cconf.initialCards);
				initialCardCount = static_cast<int>(cconf.initialCards);
			}

			if(aiOpponent && game.getPlayerCount() < aiNames.size()) {
				minPlayers = game.getPlayerCount() + 1;
			} else if(ctx.getEngineContext().getRuleSet()->getMaxPlayers() < minPlayers) {
				minPlayers = ctx.getEngineContext().getRuleSet()->getMaxPlayers();
				logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
						   << "Limiting amount of human players to " << minPlayers
						   << " (due to configuration limit).");
			}

			Server::Connection::CAPABILITIES caps;

			NetMauMau::Common::efficientAddOrUpdate(caps, "SERVER_VERSION",
													NMM_VERSION_STRING(SERVER_VERSION_MAJOR,
															SERVER_VERSION_MINOR));
			NetMauMau::Common::efficientAddOrUpdate(caps, "SERVER_VERSION_REL", PACKAGE_VERSION);
			NetMauMau::Common::efficientAddOrUpdate(caps, "AI_OPPONENT",
													aiOpponent ? "true" : "false");
			NetMauMau::Common::efficientAddOrUpdate(caps, "ULTIMATE", ultimate ? "true" : "false");
			NetMauMau::Common::efficientAddOrUpdate(caps, "ACEROUND", aceRound ? std::string(1,
													arRank ? ::toupper(arRank[0]) : 'A') : "false");
			NetMauMau::Common::efficientAddOrUpdate(caps, "HAVE_SCORES", DB::SQLite::getInstance()->
													getDBFilename().empty() ? "false" : "true");
			NetMauMau::Common::efficientAddOrUpdate(caps, "DIRCHANGE",
													dirChange ? "true" : "false");

			if(decks > 1) {
				char cc[20];
				std::snprintf(cc, 19, "%d", 32 * decks);
				NetMauMau::Common::efficientAddOrUpdate(caps, "TALON", cc);
			}

			if(initialCardCount != 5) {
				char cc[20];
				std::snprintf(cc, 19, "%d", initialCardCount);
				NetMauMau::Common::efficientAddOrUpdate(caps, "INITIAL_CARDS", cc);
			}

			if(aiOpponent) NetMauMau::Common::efficientAddOrUpdate(caps, "AI_NAME",
						std::string(aiNames[0]).substr(0, std::string(aiNames[0]).rfind(':')));

			std::ostringstream mvos;
			mvos << static_cast<uint16_t>(con.getMinClientVersion() >> 16)
				 << '.' << static_cast<uint16_t>(con.getMinClientVersion());

			NetMauMau::Common::efficientAddOrUpdate(caps, "MIN_VERSION", mvos.str());

			std::ostringstream mpos;
			mpos << (aiOpponent ? aiNames.size() + 1 : minPlayers);
			NetMauMau::Common::efficientAddOrUpdate(caps, "MAX_PLAYERS", mpos.str());

			std::ostringstream cpos;
			cpos << game.getPlayerCount();
			NetMauMau::Common::efficientAddOrUpdate(caps, "CUR_PLAYERS", cpos.str());

			updatePlayerCap(caps, game.getPlayerCount(), con);

			while(!interrupt) {

				timeval tv = { 1, 0 };

				int r;

				if((r = con.wait((game.getPlayerCount() > 0 && !aiOpponent) ? &tv : NULL)) > 0) {

					Server::Connection::INFO info;
					Server::Connection::ACCEPT_STATE state = Server::Connection::NONE;

					if(!interrupt) {

						try {
							state = con.accept(info, refuse);
						} catch(const Common::Exception::SocketException &e) {
							logDebug(NetMauMau::Common::Logger::time(TIMEFORMAT)
									 << "Client accept failed: " << e.what());
							state = Server::Connection::NONE;
						}

						if(state == Server::Connection::PLAY) {

							const bool cl = conLog(info);

							Server::Game::COLLECT_STATE cs =
								game.collectPlayers(minPlayers, new Server::Player(info.name,
													info.sockfd, con));

							if(cs == Server::Game::ACCEPTED || cs == Server::Game::ACCEPTED_READY) {

								if(cl) logger(NetMauMau::Common::Logger::time(TIMEFORMAT)
												  << "accepted");

								updatePlayerCap(caps, game.getPlayerCount(), con);

								if(cs == Server::Game::ACCEPTED_READY) {
#ifdef HAVE_LIBRT
// 									if(inetd) NetMauMau::disarmIdleTimer(timerid, its);
#endif

									try {

										refuse = true;
										game.start(ultimate);
										updatePlayerCap(caps, game.getPlayerCount(), con);
#ifdef HAVE_LIBRT
// 										if(inetd) NetMauMau::armIdleTimer(timerid, its);
#endif
									} catch(const Common::Exception::SocketException &e) {
										game.shutdown(e.what());
										game.reset(false);
									}

									refuse = false;
								}

							} else {

								if(cl) logger(REFUSED);

								con.removePlayer(info);
								game.removePlayer(info.name);

								if(info.sockfd != INVALID_SOCKET) {
									NetMauMau::Common::AbstractSocket::shutdown(info.sockfd);
								}
							}

						} else if(state == Server::Connection::REFUSED) {
							if(conLog(info)) logger(REFUSED);
						}
					}

				} else if(r == WAIT_ERROR) {
					game.reset(true);
				}

				updatePlayerCap(caps, game.getPlayerCount(), con);
			}

			NetMauMau::DB::SQLite::getInstance()->gameEnded(NOGAME_IDX);

		} catch(const Common::Exception::SocketException &e) {

#if defined(ENOTSOCK)

			if(inetd && e.error() == ENOTSOCK) {
				NetMauMau::Common::Logger::_time::enabled = true;
				NetMauMau::Common::Logger::setSilentMask(0x00);
				NetMauMau::Common::Logger::writeSyslog(false);
			}

#endif

			logError(NetMauMau::Common::Logger::time(TIMEFORMAT) << e.what());

#if defined(ENOTSOCK)

			if(inetd && e.error() == ENOTSOCK) {
				logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT)
						<< "option \'--inetd\' is meaningful only if launched from (x)inetd");
			}

#endif

			con.reset();
			return EXIT_FAILURE;
		}

#ifndef _WIN32
	} else {
		logError(NetMauMau::Common::Logger::time(TIMEFORMAT)
				 << "Changing user/group failed" << (dpErr ? ": " : "") << (dpErr ? dpErr : ""));
	}

#endif

	return EXIT_SUCCESS;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
