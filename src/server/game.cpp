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

using namespace NetMauMau::Server;

Game::Game(NetMauMau::Event::IEventHandler &evtHdlr, bool aiPlayer, const std::string &aiName) :
	m_engine(evtHdlr, !aiPlayer), m_aiOpponent(aiPlayer), m_aiPlayer(aiName), m_players() {

	m_players.reserve(50);

	if(aiPlayer) {
		logInfo("Adding AI player \"" << m_aiPlayer.getName() << "\"");
		m_engine.addPlayer(&m_aiPlayer);
	}

	logInfo("Ready for new game...");
}

Game::~Game() {
	for(std::vector<NetMauMau::Player::IPlayer *>::const_iterator i(m_players.begin());
			i != m_players.end(); ++i) {
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

void Game::start() throw(NetMauMau::Common::Exception::SocketException) {

	const std::size_t minPlayers = m_engine.getPlayerCount();

	if(m_aiOpponent) m_engine.reversePlayers();

	m_engine.distributeCards();

	while(m_engine.getPlayerCount() == minPlayers) {
		if(!m_engine.nextTurn()) break;
	}

	reset();
}

void Game::reset() throw(NetMauMau::Common::Exception::SocketException) {

	m_engine.reset();
	m_aiPlayer.resetJackState();

	if(m_aiOpponent) {
		m_aiPlayer.reset();
		m_engine.addPlayer(&m_aiPlayer);
	}

	logInfo("Ready for new game...");
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
