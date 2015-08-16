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

#include "powersuitaction.h"

#include "aceroundcondition.h"          // for AceRoundCondition
#include "powersuitcondition.h"         // for PowerSuitCondition

namespace {
const NetMauMau::AI::IConditionPtr ACEREOUNDCOND(new NetMauMau::AI::AceRoundCondition());
const NetMauMau::AI::IConditionPtr POWERSUITCOND(new NetMauMau::AI::PowerSuitCondition());
}

using namespace NetMauMau::AI;

PowerSuitAction::PowerSuitAction() throw() : AbstractAction(), m_determineSuit(true),
	m_suit(NetMauMau::Common::ICard::SUIT_ILLEGAL) {}

PowerSuitAction::PowerSuitAction(NetMauMau::Common::ICard::SUIT suit) throw() : AbstractAction(),
	m_determineSuit(false), m_suit(suit) {}

PowerSuitAction::~PowerSuitAction() throw() {}

const IConditionPtr &PowerSuitAction::perform(IAIState &state,
		const NetMauMau::Player::IPlayer::CARDS &) const throw() {

	if(m_determineSuit) {

		NetMauMau::Player::IPlayer::CARDS myCards(state.getPlayerCards());

		SUITCOUNT suitCount[4];
		AbstractAction::countSuits(suitCount, myCards);

		if(DecisionBase::count(myCards, NetMauMau::Common::ICard::SEVEN)) {

			for(unsigned int p = 0u; p < 4u; ++p) {

				AbstractAction::pull(myCards.begin(), myCards.end(), suitCount[p].suit);

				const NetMauMau::Player::IPlayer::CARDS::value_type f =
					NetMauMau::Common::find(NetMauMau::Common::ICard::SEVEN, myCards.begin(),
											myCards.end());

				if(f) {
					state.setPowerSuit(f->getSuit());
					return f != NetMauMau::Common::ICard::SUIT_ILLEGAL ?
						   ACEREOUNDCOND : POWERSUITCOND;
				}
			}

		} else {
			NetMauMau::Common::ICard::SUIT s = AbstractAction::getMaxPlayedOffSuit(state);
			state.setPowerSuit(s);
			return s != NetMauMau::Common::ICard::SUIT_ILLEGAL ? ACEREOUNDCOND : POWERSUITCOND;
		}

	} else {
		state.setPowerSuit(m_suit);
		return m_suit != NetMauMau::Common::ICard::SUIT_ILLEGAL ? ACEREOUNDCOND : POWERSUITCOND;
	}

	return POWERSUITCOND;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
