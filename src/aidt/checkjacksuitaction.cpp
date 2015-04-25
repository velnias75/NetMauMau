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

#include "checkjacksuitaction.h"

#include "stdcardfactory.h"
#include "random_gen.h"
#include "cardtools.h"
#include "iaistate.h"

namespace {

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct _isSpecialRank : std::binary_function < NetMauMau::Common::ICardPtr,
		NetMauMau::Common::ICard::RANK, bool > {

	explicit _isSpecialRank(bool nineIsEight) : m_nineIsEight(nineIsEight) {}

	bool operator()(const NetMauMau::Common::ICardPtr &c, NetMauMau::Common::ICard::RANK r) const {
		return m_nineIsEight && r == NetMauMau::Common::ICard::EIGHT ?
			   (c->getRank() == NetMauMau::Common::ICard::EIGHT ||
				c->getRank() == NetMauMau::Common::ICard::NINE) : c->getRank() == r;
	}

private:
	bool m_nineIsEight;
};
#pragma GCC diagnostic pop

}

using namespace NetMauMau::AIDT;

bool CheckJackSuitAction::_hasRankPath::operator()(const NetMauMau::Common::ICardPtr &c) const {

	bool hrp = false;

	if(c->getRank() != rank) {
		for(NetMauMau::Player::IPlayer::CARDS::const_iterator i(mCards.begin()); i != mCards.end();
				++i) {
			if((hrp = CheckJackSuitAction::hasRankPath(c, (*i)->getSuit(), rank, mCards,
					  nineIsEight))) break;
		}
	}

	return hrp;
}

CheckJackSuitAction::CheckJackSuitAction() : AbstractAction() {}

CheckJackSuitAction::~CheckJackSuitAction() {}

const IConditionPtr &CheckJackSuitAction::operator()(IAIState &state) const {

	NetMauMau::Common::ICard::SUIT s = findJackChoice(state);

	if(s == NetMauMau::Common::ICard::SUIT_ILLEGAL) {
		while(((s = getSuits()[NetMauMau::Common::genRandom<std::ptrdiff_t>(4)]) ==
				state.getUncoveredCard()->getSuit() || s == state.getCard()->getSuit()));
	}

	assert(s != NetMauMau::Common::ICard::SUIT_ILLEGAL);

	state.setCard(NetMauMau::Common::ICardPtr(NetMauMau::StdCardFactory().create(s,
				  NetMauMau::Common::ICard::RANK_ILLEGAL)));

	return getNullCondition();
}

NetMauMau::Common::ICard::SUIT CheckJackSuitAction::findJackChoice(const IAIState &state) const {

	const NetMauMau::Player::IPlayer::CARDS::const_iterator
	&f(std::find_if(state.getPlayerCards().begin(), state.getPlayerCards().end(),
					_hasRankPath(state.getPlayerCards(), NetMauMau::Common::ICard::EIGHT,
								 state.nineIsEight())));

	if(f != state.getPlayerCards().end()) {
		return (*f)->getSuit();
	} else {
		NetMauMau::Player::IPlayer::CARDS::difference_type count = 0;
		const NetMauMau::Common::ICard::SUIT s = getMaxPlayedOffSuit(state, &count);
		return count ? s : NetMauMau::Common::ICard::SUIT_ILLEGAL;
	}
}

NetMauMau::Common::ICardPtr
CheckJackSuitAction::hasRankPath(const NetMauMau::Common::ICardPtr &uc,
								 NetMauMau::Common::ICard::SUIT s, NetMauMau::Common::ICard::RANK r,
								 const NetMauMau::Player::IPlayer::CARDS &cards, bool nineIsEight) {

	NetMauMau::Player::IPlayer::CARDS mCards(cards);

	if(mCards.size() > 1) {

		const NetMauMau::Player::IPlayer::CARDS::iterator &e(std::partition(mCards.begin(),
				mCards.end(), std::bind2nd(_isSpecialRank(nineIsEight), r)));

		if(std::distance(mCards.begin(), e)) {

			NetMauMau::Player::IPlayer::CARDS::value_type
			f_src(NetMauMau::Common::findSuit(uc->getSuit(), mCards.begin(), e));

			if(f_src) {

				NetMauMau::Player::IPlayer::CARDS::value_type
				f_dest(NetMauMau::Common::findSuit(s, mCards.begin(), e));

				if(f_dest) return f_src;
			}
		}
	}

	return NetMauMau::Common::ICardPtr();
}

NetMauMau::Common::ICard::SUIT CheckJackSuitAction::getMaxPlayedOffSuit(const IAIState &state,
		NetMauMau::Player::IPlayer::CARDS::difference_type *count) const {

	AbstractAction::SUITCOUNT poSuitCount[4];
	NetMauMau::Player::IPlayer::CARDS pcVec;

	pcVec.reserve(state.getPlayedOutCards().size());

	for(std::vector<std::string>::const_iterator i(state.getPlayedOutCards().begin());
			i != state.getPlayedOutCards().end(); ++i) {

		NetMauMau::Common::ICard::SUIT s;
		NetMauMau::Common::ICard::RANK r;

		if(NetMauMau::Common::parseCardDesc(*i, &s, &r)) {
			pcVec.push_back(NetMauMau::Common::ICardPtr(NetMauMau::StdCardFactory().create(s, r)));
		}
	}

	AbstractAction::countSuits(poSuitCount, pcVec);

	if(count) *count = poSuitCount[0].count;

	return poSuitCount[0].suit;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
