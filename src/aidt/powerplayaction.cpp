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

#include <cassert>

#include "powerplayaction.h"

#include "havejackcondition.h"
#include "staticcondition.h"
#include "stdcardfactory.h"
#include "maxsuitaction.h"
#include "cardtools.h"
#include "iaistate.h"

namespace {
NetMauMau::AIDT::IConditionPtr HAVEJACKCOND(new NetMauMau::AIDT::HaveJackCondition());
NetMauMau::AIDT::IConditionPtr
MAXSUITACTION(new NetMauMau::AIDT::StaticCondition<NetMauMau::AIDT::MaxSuitAction>());

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct playedOutRank : std::binary_function<std::string, NetMauMau::Common::ICard::RANK, bool> {
	bool operator()(const std::string &desc, NetMauMau::Common::ICard::RANK rank) const {

		NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::RANK_ILLEGAL;
		NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::SUIT_ILLEGAL;

		if(NetMauMau::Common::parseCardDesc(desc, &s, &r)) {
			return r == rank;
		}

		return false;
	}
};
#pragma GCC diagnostic pop

}

using namespace NetMauMau::AIDT;

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

		const NetMauMau::Player::IPlayer::CARDS::iterator &e(std::partition(myCards.begin(),
				myCards.end(), std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
											NetMauMau::Common::ICard::SEVEN)));

		const NetMauMau::Player::IPlayer::CARDS::value_type f =
			NetMauMau::Common::findSuit(state.getJackSuit() ? *state.getJackSuit() :
										state.getUncoveredCard()->getSuit(), myCards.begin(), e);

		const NetMauMau::Player::IPlayer::CARDS::difference_type mySevens =
			std::distance(myCards.begin(), e);

		const NetMauMau::Player::IPlayer::CARDS::difference_type poSevens =
			std::count_if(state.getPlayedOutCards().begin(), state.getPlayedOutCards().end(),
						  std::bind2nd(playedOutRank(), NetMauMau::Common::ICard::SEVEN));

		if(f && (state.isPowerPlay() || mySevens + poSevens >
				 static_cast<NetMauMau::Player::IPlayer::CARDS::difference_type>(2 *
						 state.getTalonFactor()))) state.setCard(f);

		state.setPowerPlay(false);

		return f ? getNullCondition() : MAXSUITACTION;
	}

	return MAXSUITACTION;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
