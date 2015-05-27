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

#include "game.h"

#include <cstdio>

#include "abstractsocket.h"             // for AbstractSocket
#include "easyplayer.h"                 // for EasyPlayer
#include "gameconfig.h"                 // for GameConfig
#include "ieventhandler.h"              // for IEventHandler
#include "logger.h"
#include "luafatalexception.h"          // for LuaFatalException
#include "sqlite.h"                     // for SQLite

namespace {
const std::string MISCONFIGURED("Misconfigured or compromised server. Please report: ");
}

using namespace NetMauMau::Server;

long Game::m_gameServed = 0L;

Game::Game(GameConfig &cfg) throw(NetMauMau::Common::Exception::SocketException)
	: m_cfg(cfg), m_engine(cfg.getEngineConfig()), m_aiPlayers(), m_players(), m_gameIndex(0LL) {

	const std::size_t orgAI = cfg.getAINames().size();
	const std::size_t maxPl = cfg.getEngineConfig().getRuleSet(&m_engine)->getMaxPlayers();
	const std::size_t cntAi = std::min(orgAI, maxPl - 1);

	if(cfg.hasAIPlayer() && cntAi <= m_aiPlayers.max_size()) m_aiPlayers.reserve(cntAi);

	if(maxPl <= m_players.max_size()) m_players.reserve(maxPl);

	if(cfg.hasAIPlayer()) {

		std::size_t aiAdded = 0;

		if(cntAi != orgAI) logWarning("Limiting number of AI players to " << cntAi
										  << " (due to configuration limit).");

		for(std::size_t i = 0; i < cntAi; ++i) {

			if(!cfg.getAINames()[i].empty()) {

				const std::string &aiSanName(cfg.getAINames()[i][0] == '+' ?
											 & (cfg.getAINames()[i][1]) : cfg.getAINames()[i]);

				if(!aiSanName.empty()) {

					const std::string::size_type spos = aiSanName.rfind(':');

					NetMauMau::Player::IPlayer::TYPE type = NetMauMau::Player::IPlayer::HARD;

					if(spos != std::string::npos && aiSanName.length() > spos &&
							(aiSanName.substr(spos + 1)[0] == 'e' ||
							 aiSanName.substr(spos + 1)[0] == 'E')) {
						type = NetMauMau::Player::IPlayer::EASY;
					}

					m_aiPlayers.push_back(type == NetMauMau::Player::IPlayer::HARD ?
										  new NetMauMau::Player::HardPlayer(aiSanName.
												  substr(0, spos), m_engine.getPlayedOutCards()) :
										  new NetMauMau::Player::EasyPlayer(aiSanName.
												  substr(0, spos), m_engine.getPlayedOutCards()));

					logInfo("Adding AI player \"" << m_aiPlayers.back()->getName() << "\" ("
							<< (m_aiPlayers.back()->getType() ==
								NetMauMau::Player::IPlayer::HARD ? "hard" : "easy") << ")");
					m_engine.addPlayer(m_aiPlayers.back());
					++aiAdded;
				}
			}
		}

		if(cntAi != orgAI) cfg.getCardConfig() = NetMauMau::Common::getCardConfig(maxPl);

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

		NetMauMau::DB::SQLite::getInstance()->addPlayerToGame(m_gameIndex,
				m_engine.getEventHandler().getConnection().getPlayerInfo(player->getSerial()));

		if(m_engine.getPlayerCount() == minPlayers) {

			for(std::vector<NetMauMau::Player::AbstractPlayer *>::const_iterator
					i(m_aiPlayers.begin()); i != m_aiPlayers.end(); ++i) {

				NetMauMau::DB::SQLite::getInstance()->addAIPlayer(*i);
				NetMauMau::DB::SQLite::getInstance()->addPlayerToGame(m_gameIndex,
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

	if(m_cfg.hasAIPlayer()) m_engine.setFirstPlayer(m_players.back());

	m_engine.distributeCards();
	m_engine.setUltimate(ultimate);
	m_engine.gameAboutToStart();

	try {
		while(ultimate ? m_engine.getPlayerCount() >= 2 : m_engine.getPlayerCount() == minPlayers) {
			if(!m_engine.nextTurn()) break;
		}

		if(ultimate || m_cfg.hasAIPlayer()) m_engine.gameOver();

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

	if(m_cfg.hasAIPlayer()) {

		try {

			for(std::vector<NetMauMau::Player::AbstractPlayer *>::const_iterator i(m_aiPlayers.begin());
					i != m_aiPlayers.end(); ++i) {

				(*i)->reset();

				NetMauMau::DB::SQLite::getInstance()->
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
	NetMauMau::DB::SQLite::getInstance()->gameEnded(m_gameIndex);
	logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Ready for new game...");
	m_engine.setGameId(m_gameIndex = NetMauMau::DB::SQLite::getInstance()->newGame());
}

void Game::shutdown(const std::string &reason) const throw() {
	m_engine.error(reason.empty() ? "The server has been shut down" : (MISCONFIGURED + reason));
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
