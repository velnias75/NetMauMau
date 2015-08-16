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

#include "jackplusoneaction.h"
#include "checksevencondition.h"

namespace {
const NetMauMau::AI::IConditionPtr CHECKSEVENCOND(new NetMauMau::AI::CheckSevenCondition());
}

using namespace NetMauMau::AI;

JackPlusOneAction::JackPlusOneAction() throw() : AbstractAction() {}

JackPlusOneAction::~JackPlusOneAction() throw() {}

const IConditionPtr &JackPlusOneAction::perform(IAIState &state,
		const NetMauMau::Player::IPlayer::CARDS &cards) const throw() {

	NetMauMau::Player::IPlayer::CARDS myCards(cards);

	push(myCards.begin(), myCards.end(), NetMauMau::Common::ICard::JACK);

	if(!(myCards.front() == state.getAvoidSuit() || myCards.front() == state.getAvoidRank())) {

		state.setCard(NetMauMau::Common::find(NetMauMau::Common::ICard::JACK, myCards.begin(),
											  myCards.end()));
		return getNullCondition();
	}

	return CHECKSEVENCOND;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
