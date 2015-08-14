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

#include "abstractaction.h"

#include <cstring>                      // for memset

#include "random_gen.h"

#if defined(TRACE_AI) && !defined(NDEBUG)
#include "logger.h"
#endif

namespace {
const NetMauMau::AI::IConditionPtr NULLCONDITION;

const NetMauMau::Common::ICard::SUIT SUIT[4] = {
	NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::CLUBS
};

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct _isSpecialRank : std::binary_function < NetMauMau::Player::IPlayer::CARDS::value_type,
		NetMauMau::Common::ICard::RANK, bool > {

	inline explicit _isSpecialRank(bool nineIsSuspend) : m_nineIsSuspend(nineIsSuspend) {}

	inline result_type operator()(const first_argument_type &card,
								  second_argument_type rank) const {

		return !(rank == NetMauMau::Common::ICard::EIGHT && m_nineIsSuspend) ? card == rank :
			   (card == NetMauMau::Common::ICard::EIGHT || card == NetMauMau::Common::ICard::NINE);
	}

private:
	const bool m_nineIsSuspend;
};
#pragma GCC diagnostic pop

}

using namespace NetMauMau::AI;

AbstractAction::AbstractAction() : IAction(), DecisionBase() {}

AbstractAction::~AbstractAction() {}

const IConditionPtr &AbstractAction::operator()(IAIState &state) const {
#if defined(TRACE_AI) && !defined(NDEBUG)

	if(!std::getenv("NMM_NO_TRACE")) logDebug("-> " << traceLog());

#endif

#ifndef NDEBUG
	state.getCardCount();
#endif

	return perform(state, state.isNoJack() ? DecisionBase::removeJack(state.getPlayerCards()) :
				   state.getPlayerCards());
}

void AbstractAction::countSuits(SUITCOUNT *suitCount, const IAIState::PLAYEDOUTCARDS &cards) {

	std::memset(suitCount, 0, sizeof(SUITCOUNT) * 4);

	IAIState::PLAYEDOUTCARDS myCards(cards);

	removeJack(myCards);

	const bool noCards = myCards.empty();

	for(std::size_t i = 0; i < 4; ++i) suitCount[i] = SUITCOUNT(SUIT[i],
				noCards ? 0 : DecisionBase::count(myCards, SUIT[i]));

	if(!noCards) std::sort(suitCount, suitCount + 4);
}

NetMauMau::Common::ICard::SUIT AbstractAction::getMaxPlayedOffSuit(const IAIState &state,
		NetMauMau::Player::IPlayer::CARDS::difference_type *count) {

	AbstractAction::SUITCOUNT poSuitCount[4];
	AbstractAction::countSuits(poSuitCount, state.getPlayedOutCards());

	if(count) *count = poSuitCount[0].count;

	return poSuitCount[0].suit;
}

const NetMauMau::Common::ICard::SUIT *AbstractAction::getSuits() {
	return SUIT;
}

const IConditionPtr &AbstractAction::getNullCondition() {
	return NULLCONDITION;
}

NetMauMau::Player::IPlayer::CARDS::iterator
AbstractAction::pullSpecialRank(NetMauMau::Player::IPlayer::CARDS &cards,
								NetMauMau::Common::ICard::RANK rank, bool nineIsSuspend) {
	return std::partition(cards.begin(), cards.end(), std::bind2nd(_isSpecialRank(nineIsSuspend),
						  rank));
}

NetMauMau::Common::ICardPtr AbstractAction::hasRankPath(const NetMauMau::Common::ICardPtr &uc,
		NetMauMau::Common::ICard::SUIT suit, NetMauMau::Common::ICard::RANK rank,
		const NetMauMau::Player::IPlayer::CARDS &c, bool nineIsSuspend) {

	if(c.size() > 1) {

		NetMauMau::Player::IPlayer::CARDS myCards(c);

		const NetMauMau::Player::IPlayer::CARDS::iterator &e(pullSpecialRank(myCards, rank,
				nineIsSuspend));

		if(std::distance(myCards.begin(), e)) {

			const NetMauMau::Player::IPlayer::CARDS::value_type
			f_src(NetMauMau::Common::find(uc->getSuit(), myCards.begin(), e));

			if(f_src) {

				NetMauMau::Player::IPlayer::CARDS::value_type f_dest(NetMauMau::Common::find(suit,
						myCards.begin(), e));

				if(f_dest) return f_src;
			}
		}
	}

	return NetMauMau::Common::ICardPtr();
}

NetMauMau::Common::ICardPtr AbstractAction::findRankTryAvoidSuit(NetMauMau::Common::ICard::RANK r,
		const NetMauMau::Player::IPlayer::CARDS &c, NetMauMau::Common::ICard::SUIT avoidSuit) {

	NetMauMau::Common::ICardPtr ret;
	NetMauMau::Common::ICard::SUIT rndSuits[4];
	NetMauMau::Player::IPlayer::CARDS myCards(c);

	std::copy(getSuits(), getSuits() + 4, rndSuits);
	std::random_shuffle(rndSuits, rndSuits + 4, NetMauMau::Common::genRandom<std::ptrdiff_t>);

	for(unsigned int i = 0u; i < 4u; ++i) {

		const NetMauMau::Player::IPlayer::CARDS::iterator &e(pull(myCards.begin(), myCards.end(),
				rndSuits[i]));

		if(rndSuits[i] != avoidSuit && ((ret = NetMauMau::Common::find(r, myCards.begin(), e))
										&& ret == rndSuits[i])) break;
	}

	return ret ? ret : NetMauMau::Common::find(r, c.begin(), c.end());
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
