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

#include "bestsuitcondition.h"          // for BestSuitCondition
#include "cardtools.h"                  // for findRank
#include "powersuitcondition.h"         // for PowerSuitCondition

namespace {
const NetMauMau::AI::IConditionPtr POWERSUITCOND(new NetMauMau::AI::PowerSuitCondition());
const NetMauMau::AI::IConditionPtr BESTSUITCOND(new NetMauMau::AI::BestSuitCondition());
}

using namespace NetMauMau::AI;

SkipPlayerAction::SkipPlayerAction() : AbstractAction() {}

SkipPlayerAction::~SkipPlayerAction() {}

const IConditionPtr &SkipPlayerAction::perform(IAIState &state,
		const NetMauMau::Player::IPlayer::CARDS &) const {

	NetMauMau::Player::IPlayer::CARDS myCards(state.getPlayerCards());
	const NetMauMau::Common::ICard::SUIT avoid = state.getAvoidSuit();

	const NetMauMau::Player::IPlayer::CARDS::value_type nine = state.isDirChgEnabled() ?
			AbstractAction::findRankTryAvoidSuit(NetMauMau::Common::ICard::NINE, myCards, avoid) :
			NetMauMau::Common::ICardPtr();

	const NetMauMau::Player::IPlayer::CARDS::value_type seven =
		AbstractAction::findRankTryAvoidSuit(NetMauMau::Common::ICard::SEVEN, myCards, avoid);

	AbstractAction::pushRank(myCards.begin(), myCards.end(),
							 state.getNeighbourRankSuit().rank[NetMauMau::Player::IPlayer::LEFT]);
	AbstractAction::pushRank(myCards.begin(), myCards.end(),
							 state.getNeighbourRankSuit().rank[NetMauMau::Player::IPlayer::RIGHT]);

	state.setCard(nine ? nine : seven ? seven :
				  AbstractAction::findRankTryAvoidSuit(NetMauMau::Common::ICard::EIGHT, myCards,
						  avoid));

	return (state.getCard() || state.hasPlayerFewCards()) ? BESTSUITCOND : POWERSUITCOND;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
