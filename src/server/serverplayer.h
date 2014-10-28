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

#ifndef NETMAUMAU_SERVERPLAYER_H
#define NETMAUMAU_SERVERPLAYER_H

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include "stdplayer.h"
#include "socketexception.h"

namespace NetMauMau {

class Engine;

namespace Server {

class Connection;

class Player : public NetMauMau::Player::StdPlayer {
	DISALLOW_COPY_AND_ASSIGN(Player)
public:
	Player(const std::string &name, int sockfd, Connection &con);
	virtual ~Player();

	virtual int getSerial() const _PURE;
	virtual bool isAIPlayer() const _CONST;

	virtual void receiveCard(Common::ICard *card);
	virtual void receiveCardSet(const std::vector<Common::ICard *> &cards);

	virtual Common::ICard *requestCard(const Common::ICard *uncoveredCard,
									   const Common::ICard::SUIT *jackSuit) const;
	virtual REASON getNoCardReason() const _CONST;
	virtual bool cardAccepted(const Common::ICard *playedCard);

	virtual std::size_t getCardCount() const throw(Common::Exception::SocketException);

	virtual Common::ICard::SUIT getJackChoice(const Common::ICard *uncoveredCard,
			const Common::ICard *playedCard) const;

private:
	_NOUNUSED Common::ICard *findCard(const std::string &offeredCard) const;

private:
	Connection &m_connection;
	int m_sockfd;
};

}

}

#endif /* NETMAUMAU_SERVERPLAYER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
