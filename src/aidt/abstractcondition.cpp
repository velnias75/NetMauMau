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

#include "iaistate.h"
#include "nextaction.h"

#ifdef TRACE_AI
#include "logger.h"
#endif

namespace {
const NetMauMau::AIDT::IActionPtr NULLACTION;
}

using namespace NetMauMau::AIDT;

AbstractCondition::AbstractCondition() : ICondition(), JackRemoverBase() {}

AbstractCondition::~AbstractCondition() {}

IActionPtr AbstractCondition::createNextAction(const IConditionPtr &cond) const {
	return IActionPtr(new NextAction(cond));
}

const IActionPtr &AbstractCondition::getNullAction() {
	return NULLACTION;
}

IActionPtr AbstractCondition::operator()(const IAIState &state) const {
#ifdef TRACE_AI

	if(!getenv("NMM_NO_TRACE")) logDebug("* " << traceLog());

#endif

	return perform(state, state.isNoJack() ? JackRemoverBase::removeJack(state.getPlayerCards()) :
				   state.getPlayerCards());
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
