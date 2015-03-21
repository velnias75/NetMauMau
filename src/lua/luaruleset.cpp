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

#include "luaruleset.h"

using namespace NetMauMau::RuleSet;

LuaRuleSet::LuaRuleSet(bool dirChangePossible, std::size_t icc,
					   const NetMauMau::IAceRoundListener *l) : IRuleSet() {}

LuaRuleSet::~LuaRuleSet() {}

void LuaRuleSet::checkInitial(const NetMauMau::Player::IPlayer *player,
							  const NetMauMau::Common::ICard *playedCard) {}

bool LuaRuleSet::checkCard(const NetMauMau::Player::IPlayer *player,
						   const NetMauMau::Common::ICard *uncoveredCard,
						   const NetMauMau::Common::ICard *playedCard, bool ai) {
	return false;
}

bool LuaRuleSet::checkCard(const NetMauMau::Common::ICard *uncoveredCard,
						   const NetMauMau::Common::ICard *playedCard) const {
	return false;
}

std::size_t LuaRuleSet::lostPointFactor(const NetMauMau::Common::ICard *uncoveredCard) const {
	return 0;
}

bool LuaRuleSet::hasToSuspend() const {
	return false;
}

void LuaRuleSet::hasSuspended() {}

std::size_t LuaRuleSet::takeCardCount() const {
	return 0;
}

std::size_t LuaRuleSet::takeCards(const NetMauMau::Common::ICard *playedCard) const {
	return 0;
}

void LuaRuleSet::hasTakenCards() {}

std::size_t LuaRuleSet::initialCardCount() const {
	return 0;
}

bool LuaRuleSet::suspendIfNoMatchingCard() const {
	return false;
}

bool LuaRuleSet::takeIfLost() const {
	return false;
}

bool LuaRuleSet::isAceRoundPossible() const {
	return false;
}

NetMauMau::Common::ICard::RANK LuaRuleSet::getAceRoundRank() const {
	return NetMauMau::Common::ICard::RANK_ILLEGAL;
}

bool LuaRuleSet::hasDirChange() const {
	return false;
}

void LuaRuleSet::dirChanged() {}

bool LuaRuleSet::getDirChangeIsSuspend() const {
	return false;
}

void LuaRuleSet::setDirChangeIsSuspend(bool suspend) {}

bool LuaRuleSet::isAceRound() const {
	return false;
}

bool LuaRuleSet::isJackMode() const {
	return false;
}

NetMauMau::Common::ICard::SUIT LuaRuleSet::getJackSuit() const {
	return NetMauMau::Common::ICard::SUIT_ILLEGAL;
}

void LuaRuleSet::setJackModeOff() {}

std::size_t LuaRuleSet::getMaxPlayers() const {
	return 0;
}

void LuaRuleSet::setCurPlayers(std::size_t players) {}

void LuaRuleSet::reset() throw() {}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
