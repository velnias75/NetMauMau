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

#include "playjackaction.h"
#include "suspendaction.h"
#include "iruleset.h"
#include "smartptr.h"
#include "aistate.h"

namespace {

const NetMauMau::Engine::AIDT::IActionPtr NULLACTION;

const NetMauMau::Engine::AIDT::IActionPtr
PLAYJACKACTION(new NetMauMau::Engine::AIDT::PlayJackAction());

const NetMauMau::Engine::AIDT::IActionPtr
SUSPENDACTION(new NetMauMau::Engine::AIDT::SuspendAction());

}

using namespace NetMauMau::Engine::AIDT;

JackOnlyCondition::JackOnlyCondition() : ICondition() {}

JackOnlyCondition::~JackOnlyCondition() {}

const IActionPtr &JackOnlyCondition::operator()(const AIState &state) const {
	return state.getRuleSet() ?
		   ((state.getCards().size() == 1 &&
			 !(state.getUncoveredCard()->getRank() == NetMauMau::Common::ICard::JACK &&
			   (*(state.getCards()).begin())->getRank() == NetMauMau::Common::ICard::JACK))
			? PLAYJACKACTION : SUSPENDACTION) : NULLACTION; // <- find best card action
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
