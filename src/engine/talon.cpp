/*
 * Copyright 2014-2015 by Heiko Schäfer <heiko@rangun.de>
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

#include "talon.h"

#include <cassert>                      // for assert
#include <stdbool.h>
#include <algorithm>

#include "italonchange.h"               // for ITalonChange
#include "random_gen.h"                 // for genRandom
#include "stdcardfactory.h"             // for StdCardFactory

namespace {

typedef NetMauMau::StdCardFactory CF;

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct cardPusher : std::unary_function<NetMauMau::IPlayedOutCards::CARDS::value_type, void> {
	inline explicit cardPusher(NetMauMau::Talon::CARDSTACK &cs) : m_cardStack(cs) {}
	inline result_type operator()(const argument_type &c) const {
		m_cardStack.push(c);
	}

private:
	NetMauMau::Talon::CARDSTACK &m_cardStack;
};
#pragma GCC diagnostic pop

}

template<> const NetMauMau::CardsAllocator<NetMauMau::Common::ICardPtr>::value_type
NetMauMau::CardsAllocator<NetMauMau::Common::ICardPtr>::m_deck[32] _INIT_PRIO(501) = {

	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::SEVEN)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::EIGHT)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::NINE)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::TEN)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::JACK)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::QUEEN)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::KING)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::ACE)),

	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::SEVEN)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::EIGHT)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::NINE)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::TEN)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::JACK)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::QUEEN)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::KING)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::ACE)),

	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::SEVEN)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::EIGHT)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::NINE)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::TEN)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::JACK)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::QUEEN)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::KING)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::ACE)),

	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::SEVEN)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::EIGHT)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::NINE)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::TEN)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::JACK)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::QUEEN)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::KING)),
	NetMauMau::Common::ICardPtr(CF().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::ACE))
};

using namespace NetMauMau;

Talon::Talon(ITalonChange *tchg, std::size_t factor) throw() : m_talonChangeListener(tchg),
	m_playedOutCards(), m_cardStack(Talon::createCards(factor)), m_uncovered(m_playedOutCards),
	m_uncoveredDirty(false), m_factor(factor) {
	m_talonChangeListener->talonEmpty(false);
}

Talon::CARDSTACK::container_type Talon::createCards(std::size_t factor) throw() {

	Talon::CARDSTACK::container_type cards;
	const Talon::CARDSTACK::size_type resCards = 32 * factor;

	if(resCards <= cards.max_size()) cards.reserve(resCards);

	for(std::size_t i = 0; i < factor; ++i) cards.insert(cards.end(),
				CardsAllocator<Common::ICardPtr>::m_deck,
				CardsAllocator<Common::ICardPtr>::m_deck + 32);

	std::random_shuffle(cards.begin(), cards.end(), Common::genRandom<CARDS::difference_type>);

	return cards;
}

Talon::~Talon() {}

#ifndef __clang__
#pragma GCC diagnostic ignored "-Wunsafe-loop-optimizations"
#pragma GCC diagnostic push
#endif
const IPlayedOutCards::CARDS &Talon::getCards() const {

	if(m_uncoveredDirty) {

		CARDS aux;
		CARDSTACK tmp(m_uncovered);

		if(tmp.size() <= aux.max_size()) aux.reserve(tmp.size());

		while(!tmp.empty()) {
			aux.push_back(tmp.top());
			tmp.pop();
		}

		m_playedOutCards.swap(aux);
		m_uncoveredDirty = false;
	}

	return m_playedOutCards;
}
#ifndef __clang__
#pragma GCC diagnostic pop
#endif

Common::ICardPtr Talon::uncoverCard() {
	m_uncovered.push(top());
	pop();
	m_uncoveredDirty = true;
	m_talonChangeListener->uncoveredCard(m_uncovered.top());
	return m_uncovered.top();
}

void Talon::playCard(const Common::ICardPtr &card) {

	assert(!(card == Common::ICard::RANK_ILLEGAL || card == Common::ICard::SUIT_ILLEGAL));

	m_uncovered.push(card);
	m_uncoveredDirty = true;
	m_talonChangeListener->talonEmpty(false);
}

Common::ICardPtr Talon::takeCard() {

	if(empty()) {

		m_talonChangeListener->shuffled();

		Common::ICardPtr uc(m_uncovered.top());
		m_uncovered.pop();

		CARDS cards;

		if(m_uncovered.size() <= cards.max_size()) cards.reserve(m_uncovered.size());

		while(!m_uncovered.empty()) {
			cards.push_back(m_uncovered.top());
			m_uncovered.pop();
		}

		m_uncoveredDirty = true;

		std::random_shuffle(cards.begin(), cards.end(), Common::genRandom<CARDS::difference_type>);
		std::for_each(cards.begin(), cards.end(), cardPusher(m_cardStack));

		m_talonChangeListener->talonEmpty(cards.empty());

		m_uncovered.push(uc);
		m_talonChangeListener->uncoveredCard(m_uncovered.top());
	}

	if(!empty()) {

		Common::ICardPtr c(top());
		pop();

		return c;
	}

	m_talonChangeListener->talonEmpty(true);
	emitUnderFlow();

	return CardsAllocator<Common::ICardPtr>::m_nullCard;
}

void Talon::emitUnderFlow() const {
	if(m_talonChangeListener && thresholdReached()) m_talonChangeListener->underflow();
}

void Talon::reset() throw() {

	m_talonChangeListener->talonEmpty(false);
	m_playedOutCards.clear();
	m_uncoveredDirty = false;

	m_uncovered = CARDSTACK();
	m_cardStack = CARDSTACK(Talon::createCards(m_factor));
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
