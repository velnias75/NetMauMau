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

#include "clientcardfactory.h"

#include "cardtools.h"

using namespace NetMauMau::Client;

CardFactory::CardFactory(const std::string &cardDesc) : ICardFactory(), m_cardDesc(cardDesc) {}

CardFactory::~CardFactory() {}

NetMauMau::ICard *CardFactory::create(NetMauMau::ICard::SUITE, NetMauMau::ICard::VALUE) const {

	NetMauMau::ICard::SUITE suite = NetMauMau::ICard::HEART;
	NetMauMau::ICard::VALUE value = NetMauMau::ICard::ACE;

	if(NetMauMau::Common::parseCardDesc(m_cardDesc, &suite, &value)) {
		return new Card(suite, value, m_cardDesc);
	} else {
		return 0L;
	}
}

CardFactory::Card::Card(NetMauMau::ICard::SUITE s, NetMauMau::ICard::VALUE v,
						const std::string &desc) : m_suite(s), m_value(v), m_desc(desc) {}

CardFactory::Card::~Card() {}

std::size_t CardFactory::Card::getPoints() const {
	return NetMauMau::Common::getCardPoints(m_value);
}

std::string CardFactory::Card::description(bool ansi) const {
	return ansi ? NetMauMau::Common::createCardDesc(m_suite, m_value, ansi) : m_desc;
}

NetMauMau::ICard::SUITE CardFactory::Card::getSuite() const {
	return m_suite;
}

NetMauMau::ICard::VALUE CardFactory::Card::getValue() const {
	return m_value;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
