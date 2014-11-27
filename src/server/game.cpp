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

#include "game.h"
#include "logger.h"
#include "abstractconnection.h"

namespace {
const char *GAMEREADY = "Ready for new game...";

std::size_t countAI(const std::string *aiNames) {

	std::size_t cnt = 0;

	for(int i = 0; i < 4; ++i) {
		if(!aiNames[i].empty()) ++cnt;
	}

	return cnt;
}

}

using namespace NetMauMau::Server;

Game::Game(NetMauMau::Event::IEventHandler &evtHdlr, long aiDelay, bool aiPlayer,
		   const std::string *aiName) : m_engine(evtHdlr, aiDelay,
					   !aiPlayer || countAI(aiName) > 1), m_aiOpponent(aiPlayer), m_aiPlayers(),
	m_players() {

	m_players.reserve(50);

	if(aiPlayer) {

		std::size_t aiAdded = 0;

		for(int i = 0; i < 4; ++i) {
			if(!aiName[i].empty()) {
				m_aiPlayers.push_back(new Player::StdPlayer(aiName[i]));
				logInfo("Adding AI player \"" << m_aiPlayers.back()->getName() << "\"");
				m_engine.addPlayer(m_aiPlayers.back());
				++aiAdded;
			}
		}

		m_engine.setAlwaysWait(aiAdded > 1);
	}

	logInfo(GAMEREADY);
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

		if(!addPlayer(player)) return REFUSED;

		if(m_engine.getPlayerCount() == std::max<std::size_t>(2, minPlayers)) {
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

	if(m_engine.addPlayer(m_players.back())) {
		return true;
	} else {
		delete m_players.back();
		m_players.pop_back();
		return false;
	}
}

void Game::start(bool ultimate) throw(NetMauMau::Common::Exception::SocketException) {

	const std::size_t minPlayers = m_engine.getPlayerCount();

	if(m_aiOpponent) m_engine.setFirstPlayer(m_players.back());

	m_engine.distributeCards();
	m_engine.setUltimate(ultimate);

	while(ultimate ? m_engine.getPlayerCount() >= 2 : m_engine.getPlayerCount() == minPlayers) {
		if(!m_engine.nextTurn()) break;
	}

	if(ultimate || m_aiOpponent) m_engine.gameOver();

	reset();
}

void Game::reset() throw() {

	m_engine.reset();

	if(m_aiOpponent) {

		try {

			for(std::vector<NetMauMau::Player::StdPlayer *>::const_iterator i(m_aiPlayers.begin());
					i != m_aiPlayers.end(); ++i) {
				(*i)->resetJackState();
				(*i)->reset();
				m_engine.addPlayer(*i);
			}

			m_engine.setAlwaysWait(m_aiPlayers.size() > 1);

		} catch(const NetMauMau::Common::Exception::SocketException &e) {
			logDebug(__PRETTY_FUNCTION__ << ": failed to add AI player: " << e.what());
		}
	}

	logInfo(GAMEREADY);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
