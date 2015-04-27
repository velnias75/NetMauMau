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

#include "skipplayeraction.h"

#include "powersuitcondition.h"
#include "bestsuitcondition.h"
#include "cardtools.h"
#include "iaistate.h"

namespace {
NetMauMau::AIDT::IConditionPtr POWERSUITCOND(new NetMauMau::AIDT::PowerSuitCondition());
NetMauMau::AIDT::IConditionPtr BESTSUITCOND(new NetMauMau::AIDT::BestSuitCondition());
}

using namespace NetMauMau::AIDT;

SkipPlayerAction::SkipPlayerAction() : AbstractAction() {}

SkipPlayerAction::~SkipPlayerAction() {}

const IConditionPtr &SkipPlayerAction::perform(IAIState &state,
		const NetMauMau::Player::IPlayer::CARDS &) const {

	const NetMauMau::Player::IPlayer::CARDS::value_type nine = state.isDirChgEnabled() ?
			NetMauMau::Common::findRank(NetMauMau::Common::ICard::NINE,
										state.getPlayerCards().begin(),
										state.getPlayerCards().end()) :
			NetMauMau::Common::ICardPtr();

	const NetMauMau::Player::IPlayer::CARDS::value_type seven =
		NetMauMau::Common::findRank(NetMauMau::Common::ICard::SEVEN,
									state.getPlayerCards().begin(), state.getPlayerCards().end());

	state.setCard(nine ? nine : seven ? seven :
				  NetMauMau::Common::findRank(NetMauMau::Common::ICard::EIGHT,
						  state.getPlayerCards().begin(), state.getPlayerCards().end()));

	return (state.getCard() || state.hasPlayerFewCards()) ? BESTSUITCOND : POWERSUITCOND;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
