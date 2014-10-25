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

namespace {

const NetMauMau::ICard::SUITE SUITES[4] = {
	NetMauMau::ICard::HEART,
	NetMauMau::ICard::DIAMOND,
	NetMauMau::ICard::SPADE,
	NetMauMau::ICard::CLUB
};

typedef struct _suiteCount {
	bool operator<(const _suiteCount &sc) const {
		return !(count < sc.count);
	}

	bool operator==(NetMauMau::ICard::SUITE s) const {
		return suite == s;
	}

	NetMauMau::ICard::SUITE suite;
	std::vector<NetMauMau::ICard *>::difference_type count;
} SUITECOUNT;

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct cardGreater : std::binary_function<NetMauMau::ICard *, NetMauMau::ICard *, bool> {
	bool operator()(const NetMauMau::ICard *x, NetMauMau::ICard *y) const {
		return (NetMauMau::Common::getCardPoints(x->getValue()) <
				NetMauMau::Common::getCardPoints(y->getValue()));
	}
};

struct isSuite : std::binary_function<NetMauMau::ICard *, NetMauMau::ICard::SUITE, bool> {
	bool operator()(const NetMauMau::ICard *c, NetMauMau::ICard::SUITE s) const {
		return c->getSuite() == s;
	}
};

struct isValue : std::binary_function<NetMauMau::ICard *, NetMauMau::ICard::VALUE, bool> {
	bool operator()(const NetMauMau::ICard *c, NetMauMau::ICard::VALUE v) const {
		return c->getValue() == v;
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
	for(std::vector<NetMauMau::ICard *>::const_iterator i(m_cards.begin()); i != m_cards.end();
			++i) {
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

void StdPlayer::receiveCard(NetMauMau::ICard *card) {
	m_cards.push_back(card);
}

void StdPlayer::receiveCardSet(const std::vector<NetMauMau::ICard *> &cards) {
	m_cards.insert(m_cards.end(), cards.begin(), cards.end());
	std::random_shuffle(m_cards.begin(), m_cards.end(), NetMauMau::Common::
						genRandom<std::vector<NetMauMau::ICard *>::difference_type>);
}

NetMauMau::ICard *StdPlayer::findBestCard(const NetMauMau::ICard *uc,
		const NetMauMau::ICard::SUITE *js, bool noJack) const {

	std::vector<NetMauMau::ICard *> myCards(m_cards);

	if(noJack) {
		myCards.erase(std::remove_if(myCards.begin(), myCards.end(),
									 std::bind2nd(isValue(), NetMauMau::ICard::JACK)),
					  myCards.end());
	}

	NetMauMau::ICard *bestCard = 0L;

	if(uc->getValue() == NetMauMau::ICard::SEVEN) {

		const std::vector<NetMauMau::ICard *>::iterator &e(js ?
				std::partition(myCards.begin(), myCards.end(), std::bind2nd(isSuite(), *js)) :
				myCards.end());

		const std::vector<NetMauMau::ICard *>::const_iterator &f(std::find_if(myCards.begin(),
				e, std::bind2nd(isValue(), NetMauMau::ICard::SEVEN)));

		if(f != e) bestCard = *f;

	}

	if(!bestCard) {
		const std::vector<NetMauMau::ICard *>::iterator &e(std::partition(myCards.begin(),
				myCards.end(), std::bind2nd(isValue(), NetMauMau::ICard::SEVEN)));

		const std::vector<NetMauMau::ICard *>::const_iterator &f(std::find_if(myCards.begin(),
				e, std::bind2nd(isSuite(), js ? *js : uc->getSuite())));

		if(f != e) bestCard = *f;
	}

	if(!bestCard) {
		SUITECOUNT suiteCount[4];

		for(std::size_t i = 0; i < 4; ++i) {
			SUITECOUNT sc = { SUITES[i],
							  std::count_if(myCards.begin(), myCards.end(),
											std::bind2nd(isSuite(), SUITES[i]))
							};
			suiteCount[i] = sc;
		}

		std::sort(suiteCount, suiteCount + 4);

		for(std::size_t i = 0; i < 4; ++i) {

			const std::vector<NetMauMau::ICard *>::iterator &e(std::partition(myCards.begin(),
					myCards.end(), std::bind2nd(isSuite(), suiteCount[i].suite)));

			if(js) {
				if(suiteCount[i].count && (suiteCount[i].suite == *js)) {
					std::partition(myCards.begin(), e, std::bind2nd(isValue(),
								   NetMauMau::ICard::SEVEN));
					bestCard = myCards.front();
					break;
				}
			} else {
				std::sort(myCards.begin(), e, cardGreater());
				std::partition(myCards.begin(), e, std::bind2nd(isValue(),
							   NetMauMau::ICard::SEVEN));
				const std::vector<NetMauMau::ICard *>::iterator &f(std::find_if(myCards.begin(),
						e, std::bind2nd(isValue(), uc->getValue())));

				if(f != e) {
					bestCard = *f;
					break;
				}
			}
		}

		if(!bestCard) {
			const std::vector<NetMauMau::ICard *>::iterator &e(std::partition(myCards.begin(),
					myCards.end(), std::bind2nd(isValue(), NetMauMau::ICard::SEVEN)));

			if(!noJack) std::partition(e, myCards.end(), std::not1(std::bind2nd(isValue(),
										   NetMauMau::ICard::JACK)));

			const std::vector<NetMauMau::ICard *>::const_iterator &f(std::find_if(myCards.begin(),
					myCards.end(), std::bind2nd(isSuite(), js ? *js : uc->getSuite())));

			if(f != myCards.end() && (*f)->getValue() != NetMauMau::ICard::JACK)  bestCard = *f;
		}
	}

	if(!noJack && !bestCard) {
		const std::vector<NetMauMau::ICard *>::size_type jackCnt = std::count_if(myCards.begin(),
				myCards.end(), std::bind2nd(isValue(), NetMauMau::ICard::JACK));

		if(jackCnt) {
			std::partition(myCards.begin(), myCards.end(),
						   std::bind2nd(isValue(), NetMauMau::ICard::JACK));
			bestCard = myCards[NetMauMau::Common::genRandom
							   <std::vector<NetMauMau::ICard *>::difference_type>(jackCnt)];
		}
	}

	return bestCard;
}

NetMauMau::ICard *StdPlayer::requestCard(const NetMauMau::ICard *uc,
		const NetMauMau::ICard::SUITE *js) const {

	NetMauMau::ICard *bestCard = findBestCard(uc, js, false);

	if(bestCard && bestCard->getValue() == NetMauMau::ICard::JACK &&
			uc->getValue() == NetMauMau::ICard::JACK) {
		bestCard = findBestCard(uc, js, true);

	} else if(!bestCard) {

		const std::vector<NetMauMau::ICard *>::iterator &f(std::find_if(m_cards.begin(),
				m_cards.end(), std::bind2nd(isSuite(), js ? *js : uc->getSuite())));

		if(f != m_cards.end()) bestCard = *f;
	}

	return bestCard;
}

NetMauMau::ICard::SUITE StdPlayer::getJackChoice(const NetMauMau::ICard *uncoveredCard,
		const NetMauMau::ICard *playedCard) const {

	if(!m_jackPlayed) {
		m_jackPlayed = true;
		return uncoveredCard->getSuite();
	}

	NetMauMau::ICard *bc = 0L;

	if((bc = findBestCard(uncoveredCard, 0L, true))) {
		return bc->getSuite();
	}

	NetMauMau::ICard::SUITE s;

	while(((s = SUITES[NetMauMau::Common::genRandom<std::size_t>(4)]) == uncoveredCard->getSuite() ||
			s == playedCard->getSuite()));

	return s;
}

IPlayer::REASON StdPlayer::getNoCardReason() const {
	return m_cards.empty() ? MAUMAU : NOMATCH;
}

bool StdPlayer::cardAccepted(const NetMauMau::ICard *playedCard) {

	const std::vector<NetMauMau::ICard *>::iterator &i(std::find(m_cards.begin(), m_cards.end(),
			playedCard));

	if(i != m_cards.end()) m_cards.erase(i);

	if(!m_cards.empty()) {
		std::random_shuffle(m_cards.begin(), m_cards.end(), NetMauMau::Common::
							genRandom<std::vector<NetMauMau::ICard *>::difference_type>);
	}

	return m_cards.empty();
}

std::size_t StdPlayer::getCardCount() const {
	return m_cards.size();
}

std::size_t StdPlayer::getPoints() const {

	std::size_t pts = 0;

	for(std::vector<NetMauMau::ICard *>::const_iterator i(m_cards.begin()); i != m_cards.end();
			++i) {
		pts += (*i)->getPoints();
	}

	return pts;
}

const std::vector<NetMauMau::ICard *> &StdPlayer::getPlayerCards() const {
	return m_cards;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
