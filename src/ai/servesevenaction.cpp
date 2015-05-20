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

#include "servesevenaction.h"

#include "cardtools.h"                  // for findRank, getIllegalCard
#include "skipplayercondition.h"        // for SkipPlayerCondition

namespace {
const NetMauMau::AI::IConditionPtr SKIPPLAYERCOND(new NetMauMau::AI::SkipPlayerCondition());
}

using namespace NetMauMau::AI;

ServeSevenAction::ServeSevenAction() : AbstractAction() {}

ServeSevenAction::~ServeSevenAction() {}

const IConditionPtr &ServeSevenAction::perform(IAIState &state,
		const NetMauMau::Player::IPlayer::CARDS &cards) const {

	const NetMauMau::Player::IPlayer::CARDS::value_type f =
		NetMauMau::Common::findRank(NetMauMau::Common::ICard::SEVEN, cards.begin(), cards.end());

	if(f) {
		state.setCard(f);
		return AbstractAction::getNullCondition();
	} else if(!state.hasTakenCards()) {
		state.setCard(NetMauMau::Common::ICardPtr(const_cast<const NetMauMau::Common::ICard *>
					  (NetMauMau::Common::getIllegalCard())));
		state.setCardsTaken(true);
		return AbstractAction::getNullCondition();
	}

	return SKIPPLAYERCOND;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
