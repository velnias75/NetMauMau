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

#include "stdcardfactory.h"
#include "cardtools.h"

using namespace NetMauMau;

StdCardFactory::StdCardFactory() {}

StdCardFactory::~StdCardFactory() {}

Common::ICard *StdCardFactory::create(Common::ICard::SUIT f, Common::ICard::VALUE v) const {
	return new StdCard(f, v);
}

StdCardFactory::StdCard::StdCard(ICard::SUIT f, ICard::VALUE v) : ICard(), m_suit(f), m_value(v) {}

StdCardFactory::StdCard::~StdCard() {}

Common::ICard::SUIT StdCardFactory::StdCard::getSuit() const {
	return m_suit;
}

Common::ICard::VALUE StdCardFactory::StdCard::getValue() const {
	return m_value;
}

std::size_t StdCardFactory::StdCard::getPoints() const {
	return Common::getCardPoints(m_value);
}

std::string StdCardFactory::StdCard::description(bool ansi) const {
	return Common::createCardDesc(m_suit, m_value, ansi);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
