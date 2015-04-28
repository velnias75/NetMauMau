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

#include <cstring>

#include "abstractaction.h"

#include "stdcardfactory.h"
#include "cardtools.h"
#include "iaistate.h"

namespace {
NetMauMau::AI::IConditionPtr NULLCONDITION;

const NetMauMau::Common::ICard::SUIT SUIT[4] = {
	NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::CLUBS
};

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

using namespace NetMauMau::AI;

AbstractAction::AbstractAction() : IAction(), DecisionBase() {}

AbstractAction::~AbstractAction() {}

const IConditionPtr &AbstractAction::operator()(IAIState &state) const {

#ifndef NDEBUG
	state.getCardCount();
#endif

	return perform(state, state.isNoJack() ? DecisionBase::removeJack(state.getPlayerCards()) :
				   state.getPlayerCards());
}

void AbstractAction::countSuits(SUITCOUNT *suitCount,
								const NetMauMau::Player::IPlayer::CARDS &myCards) {

	std::memset(suitCount, 0, sizeof(SUITCOUNT) * 4);

	const bool noCards = myCards.empty();

	for(std::size_t i = 0; i < 4; ++i) {

		const SUITCOUNT sc = {
			SUIT[i], noCards ? 0 : DecisionBase::countSuit(myCards, SUIT[i])
		};

		suitCount[i] = sc;
	}

	if(!noCards) std::sort(suitCount, suitCount + 4);
}

NetMauMau::Common::ICard::SUIT AbstractAction::getMaxPlayedOffSuit(const IAIState &state,
		NetMauMau::Player::IPlayer::CARDS::difference_type *count) {

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

const NetMauMau::Common::ICard::SUIT *AbstractAction::getSuits() {
	return SUIT;
}

const IConditionPtr &AbstractAction::getNullCondition() {
	return NULLCONDITION;
}

NetMauMau::Player::IPlayer::CARDS::iterator
AbstractAction::pullSuit(NetMauMau::Player::IPlayer::CARDS &cards,
						 NetMauMau::Common::ICard::SUIT suit) {
	return std::partition(cards.begin(), cards.end(),
						  std::bind2nd(std::ptr_fun(NetMauMau::Common::isSuit), suit));
}

NetMauMau::Player::IPlayer::CARDS::iterator
AbstractAction::pullRank(NetMauMau::Player::IPlayer::CARDS &cards,
						 NetMauMau::Common::ICard::RANK rank) {
	return std::stable_partition(cards.begin(), cards.end(),
								 std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank), rank));
}

NetMauMau::Player::IPlayer::CARDS::iterator
AbstractAction::pullRank(const NetMauMau::Player::IPlayer::CARDS::iterator &first,
						 const NetMauMau::Player::IPlayer::CARDS::iterator &last,
						 NetMauMau::Common::ICard::RANK rank) {
	return std::stable_partition(first, last, std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
								 rank));
}

NetMauMau::Player::IPlayer::CARDS::iterator
AbstractAction::pushRank(const NetMauMau::Player::IPlayer::CARDS::iterator &first,
						 const NetMauMau::Player::IPlayer::CARDS::iterator &last,
						 NetMauMau::Common::ICard::RANK rank) {
	return std::partition(first, last,
						  std::not1(std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank), rank)));
}

NetMauMau::Player::IPlayer::CARDS::iterator
AbstractAction::pullSpecialRank(NetMauMau::Player::IPlayer::CARDS &cards,
								NetMauMau::Common::ICard::RANK rank, bool nineIsEight) {
	return std::partition(cards.begin(), cards.end(), std::bind2nd(_isSpecialRank(nineIsEight),
						  rank));
}

NetMauMau::Common::ICardPtr
AbstractAction::hasRankPath(const NetMauMau::Common::ICardPtr &uc,
							NetMauMau::Common::ICard::SUIT s, NetMauMau::Common::ICard::RANK r,
							const NetMauMau::Player::IPlayer::CARDS &c, bool nineIsEight) {

	NetMauMau::Player::IPlayer::CARDS mCards(c);

	if(mCards.size() > 1) {

		const NetMauMau::Player::IPlayer::CARDS::iterator &e(pullSpecialRank(mCards, r,
				nineIsEight));

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

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
