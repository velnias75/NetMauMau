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

#ifndef NETMAUMAU_IPLAYER_H
#define NETMAUMAU_IPLAYER_H

#include <vector>

#include "icard.h"

namespace NetMauMau {

namespace Player {

class IPlayer {
	DISALLOW_COPY_AND_ASSIGN(IPlayer)
public:
	typedef enum { MAUMAU, NOMATCH, SUSPEND } REASON;

	virtual ~IPlayer() {}

	virtual std::string getName() const = 0;
	virtual int getSerial() const = 0;

	virtual void receiveCard(ICard *card) = 0;
	virtual void receiveCardSet(const std::vector<ICard *> &cards) = 0;

	virtual ICard *requestCard(const ICard *uncoveredCard, const ICard::SUITE *jackSuite) const = 0;
	virtual ICard::SUITE getJackChoice(const ICard *uncoveredCard,
									   const ICard *playedCard) const = 0;

	virtual bool cardAccepted(const ICard *playedCard) = 0;

	virtual REASON getNoCardReason() const = 0;

	virtual std::size_t getCardCount() const = 0;
	virtual std::size_t getPoints() const = 0;

	virtual void reset() = 0;

protected:
	IPlayer() {}
};

}

}

#endif /* NETMAUMAU_IPLAYER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
