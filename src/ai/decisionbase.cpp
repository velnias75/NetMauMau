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

#include "decisionbase.h"

#include "cardtools.h"

namespace {

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

using namespace NetMauMau::AI;

DecisionBase::DecisionBase() {}

DecisionBase::~DecisionBase() {}

NetMauMau::Player::IPlayer::CARDS
DecisionBase::removeJack(const NetMauMau::Player::IPlayer::CARDS &cards) {

	NetMauMau::Player::IPlayer::CARDS myCards(cards);

	myCards.erase(std::remove_if(myCards.begin(), myCards.end(),
								 std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
										 NetMauMau::Common::ICard::JACK)), myCards.end());
	return myCards;
}

NetMauMau::Player::IPlayer::CARDS::difference_type
DecisionBase::countSuit(const NetMauMau::Player::IPlayer::CARDS &cards,
						   NetMauMau::Common::ICard::SUIT suit) {
	return std::count_if(cards.begin(), cards.end(),
						 std::bind2nd(std::ptr_fun(NetMauMau::Common::isSuit), suit));
}

NetMauMau::Player::IPlayer::CARDS::difference_type
DecisionBase::countRank(const NetMauMau::Player::IPlayer::CARDS &cards,
						   NetMauMau::Common::ICard::RANK rank) {
	return std::count_if(cards.begin(), cards.end(),
						 std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank), rank));
}

std::vector<std::string>::difference_type
DecisionBase::countPlayedOutRank(const std::vector<std::string> &porv,
									NetMauMau::Common::ICard::RANK rank) {
	return std::count_if(porv.begin(), porv.end(), std::bind2nd(playedOutRank(), rank));
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
