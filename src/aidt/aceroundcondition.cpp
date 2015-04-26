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

#include "aceroundcondition.h"

#include "randomjackcondition.h"
#include "aceroundaction.h"
#include "smartptr.h"
#include "iaistate.h"
#include "iruleset.h"

namespace {
NetMauMau::AIDT::IActionPtr ACEROUNDACTION(new NetMauMau::AIDT::AceRoundAction());
NetMauMau::AIDT::IConditionPtr RANDOMJACKCOND(new NetMauMau::AIDT::RandomJackCondition());
}

using namespace NetMauMau::AIDT;

AceRoundCondition::AceRoundCondition() : AbstractCondition() {}

AceRoundCondition::~AceRoundCondition() {}

IActionPtr AceRoundCondition::perform(const IAIState &state,
									  const NetMauMau::Player::IPlayer::CARDS &) const {
	return state.tryAceRound() || (!state.getRuleSet()->isAceRound() &&
								   state.getRuleSet()->isAceRoundPossible()) ?
		   ACEROUNDACTION : createNextAction(RANDOMJACKCOND);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
