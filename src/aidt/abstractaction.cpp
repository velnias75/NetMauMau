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

#include <algorithm>

#include "abstractaction.h"

#include "cardtools.h"
#include "smartptr.h"
#include "aistate.h"

namespace {
NetMauMau::Engine::AIDT::IConditionPtr NULLCONDITION;
}

using namespace NetMauMau::Engine::AIDT;

AbstractAction::AbstractAction() : IAction() {}

AbstractAction::~AbstractAction() {}

NetMauMau::Player::IPlayer::CARDS AbstractAction::pullSuit(const AIState &state,
		NetMauMau::Common::ICard::SUIT suit) {

	NetMauMau::Player::IPlayer::CARDS myCards(state.getCards());

	std::partition(myCards.begin(), myCards.end(),
				   std::bind2nd(std::ptr_fun(NetMauMau::Common::isSuit), suit));

	return myCards;
}

const IConditionPtr &AbstractAction::getNullCondition() {
	return NULLCONDITION;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
