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

#include <ctime>
#include <cstdio>

#include "game.h"
#include "logger.h"
#include "sqlite.h"
#include "stdplayer.h"
#include "gameconfig.h"
#include "ieventhandler.h"
#include "abstractsocket.h"
#include "luafatalexception.h"

namespace {
const std::string MISCONFIGURED("Misconfigured or compromised server. Please report: ");
}

using namespace NetMauMau::Server;

long Game::m_gameServed = 0L;

Game::Game(GameConfig &cfg) throw(NetMauMau::Common::Exception::SocketException)
	: m_cfg(cfg), m_engine(cfg.getEngineConfig()), m_aiPlayers(), m_players(), m_gameIndex(0LL) {

	m_players.reserve(50);

	if(cfg.getAIPlayer()) {

		std::size_t aiAdded = 0;

		for(int i = 0; i < 4; ++i) {

			if(!cfg.getAIName()[i].empty()) {

				const std::string &aiSanName(cfg.getAIName()[i][0] == '+' ?
											 & (cfg.getAIName()[i][1]) : cfg.getAIName()[i]);

				if(!aiSanName.empty()) {
					m_aiPlayers.push_back(new NetMauMau::Player::StdPlayer(aiSanName));
					logInfo("Adding AI player \"" << m_aiPlayers.back()->getName() << "\"");
					m_engine.addPlayer(m_aiPlayers.back());
					++aiAdded;
				}
			}
		}

		m_engine.setAlwaysWait(aiAdded > 1);
	}

	gameReady();
}

Game::~Game() {

	for(std::vector<NetMauMau::Player::IPlayer *>::const_iterator i(m_players.begin());
			i != m_players.end(); ++i) {
		delete *i;
	}

	for(std::vector<NetMauMau::Player::StdPlayer *>::const_iterator i(m_aiPlayers.begin());
			i != m_aiPlayers.end(); ++i) {
		delete *i;
	}
}

Game::COLLECT_STATE Game::collectPlayers(std::size_t minPlayers,
		NetMauMau::Player::IPlayer *player) {

	if(m_engine.getPlayerCount() < std::max<std::size_t>(2, minPlayers)) {

		if(!addPlayer(player)) return Game::REFUSED;

		NetMauMau::DB::SQLite::getInstance().addPlayerToGame(m_gameIndex,
				m_engine.getEventHandler().getConnection()->getPlayerInfo(player->getSerial()));

		if(m_engine.getPlayerCount() == std::max<std::size_t>(2, minPlayers)) {

			for(std::vector<NetMauMau::Player::StdPlayer *>::const_iterator i(m_aiPlayers.begin());
					i != m_aiPlayers.end(); ++i) {

				NetMauMau::DB::SQLite::getInstance().addAIPlayer(*i);
				NetMauMau::DB::SQLite::getInstance().addPlayerToGame(m_gameIndex,
						NetMauMau::Common::IConnection::NAMESOCKFD((*i)->getName(), "",
								INVALID_SOCKET, MAKE_VERSION(SERVER_VERSION_MAJOR,
										SERVER_VERSION_MINOR)));
			}

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

	if(m_cfg.getAIPlayer()) m_engine.setFirstPlayer(m_players.back());

	m_engine.distributeCards();
	m_engine.setUltimate(ultimate);
	m_engine.gameAboutToStart();

	try {
		while(ultimate ? m_engine.getPlayerCount() >= 2 : m_engine.getPlayerCount() == minPlayers) {
			if(!m_engine.nextTurn()) break;
		}

		if(ultimate || m_cfg.getAIPlayer()) m_engine.gameOver();

	} catch(NetMauMau::Lua::Exception::LuaFatalException &e) {
		logFatal(e);
		m_engine.error(MISCONFIGURED + e.what());
		m_engine.gameOver();
	}

	reset(false);
}

void Game::removePlayer(const std::string &player) {
	m_engine.removePlayer(player);
}

void Game::reset(bool playerLost) throw() {

	++m_gameServed;

	if(playerLost) m_engine.error("Lost connection to a waiting player.");

	m_engine.reset();

	if(m_cfg.getAIPlayer()) {

		try {

			for(std::vector<NetMauMau::Player::StdPlayer *>::const_iterator i(m_aiPlayers.begin());
					i != m_aiPlayers.end(); ++i) {

				(*i)->reset();

				NetMauMau::DB::SQLite::getInstance().
				logOutPlayer(NetMauMau::Common::IConnection::NAMESOCKFD((*i)->getName(), "",
							 INVALID_SOCKET, MAKE_VERSION(SERVER_VERSION_MAJOR,
									 SERVER_VERSION_MINOR)));

				m_engine.addPlayer(*i);
			}

			m_engine.setAlwaysWait(m_aiPlayers.size() > 1);

		} catch(const NetMauMau::Common::Exception::SocketException &e) {
			logDebug(__PRETTY_FUNCTION__ << ": failed to add AI player: " << e.what());
		}
	}

	char sr[128];

	std::snprintf(sr, 127, "Received %.2f kb; sent %.2f kb", static_cast<double>(NetMauMau::Common::
				  AbstractSocket::getReceivedBytes()) / 1024.0,
				  static_cast<double>(NetMauMau::Common::AbstractSocket::getSentBytes()) / 1024.0);

	logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << sr);

	NetMauMau::Common::AbstractSocket::resetReceivedBytes();
	NetMauMau::Common::AbstractSocket::resetSentBytes();

	gameReady();
}

void Game::gameReady() {
	NetMauMau::DB::SQLite::getInstance().gameEnded(m_gameIndex);
	logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Ready for new game...");
	m_engine.setGameId(m_gameIndex = NetMauMau::DB::SQLite::getInstance().newGame());
}

void Game::shutdown(const std::string &reason) const throw() {
	m_engine.error(reason.empty() ? "The server has been shut down" : (MISCONFIGURED + reason));
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
