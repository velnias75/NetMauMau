/*
 * Copyright 2015 by Heiko Schäfer <heiko@rangun.de>
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

#ifndef NETMAUMAU_PLAYER_HARDPLAYER_H
#define NETMAUMAU_PLAYER_HARDPLAYER_H

#include "aiplayerbase.h"               // for AIPlayerBase

namespace NetMauMau {

namespace AI {
class JackOnlyCondition;
class PowerJackCondition;
}

namespace Player {

class HardPlayer : public AIPlayerBase<AI::JackOnlyCondition, AI::PowerJackCondition> {
	DISALLOW_COPY_AND_ASSIGN(HardPlayer)
public:
	explicit HardPlayer(const std::string &name, const IPlayedOutCards *poc);
	virtual ~HardPlayer() throw();

	virtual TYPE getType() const throw() _CONST;
};

}

}

#endif /* NETMAUMAU_PLAYER_HARDPLAYER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
