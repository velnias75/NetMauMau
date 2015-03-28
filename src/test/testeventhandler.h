/*
 * Copyright 2014-2015 by Heiko Schäfer <heiko@rangun.de>
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
	explicit TestEventHandler();
	virtual ~TestEventHandler();

	virtual void playerAdded(const NetMauMau::Player::IPlayer *player) const
	throw(NetMauMau::Common::Exception::SocketException);

	virtual void cardsDistributed(const NetMauMau::Player::IPlayer *player,
								  const CARDS &cards) const
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void initialCard(const NetMauMau::Common::ICard *initialCard) const
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void cardsAlreadyDistributed() const
	throw(NetMauMau::Common::Exception::SocketException);

	virtual void playerPicksCard(const NetMauMau::Player::IPlayer *player,
								 const NetMauMau::Common::ICard *card) const
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void playerPicksCards(const NetMauMau::Player::IPlayer *player,
								  std::size_t cardCount) const
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void playerSuspends(const NetMauMau::Player::IPlayer *player,
								const NetMauMau::Common::ICard *dueCard) const
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void playerPlaysCard(const NetMauMau::Player::IPlayer *player,
								 const NetMauMau::Common::ICard *playedCard,
								 const NetMauMau::Common::ICard *unvoredCard) const
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void playerChooseJackSuit(const NetMauMau::Player::IPlayer *player,
									  NetMauMau::Common::ICard::SUIT suit) const
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void playerWins(const NetMauMau::Player::IPlayer *player, std::size_t turn,
							bool ultimate) const
	throw(NetMauMau::Common::Exception::SocketException);
	virtual std::size_t playerLost(const NetMauMau::Player::IPlayer *player, std::size_t turn,
								   std::size_t pointFactor) const
	throw(NetMauMau::Common::Exception::SocketException);

	virtual void setJackModeOff() const throw(NetMauMau::Common::Exception::SocketException) _CONST;

	virtual void aceRoundStarted(const NetMauMau::Player::IPlayer *player)
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void aceRoundEnded(const NetMauMau::Player::IPlayer *player)
	throw(NetMauMau::Common::Exception::SocketException);

private:
	bool m_isatty;
};

#endif /* NETMAUMAU_TESTEVENTHANDLER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
