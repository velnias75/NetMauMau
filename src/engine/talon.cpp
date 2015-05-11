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

#include <algorithm>
#include <cassert>

#include "talon.h"

#include "random_gen.h"
#include "italonchange.h"
#include "stdcardfactory.h"

namespace {

const NetMauMau::Common::ICardPtr _DECK[32] _INIT_PRIO(501) = {
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::SEVEN)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::EIGHT)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::NINE)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::TEN)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::JACK)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::QUEEN)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::KING)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::ACE)),

	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::SEVEN)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::EIGHT)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::NINE)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::TEN)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::JACK)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::QUEEN)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::KING)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::ACE)),

	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::SEVEN)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::EIGHT)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::NINE)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::TEN)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::JACK)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::QUEEN)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::KING)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::ACE)),

	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::SEVEN)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::EIGHT)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::NINE)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::TEN)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::JACK)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::QUEEN)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::KING)),
	NetMauMau::Common::ICardPtr
	(NetMauMau::StdCardFactory().create(NetMauMau::Common::ICard::CLUBS,
	NetMauMau::Common::ICard::ACE))
};

const NetMauMau::Common::ICardPtr NULLCARD;

}

using namespace NetMauMau;

Talon::Talon(const ITalonChange *tchg, std::size_t factor) throw() : m_talonChangeListener(tchg),
	m_playedOutCards(), m_cardStack(Talon::createCards(factor)), m_uncovered(m_playedOutCards),
	m_uncoveredDirty(false) {
	m_talonChangeListener->talonEmpty(false);
}

Talon::CARDSTACK::container_type Talon::createCards(std::size_t factor) throw() {

	Talon::CARDSTACK::container_type cards;
	cards.reserve(32 * factor);

	for(std::size_t i = 0; i < factor; ++i) {
		cards.insert(cards.end(), _DECK, _DECK + 32);
	}

	std::random_shuffle(cards.begin(), cards.end(), Common::genRandom<CARDS::difference_type>);

	return cards;
}

Talon::~Talon() {}

const IPlayedOutCards::CARDS &Talon::getCards() const {

	if(m_uncoveredDirty) {
		
		CARDS aux;
		CARDSTACK tmp(m_uncovered);

		aux.reserve(tmp.size());

		while(!tmp.empty()) {
			aux.push_back(tmp.top());
			tmp.pop();
		}

		m_playedOutCards.swap(aux);
		m_uncoveredDirty = false;
	}

	return m_playedOutCards;
}

Common::ICardPtr Talon::uncoverCard() {
	m_uncovered.push(top());
	pop();
	m_uncoveredDirty = true;
	m_talonChangeListener->uncoveredCard(m_uncovered.top());
	return m_uncovered.top();
}

void Talon::playCard(const Common::ICardPtr &card) {

	assert(!(card->getRank() == Common::ICard::RANK_ILLEGAL ||
			 card->getSuit() == Common::ICard::SUIT_ILLEGAL));

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
		cards.reserve(m_uncovered.size());

		while(!m_uncovered.empty()) {
			cards.push_back(m_uncovered.top());
			m_uncovered.pop();
		}

		m_uncoveredDirty = true;

		std::random_shuffle(cards.begin(), cards.end(), Common::genRandom<CARDS::difference_type>);

		for(CARDS::const_iterator i(cards.begin()); i != cards.end(); ++i) {
			m_cardStack.push(*i);
		}

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

	return NULLCARD;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
