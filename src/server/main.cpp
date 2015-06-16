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

#ifdef _WIN32
#define COPY "\270"
#define AUML "\204"
#else
#define COPY "\u00a9"
#define AUML "\u00e4"
#endif

#include <popt.h>                       // for poptFreeContext, etc

#include <limits>
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
#include <ctime>
#endif

#include "logger.h"                     // for BasicLogger, logger, etc
#include "game.h"                       // for Game, etc
#include "gamecontext.h"                // for GameConfig
#include "helpers.h"                    // for arRank, minPlayers, decks, etc
#include "servereventhandler.h"         // for EventHandler
#include "serverplayer.h"               // for Player
#include "iruleset.h"
#include "ci_char_traits.h"

namespace {

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
		"Set the port to listen to", "PORT"
	},
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
				logWarning("Duplicate AI player name: \"" << typeStripped << "\"");
			} else {
				aiNames.push_back(aiName);
			}
		}
		break;

		case 'V': {

#if !(defined(_WIN32) || defined(NOH2M))

			if(!getenv("HELP2MAN_OUTPUT")) {
#endif
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
				// cppcheck-suppress nonreentrantFunctionslocaltime
				std::strftime(dateOut, sizeof(dateOut), "%x", std::localtime(&t));

				logger("Built " << dateOut << node.str() << " (" << BUILD_HOST << ")"
					   << cppversion.str());

				logger("");
				// cppcheck-suppress nonreentrantFunctionslocaltime
				std::strftime(dateOut, sizeof(dateOut), "%Y", std::localtime(&t));
				logger("Copyright " COPY " " << dateOut << " Heiko Sch" AUML "fer <"
					   << PACKAGE_BUGREPORT << ">");

#ifdef PACKAGE_URL

				if(*PACKAGE_URL) {
					logger("");
					logger("WWW: " << PACKAGE_URL);
				}

#endif
				logger("");
				logger("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A "
					   "PARTICULAR PURPOSE.");

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

				logError("\'" << arRank << "\' is not a valid ace round rank. " \
						 "Valid ranks are ACE, KING, or QUEEN.");
				poptFreeContext(pctx);
				return EXIT_FAILURE;
			}

			break;

		case 'I':
#ifndef WIN32
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

#ifndef _WIN32

	if(inetd) {
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
		logWarning("Couldn't create " << PIDFILE);
	}

#endif

	if(std::atexit(exit_hdlr)) logWarning("Couldn't register atexit function");

#endif

	logInfo("Welcome to " << PACKAGE_STRING);

#ifndef _WIN32
	struct sigaction sa;

	std::memset(&sa, 0, sizeof(struct sigaction));

	sa.sa_handler = sh_interrupt;
	sa.sa_flags = static_cast<int>(SA_RESETHAND);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	std::memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = sh_dump;
	sa.sa_flags = static_cast<int>(SA_SIGINFO);
	sigaction(SIGUSR1, &sa, NULL);

#ifdef HAVE_LIBRT

	timer_t timerid = 0;
	struct sigevent sev;
	struct itimerspec its;

	if(inetd) {

		sev.sigev_notify = SIGEV_SIGNAL;
		sev.sigev_signo = SIGRTMIN;
		sev.sigev_value.sival_ptr = &timerid;

		if(timer_create(CLOCK_REALTIME, &sev, &timerid) != -1) {
			sigaction(SIGRTMIN, &sa, NULL);
		} else {
			logWarning("Could not create timer for idle shutdown");
		}
	}

#endif

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
							   MAKE_VERSION(MIN_MAJOR, MIN_MINOR), false, port,
							   *host ? host : NULL);

		ultimate = (!aiOpponent && minPlayers > 2) ? ultimate : true;

		if(ultimate) logInfo("Running in ultimate mode");

		logInfo("Server accepts clients >= "
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

			if(cconf.decks != static_cast<std::size_t>(decks)) {
				logWarning("Adjusted amount of card decks from " << decks << " to " << cconf.decks);
				decks = static_cast<int>(cconf.decks);
			}

			if(cconf.initialCards != static_cast<std::size_t>(initialCardCount)) {
				logWarning("Adjusted amount of initial cards from " << initialCardCount << " to "
						   << cconf.initialCards);
				initialCardCount = static_cast<int>(cconf.initialCards);
			}

			if(aiOpponent && game.getPlayerCount() < aiNames.size()) {
				minPlayers = game.getPlayerCount() + 1;
			} else if(ctx.getEngineContext().getRuleSet()->getMaxPlayers() < minPlayers) {
				minPlayers = ctx.getEngineContext().getRuleSet()->getMaxPlayers();
				logWarning("Limiting amount of human players to " << minPlayers <<
						   " (due to configuration limit).");
			}

			Server::Connection::CAPABILITIES caps;

			caps.insert(std::make_pair("SERVER_VERSION", NMM_VERSION_STRING(SERVER_VERSION_MAJOR,
									   SERVER_VERSION_MINOR)));
			caps.insert(std::make_pair("SERVER_VERSION_REL", PACKAGE_VERSION));
			caps.insert(std::make_pair("AI_OPPONENT", aiOpponent ? "true" : "false"));
			caps.insert(std::make_pair("ULTIMATE", ultimate ? "true" : "false"));
			caps.insert(std::make_pair("ACEROUND", aceRound ? std::string(1, arRank ?
									   ::toupper(arRank[0]) : 'A') : "false"));
			caps.insert(std::make_pair("HAVE_SCORES",
									   DB::SQLite::getInstance()->getDBFilename().empty() ? "false"
									   : "true"));
			caps.insert(std::make_pair("DIRCHANGE", dirChange ? "true" : "false"));

			if(decks > 1) {
				char cc[20];
				std::snprintf(cc, 19, "%d", 32 * decks);
				caps.insert(std::make_pair("TALON", cc));
			}

			if(initialCardCount != 5) {
				char cc[20];
				std::snprintf(cc, 19, "%d", initialCardCount);
				caps.insert(std::make_pair("INITIAL_CARDS", cc));
			}

			if(aiOpponent) caps.insert(std::make_pair("AI_NAME", std::string(aiNames[0]).substr(0,
										   std::string(aiNames[0]).rfind(':'))));

			std::ostringstream mvos;
			mvos << static_cast<uint16_t>(con.getMinClientVersion() >> 16)
				 << '.' << static_cast<uint16_t>(con.getMinClientVersion());

			caps.insert(std::make_pair("MIN_VERSION", mvos.str()));

			std::ostringstream mpos;
			mpos << (aiOpponent ? aiNames.size() + 1 : minPlayers);
			caps.insert(std::make_pair("MAX_PLAYERS", mpos.str()));

			std::ostringstream cpos;
			cpos << game.getPlayerCount();
			caps.insert(std::make_pair("CUR_PLAYERS", cpos.str()));

			con.setCapabilities(caps);

			while(!interrupt) {

// 				if(interrupt) game.shutdown();

				timeval tv = { 1, 0 };

				int r;

				if((r = con.wait((game.getPlayerCount() > 0 && !aiOpponent) ? &tv : NULL)) > 0) {

					Server::Connection::INFO info;
					Server::Connection::ACCEPT_STATE state = Server::Connection::NONE;

					if(!interrupt) {

						try {
							state = con.accept(info, refuse);
						} catch(const Common::Exception::SocketException &e) {
							logDebug("Client accept failed: " << e.what());
							state = Server::Connection::NONE;
						}

						if(state == Server::Connection::PLAY) {

							conLog(info);

							Server::Game::COLLECT_STATE cs =
								game.collectPlayers(minPlayers, new Server::Player(info.name,
													info.sockfd, con));

							if(cs == Server::Game::ACCEPTED || cs == Server::Game::ACCEPTED_READY) {

								logger("accepted");
								updatePlayerCap(caps, game.getPlayerCount(), con, aiOpponent);

								if(cs == Server::Game::ACCEPTED_READY) {

									refuse = true;
#ifdef HAVE_LIBRT

									if(inetd) {

										its.it_value.tv_sec = 0;
										its.it_value.tv_nsec = 0;
										its.it_interval.tv_sec = 0;
										its.it_interval.tv_nsec = 0;

										if(timer_settime(timerid, 0, &its, NULL) == -1) {
											logWarning("Could not disarm idle timer");
										} else {
											logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) <<
													"Idle timer disarmed");
										}
									}

#endif

									try {
										game.start(ultimate);

#ifdef HAVE_LIBRT

										if(!inetd) {
#endif
											updatePlayerCap(caps, game.getPlayerCount(), con,
															aiOpponent);
#ifdef HAVE_LIBRT
										} else {

											its.it_value.tv_sec = 60;
											its.it_value.tv_nsec = 0;
											its.it_interval.tv_sec = 0;
											its.it_interval.tv_nsec = 0;

											if(timer_settime(timerid, 0, &its, NULL) == -1) {
												logWarning("Could not arm idle timer");
											} else {
												logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT)
														<< "Idle timer armed");
											}
										}

#endif
										refuse = false;

									} catch(const Common::Exception::SocketException &e) {
										game.shutdown(e.what());
										game.reset(false);
									}
								}

							} else {
								logger("refused");

								con.removePlayer(info);
								game.removePlayer(info.name);

								if(info.sockfd != INVALID_SOCKET) {
									NetMauMau::Common::AbstractSocket::shutdown(info.sockfd);
								}
							}

						} else if(state == Server::Connection::REFUSED) {
							conLog(info);
							logger("refused");
						}
					}

				} else if(r == WAIT_ERROR) {
					game.reset(true);
				}

				updatePlayerCap(caps, game.getPlayerCount(), con, aiOpponent);
			}

			NetMauMau::DB::SQLite::getInstance()->gameEnded(NOGAME_IDX);

		} catch(const Common::Exception::SocketException &e) {
			logError(e.what());
			con.reset();
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
