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

#include "powerjackcondition.h"

#include "powerjackaction.h"
#include "bestjackaction.h"

namespace {
const NetMauMau::AI::IActionPtr POWERJACKACTION(new NetMauMau::AI::PowerJackAction());
const NetMauMau::AI::IActionPtr BESTJACKACTION(new NetMauMau::AI::BestJackAction());
}

using namespace NetMauMau::AI;

PowerJackCondition::PowerJackCondition() throw() : AbstractCondition() {}

PowerJackCondition::~PowerJackCondition() throw() {}

IActionPtr
PowerJackCondition::perform(const IAIState &,
							const NetMauMau::Player::IPlayer::CARDS &cards) const throw() {
	return cards.size() == 1u ? POWERJACKACTION : BESTJACKACTION;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
