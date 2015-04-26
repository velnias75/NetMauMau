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

#include "skipplayercondition.h"

#include "bestsuitcondition.h"
#include "skipplayeraction.h"
#include "cardtools.h"
#include "iaistate.h"

namespace {

NetMauMau::AIDT::IConditionPtr BESTSUITCOND(new NetMauMau::AIDT::BestSuitCondition());

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

SkipPlayerCondition::SkipPlayerCondition() : AbstractCondition() {}

SkipPlayerCondition::~SkipPlayerCondition() {}

IActionPtr SkipPlayerCondition::perform(const IAIState &state,
										const NetMauMau::Player::IPlayer::CARDS &) const {
	return state.getPlayerCount() > 2 && (state.getRightCount() < state.getCardCount() ||
										  state.getRightCount() < state.getLeftCount()) &&
		   std::count_if(state.getPlayedOutCards().begin(), state.getPlayedOutCards().end(),
						 std::bind2nd(playedOutRank(), NetMauMau::Common::ICard::SEVEN)) ?
		   IActionPtr(new SkipPlayerAction()) : AbstractCondition::createNextAction(BESTSUITCOND);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
