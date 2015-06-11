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

#ifndef NETMAUMAU_ENGINE_AI_DECISIONBASE_H
#define NETMAUMAU_ENGINE_AI_DECISIONBASE_H

#include "iaistate.h"                   // for IAIState, etc
#include "cardtools.h"

namespace NetMauMau {

namespace AI {

class DecisionBase {
	DISALLOW_COPY_AND_ASSIGN(DecisionBase)
public:
	virtual ~DecisionBase();

	static Player::IPlayer::CARDS removeJack(const Player::IPlayer::CARDS &cards);

	template<class CardType, class Tp>
	static typename CardType::difference_type count(CardType cards, Tp arg) {
		return std::count_if(cards.begin(), cards.end(), std::bind2nd(NetMauMau::Common::equalTo
							 <typename CardType::value_type, Tp>(), arg));
	}

protected:
	explicit DecisionBase();
};

}

}

#endif /* NETMAUMAU_ENGINE_AI_DECISIONBASE_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
