/*
 * Copyright 2014 by Heiko Schäfer <heiko@rangun.de>
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

#ifndef NETMAUMAU_TESTEVENTHANDLER_H
#define NETMAUMAU_TESTEVENTHANDLER_H

#include "defaulteventhandler.h"

class TestEventHandler : public NetMauMau::Event::DefaultEventHandler {
	DISALLOW_COPY_AND_ASSIGN(TestEventHandler)
public:
	TestEventHandler();
	virtual ~TestEventHandler();

	virtual void playerAdded(const NetMauMau::Player::IPlayer *player)
	throw(NetMauMau::Common::Exception::SocketException);

	virtual void cardsDistributed(const NetMauMau::Player::IPlayer *player,
								  const std::vector<NetMauMau::ICard *> &cards)
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void initialCard(const NetMauMau::ICard *initialCard)
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void cardsAlreadyDistributed() throw(NetMauMau::Common::Exception::SocketException);

	virtual void playerPicksCard(const NetMauMau::Player::IPlayer *player,
								 const NetMauMau::ICard *card)
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void playerPicksCards(const NetMauMau::Player::IPlayer *player, std::size_t cardCount)
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void playerSuspends(const NetMauMau::Player::IPlayer *player,
								const NetMauMau::ICard *dueCard)
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void playerPlaysCard(const NetMauMau::Player::IPlayer *player,
								 const NetMauMau::ICard *playedCard,
								 const NetMauMau::ICard *unvoredCard)
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void playerChooseJackSuite(const NetMauMau::Player::IPlayer *player,
									   NetMauMau::ICard::SUITE suite)
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void playerWins(const NetMauMau::Player::IPlayer *player,
							std::size_t turn) throw(NetMauMau::Common::Exception::SocketException);
	virtual void playerLost(const NetMauMau::Player::IPlayer *player,
							std::size_t turn) throw(NetMauMau::Common::Exception::SocketException);
};

#endif /* NETMAUMAU_TESTEVENTHANDLER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
