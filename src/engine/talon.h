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

#ifndef NETMAUMAU_TALON_H
#define NETMAUMAU_TALON_H

#include <stack>
#include <vector>

#include "linkercontrol.h"

namespace NetMauMau {

class ICard;
class ICardFactory;

class Talon {
	DISALLOW_COPY_AND_ASSIGN(Talon)
public:

	Talon();
	~Talon();

	bool empty() const _PURE;
	ICard *top() const _PURE;
	void pop();

	ICard *uncoverCard();
	ICard *getUncoveredCard() const _PURE;

	void playCard(ICard *card);
	ICard *takeCard();

private:
	std::vector<ICard *> createCards() const;

private:
	std::stack<ICard *, std::vector<ICard *> > m_cardStack;
	std::stack<ICard *, std::vector<ICard *> > m_uncovered;
};

}

#endif /* NETMAUMAU_TALON_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
