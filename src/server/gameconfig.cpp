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

#include "gameconfig.h"

namespace {

std::size_t countAI(const std::string *aiNames) {

	std::size_t cnt = 0;

	for(int i = 0; i < 4; ++i) {
		if(!aiNames[i].empty()) ++cnt;
	}

	return cnt;
}

}

using namespace NetMauMau::Server;

GameConfig::GameConfig(NetMauMau::Event::IEventHandler &evtHdlr, long aiDelay, bool dirChange,
					   bool aiPlayer, const std::string *aiName, char aceRound, std::size_t factor,
					   std::size_t initialCardCount) : m_aiPlayer(aiPlayer), m_aiName(aiName),
	m_engineCfg(evtHdlr, dirChange, aiDelay, !aiPlayer || countAI(aiName) > 1, aceRound, factor,
				initialCardCount) {}

GameConfig::GameConfig(const GameConfig &o) : m_aiPlayer(o.m_aiPlayer), m_aiName(o.m_aiName),
	m_engineCfg(o.m_engineCfg) {}

GameConfig::~GameConfig() {}

const std::string *GameConfig::getAIName() const {
	return m_aiName;
}

bool GameConfig::getAIPlayer() const {
	return m_aiPlayer;
}

NetMauMau::EngineConfig &GameConfig::getEngineConfig() {
	return m_engineCfg;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
