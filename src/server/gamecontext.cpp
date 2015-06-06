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

#include "gamecontext.h"

using namespace NetMauMau::Server;

GameContext::GameContext(NetMauMau::Event::IEventHandler &evtHdlr, long aiDelay, bool dirChange,
						 NetMauMau::Common::CARDCONFIG &cc, bool aiPlayer,
						 const std::vector<std::string> &aiName, char aceRound)
	: m_aiPlayer(aiPlayer), m_aiNames(aiName), m_engineCtx(evtHdlr, dirChange, aiDelay,
			!aiPlayer || !getAINames().empty(), aceRound, cc), m_cardConfig(cc) {}

GameContext::GameContext(const GameContext &o) : m_aiPlayer(o.m_aiPlayer), m_aiNames(o.m_aiNames),
	m_engineCtx(o.m_engineCtx), m_cardConfig(o.m_cardConfig) {}

GameContext::~GameContext() {}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
