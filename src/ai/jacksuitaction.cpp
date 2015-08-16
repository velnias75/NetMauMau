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

#include "jacksuitaction.h"

#include <cassert>                      // for assert

#include "bestjackaction.h"             // for BestJackAction
#include "havelessthancondition.h"      // for HaveLessThanCondition

namespace {
const NetMauMau::AI::IConditionPtr
HAVELESSTHANEIGHTCOND(new NetMauMau::AI::HaveLessThanCondition<8>(
						  NetMauMau::AI::IActionPtr(new NetMauMau::AI::BestJackAction()),
						  NetMauMau::AI::IActionPtr()));
}

using namespace NetMauMau::AI;

JackSuitAction::JackSuitAction() throw() : AbstractAction() {}

JackSuitAction::~JackSuitAction() throw() {}

const IConditionPtr &JackSuitAction::perform(IAIState &state,
		const NetMauMau::Player::IPlayer::CARDS &) const throw() {

	const NetMauMau::Player::IPlayer::CARDS::const_iterator
	&f(std::find_if(state.getPlayerCards().begin(), state.getPlayerCards().end(),
					std::not1(std::bind2nd(NetMauMau::Common::equalTo
										   < NetMauMau::Player::IPlayer::CARDS::value_type,
										   NetMauMau::Common::ICard::RANK > (),
										   NetMauMau::Common::ICard::JACK))));

	if(f != state.getPlayerCards().end()) {
		assert(*f != NetMauMau::Common::ICard::SUIT_ILLEGAL);
		state.setCard(*f);
		return AbstractAction::getNullCondition();
	}

	return HAVELESSTHANEIGHTCOND;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
