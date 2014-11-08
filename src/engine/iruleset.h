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

#ifndef NETMAUMAU_IRULESET_H
#define NETMAUMAU_IRULESET_H

#include "icard.h"

namespace NetMauMau {

namespace Player {
class IPlayer;
}

namespace RuleSet {

class IRuleSet {
	DISALLOW_COPY_AND_ASSIGN(IRuleSet)
public:
	virtual ~IRuleSet() {}

	virtual void checkInitial(const Player::IPlayer *player, const Common::ICard *playedCard) = 0;
	virtual bool checkCard(const Player::IPlayer *player, const Common::ICard *uncoveredCard,
						   const Common::ICard *playedCard, bool ai) = 0;

	virtual bool hasToSuspend() const = 0;
	virtual void hasSuspended() = 0;
	virtual std::size_t takeCards(const Common::ICard *playedCard) const = 0;;
	virtual void hasTakenCards() = 0;
	virtual bool suspendIfNoMatchingCard() const = 0;

	virtual bool isJackMode() const = 0;
	virtual Common::ICard::SUIT getJackSuit() const = 0;
	virtual void setJackModeOff() = 0;

	virtual std::size_t getMaxPlayers() const = 0;

	virtual void reset() = 0;

protected:
	IRuleSet() {}
};

}

}

#endif /* NETMAUMAU_IRULESET_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
