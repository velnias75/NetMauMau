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

#include "stdcardfactory.h"
#include "cardtools.h"

using namespace NetMauMau;

StdCardFactory::StdCardFactory() : ICardFactory() {}

StdCardFactory::~StdCardFactory() {}

Common::ICard *StdCardFactory::create(Common::ICard::SUIT s, Common::ICard::RANK r) const {
	return new StdCard(s, r);
}

StdCardFactory::StdCard::StdCard(ICard::SUIT s, ICard::RANK r) : ICard(), m_suit(s), m_rank(r),
	m_desc(Common::createCardDesc(m_suit, m_rank, false)),
	m_descAnsi(Common::createCardDesc(m_suit, m_rank, true)) {}

StdCardFactory::StdCard::~StdCard() {}

Common::ICard::SUIT StdCardFactory::StdCard::getSuit() const {
	return m_suit;
}

Common::ICard::RANK StdCardFactory::StdCard::getRank() const {
	return m_rank;
}

std::size_t StdCardFactory::StdCard::getPoints() const {
	return Common::getCardPoints(m_rank);
}

const std::string &StdCardFactory::StdCard::description(bool ansi) const {
	return !ansi ? m_desc : m_descAnsi;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
