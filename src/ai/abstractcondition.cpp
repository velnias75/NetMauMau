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

#include "abstractcondition.h"

#include "nextaction.h"

#if defined(TRACE_AI) && !defined(NDEBUG)
#include "logger.h"
#endif

namespace {
const NetMauMau::AI::IActionPtr NULLACTION;
}

using namespace NetMauMau::AI;

AbstractCondition::AbstractCondition() : ICondition(), DecisionBase() {}

AbstractCondition::~AbstractCondition() {}

IActionPtr AbstractCondition::createNextAction(const IConditionPtr &cond) const {
	return IActionPtr(new NextAction(cond));
}

const IActionPtr &AbstractCondition::getNullAction() {
	return NULLACTION;
}

IActionPtr AbstractCondition::operator()(const IAIState &state) const {

#if defined(TRACE_AI) && !defined(NDEBUG)

	if(!std::getenv("NMM_NO_TRACE")) logDebug(" * " << traceLog());

#endif

#ifndef NDEBUG
	state.getCardCount();
#endif

	return perform(state, state.isNoJack() ? DecisionBase::removeJack(state.getPlayerCards()) :
				   state.getPlayerCards());
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
