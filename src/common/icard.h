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

/**
 * @file icard.h
 * @author Heiko Schäfer <heiko@rangun.de>
 */

#ifndef NETMAUMAU_ICARD_H
#define NETMAUMAU_ICARD_H

#include <string>

#include "linkercontrol.h"

namespace NetMauMau {

namespace Common {

/**
 * @interface ICard
 * @brief tbw
 */
class ICard {
	DISALLOW_COPY_AND_ASSIGN(ICard)
public:
	/**
	 * @brief tbw
	 */
	typedef enum { DIAMONDS, ///< &diams;
				   HEARTS, ///< &hearts;
				   SPADES, ///< &spades;
				   CLUBS ///< &clubs;
				 } SUIT;

	typedef enum { SEVEN = 7,
				   EIGHT = 8,
				   NINE = 9,
				   TEN = 10,
				   JACK,
				   QUEEN,
				   KING,
				   ACE
				 } VALUE;

	virtual ~ICard() {}

	virtual SUIT getSuit() const = 0;
	virtual VALUE getValue() const = 0;
	virtual std::size_t getPoints() const = 0;

	virtual std::string description(bool ansi = false) const = 0;

protected:
	ICard() {}
};

}

}

#endif /* NETMAUMAU_ICARD_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
