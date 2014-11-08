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
	m_takeCardCount(0), m_jackMode(false), m_jackSuit(NetMauMau::Common::ICard::HEARTS) {}

StdRuleSet::~StdRuleSet() {}

std::size_t StdRuleSet::getMaxPlayers() const {
	return 5;
}

void StdRuleSet::checkInitial(const NetMauMau::Player::IPlayer *player,
							  const NetMauMau::Common::ICard *playedCard) {
	checkCard(player, 0L, playedCard, false);
}

bool StdRuleSet::checkCard(const NetMauMau::Player::IPlayer *player,
						   const NetMauMau::Common::ICard *uncoveredCard,
						   const NetMauMau::Common::ICard *playedCard, bool ai) {

	const bool accepted = uncoveredCard ? (playedCard->getRank() == NetMauMau::Common::ICard::JACK
										   && uncoveredCard->getRank() !=
										   NetMauMau::Common::ICard::JACK) ||
						  ((((isJackMode() && getJackSuit() == playedCard->getSuit()) ||
							 (!isJackMode() && (uncoveredCard->getSuit() == playedCard->getSuit() ||
									 (uncoveredCard->getRank() == playedCard->getRank()))))) &&
						   !(playedCard->getRank() == NetMauMau::Common::ICard::JACK &&
							 uncoveredCard->getRank() == NetMauMau::Common::ICard::JACK)) : true;

	m_hasToSuspend = accepted && playedCard->getRank() == NetMauMau::Common::ICard::EIGHT;
	m_hasSuspended = false;

	if(accepted && playedCard->getRank() == NetMauMau::Common::ICard::SEVEN) {
		m_takeCardCount += 2;
	} else if(accepted && playedCard->getRank() == NetMauMau::Common::ICard::JACK) {

		if(!(ai && !player->isAIPlayer() && player->getCardCount() == 1)) {
			m_jackSuit = player->getJackChoice(uncoveredCard ? uncoveredCard :
											   playedCard, playedCard);
		} else {
			m_jackSuit = NetMauMau::Common::ICard::HEARTS;
		}

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

std::size_t StdRuleSet::takeCards(const NetMauMau::Common::ICard *playedCard) const {
	return (playedCard && playedCard->getRank() == NetMauMau::Common::ICard::SEVEN) ? 0 :
		   m_takeCardCount;
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

NetMauMau::Common::ICard::SUIT StdRuleSet::getJackSuit() const {
	return m_jackSuit;
}

void StdRuleSet::reset() {
	m_hasToSuspend = false;
	m_hasSuspended = false;
	m_takeCardCount = 0;
	m_jackMode = false;
	m_jackSuit = NetMauMau::Common::ICard::HEARTS;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
