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

#ifndef NETMAUMAU_STDCARDFACTORY_H
#define NETMAUMAU_STDCARDFACTORY_H

#include "icardfactory.h"

namespace NetMauMau {

class StdCardFactory : public ICardFactory {
public:
	StdCardFactory();
	~StdCardFactory();

	virtual ICard *create(ICard::SUITE f, ICard::VALUE v) const;

private:
	class StdCard : public ICard {
		DISALLOW_COPY_AND_ASSIGN(StdCard)
	public:
		StdCard(ICard::SUITE f, ICard::VALUE v);
		virtual ~StdCard();

		virtual SUITE getSuite() const _PURE;
		virtual VALUE getValue() const _PURE;
		virtual std::size_t getPoints() const _PURE;

		virtual std::string description(bool ansi) const;

	private:
		const ICard::SUITE m_suite;
		const ICard::VALUE m_value;
	};
};

}

#endif /* NETMAUMAU_STDCARDFACTORY_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
