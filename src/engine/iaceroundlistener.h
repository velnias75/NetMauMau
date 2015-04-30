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

#ifndef NETMAUMAU_ENGINE_IACEROUNDLISTENER_H
#define NETMAUMAU_ENGINE_IACEROUNDLISTENER_H

#include "icard.h"

namespace NetMauMau {

namespace Player {
class IPlayer;
}

class IAceRoundListener {
	DISALLOW_COPY_AND_ASSIGN(IAceRoundListener)
public:
	virtual ~IAceRoundListener() {}

	virtual Common::ICard::RANK getAceRoundRank() const = 0;
	virtual void aceRoundStarted(const Player::IPlayer *player) const = 0;
	virtual void aceRoundEnded(const Player::IPlayer *player) const = 0;

protected:
	explicit IAceRoundListener() {}
};

}

#endif /* NETMAUMAU_ENGINE_IACEROUNDLISTENER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
