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

#include "powerjackaction.h"

using namespace NetMauMau::AI;

PowerJackAction::PowerJackAction() throw() : AbstractAction() {}

PowerJackAction::~PowerJackAction() throw() {}

const IConditionPtr &PowerJackAction::perform(IAIState &state,
		const NetMauMau::Player::IPlayer::CARDS &cards) const throw() {

	if(cards.size() == 1u) state.setCard(cards.front());

	return getNullCondition();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
