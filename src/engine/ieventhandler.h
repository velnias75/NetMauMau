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

#ifndef NETMAUMAU_IEVENTHANDLER_H
#define NETMAUMAU_IEVENTHANDLER_H

#include "icard.h"
#include "engine.h"
#include "socketexception.h"

namespace NetMauMau {

namespace Common {
class AbstractConnection;
}

namespace Player {
class IPlayer;
}

namespace Event {

class IEventHandler {
	DISALLOW_COPY_AND_ASSIGN(IEventHandler)
public:
	virtual ~IEventHandler() {}

	virtual Common::AbstractConnection *getConnection() const = 0;
	virtual bool shutdown() const = 0;
	virtual void reset() throw() = 0;

	virtual void message(const std::string &msg,
						 const std::vector<std::string> &except = std::vector<std::string>())
	throw(Common::Exception::SocketException) = 0;

	virtual void error(const std::string &msg,
					   const std::vector<std::string> &except = std::vector<std::string>())
	throw(Common::Exception::SocketException) = 0;

	virtual void acceptingPlayers() throw(Common::Exception::SocketException) = 0;
	virtual void stats(const Engine::PLAYERS &m_players)
	throw(Common::Exception::SocketException) = 0;
	virtual void playerAdded(const Player::IPlayer *player)
	throw(Common::Exception::SocketException) = 0;
	virtual void playerRejected(const Player::IPlayer *player)
	throw(Common::Exception::SocketException) = 0;

	virtual void cardsDistributed(const Player::IPlayer *player,
								  const std::vector<Common::ICard *> &cards)
	throw(Common::Exception::SocketException) = 0;
	virtual void initialCard(const Common::ICard *initialCard)
	throw(Common::Exception::SocketException) = 0;
	virtual void uncoveredCard(const Common::ICard *uncovedCard)
	throw(Common::Exception::SocketException) = 0;
	virtual void talonEmpty(bool empty) throw(Common::Exception::SocketException) = 0;
	virtual void cardsAlreadyDistributed() throw(Common::Exception::SocketException) = 0;

	virtual void turn(std::size_t turn) throw(Common::Exception::SocketException) = 0;

	virtual void playerPicksCard(const Player::IPlayer *player, const Common::ICard *card)
	throw(Common::Exception::SocketException) = 0;
	virtual void playerPicksCards(const Player::IPlayer *player, std::size_t cardCount)
	throw(Common::Exception::SocketException) = 0;
	virtual void playerSuspends(const Player::IPlayer *player, const Common::ICard *dueCard = 0L)
	throw(Common::Exception::SocketException) = 0;
	virtual void playerPlaysCard(const Player::IPlayer *player, const Common::ICard *playedCard,
								 const Common::ICard *uncoveredCard)
	throw(Common::Exception::SocketException) = 0;
	virtual void cardRejected(Player::IPlayer *player, const Common::ICard *uncoveredCard,
							  const Common::ICard *playedCard)
	throw(Common::Exception::SocketException) = 0;
	virtual void playerChooseJackSuit(const Player::IPlayer *player, Common::ICard::SUIT suit)
	throw(Common::Exception::SocketException) = 0;
	virtual void playerWins(const Player::IPlayer *player,
							std::size_t turn) throw(Common::Exception::SocketException) = 0;
	virtual void playerLost(const Player::IPlayer *player,
							std::size_t turn) throw(Common::Exception::SocketException) = 0;
	virtual void nextPlayer(const Player::IPlayer *player)
	throw(Common::Exception::SocketException) = 0;

protected:
	IEventHandler() {}
};

}

}

#endif /* NETMAUMAU_IEVENTHANDLER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
