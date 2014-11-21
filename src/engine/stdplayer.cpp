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

#include <cstring>

#include "stdplayer.h"

#include "cardtools.h"
#include "random_gen.h"
#include "stdcardfactory.h"

namespace {

const NetMauMau::Common::ICard::SUIT SUIT[4] = {
	NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::CLUBS
};

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct cardGreater : std::binary_function < NetMauMau::Common::ICard *, NetMauMau::Common::ICard *,
		bool > {

	cardGreater(const NetMauMau::Player::IPlayer::CARDS &c) : cards(c) {}

	bool operator()(const NetMauMau::Common::ICard *x, NetMauMau::Common::ICard *y) const {
		return !(std::count_if(cards.begin(), cards.end(),
							   std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank), x->getRank()))
				 < std::count_if(cards.begin(), cards.end(),
								 std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
											  y->getRank())));
	}

private:
	const NetMauMau::Player::IPlayer::CARDS &cards;
};
#pragma GCC diagnostic pop

}

using namespace NetMauMau::Player;

bool StdPlayer::m_jackPlayed = false;

StdPlayer::StdPlayer(const std::string &name) : m_name(name), m_cards(), m_cardsTaken(false),
	m_ruleset(0), m_playerHasFewCards(false), m_powerSuit(NetMauMau::Common::ICard::SUIT_ILLEGAL) {
	m_cards.reserve(32);
}

StdPlayer::~StdPlayer() {
	for(CARDS::const_iterator i(m_cards.begin()); i != m_cards.end(); ++i) {
		delete *i;
	}
}

void StdPlayer::reset() throw() {
	m_powerSuit = NetMauMau::Common::ICard::SUIT_ILLEGAL;
	m_cardsTaken = false;
	m_cards.clear();
}

void StdPlayer::resetJackState() throw() {
	m_jackPlayed = false;
}

std::string StdPlayer::getName() const {
	return m_name;
}

int StdPlayer::getSerial() const {
	return -1;
}

bool StdPlayer::isAIPlayer() const {
	return true;
}

void StdPlayer::setRuleSet(const NetMauMau::RuleSet::IRuleSet *ruleset) {
	m_ruleset = ruleset;
}

const NetMauMau::RuleSet::IRuleSet *StdPlayer::getRuleSet() const {
	return m_ruleset;
}

void StdPlayer::receiveCard(NetMauMau::Common::ICard *card) {
	if(card) m_cards.push_back(card);
}

void StdPlayer::receiveCardSet(const CARDS &cards) {
	m_cards.insert(m_cards.end(), cards.begin(), cards.end());
	shuffleCards();
}

void StdPlayer::shuffleCards() {
	std::random_shuffle(m_cards.begin(), m_cards.end(),
						NetMauMau::Common::genRandom<CARDS::difference_type>);
}

void StdPlayer::countSuits(SUITCOUNT *suitCount, const CARDS &myCards) const {

	std::memset(suitCount, 0, sizeof(SUITCOUNT) * 4);

	const bool noCards = myCards.empty();

	for(std::size_t i = 0; i < 4; ++i) {
		SUITCOUNT sc = { SUIT[i], noCards ? 0 : std::count_if(myCards.begin(), myCards.end(),
						 std::bind2nd(std::ptr_fun(NetMauMau::Common::isSuit), SUIT[i]))
					   };

		suitCount[i] = sc;
	}

	if(!noCards) std::sort(suitCount, suitCount + 4);
}

NetMauMau::Common::ICard *
StdPlayer::hasEightPath(const NetMauMau::Common::ICard *uc, NetMauMau::Common::ICard::SUIT s,
						CARDS &cards) const {

	if(cards.size() > 1) {

		const CARDS::iterator &e(std::partition(cards.begin(), cards.end(),
												std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
														NetMauMau::Common::ICard::EIGHT)));

		if(std::distance(cards.begin(), e)) {

			CARDS::value_type f_src(NetMauMau::Common::findSuit(uc->getSuit(), cards.begin(), e));

			if(f_src) {
				CARDS::value_type f_dest(NetMauMau::Common::findSuit(s, cards.begin(), e));

				if(f_dest) return f_src;
			}
		}
	}

	return 0L;
}

NetMauMau::Common::ICard *StdPlayer::findBestCard(const NetMauMau::Common::ICard *uc,
		const NetMauMau::Common::ICard::SUIT *js, bool noJack) const {

	CARDS myCards(m_cards);

	if(noJack) {
		myCards.erase(std::remove_if(myCards.begin(), myCards.end(),
									 std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
											 NetMauMau::Common::ICard::JACK)), myCards.end());
	}

	NetMauMau::Common::ICard *bestCard = 0L;

	if(uc->getRank() == NetMauMau::Common::ICard::SEVEN) {

		const CARDS::iterator &e(js ? std::partition(myCards.begin(), myCards.end(),
								 std::bind2nd(std::ptr_fun(NetMauMau::Common::isSuit), *js)) :
								 myCards.end());

		CARDS::value_type f = NetMauMau::Common::findRank(NetMauMau::Common::ICard::SEVEN,
							  myCards.begin(), e);

		if(f) {
			bestCard = f;
		} else if(!m_cardsTaken) {
			bestCard = NetMauMau::Common::getIllegalCard();
			m_cardsTaken = true;
		}
	}


	if(!bestCard && !noJack && m_playerHasFewCards && std::count_if(myCards.begin(), myCards.end(),
			std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
						 NetMauMau::Common::ICard::SEVEN)) &&
			std::count_if(myCards.begin(), myCards.end(),
						  std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
									   NetMauMau::Common::ICard::JACK))) {

		m_powerSuit = NetMauMau::Common::findRank(NetMauMau::Common::ICard::SEVEN, myCards.begin(),
					  myCards.end())->getSuit();

	} else {
		m_powerSuit = NetMauMau::Common::ICard::SUIT_ILLEGAL;
	}

	if(m_powerSuit == NetMauMau::Common::ICard::SUIT_ILLEGAL) {

		if(!bestCard) {
			const CARDS::iterator
			&e(std::partition(myCards.begin(), myCards.end(),
							  std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
										   NetMauMau::Common::ICard::SEVEN)));

			CARDS::value_type f = NetMauMau::Common::findSuit(js ? *js : uc->getSuit(),
								  myCards.begin(), e);

			if(f) bestCard = f;
		}

		if(!bestCard) {

			SUITCOUNT suitCount[4];
			countSuits(suitCount, myCards);

			for(std::size_t i = 0; i < 4; ++i) {

				const CARDS::iterator
				&e(std::partition(myCards.begin(), myCards.end(),
								  std::bind2nd(std::ptr_fun(NetMauMau::Common::isSuit),
											   suitCount[i].suit)));

				if(js) {
					if(suitCount[i].count && (suitCount[i].suit == *js)) {
						std::partition(myCards.begin(), e,
									   std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
													NetMauMau::Common::ICard::SEVEN));
						bestCard = myCards.front();
						break;
					}
				} else {

					if(!(bestCard = hasEightPath(uc, suitCount[i].suit, myCards))) {

						std::sort(myCards.begin(), e, cardGreater(myCards));

						std::stable_partition(myCards.begin(), e,
											  std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
														   NetMauMau::Common::ICard::SEVEN));

						CARDS::value_type f = NetMauMau::Common::findRank(uc->getRank(),
											  myCards.begin(), e);

						if(f) {
							bestCard = f;
							break;
						}
					}
				}
			}

			if(!bestCard) {
				const CARDS::iterator
				&e(std::partition(myCards.begin(), myCards.end(),
								  std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
											   NetMauMau::Common::ICard::SEVEN)));

				if(!noJack) std::partition(e, myCards.end(), std::not1
											   (std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
													   NetMauMau::Common::ICard::JACK)));

				CARDS::value_type f = NetMauMau::Common::findSuit(js ? *js : uc->getSuit(),
									  myCards.begin(), myCards.end());

				if(f && f->getRank() != NetMauMau::Common::ICard::JACK) bestCard = f;
			}
		}
	}

	if(!noJack && (!bestCard || m_powerSuit != NetMauMau::Common::ICard::SUIT_ILLEGAL)) {

		const CARDS::size_type jackCnt = std::count_if(myCards.begin(), myCards.end(),
										 std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
												 NetMauMau::Common::ICard::JACK));

		if(jackCnt) {
			std::partition(myCards.begin(), myCards.end(),
						   std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
										NetMauMau::Common::ICard::JACK));

			bestCard = myCards[NetMauMau::Common::genRandom<CARDS::difference_type>(jackCnt)];
		}
	}

	return bestCard;
}

NetMauMau::Common::ICard *StdPlayer::requestCard(const NetMauMau::Common::ICard *uc,
		const NetMauMau::Common::ICard::SUIT *js) const {

	NetMauMau::Common::ICard *bestCard = findBestCard(uc, js, false);

	if(bestCard && bestCard->getRank() == NetMauMau::Common::ICard::JACK &&
			uc->getRank() == NetMauMau::Common::ICard::JACK) {
		bestCard = findBestCard(uc, js, true);
	}

	return bestCard;
}

NetMauMau::Common::ICard::SUIT
StdPlayer::getJackChoice(const NetMauMau::Common::ICard *uncoveredCard,
						 const NetMauMau::Common::ICard *playedCard) const {

	if(m_powerSuit != NetMauMau::Common::ICard::SUIT_ILLEGAL) {
		NetMauMau::Common::ICard::SUIT s = m_powerSuit;
		m_powerSuit = NetMauMau::Common::ICard::SUIT_ILLEGAL;
		return s;
	}

	if(m_cards.size() < 8) {

		NetMauMau::Common::ICard *bc = 0L;

		if((bc = findBestCard(uncoveredCard, 0L, true))) {
			return bc->getSuit();
		}
	}

	NetMauMau::Common::ICard::SUIT s = findJackChoice();

	if(s == NetMauMau::Common::ICard::SUIT_ILLEGAL) {
		while(((s = SUIT[NetMauMau::Common::genRandom<std::size_t>(4)]) == uncoveredCard->getSuit()
				|| s == playedCard->getSuit()));
	}

	return s;
}

NetMauMau::Common::ICard::SUIT StdPlayer::findJackChoice() const {

	SUITCOUNT poSuitCount[4];
	CARDS pcVec;

	pcVec.reserve(m_playedOutCards.size());

	for(std::set<std::string>::const_iterator i(m_playedOutCards.begin());
			i != m_playedOutCards.end(); ++i) {

		NetMauMau::Common::ICard::SUIT s;
		NetMauMau::Common::ICard::RANK r;

		if(NetMauMau::Common::parseCardDesc(*i, &s, &r)) {
			pcVec.push_back(NetMauMau::StdCardFactory().create(s, r));
		}
	}

	countSuits(poSuitCount, pcVec);

	for(CARDS::const_iterator i(pcVec.begin()); i != pcVec.end(); ++i) delete *i;

	if(poSuitCount[0].count) {
		return poSuitCount[0].suit;
	}

	return NetMauMau::Common::ICard::SUIT_ILLEGAL;
}

IPlayer::REASON StdPlayer::getNoCardReason() const {
	return m_cards.empty() ? MAUMAU : NOMATCH;
}

bool StdPlayer::cardAccepted(const NetMauMau::Common::ICard *playedCard) {

	const CARDS::iterator &i(std::find(m_cards.begin(), m_cards.end(), playedCard));

	if(i != m_cards.end()) m_cards.erase(i);

	if(!m_cards.empty()) shuffleCards();

	m_cardsTaken = false;

	return m_cards.empty();
}

void StdPlayer::cardPlayed(NetMauMau::Common::ICard *playedCard) {
	m_playedOutCards.insert(playedCard->description());
}

void StdPlayer::talonShuffled() {
	m_playedOutCards.clear();
}

std::size_t StdPlayer::getCardCount() const {
	return m_cards.size();
}

std::size_t StdPlayer::getPoints() const {

	std::size_t pts = 0;

	for(CARDS::const_iterator i(m_cards.begin()); i != m_cards.end(); ++i) {
		pts += (*i)->getPoints();
	}

	return pts;
}

const StdPlayer::CARDS &StdPlayer::getPlayerCards() const {
	return m_cards;
}

void StdPlayer::informAIStat(const IPlayer *, std::size_t count) {
	m_playerHasFewCards = count < 3;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
