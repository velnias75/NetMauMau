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

#include "bestsuitcondition.h"

#include "powersuitaction.h"
#include "iaistate.h"

using namespace NetMauMau::AIDT;

BestSuitCondition::BestSuitCondition() : AbstractCondition() {}

BestSuitCondition::~BestSuitCondition() {}

IActionPtr BestSuitCondition::perform(const IAIState &state,
									  const NetMauMau::Player::IPlayer::CARDS &cards) const {

	if(!state.getCard() && !state.isNoJack() && state.hasPlayerFewCards() &&
			JackRemoverBase::countRank(cards, NetMauMau::Common::ICard::JACK)) {
		return IActionPtr(new PowerSuitAction());
	} else {
		return IActionPtr(new PowerSuitAction(NetMauMau::Common::ICard::SUIT_ILLEGAL));
	}
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
