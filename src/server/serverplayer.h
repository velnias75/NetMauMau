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

#ifndef NETMAUMAU_SERVERPLAYER_H
#define NETMAUMAU_SERVERPLAYER_H

#include <stdint.h>                     // for uint32_t

#include "abstractplayer.h"             // for AbstractPlayer
#include "socketexception.h"            // for SocketException

namespace NetMauMau {

namespace Server {

class Connection;

class Player : public NetMauMau::Player::AbstractPlayer {
	DISALLOW_COPY_AND_ASSIGN(Player)
public:
	explicit Player(const std::string &name, int sockfd, Connection &con);
	virtual ~Player();

	virtual int getSerial() const _PURE;
	virtual bool isAIPlayer() const throw() _CONST;
	virtual bool isAlive() const;
	virtual TYPE getType() const throw() _CONST;

	virtual void receiveCard(const Common::ICardPtr &card);
	virtual void receiveCardSet(const CARDS &cards) throw(Common::Exception::SocketException);

	virtual Common::ICardPtr requestCard(const Common::ICardPtr &uncoveredCard,
										 const Common::ICard::SUIT *jackSuit,
										 std::size_t takeCount, bool noSuspend) const;
	virtual REASON getNoCardReason(const NetMauMau::Common::ICardPtr &uncoveredCard,
								   const NetMauMau::Common::ICard::SUIT *suit) const;
	virtual bool cardAccepted(const Common::ICard *playedCard)
	throw(NetMauMau::Common::Exception::SocketException);
	virtual void talonShuffled() throw(NetMauMau::Common::Exception::SocketException);

	virtual std::size_t getCardCount() const throw(Common::Exception::SocketException);

	virtual Common::ICard::SUIT getJackChoice(const Common::ICardPtr &uncoveredCard,
			const Common::ICardPtr &playedCard) const
	throw(NetMauMau::Common::Exception::SocketException);

	virtual bool getAceRoundChoice() const throw(NetMauMau::Common::Exception::SocketException);

protected:
	virtual void shuffleCards() _CONST;

private:
	_NOUNUSED Common::ICardPtr findCard(const std::string &offeredCard) const;
	uint32_t getClientVersion() const;

	std::string playerRead(SOCKET fd, std::size_t len = 1024) const
	throw(NetMauMau::Common::Exception::SocketException);

private:
	Connection &m_connection;
	const int m_sockfd;
};

}

}

#endif /* NETMAUMAU_SERVERPLAYER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
