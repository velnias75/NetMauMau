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
	virtual void gameAboutToStart() const;
	virtual void gameOver() const throw(Common::Exception::SocketException);
	virtual bool shutdown() const throw() _PURE;
	virtual void reset() throw();

	virtual void message(const std::string &msg, const std::vector<std::string> &except) const
	throw(Common::Exception::SocketException);
	virtual void error(const std::string &msg, const std::vector<std::string> &except) const
	throw(Common::Exception::SocketException);

	virtual void directionChange() const throw(Common::Exception::SocketException);

	virtual void playerAdded(const Player::IPlayer *player) const
	throw(Common::Exception::SocketException);
	virtual void playerRejected(const Player::IPlayer *player) const
	throw(Common::Exception::SocketException);

	virtual void initialCard(const Common::ICard *initialCard) const
	throw(Common::Exception::SocketException);
	virtual void uncoveredCard(const Common::ICard *uncovedCard) const
	throw(Common::Exception::SocketException);
	virtual void talonEmpty(bool empty) const throw(Common::Exception::SocketException);

	virtual void stats(const Engine::PLAYERS &m_players) const
	throw(Common::Exception::SocketException);
	virtual void turn(std::size_t turn) const throw(Common::Exception::SocketException);

	virtual void playerWins(const Player::IPlayer *player, std::size_t turn,
							bool ultimate) const throw(Common::Exception::SocketException);
	virtual std::size_t playerLost(const Player::IPlayer *player, std::size_t turn,
								   std::size_t pointFactor) const
	throw(Common::Exception::SocketException);
	virtual void playerPlaysCard(const Player::IPlayer *player, const Common::ICard *playedCard,
								 const Common::ICard *uncoveredCard) const
	throw(Common::Exception::SocketException);
	virtual void cardRejected(Player::IPlayer *player, const Common::ICard *uncoveredCard,
							  const Common::ICard *playedCard) const
	throw(Common::Exception::SocketException);
	virtual void playerSuspends(const Player::IPlayer *player, const Common::ICard *dueCard) const
	throw(Common::Exception::SocketException);
	virtual void playerChooseJackSuit(const Player::IPlayer *player, Common::ICard::SUIT suit) const
	throw(Common::Exception::SocketException);
	virtual void playerPicksCard(const Player::IPlayer *player,
								 const Common::ICard *card) const
	throw(Common::Exception::SocketException);
	virtual void playerPicksCards(const Player::IPlayer *player, std::size_t cardCount) const
	throw(Common::Exception::SocketException);
	virtual void nextPlayer(const Player::IPlayer *player) const
	throw(Common::Exception::SocketException);

	virtual void setJackModeOff() const throw(Common::Exception::SocketException);

	virtual void aceRoundStarted(const Player::IPlayer *player)
	throw(Common::Exception::SocketException);
	virtual void aceRoundEnded(const Player::IPlayer *player)
	throw(Common::Exception::SocketException);

	static void setInterrupted();

private:
	void message_internal(const std::string &type, const std::string &msg,
						  const std::vector<std::string> &except) const
	throw(Common::Exception::SocketException);

private:
	static bool m_interrupt;
	Connection &m_connection;
	mutable std::string m_lastMsg;
};

}

}

#endif /* NETMAUMAU_SERVEREVENTHANDLER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
