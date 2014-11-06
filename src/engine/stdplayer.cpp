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

#include <algorithm>

#include "stdplayer.h"

#include "cardtools.h"
#include "random_gen.h"

namespace {

const NetMauMau::Common::ICard::SUIT SUIT[4] = {
	NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::CLUBS
};

typedef struct _suitCount {
	bool operator<(const _suitCount &sc) const {
		return !(count < sc.count);
	}

	bool operator==(NetMauMau::Common::ICard::SUIT s) const {
		return suit == s;
	}

	NetMauMau::Common::ICard::SUIT suit;
	std::vector<NetMauMau::Common::ICard *>::difference_type count;
} SUITCOUNT;

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct cardGreater : std::binary_function < NetMauMau::Common::ICard *, NetMauMau::Common::ICard *,
		bool > {
	bool operator()(const NetMauMau::Common::ICard *x, NetMauMau::Common::ICard *y) const {
		return (NetMauMau::Common::getCardPoints(x->getRank()) <
				NetMauMau::Common::getCardPoints(y->getRank()));
	}
};

struct isSuit : std::binary_function < NetMauMau::Common::ICard *, NetMauMau::Common::ICard::SUIT,
		bool > {
	bool operator()(const NetMauMau::Common::ICard *c, NetMauMau::Common::ICard::SUIT s) const {
		return c->getSuit() == s;
	}
};

struct isRank : std::binary_function < NetMauMau::Common::ICard *, NetMauMau::Common::ICard::RANK,
		bool > {
	bool operator()(const NetMauMau::Common::ICard *c, NetMauMau::Common::ICard::RANK r) const {
		return c->getRank() == r;
	}
};
#pragma GCC diagnostic pop

}

using namespace NetMauMau::Player;

bool StdPlayer::m_jackPlayed = false;

StdPlayer::StdPlayer(const std::string &name) : m_name(name), m_cards() {
	m_cards.reserve(32);
}

StdPlayer::~StdPlayer() {
	for(std::vector<NetMauMau::Common::ICard *>::const_iterator i(m_cards.begin());
			i != m_cards.end(); ++i) {
		delete *i;
	}
}

void StdPlayer::reset() {
	m_cards.clear();
}

void StdPlayer::resetJackState() {
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

void StdPlayer::receiveCard(NetMauMau::Common::ICard *card) {
	m_cards.push_back(card);
}

void StdPlayer::receiveCardSet(const std::vector<NetMauMau::Common::ICard *> &cards) {
	m_cards.insert(m_cards.end(), cards.begin(), cards.end());
	std::random_shuffle(m_cards.begin(), m_cards.end(), NetMauMau::Common::
						genRandom<std::vector<NetMauMau::Common::ICard *>::difference_type>);
}

NetMauMau::Common::ICard *StdPlayer::findBestCard(const NetMauMau::Common::ICard *uc,
		const NetMauMau::Common::ICard::SUIT *js, bool noJack) const {

	std::vector<NetMauMau::Common::ICard *> myCards(m_cards);

	if(noJack) {
		myCards.erase(std::remove_if(myCards.begin(), myCards.end(),
									 std::bind2nd(isRank(), NetMauMau::Common::ICard::JACK)),
					  myCards.end());
	}

	NetMauMau::Common::ICard *bestCard = 0L;

	if(uc->getRank() == NetMauMau::Common::ICard::SEVEN) {

		const std::vector<NetMauMau::Common::ICard *>::iterator &e(js ?
				std::partition(myCards.begin(), myCards.end(), std::bind2nd(isSuit(), *js)) :
				myCards.end());

		const std::vector<NetMauMau::Common::ICard *>::const_iterator
		&f(std::find_if(myCards.begin(), e,
						std::bind2nd(isRank(), NetMauMau::Common::ICard::SEVEN)));

		if(f != e) bestCard = *f;

	}

	if(!bestCard) {
		const std::vector<NetMauMau::Common::ICard *>::iterator &e(std::partition(myCards.begin(),
				myCards.end(), std::bind2nd(isRank(), NetMauMau::Common::ICard::SEVEN)));

		const std::vector<NetMauMau::Common::ICard *>::const_iterator
		&f(std::find_if(myCards.begin(), e, std::bind2nd(isSuit(), js ? *js : uc->getSuit())));

		if(f != e) bestCard = *f;
	}

	if(!bestCard) {
		SUITCOUNT suitCount[4];

		for(std::size_t i = 0; i < 4; ++i) {
			SUITCOUNT sc = { SUIT[i], std::count_if(myCards.begin(), myCards.end(),
													std::bind2nd(isSuit(), SUIT[i]))
						   };
			suitCount[i] = sc;
		}

		std::sort(suitCount, suitCount + 4);

		for(std::size_t i = 0; i < 4; ++i) {

			const std::vector<NetMauMau::Common::ICard *>::iterator
			&e(std::partition(myCards.begin(), myCards.end(),
							  std::bind2nd(isSuit(), suitCount[i].suit)));

			if(js) {
				if(suitCount[i].count && (suitCount[i].suit == *js)) {
					std::partition(myCards.begin(), e, std::bind2nd(isRank(),
								   NetMauMau::Common::ICard::SEVEN));
					bestCard = myCards.front();
					break;
				}
			} else {
				std::sort(myCards.begin(), e, cardGreater());
				std::partition(myCards.begin(), e, std::bind2nd(isRank(),
							   NetMauMau::Common::ICard::SEVEN));
				const std::vector<NetMauMau::Common::ICard *>::iterator
				&f(std::find_if(myCards.begin(), e, std::bind2nd(isRank(), uc->getRank())));

				if(f != e) {
					bestCard = *f;
					break;
				}
			}
		}

		if(!bestCard) {
			const std::vector<NetMauMau::Common::ICard *>::iterator
			&e(std::partition(myCards.begin(), myCards.end(),
							  std::bind2nd(isRank(), NetMauMau::Common::ICard::SEVEN)));

			if(!noJack) std::partition(e, myCards.end(), std::not1(std::bind2nd(isRank(),
										   NetMauMau::Common::ICard::JACK)));

			const std::vector<NetMauMau::Common::ICard *>::const_iterator
			&f(std::find_if(myCards.begin(), myCards.end(), std::bind2nd(isSuit(),
							js ? *js : uc->getSuit())));

			if(f != myCards.end() && (*f)->getRank() != NetMauMau::Common::ICard::JACK) {
				bestCard = *f;
			}
		}
	}

	if(!noJack && !bestCard) {
		const std::vector<NetMauMau::Common::ICard *>::size_type jackCnt =
			std::count_if(myCards.begin(), myCards.end(), std::bind2nd(isRank(),
						  NetMauMau::Common::ICard::JACK));

		if(jackCnt) {
			std::partition(myCards.begin(), myCards.end(),
						   std::bind2nd(isRank(), NetMauMau::Common::ICard::JACK));
			bestCard = myCards[NetMauMau::Common::genRandom
							   <std::vector<NetMauMau::Common::ICard *>::difference_type>(jackCnt)];
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

	if(!m_jackPlayed) {
		m_jackPlayed = true;
		return uncoveredCard->getSuit();
	}

	NetMauMau::Common::ICard *bc = 0L;

	if((bc = findBestCard(uncoveredCard, 0L, true))) {
		return bc->getSuit();
	}

	NetMauMau::Common::ICard::SUIT s;

	while(((s = SUIT[NetMauMau::Common::genRandom<std::size_t>(4)]) == uncoveredCard->getSuit()
			|| s == playedCard->getSuit()));

	return s;
}

IPlayer::REASON StdPlayer::getNoCardReason() const {
	return m_cards.empty() ? MAUMAU : NOMATCH;
}

bool StdPlayer::cardAccepted(const NetMauMau::Common::ICard *playedCard) {

	const std::vector<NetMauMau::Common::ICard *>::iterator &i(std::find(m_cards.begin(),
			m_cards.end(), playedCard));

	if(i != m_cards.end()) m_cards.erase(i);

	if(!m_cards.empty()) {
		std::random_shuffle(m_cards.begin(), m_cards.end(), NetMauMau::Common::
							genRandom<std::vector<NetMauMau::Common::ICard *>::difference_type>);
	}

	return m_cards.empty();
}

std::size_t StdPlayer::getCardCount() const {
	return m_cards.size();
}

std::size_t StdPlayer::getPoints() const {

	std::size_t pts = 0;

	for(std::vector<NetMauMau::Common::ICard *>::const_iterator i(m_cards.begin());
			i != m_cards.end(); ++i) {
		pts += (*i)->getPoints();
	}

	return pts;
}

const std::vector<NetMauMau::Common::ICard *> &StdPlayer::getPlayerCards() const {
	return m_cards;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
