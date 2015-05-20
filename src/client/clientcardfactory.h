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

#ifndef NETMAUMAU_CLIENTCARDFACTORY_H
#define NETMAUMAU_CLIENTCARDFACTORY_H

#include <cstddef>                      // for size_t

#include "icardfactory.h"               // for ICardFactory

namespace NetMauMau {

namespace Client {

class CardFactory : public ICardFactory {
	DISALLOW_COPY_AND_ASSIGN(CardFactory)
public:
	explicit CardFactory(const std::string &cardDesc);
	virtual ~CardFactory();

	virtual _NOUNUSED Common::ICard *create(Common::ICard::SUIT s = Common::ICard::HEARTS,
											Common::ICard::RANK r = Common::ICard::ACE) const;

private:
	class Card : public Common::ICard {
		DISALLOW_COPY_AND_ASSIGN(Card)
	public:
		explicit Card(Common::ICard::SUIT s, ICard::RANK r, const std::string &desc);
		virtual ~Card();

		virtual const std::string &description(bool ansi = false) const _CONST;
		virtual std::size_t getPoints() const _PURE;

		virtual SUIT getSuit() const _PURE;
		virtual RANK getRank() const _PURE;

	private:
		Common::ICard::SUIT m_suit;
		Common::ICard::RANK m_rank;
		std::string m_desc;
		std::string m_descAnsi;
	};

	std::string m_cardDesc;
};

}

}

#endif /* NETMAUMAU_CLIENTCARDFACTORY_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
