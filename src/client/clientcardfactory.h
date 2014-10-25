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

#ifndef NETMAUMAU_CLIENTCARDFACTORY_H
#define NETMAUMAU_CLIENTCARDFACTORY_H

#include "icardfactory.h"

namespace NetMauMau {

namespace Client {

class CardFactory : public ICardFactory {
	DISALLOW_COPY_AND_ASSIGN(CardFactory)
public:
	CardFactory(const std::string &cardDesc);
	virtual ~CardFactory();

	virtual _NOUNUSED ICard *create(ICard::SUITE s = ICard::HEART,
									ICard::VALUE v = ICard::ACE) const;

private:
	class Card : public ICard {
		DISALLOW_COPY_AND_ASSIGN(Card)
	public:
		Card(ICard::SUITE s, ICard::VALUE v, const std::string &desc);
		virtual ~Card();

		virtual std::string description(bool ansi = false) const;
		virtual std::size_t getPoints() const _PURE;

		virtual SUITE getSuite() const _PURE;
		virtual VALUE getValue() const _PURE;

	private:
		ICard::SUITE m_suite;
		ICard::VALUE m_value;
		std::string m_desc;
	};

	std::string m_cardDesc;
};

}

}

#endif /* NETMAUMAU_CLIENTCARDFACTORY_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
