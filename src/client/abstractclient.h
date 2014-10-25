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

#ifndef NETMAUMAU_ABSTRACTCLIENT_H
#define NETMAUMAU_ABSTRACTCLIENT_H

#include <vector>

#include "icard.h"
#include "clientconnection.h"

namespace NetMauMau {

namespace Client {

class _EXPORT AbstractClient {
	DISALLOW_COPY_AND_ASSIGN(AbstractClient)
public:
	virtual ~AbstractClient();

	void play(struct timeval *timeout = 0L) throw(Common::Exception::SocketException);
	Connection::CAPABILITIES capabilities(struct timeval *timeout = 0L)
	throw(Common::Exception::SocketException);

protected:
	typedef struct {
		std::string playerName;
		std::size_t cardCount;
	} STAT;

	typedef std::vector<STAT> STATS;
	typedef std::vector<ICard *> CARDS;

	AbstractClient(const std::string &pName, const std::string &server, uint16_t port);

	std::string getPlayerName() const;

	virtual void message(const std::string &msg) = 0;
	virtual void error(const std::string &msg) = 0;
	virtual void turn(std::size_t turn) const = 0;
	virtual void stats(const STATS &stats) const = 0;
	virtual void gameOver() const = 0;

	virtual ICard *playCard(const CARDS &cards) const = 0;
	virtual ICard::SUITE getJackSuiteChoice() const = 0;

	virtual void playerJoined(const std::string &player) const = 0;
	virtual void playerRejected(const std::string &player) const = 0;
	virtual void playerSuspends(const std::string &player) const = 0;
	virtual void playedCard(const std::string &player, const ICard *card) const = 0;
	virtual void playerWins(const std::string &player, std::size_t turn) const = 0;
	virtual void playerPicksCard(const std::string &player, const ICard *card) const = 0;
	virtual void playerPicksCard(const std::string &player, std::size_t count) const = 0;
	virtual void nextPlayer(const std::string &player) const = 0;

	virtual void cardSet(const CARDS &cards) const = 0;
	virtual void initialCard(const ICard *card) const = 0;
	virtual void openCard(const ICard *card, const std::string &jackSuite) const = 0;
	virtual void cardRejected(const std::string &player, const ICard *card) const = 0;
	virtual void jackSuite(const std::string &suite) = 0;

private:
	CARDS getCards(const std::vector<NetMauMau::ICard *>::const_iterator &first) const;

private:
	Connection m_connection;
	std::string m_pName;
	std::vector<ICard *> m_cards;
	ICard *m_openCard;
};

}

}

#endif /* NETMAUMAU_ABSTRACTCLIENT_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
