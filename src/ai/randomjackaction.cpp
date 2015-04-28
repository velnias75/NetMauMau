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

#include "randomjackaction.h"

#include "random_gen.h"
#include "iaistate.h"

using namespace NetMauMau::AI;

RandomJackAction::RandomJackAction() : AbstractAction() {}

RandomJackAction::~RandomJackAction() {}

const IConditionPtr &RandomJackAction::perform(IAIState &state,
		const NetMauMau::Player::IPlayer::CARDS &cards) const {

	NetMauMau::Player::IPlayer::CARDS myCards(cards);

	const NetMauMau::Player::IPlayer::CARDS::size_type jackCnt =
		static_cast<NetMauMau::Player::IPlayer::CARDS::size_type>
		(DecisionBase::countRank(myCards, NetMauMau::Common::ICard::JACK));

	if(jackCnt) {

		pullRank(myCards, NetMauMau::Common::ICard::JACK);

		const NetMauMau::Player::IPlayer::CARDS::difference_type r =
			NetMauMau::Common::genRandom<NetMauMau::Player::IPlayer::CARDS::difference_type>
			(static_cast<NetMauMau::Player::IPlayer::CARDS::difference_type>(jackCnt));

		state.setCard(myCards[static_cast<NetMauMau::Player::IPlayer::CARDS::size_type>(r)]);
	}

	return getNullCondition();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
