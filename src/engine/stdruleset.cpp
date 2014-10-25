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

#include "stdruleset.h"

#include "iplayer.h"

using namespace NetMauMau::RuleSet;

StdRuleSet::StdRuleSet() : IRuleSet(), m_hasToSuspend(false), m_hasSuspended(false),
	m_takeCardCount(0), m_jackMode(false), m_jackSuite(NetMauMau::ICard::HEART) {}

StdRuleSet::~StdRuleSet() {}

std::size_t StdRuleSet::getMaxPlayers() const {
	return 5;
}

void StdRuleSet::checkInitial(const NetMauMau::Player::IPlayer *player,
							  const NetMauMau::ICard *playedCard) {
	checkCard(player, 0L, playedCard);
}

bool StdRuleSet::checkCard(const NetMauMau::Player::IPlayer *player,
						   const NetMauMau::ICard *uncoveredCard,
						   const NetMauMau::ICard *playedCard) {

	const bool accepted = uncoveredCard ? (playedCard->getValue() == NetMauMau::ICard::JACK &&
										   uncoveredCard->getValue() != NetMauMau::ICard::JACK) ||
						  ((((isJackMode() && getJackSuite() == playedCard->getSuite()) ||
							 (!isJackMode() &&
							  (uncoveredCard->getSuite() == playedCard->getSuite() ||
							   (uncoveredCard->getValue() == playedCard->getValue()))))) &&
						   !(playedCard->getValue() == NetMauMau::ICard::JACK &&
							 uncoveredCard->getValue() == NetMauMau::ICard::JACK)) : true;

	m_hasToSuspend = accepted && playedCard->getValue() == NetMauMau::ICard::EIGHT;
	m_hasSuspended = false;

	if(accepted && playedCard->getValue() == NetMauMau::ICard::SEVEN) {
		m_takeCardCount += 2;
	} else if(accepted && playedCard->getValue() == NetMauMau::ICard::JACK) {
		m_jackSuite = player->getJackChoice(uncoveredCard ? uncoveredCard : playedCard, playedCard);
		m_jackMode = true;
	}

	return accepted;
}

bool StdRuleSet::hasToSuspend() const {
	return m_hasToSuspend && !m_hasSuspended;
}

void StdRuleSet::hasSuspended() {
	m_hasSuspended = true;
}

std::size_t StdRuleSet::takeCards(const ICard *playedCard) const {
	return (playedCard && playedCard->getValue() == NetMauMau::ICard::SEVEN) ? 0 : m_takeCardCount;
}

void StdRuleSet::hasTakenCards() {
	m_takeCardCount = 0;
}

bool StdRuleSet::suspendIfNoMatchingCard() const {
	return false;
}

bool StdRuleSet::isJackMode() const {
	return m_jackMode;
}

void StdRuleSet::setJackModeOff() {
	m_jackMode = false;
}

NetMauMau::ICard::SUITE StdRuleSet::getJackSuite() const {
	return m_jackSuite;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
