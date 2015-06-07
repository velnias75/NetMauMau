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

#include "checkjacksuitaction.h"

#include <algorithm>                    // for find_if
#include <cassert>                      // for assert
#include <stdbool.h>

#include "random_gen.h"                 // for genRandom
#include "stdcardfactory.h"             // for StdCardFactory

using namespace NetMauMau::AI;

CheckJackSuitAction::_hasRankPath::result_type
CheckJackSuitAction::_hasRankPath::operator()
(const CheckJackSuitAction::_hasRankPath::argument_type &c) const {

	bool hrp = false;

	if(c != rank) {
		for(NetMauMau::Player::IPlayer::CARDS::const_iterator i(mCards.begin());
				i != mCards.end(); ++i) {
			if((hrp = AbstractAction::hasRankPath(c, (*i)->getSuit(), rank, mCards,
												  nineIsSuspend))) break;
		}
	}

	return hrp;
}

CheckJackSuitAction::CheckJackSuitAction() : AbstractAction() {}

CheckJackSuitAction::~CheckJackSuitAction() {}

const IConditionPtr &CheckJackSuitAction::perform(IAIState &state,
		const NetMauMau::Player::IPlayer::CARDS &) const {

	NetMauMau::Common::ICard::SUIT s = CheckJackSuitAction::findJackChoice(state);

	if(s == NetMauMau::Common::ICard::SUIT_ILLEGAL) {
		while((state.getUncoveredCard() ==
				(s = AbstractAction::getSuits()[NetMauMau::Common::genRandom<std::ptrdiff_t>(4)]))
				|| state.getPlayedCard() == s);
	}

	assert(s != NetMauMau::Common::ICard::SUIT_ILLEGAL);

	state.setCard(NetMauMau::Common::ICardPtr(NetMauMau::StdCardFactory().create(s,
				  NetMauMau::Common::ICard::RANK_ILLEGAL)));

	return AbstractAction::getNullCondition();
}

NetMauMau::Common::ICard::SUIT CheckJackSuitAction::findJackChoice(const IAIState &state) {

	if(state.getCard()) {
		assert(state.getCard() != NetMauMau::Common::ICard::SUIT_ILLEGAL);
		return state.getCard()->getSuit();
	}

	const NetMauMau::Player::IPlayer::CARDS::const_iterator
	&f(std::find_if(state.getPlayerCards().begin(), state.getPlayerCards().end(),
					_hasRankPath(state.getPlayerCards(), NetMauMau::Common::ICard::EIGHT,
								 state.nineIsSuspend())));

	if(f != state.getPlayerCards().end()) {
		return (*f)->getSuit();
	} else {
		NetMauMau::Player::IPlayer::CARDS::difference_type count = 0;
		const NetMauMau::Common::ICard::SUIT s = AbstractAction::getMaxPlayedOffSuit(state, &count);
		return count ? s : NetMauMau::Common::ICard::SUIT_ILLEGAL;
	}
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
