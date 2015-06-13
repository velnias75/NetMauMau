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

#include "powerplayaction.h"

#include <cassert>                      // for assert

#include "aceroundcondition.h"          // for AceRoundCondition
#include "havejackcondition.h"          // for HaveJackCondition
#include "maxsuitaction.h"              // for MaxSuitAction
#include "staticcondition.h"            // for StaticCondition
#include "stdcardfactory.h"             // for StdCardFactory

namespace {
const NetMauMau::AI::IConditionPtr
MAXSUITACTION(new NetMauMau::AI::StaticCondition<NetMauMau::AI::MaxSuitAction>());
const NetMauMau::AI::IConditionPtr HAVEJACKCOND(new NetMauMau::AI::HaveJackCondition());
const NetMauMau::AI::IConditionPtr ACEROUNDCOND(new NetMauMau::AI::AceRoundCondition());
}

using namespace NetMauMau::AI;

PowerPlayAction::PowerPlayAction(bool set) : AbstractAction(), m_set(set) {}

PowerPlayAction::~PowerPlayAction() {}

const IConditionPtr &PowerPlayAction::perform(IAIState &state,
		const NetMauMau::Player::IPlayer::CARDS &cards) const {

	if(m_set) {

		const NetMauMau::Common::ICard::SUIT s = state.getPowerSuit();

		state.setPowerSuit(NetMauMau::Common::ICard::SUIT_ILLEGAL);
		state.setPowerPlay(true);

		assert(s != NetMauMau::Common::ICard::SUIT_ILLEGAL);

		state.setCard(NetMauMau::Common::ICardPtr(NetMauMau::StdCardFactory().create(s,
					  NetMauMau::Common::ICard::RANK_ILLEGAL)));

		return HAVEJACKCOND;

	} else if(!state.getCard()) {

		NetMauMau::Player::IPlayer::CARDS myCards(cards);

		const NetMauMau::Player::IPlayer::CARDS::iterator &e(stable_pull(myCards.begin(),
				myCards.end(), NetMauMau::Common::ICard::SEVEN));

		const NetMauMau::Player::IPlayer::CARDS::value_type f =
			NetMauMau::Common::find(state.getJackSuit() ? *state.getJackSuit() :
									state.getUncoveredCard()->getSuit(), myCards.begin(), e);

		const NetMauMau::Player::IPlayer::CARDS::difference_type mySevens =
			std::distance(myCards.begin(), e);

		const NetMauMau::Player::IPlayer::CARDS::difference_type poSevens =
			DecisionBase::count(state.getPlayedOutCards(), NetMauMau::Common::ICard::SEVEN);

		if(f && (state.isPowerPlay() || mySevens + poSevens >
				 static_cast<NetMauMau::Player::IPlayer::CARDS::difference_type>(2 *
						 state.getTalonFactor()))) state.setCard(f);

		state.setPowerPlay(false);
	}

	if(!state.isCardPossible()) {
		state.setCard();
		return ACEROUNDCOND;
	}

	return MAXSUITACTION;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
