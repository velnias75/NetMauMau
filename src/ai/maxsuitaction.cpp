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

#include "maxsuitaction.h"

#include <cassert>                      // for assert

#include "aceroundcondition.h"          // for AceRoundCondition
#include "cardtools.h"                  // for findRank, findSuit
#include "iruleset.h"                   // for IRuleSet

namespace {

const NetMauMau::AI::IConditionPtr ACEROUNDCOND(new NetMauMau::AI::AceRoundCondition());

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct cardGreater : std::binary_function < NetMauMau::Common::ICardPtr,
		NetMauMau::Common::ICardPtr, bool > {
	inline result_type operator()(const first_argument_type &x,
								  const second_argument_type &y) const {
		return !(x->getPoints() < y->getPoints());
	}
};
#pragma GCC diagnostic pop
}

using namespace NetMauMau::AI;

MaxSuitAction::MaxSuitAction() : AbstractAction() {}

MaxSuitAction::~MaxSuitAction() {}

const IConditionPtr &MaxSuitAction::perform(IAIState &state,
		const NetMauMau::Player::IPlayer::CARDS &cards) const {

	assert(!state.getCard());

	NetMauMau::Player::IPlayer::CARDS myCards(cards);

	SUITCOUNT suitCount[4];
	AbstractAction::countSuits(suitCount, myCards);

	NetMauMau::Common::ICardPtr bestCard;

	for(std::size_t i = suitCount[0].suit != state.getAvoidSuit() ? 0u : 1u; i < 4u; ++i) {

		const NetMauMau::Player::IPlayer::CARDS::iterator &e(AbstractAction::pullSuit(myCards,
				suitCount[i].suit));

		if(state.getJackSuit()) {

			if(suitCount[i].count && (suitCount[i].suit == *state.getJackSuit())) {
				AbstractAction::pullRank(myCards.begin(), e, NetMauMau::Common::ICard::SEVEN);
				bestCard = myCards.front();
				break;
			}

		} else if(!((state.getRuleSet()->isAceRoundPossible() &&
					 (bestCard = AbstractAction::hasRankPath(state.getUncoveredCard(),
								 suitCount[i].suit, state.getRuleSet()->getAceRoundRank(), myCards,
								 state.nineIsEight()))) ||
					(bestCard = AbstractAction::hasRankPath(state.getUncoveredCard(),
								suitCount[i].suit, NetMauMau::Common::ICard::EIGHT, myCards,
								state.nineIsEight())))) {

			std::sort(myCards.begin(), e, cardGreater());

			AbstractAction::pullRank(myCards.begin(), e, NetMauMau::Common::ICard::SEVEN);

			const NetMauMau::Player::IPlayer::CARDS::value_type f =
				NetMauMau::Common::findRank(state.getUncoveredCard()->getRank(), myCards.begin(),
											e);

			if(f && (f != NetMauMau::Common::ICard::SEVEN || state.getPlayedOutCards().size() >
					 (4 * state.getTalonFactor()))) {
				bestCard = f;
				break;
			}
		}
	}

	if(!bestCard) {

		const NetMauMau::Player::IPlayer::CARDS::iterator
		&e(AbstractAction::pullRank(myCards.begin(), myCards.end(),
									NetMauMau::Common::ICard::SEVEN));

		if(!state.isNoJack()) AbstractAction::pushRank(e, myCards.end(),
					NetMauMau::Common::ICard::JACK);

		const NetMauMau::Player::IPlayer::CARDS::value_type f =
			NetMauMau::Common::findSuit(state.getJackSuit() ? *state.getJackSuit() :
										state.getUncoveredCard()->getSuit(), myCards.begin(),
										myCards.end());

		if(f && f != NetMauMau::Common::ICard::JACK) bestCard = f;
	}

	state.setCard(bestCard);

	return ACEROUNDCOND;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
