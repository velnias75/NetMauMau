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

#ifndef NETMAUMAU_SERVEREVENTHANDLER_H
#define NETMAUMAU_SERVEREVENTHANDLER_H

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#include "defaulteventhandler.h"

#include "serverconnection.h"

namespace NetMauMau {

namespace Server {

class EventHandler : public Event::DefaultEventHandler {
	DISALLOW_COPY_AND_ASSIGN(EventHandler)
public:
	EventHandler(Connection &con);
	virtual ~EventHandler();

	virtual Connection *getConnection() const _PURE;
	virtual bool shutdown() const _PURE;
	virtual void reset();

	virtual void message(const std::string &msg, const std::vector<std::string> &except)
	throw(Common::Exception::SocketException);
	virtual void error(const std::string &msg, const std::vector<std::string> &except)
	throw(Common::Exception::SocketException);

	virtual void playerAdded(const Player::IPlayer *player)
	throw(Common::Exception::SocketException);
	virtual void playerRejected(const Player::IPlayer *player)
	throw(Common::Exception::SocketException);

	virtual void initialCard(const Common::ICard *initialCard)
	throw(Common::Exception::SocketException);

	virtual void stats(const Engine::PLAYERS &m_players) throw(Common::Exception::SocketException);
	virtual void turn(std::size_t turn) throw(Common::Exception::SocketException);

	virtual void playerWins(const Player::IPlayer *player,
							std::size_t turn) throw(Common::Exception::SocketException);
	virtual void playerPlaysCard(const Player::IPlayer *player, const Common::ICard *playedCard,
								 const Common::ICard *uncoveredCard)
	throw(Common::Exception::SocketException);
	virtual void cardRejected(Player::IPlayer *player, const Common::ICard *uncoveredCard,
							  const Common::ICard *playedCard)
	throw(Common::Exception::SocketException);
	virtual void playerSuspends(const Player::IPlayer *player, const Common::ICard *dueCard)
	throw(Common::Exception::SocketException);
	virtual void playerChooseJackSuit(const Player::IPlayer *player, Common::ICard::SUIT suit)
	throw(Common::Exception::SocketException);
	virtual void playerPicksCard(const Player::IPlayer *player,
								 const Common::ICard *card)
	throw(Common::Exception::SocketException);
	virtual void playerPicksCards(const Player::IPlayer *player,
								  std::size_t cardCount) throw(Common::Exception::SocketException);
	virtual void nextPlayer(const Player::IPlayer *player)
	throw(Common::Exception::SocketException);

	static void setInterrupted();

private:
	void message_internal(const std::string &type, const std::string &msg,
						  const std::vector<std::string> &except)
	throw(Common::Exception::SocketException);

private:
	static bool m_interrupt;
	Connection &m_connection;
	std::string m_lastMsg;
};

}

}

#endif /* NETMAUMAU_SERVEREVENTHANDLER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
