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

using namespace NetMauMau::Server;

GameConfig::GameConfig(NetMauMau::Event::IEventHandler &evtHdlr, long aiDelay, bool dirChange,
					   NetMauMau::Common::CARDCONFIG &cc, bool aiPlayer,
					   const std::vector<std::string> &aiName, char aceRound)
	: m_aiPlayer(aiPlayer), m_aiName(aiName), m_engineCfg(evtHdlr, dirChange, aiDelay,
			!aiPlayer || !getAINames().empty(), aceRound, cc), m_cardConfig(cc) {}

GameConfig::GameConfig(const GameConfig &o) : m_aiPlayer(o.m_aiPlayer), m_aiName(o.m_aiName),
	m_engineCfg(o.m_engineCfg), m_cardConfig(o.m_cardConfig) {}

GameConfig::~GameConfig() {}

const std::vector<std::string> &GameConfig::getAINames() const {
	return m_aiName;
}

bool GameConfig::hasAIPlayer() const {
	return m_aiPlayer;
}

NetMauMau::EngineConfig &GameConfig::getEngineConfig() {
	return m_engineCfg;
}

NetMauMau::Common::CARDCONFIG &GameConfig::getCardConfig() const {
	return m_cardConfig;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
