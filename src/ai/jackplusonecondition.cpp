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

#include "jackplusonecondition.h"

#include "checksevencondition.h"
#include "jackplusoneaction.h"

namespace {
const NetMauMau::AI::IActionPtr JACKPLUSONEACTION(new NetMauMau::AI::JackPlusOneAction());
const NetMauMau::AI::IConditionPtr CHECKSEVENCOND(new NetMauMau::AI::CheckSevenCondition());
}

using namespace NetMauMau::AI;

JackPlusOneCondition::JackPlusOneCondition() : AbstractCondition() {}

JackPlusOneCondition::~JackPlusOneCondition() {}

IActionPtr JackPlusOneCondition::perform(const IAIState &/*state*/,
		const NetMauMau::Player::IPlayer::CARDS &cards) const {

	return (/*state.getPlayerCount() > 2 &&*/ cards.size() == 2u &&
			!NetMauMau::Common::find(NetMauMau::Common::ICard::SEVEN, cards.begin(), cards.end())
			&& NetMauMau::Common::find(NetMauMau::Common::ICard::JACK, cards.begin(), cards.end()))
		   ? JACKPLUSONEACTION : createNextAction(CHECKSEVENCOND);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
