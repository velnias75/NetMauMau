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

std::vector<Common::ICard *> Talon::createCards() const {

	StdCardFactory cardFactory;

	std::vector<Common::ICard *> cards;
	cards.reserve(32);

	cards.push_back(cardFactory.create(Common::ICard::DIAMONDS, Common::ICard::SEVEN));
	cards.push_back(cardFactory.create(Common::ICard::DIAMONDS, Common::ICard::EIGHT));
	cards.push_back(cardFactory.create(Common::ICard::DIAMONDS, Common::ICard::NINE));
	cards.push_back(cardFactory.create(Common::ICard::DIAMONDS, Common::ICard::TEN));
	cards.push_back(cardFactory.create(Common::ICard::DIAMONDS, Common::ICard::JACK));
	cards.push_back(cardFactory.create(Common::ICard::DIAMONDS, Common::ICard::QUEEN));
	cards.push_back(cardFactory.create(Common::ICard::DIAMONDS, Common::ICard::KING));
	cards.push_back(cardFactory.create(Common::ICard::DIAMONDS, Common::ICard::ACE));

	cards.push_back(cardFactory.create(Common::ICard::HEARTS, Common::ICard::SEVEN));
	cards.push_back(cardFactory.create(Common::ICard::HEARTS, Common::ICard::EIGHT));
	cards.push_back(cardFactory.create(Common::ICard::HEARTS, Common::ICard::NINE));
	cards.push_back(cardFactory.create(Common::ICard::HEARTS, Common::ICard::TEN));
	cards.push_back(cardFactory.create(Common::ICard::HEARTS, Common::ICard::JACK));
	cards.push_back(cardFactory.create(Common::ICard::HEARTS, Common::ICard::QUEEN));
	cards.push_back(cardFactory.create(Common::ICard::HEARTS, Common::ICard::KING));
	cards.push_back(cardFactory.create(Common::ICard::HEARTS, Common::ICard::ACE));

	cards.push_back(cardFactory.create(Common::ICard::SPADES, Common::ICard::SEVEN));
	cards.push_back(cardFactory.create(Common::ICard::SPADES, Common::ICard::EIGHT));
	cards.push_back(cardFactory.create(Common::ICard::SPADES, Common::ICard::NINE));
	cards.push_back(cardFactory.create(Common::ICard::SPADES, Common::ICard::TEN));
	cards.push_back(cardFactory.create(Common::ICard::SPADES, Common::ICard::JACK));
	cards.push_back(cardFactory.create(Common::ICard::SPADES, Common::ICard::QUEEN));
	cards.push_back(cardFactory.create(Common::ICard::SPADES, Common::ICard::KING));
	cards.push_back(cardFactory.create(Common::ICard::SPADES, Common::ICard::ACE));

	cards.push_back(cardFactory.create(Common::ICard::CLUBS, Common::ICard::SEVEN));
	cards.push_back(cardFactory.create(Common::ICard::CLUBS, Common::ICard::EIGHT));
	cards.push_back(cardFactory.create(Common::ICard::CLUBS, Common::ICard::NINE));
	cards.push_back(cardFactory.create(Common::ICard::CLUBS, Common::ICard::TEN));
	cards.push_back(cardFactory.create(Common::ICard::CLUBS, Common::ICard::JACK));
	cards.push_back(cardFactory.create(Common::ICard::CLUBS, Common::ICard::QUEEN));
	cards.push_back(cardFactory.create(Common::ICard::CLUBS, Common::ICard::KING));
	cards.push_back(cardFactory.create(Common::ICard::CLUBS, Common::ICard::ACE));

	std::random_shuffle(cards.begin(), cards.end(),
						Common::genRandom<std::vector<Common::ICard *>::difference_type>);

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

Common::ICard *Talon::top() const {
	return m_cardStack.top();
}

void Talon::pop() {
	m_cardStack.pop();
}

Common::ICard *Talon::uncoverCard() {
	m_uncovered.push(top());
	pop();
	return m_uncovered.top();
}

Common::ICard *Talon::getUncoveredCard() const {
	return m_uncovered.top();
}

Common::ICard *Talon::takeCard() {

	if(empty()) {

		Common::ICard *uc = m_uncovered.top();
		m_uncovered.pop();

		std::vector<Common::ICard *> cards;
		cards.reserve(m_uncovered.size());

		while(!m_uncovered.empty()) {
			cards.push_back(m_uncovered.top());
			m_uncovered.pop();
		}

		std::random_shuffle(cards.begin(), cards.end(),
							Common::genRandom<std::vector<Common::ICard *>::difference_type>);

		for(std::vector<Common::ICard *>::const_iterator i(cards.begin()); i != cards.end(); ++i) {
			m_cardStack.push(*i);
		}

		m_uncovered.push(uc);
	}

	Common::ICard *c = top();
	pop();
	return c;
}

void Talon::playCard(Common::ICard *card) {
	m_uncovered.push(card);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
