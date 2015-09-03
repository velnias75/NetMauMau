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

#include <cstdio>

#include "game.h"

#ifdef ENABLE_THREADS
#include "mutexlocker.h"
#endif

#include "abstractsocket.h"             // for AbstractSocket
#include "easyplayer.h"                 // for EasyPlayer
#include "hardplayer.h"
#include "gamecontext.h"                // for GameConfig
#include "ieventhandler.h"              // for IEventHandler
#include "logger.h"
#include "luafatalexception.h"          // for LuaFatalException
#include "serverplayerexception.h"
#include "protocol.h"

#ifndef _WIN32
#define ELLIPSIS "…"
#else
#define ELLIPSIS "..."
#endif

using namespace NetMauMau::Server;

long Game::m_gameServed = 0L;
volatile bool Game::m_interrupted = false;

Game::Game(GameContext &ctx) throw(NetMauMau::Common::Exception::SocketException) :
	Common::Observable<Game, NOTIFYWHAT>(), m_ctx(ctx), m_engine(ctx.getEngineContext()),
	m_db(NetMauMau::DB::SQLite::getInstance()), m_aiPlayers(), m_players(), m_gameIndex(0LL),
	m_running(false) {

	const std::size_t orgAI = ctx.getAINames().size();
	const std::size_t maxPl = ctx.getEngineContext().getRuleSet(&m_engine)->getMaxPlayers();
	const std::size_t cntAi = std::min(orgAI, maxPl - 1);

	if(ctx.hasAIPlayer() && cntAi <= m_aiPlayers.max_size()) m_aiPlayers.reserve(cntAi);

	if(maxPl <= m_players.max_size()) m_players.reserve(maxPl);

	if(ctx.hasAIPlayer()) {

		std::size_t aiAdded = 0;

		if(cntAi != orgAI) logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
										  << "Limiting number of AI players to " << cntAi
										  << " (due to configuration limit).");

		for(std::size_t i = 0; i < cntAi; ++i) {

			if(!ctx.getAINames()[i].empty()) {

				const GameContext::AINAMES::value_type &aiSanName(ctx.getAINames()[i][0] == '+' ?
						& (ctx.getAINames()[i][1]) : ctx.getAINames()[i]);

				if(!aiSanName.empty()) {

					const GameContext::AINAMES::value_type::size_type spos = aiSanName.rfind(':');

					NetMauMau::Player::IPlayer::TYPE type = NetMauMau::Player::IPlayer::HARD;

					if(spos != GameContext::AINAMES::value_type::npos && aiSanName.length() >
							spos && (aiSanName.substr(spos + 1)[0] == 'e' ||
									 aiSanName.substr(spos + 1)[0] == 'E')) {
						type = NetMauMau::Player::IPlayer::EASY;
					}

					m_aiPlayers.push_back(type == NetMauMau::Player::IPlayer::HARD ?
										  static_cast<NetMauMau::Player::AbstractPlayer *>
										  (new NetMauMau::Player::HardPlayer(aiSanName.
												  substr(0, spos), m_engine.getPlayedOutCards())) :
										  static_cast<NetMauMau::Player::AbstractPlayer *>
										  (new NetMauMau::Player::EasyPlayer(aiSanName.
												  substr(0, spos), m_engine.getPlayedOutCards())));

					logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Adding AI player \""
							<< m_aiPlayers.back()->getName() << "\" ("
							<< (m_aiPlayers.back()->getType() ==
								NetMauMau::Player::IPlayer::HARD ? "hard" : "easy") << ")");

					m_engine.addPlayer(m_aiPlayers.back());
					++aiAdded;
				}
			}
		}

		if(cntAi != orgAI) ctx.getCardConfig() = NetMauMau::Common::getCardConfig(maxPl);

		m_engine.setAlwaysWait(aiAdded > 1);
	}

	gameReady();
}

Game::~Game() {

	for(std::vector<NetMauMau::Player::IPlayer *>::const_iterator i(m_players.begin());
			i != m_players.end(); ++i) {
		delete *i;
	}

	for(std::vector<NetMauMau::Player::AbstractPlayer *>::const_iterator i(m_aiPlayers.begin());
			i != m_aiPlayers.end(); ++i) {
		delete *i;
	}
}

Game::COLLECT_STATE Game::collectPlayers(std::size_t minPlayers,
		NetMauMau::Player::IPlayer *player) {

	if(m_engine.getPlayerCount() < minPlayers) {

		if(!addPlayer(player)) return Game::REFUSED;

		m_db->addPlayerToGame(m_gameIndex, m_engine.getEventHandler().getConnection().
							  getPlayerInfo(player->getSerial()));

		if(m_engine.getPlayerCount() == minPlayers) {

			for(std::vector<NetMauMau::Player::AbstractPlayer *>::const_iterator
					i(m_aiPlayers.begin()); i != m_aiPlayers.end(); ++i) {

				const std::vector<NetMauMau::Player::AbstractPlayer *>::value_type p(*i);

				m_db->addAIPlayer(p);
				m_db->addPlayerToGame(m_gameIndex,
									  NetMauMau::Common::IConnection::NAMESOCKFD(p->getName(),
											  "", INVALID_SOCKET, MAKE_VERSION(SERVER_VERSION_MAJOR,
													  SERVER_VERSION_MINOR)));
			}

			notify(READY);
			return ACCEPTED_READY;

		} else {
			return ACCEPTED;
		}

	} else {
		return REFUSED_FULL;
	}
}

bool Game::addPlayer(NetMauMau::Player::IPlayer *player) {

	m_players.push_back(player);

	try {

		if(m_engine.addPlayer(m_players.back())) {
			notify(PLAYERADDED);
			return true;
		} else {
			delete m_players.back();
			m_players.pop_back();
			return false;
		}

	} catch(const NetMauMau::Common::Exception::SocketException &) {
		m_players.pop_back();
		return false;
	}
}

void Game::start(bool ultimate) throw(NetMauMau::Common::Exception::SocketException) {

	const std::size_t minPlayers = m_engine.getPlayerCount();

	if(m_ctx.hasAIPlayer()) m_engine.setFirstPlayer(m_players.back());

	m_engine.distributeCards();
	m_engine.setUltimate(ultimate);
	m_engine.gameAboutToStart();

	try {

		m_running = true;

		notify(GAMESTARTED);

		m_engine.initialTurn();

		while(ultimate ? m_engine.getPlayerCount() >= 2u :
				m_engine.getPlayerCount() == minPlayers) {

			if(!m_engine.nextTurn()) break;

			if(m_interrupted) {
				shutdown();
				break;
			}
		}

		if(ultimate || m_ctx.hasAIPlayer()) m_engine.gameOver();

	} catch(const Exception::ServerPlayerException &e) {
		logFatal(NetMauMau::Common::Logger::time(TIMEFORMAT) << e);
		m_running = false;
		m_engine.error(NetMauMau::Common::Protocol::V15::ERR_TO_EXC_PLAYER +
					   e.player() + ": " + e.what());
		m_engine.gameOver();
	} catch(const NetMauMau::Lua::Exception::LuaFatalException &e) {
		logFatal(NetMauMau::Common::Logger::time(TIMEFORMAT) << e);
		m_running = false;
		m_engine.error(NetMauMau::Common::Protocol::V15::ERR_TO_EXC_MISCONFIGURED + e.what());
		m_engine.gameOver();
	}

	reset(false);
}

void Game::removePlayer(const std::string &player) {
	m_engine.removePlayer(player);
	notify(PLAYERREMOVED);
}

void Game::reset(bool playerLost) throw() {

	++m_gameServed;

	m_running = m_interrupted = false;

	if(playerLost) m_engine.error(NetMauMau::Common::Protocol::V15::ERR_TO_EXC_LOSTCONN);

	m_engine.reset();

	if(m_ctx.hasAIPlayer()) {

		try {

			for(std::vector<NetMauMau::Player::AbstractPlayer *>::const_iterator
					i(m_aiPlayers.begin()); i != m_aiPlayers.end(); ++i) {

				std::vector<NetMauMau::Player::AbstractPlayer *>::value_type p(*i);

				p->reset();

				m_db->logOutPlayer(NetMauMau::Common::IConnection::NAMESOCKFD(p->getName(), "",
								   INVALID_SOCKET, MAKE_VERSION(SERVER_VERSION_MAJOR,
										   SERVER_VERSION_MINOR)));

				m_engine.addPlayer(p);
			}

			m_engine.setAlwaysWait(m_aiPlayers.size() > 1);

		} catch(const NetMauMau::Common::Exception::SocketException &e) {
			logDebug(NetMauMau::Common::Logger::time(TIMEFORMAT) << __PRETTY_FUNCTION__
					 << ": failed to add AI player: " << e.what());
		}
	}

	char sr[128];

#ifdef ENABLE_THREADS

	try {
#endif
		std::snprintf(sr, 127, "Received %.2f kBytes; sent %.2f kBytes", static_cast<double>
					  (NetMauMau::Common::AbstractSocket::getReceivedBytes()) / 1024.0,
					  static_cast<double>(NetMauMau::Common::AbstractSocket::getSentBytes()) /
					  1024.0);

		logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << sr);

		NetMauMau::Common::AbstractSocket::resetReceivedBytes();
		NetMauMau::Common::AbstractSocket::resetSentBytes();
#ifdef ENABLE_THREADS
	} catch(NetMauMau::Common::MutexException &) {}

#endif

	m_interrupted = false;

	notify(GAMEENDED);
	gameReady();
}

void Game::gameReady() {
	m_db->gameEnded(m_gameIndex);
	logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Ready for new game " << ELLIPSIS);
	m_engine.setGameId(m_gameIndex = m_db->newGame());
}

void Game::shutdown(const std::string &reason) const throw() {
	m_engine.error(reason.empty() ? NetMauMau::Common::Protocol::V15::ERR_TO_EXC_SHUTDOWNMSG :
				   (NetMauMau::Common::Protocol::V15::ERR_TO_EXC_MISCONFIGURED + reason));
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
