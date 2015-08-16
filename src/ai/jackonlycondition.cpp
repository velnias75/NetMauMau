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

#include "jackonlycondition.h"

#include "aceroundaction.h"             // for AceRoundAction
#include "jackplusonecondition.h"
#include "iruleset.h"                   // for IRuleSet
#include "playjackaction.h"             // for PlayJackAction
#include "suspendaction.h"              // for SuspendAction

namespace {
const NetMauMau::AI::IActionPtr ACEROUNDACTION(new NetMauMau::AI::AceRoundAction());
const NetMauMau::AI::IActionPtr PLAYJACKACTION(new NetMauMau::AI::PlayJackAction());
const NetMauMau::AI::IActionPtr SUSPENDACTION(new NetMauMau::AI::SuspendAction());
const NetMauMau::AI::IConditionPtr JACKPLUSONECOND(new NetMauMau::AI::JackPlusOneCondition());
}

using namespace NetMauMau::AI;

JackOnlyCondition::JackOnlyCondition() throw() : AbstractCondition() {}

JackOnlyCondition::~JackOnlyCondition() throw() {}

IActionPtr
JackOnlyCondition::perform(const IAIState &state,
						   const NetMauMau::Player::IPlayer::CARDS &cards) const throw() {

	const bool oneCard = cards.size() == 1u;

	return state.getRuleSet() ?
		   (state.getRuleSet()->isAceRound() ? ACEROUNDACTION : ((oneCard &&
				   !(state.getUncoveredCard() == NetMauMau::Common::ICard::JACK && cards.front() ==
					 NetMauMau::Common::ICard::JACK)) ? PLAYJACKACTION : (oneCard ? SUSPENDACTION :
							 createNextAction(JACKPLUSONECOND)))) :
			   createNextAction(JACKPLUSONECOND);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
