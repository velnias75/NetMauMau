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

#include "checksevencondition.h"

#include "skipplayercondition.h"
#include "servesevenaction.h"
#include "smartptr.h"
#include "iaistate.h"

namespace {

NetMauMau::AIDT::IActionPtr SERVESEVENACTION(new NetMauMau::AIDT::ServeSevenAction());

NetMauMau::AIDT::IConditionPtr SKIPPLAYERACTION(new NetMauMau::AIDT::SkipPlayerCondition());

}

using namespace NetMauMau::AIDT;

CheckSevenCondition::CheckSevenCondition() : AbstractCondition() {}

CheckSevenCondition::~CheckSevenCondition() {}

IActionPtr CheckSevenCondition::operator()(const IAIState &state) const {
	return state.getPlayedOutCards().size() > (4 * state.getTalonFactor()) &&
		   state.getUncoveredCard()->getRank() == NetMauMau::Common::ICard::SEVEN ?
		   SERVESEVENACTION : createNextAction(SKIPPLAYERACTION);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
