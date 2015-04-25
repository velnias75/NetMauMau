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

#include "aceroundcondition.h"
#include "cardtools.h"
#include "iaistate.h"
#include "iruleset.h"

namespace {

NetMauMau::AIDT::IConditionPtr ACEROUNDCOND(new NetMauMau::AIDT::AceRoundCondition());

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct cardGreater : std::binary_function < NetMauMau::Common::ICardPtr,
		NetMauMau::Common::ICardPtr, bool > {
	bool operator()(const NetMauMau::Common::ICardPtr &x,
					const NetMauMau::Common::ICardPtr &y) const {
		return !(x->getPoints() < y->getPoints());
	}
};

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

MaxSuitAction::MaxSuitAction() {}

MaxSuitAction::~MaxSuitAction() {}

const IConditionPtr &MaxSuitAction::operator()(IAIState &state) const {

	NetMauMau::Player::IPlayer::CARDS mc(state.getPlayerCards());

	SUITCOUNT suitCount[4];
	AbstractAction::countSuits(suitCount, mc);

	NetMauMau::Common::ICardPtr bestCard;

	for(std::size_t i = 0; i < 4; ++i) {

		const NetMauMau::Player::IPlayer::CARDS::iterator
		&e(std::partition(mc.begin(), mc.end(),
						  std::bind2nd(std::ptr_fun(NetMauMau::Common::isSuit),
									   suitCount[i].suit)));

		if(state.getJackSuit()) {

			if(suitCount[i].count && (suitCount[i].suit == *state.getJackSuit())) {
				std::partition(mc.begin(), e, std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
							   NetMauMau::Common::ICard::SEVEN));
				bestCard = mc.front();
				break;
			}

		} else if(!((state.getRuleSet()->isAceRoundPossible() && (bestCard =
						 hasRankPath(state.getUncoveredCard(), suitCount[i].suit,
									 state.getRuleSet()->getAceRoundRank(), mc,
									 state.nineIsEight()))) || (bestCard =
											 hasRankPath(state.getUncoveredCard(),
													 suitCount[i].suit,
													 NetMauMau::Common::ICard::EIGHT,
													 mc, state.nineIsEight())))) {

			std::sort(mc.begin(), e, cardGreater());

			std::stable_partition(mc.begin(), e,
								  std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
											   NetMauMau::Common::ICard::SEVEN));

			const NetMauMau::Player::IPlayer::CARDS::value_type f =
				NetMauMau::Common::findRank(state.getUncoveredCard()->getRank(), mc.begin(), e);

			if(f && (f->getRank() != NetMauMau::Common::ICard::SEVEN ||
					 state.getPlayedOutCards().size() > (4 * state.getTalonFactor()))) {
				bestCard = f;
				break;
			}
		}
	}

	if(!bestCard) {

		const NetMauMau::Player::IPlayer::CARDS::iterator
		&e(std::partition(mc.begin(), mc.end(),
						  std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
									   NetMauMau::Common::ICard::SEVEN)));

		if(!state.isNoJack()) std::partition(e, mc.end(),
												 std::not1(std::bind2nd
														 (std::ptr_fun(NetMauMau::Common::isRank),
																 NetMauMau::Common::ICard::JACK)));

		const NetMauMau::Player::IPlayer::CARDS::value_type f =
			NetMauMau::Common::findSuit(state.getJackSuit() ? *state.getJackSuit() :
										state.getUncoveredCard()->getSuit(), mc.begin(), mc.end());

		if(f && f->getRank() != NetMauMau::Common::ICard::JACK) bestCard = f;
	}

	state.setCard(bestCard);

	return ACEROUNDCOND;
}

NetMauMau::Common::ICardPtr
MaxSuitAction::hasRankPath(const NetMauMau::Common::ICardPtr &uc, NetMauMau::Common::ICard::SUIT s,
						   NetMauMau::Common::ICard::RANK r,
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

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
