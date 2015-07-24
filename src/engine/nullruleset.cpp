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

#include "nullruleset.h"

using namespace NetMauMau::RuleSet;

NullRuleSet::NullRuleSetPtr NullRuleSet::m_instance;

NullRuleSet::NullRuleSet() : IRuleSet() {}

NullRuleSet::~NullRuleSet() {}

NullRuleSet::NullRuleSetPtr NullRuleSet::getInstance() {

	if(!m_instance) m_instance = NullRuleSetPtr(new NullRuleSet());

	return m_instance;
}

bool NullRuleSet::isNull() const {
	return true;
}

void NullRuleSet::checkInitial(const NetMauMau::Player::IPlayer *,
							   const NetMauMau::Common::ICardPtr &) {}

bool NullRuleSet::checkCard(const NetMauMau::Player::IPlayer *,
							const NetMauMau::Common::ICardPtr &,
							const NetMauMau::Common::ICardPtr &, bool) {
	return false;
}

bool NullRuleSet::checkCard(const NetMauMau::Common::ICardPtr &,
							const NetMauMau::Common::ICardPtr &) const {
	return false;
}

std::size_t NullRuleSet::lostPointFactor(const NetMauMau::Common::ICardPtr &) const {
	return 0u;
}

bool NullRuleSet::hasToSuspend() const {
	return false;
}

void NullRuleSet::hasSuspended() {}

void NullRuleSet::hasTakenCards() {}

std::size_t NullRuleSet::takeCardCount() const {
	return 0u;
}

std::size_t NullRuleSet::takeCards(const NetMauMau::Common::ICard *) const {
	return 0u;
}

std::size_t NullRuleSet::initialCardCount() const {
	return 0;
}

bool NullRuleSet::takeAfterSevenIfNoMatch() const {
	return false;
}

bool NullRuleSet::takeIfLost() const {
	return false;
}

bool NullRuleSet::isAceRoundPossible() const {
	return false;
}

NetMauMau::Common::ICard::RANK NullRuleSet::getAceRoundRank() const {
	return NetMauMau::Common::ICard::RANK_ILLEGAL;
}

bool NullRuleSet::hasDirChange() const {
	return false;
}

void NullRuleSet::dirChanged() {}

bool NullRuleSet::getDirChangeIsSuspend() const {
	return false;
}

void NullRuleSet::setDirChangeIsSuspend(bool) {}

bool NullRuleSet::isAceRound() const {
	return false;
}

bool NullRuleSet::isJackMode() const {
	return false;
}

NetMauMau::Common::ICard::SUIT NullRuleSet::getJackSuit() const {
	return 	NetMauMau::Common::ICard::SUIT_ILLEGAL;
}

void NullRuleSet::setJackModeOff() {}

std::size_t NullRuleSet::getMaxPlayers() const {
	return 0u;
}

void NullRuleSet::setCurPlayers(std::size_t) {}

void NullRuleSet::reset() throw() {}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
