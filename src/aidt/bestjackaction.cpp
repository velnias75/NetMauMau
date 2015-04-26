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

#include "bestjackaction.h"

#include "checkjacksuitaction.h"
#include "checksevencondition.h"
#include "staticcondition.h"
#include "decisiontree.h"

namespace {
NetMauMau::AIDT::IConditionPtr
CHECKJACKSUITACTION(new NetMauMau::AIDT::StaticCondition<NetMauMau::AIDT::CheckJackSuitAction>());
}

using namespace NetMauMau::AIDT;

BestJackAction::BestJackAction() : AbstractAction() {}

BestJackAction::~BestJackAction() {}

const IConditionPtr &BestJackAction::perform(IAIState &state,
		const NetMauMau::Player::IPlayer::CARDS &) const {

	const NetMauMau::Common::ICardPtr sc(state.getCard());
	state.setCard(NetMauMau::Common::ICardPtr());
	const NetMauMau::Common::ICardPtr bc(DecisionTree<CheckSevenCondition>(state).getCard(true));
	state.setCard((bc && bc->getSuit() != NetMauMau::Common::ICard::SUIT_ILLEGAL) ? bc : sc);

	return CHECKJACKSUITACTION;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
