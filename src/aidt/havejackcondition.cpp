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

#include "havejackcondition.h"

#include "havelessthancondition.h"
#include "bestjackaction.h"
#include "nextaction.h"
#include "cardtools.h"
#include "iaistate.h"
#include "smartptr.h"

namespace {

NetMauMau::AIDT::IConditionPtr
HAVELESSTHANEIGHTCOND(new NetMauMau::AIDT::HaveLessThanCondition<8>(
						  NetMauMau::AIDT::IActionPtr(new NetMauMau::AIDT::BestJackAction()),
						  NetMauMau::AIDT::IActionPtr()));

}

using namespace NetMauMau::AIDT;

HaveJackCondition::HaveJackCondition() : AbstractCondition() {}

HaveJackCondition::~HaveJackCondition() {}

IActionPtr HaveJackCondition::operator()(const IAIState &state) const {

	(state.getPlayerCards().size() == 2 &&
	 NetMauMau::Common::findRank(NetMauMau::Common::ICard::JACK,
								 state.getPlayerCards().begin(), state.getPlayerCards().end()));

	return createNextAction(HAVELESSTHANEIGHTCOND);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
