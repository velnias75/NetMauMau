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

#include "aistate.h"

#include "iruleset.h"

using namespace NetMauMau::Engine::AIDT;

AIState::AIState(const NetMauMau::Player::IPlayer::CARDS &cards,
				 const NetMauMau::Common::ICardPtr &uc, const IRuleSetPtr &ruleSet) : m_card(),
	m_cards(cards), m_uncoveredCard(uc), m_ruleSet(ruleSet), m_playedOutCards(),
	m_cardsTaken(false) {}

AIState::~AIState() {}

AIState::IRuleSetPtr AIState::getRuleSet() const {
	return m_ruleSet;
}

void AIState::setCard(const NetMauMau::Common::ICardPtr &card) {
	m_card = card;
}

#warning TODO implement 'AIState::getTalonFactor()' using 'EngineConfig'
std::size_t AIState::getTalonFactor() const {
	return 1;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
