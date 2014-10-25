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

#include "talon.h"

#include "cardtools.h"
#include "stdcardfactory.h"

using namespace NetMauMau;

Talon::Talon() : m_cardStack(createCards()), m_uncovered() {}

std::vector< ICard * > Talon::createCards() const {

	StdCardFactory cardFactory;

	std::vector<ICard *> cards;
	cards.reserve(32);

	cards.push_back(cardFactory.create(ICard::DIAMOND, ICard::SEVEN));
	cards.push_back(cardFactory.create(ICard::DIAMOND, ICard::EIGHT));
	cards.push_back(cardFactory.create(ICard::DIAMOND, ICard::NINE));
	cards.push_back(cardFactory.create(ICard::DIAMOND, ICard::TEN));
	cards.push_back(cardFactory.create(ICard::DIAMOND, ICard::JACK));
	cards.push_back(cardFactory.create(ICard::DIAMOND, ICard::QUEEN));
	cards.push_back(cardFactory.create(ICard::DIAMOND, ICard::KING));
	cards.push_back(cardFactory.create(ICard::DIAMOND, ICard::ACE));

	cards.push_back(cardFactory.create(ICard::HEART, ICard::SEVEN));
	cards.push_back(cardFactory.create(ICard::HEART, ICard::EIGHT));
	cards.push_back(cardFactory.create(ICard::HEART, ICard::NINE));
	cards.push_back(cardFactory.create(ICard::HEART, ICard::TEN));
	cards.push_back(cardFactory.create(ICard::HEART, ICard::JACK));
	cards.push_back(cardFactory.create(ICard::HEART, ICard::QUEEN));
	cards.push_back(cardFactory.create(ICard::HEART, ICard::KING));
	cards.push_back(cardFactory.create(ICard::HEART, ICard::ACE));

	cards.push_back(cardFactory.create(ICard::SPADE, ICard::SEVEN));
	cards.push_back(cardFactory.create(ICard::SPADE, ICard::EIGHT));
	cards.push_back(cardFactory.create(ICard::SPADE, ICard::NINE));
	cards.push_back(cardFactory.create(ICard::SPADE, ICard::TEN));
	cards.push_back(cardFactory.create(ICard::SPADE, ICard::JACK));
	cards.push_back(cardFactory.create(ICard::SPADE, ICard::QUEEN));
	cards.push_back(cardFactory.create(ICard::SPADE, ICard::KING));
	cards.push_back(cardFactory.create(ICard::SPADE, ICard::ACE));

	cards.push_back(cardFactory.create(ICard::CLUB, ICard::SEVEN));
	cards.push_back(cardFactory.create(ICard::CLUB, ICard::EIGHT));
	cards.push_back(cardFactory.create(ICard::CLUB, ICard::NINE));
	cards.push_back(cardFactory.create(ICard::CLUB, ICard::TEN));
	cards.push_back(cardFactory.create(ICard::CLUB, ICard::JACK));
	cards.push_back(cardFactory.create(ICard::CLUB, ICard::QUEEN));
	cards.push_back(cardFactory.create(ICard::CLUB, ICard::KING));
	cards.push_back(cardFactory.create(ICard::CLUB, ICard::ACE));

	std::random_shuffle(cards.begin(), cards.end(),
						Common::genRandom<std::vector<ICard *>::difference_type>);

	return cards;
}

Talon::~Talon() {

	while(!m_cardStack.empty()) {
		delete m_cardStack.top();
		m_cardStack.pop();
	}

	while(!m_uncovered.empty()) {
		delete m_uncovered.top();
		m_uncovered.pop();
	}
}

bool Talon::empty() const {
	return m_cardStack.empty();
}

ICard *Talon::top() const {
	return m_cardStack.top();
}

void Talon::pop() {
	m_cardStack.pop();
}

ICard *Talon::uncoverCard() {
	m_uncovered.push(top());
	pop();
	return m_uncovered.top();
}

ICard *Talon::getUncoveredCard() const {
	return m_uncovered.top();
}

ICard *Talon::takeCard() {

	if(empty()) {

		ICard *uc = m_uncovered.top();
		m_uncovered.pop();

		std::vector<ICard *> cards;
		cards.reserve(m_uncovered.size());

		while(!m_uncovered.empty()) {
			cards.push_back(m_uncovered.top());
			m_uncovered.pop();
		}

		std::random_shuffle(cards.begin(), cards.end(),
							Common::genRandom<std::vector<ICard *>::difference_type>);

		for(std::vector<ICard *>::const_iterator i(cards.begin()); i != cards.end(); ++i) {
			m_cardStack.push(*i);
		}

		m_uncovered.push(uc);
	}

	ICard *c = top();
	pop();
	return c;
}

void Talon::playCard(ICard *card) {
	m_uncovered.push(card);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
