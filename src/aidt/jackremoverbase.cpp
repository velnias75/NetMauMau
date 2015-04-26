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

#include "jackremoverbase.h"

#include "smartptr.h"
#include "cardtools.h"

using namespace NetMauMau::AIDT;

JackRemoverBase::JackRemoverBase() {}

JackRemoverBase::~JackRemoverBase() {}

NetMauMau::Player::IPlayer::CARDS
JackRemoverBase::removeJack(const NetMauMau::Player::IPlayer::CARDS &cards) {

	NetMauMau::Player::IPlayer::CARDS myCards(cards);

	myCards.erase(std::remove_if(myCards.begin(), myCards.end(),
								 std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
										 NetMauMau::Common::ICard::JACK)), myCards.end());
	return myCards;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
