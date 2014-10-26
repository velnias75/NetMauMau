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

/**
 * @file abstractclient.h
 * @author Heiko Schäfer <heiko@rangun.de>
 */

#ifndef NETMAUMAU_ABSTRACTCLIENT_H
#define NETMAUMAU_ABSTRACTCLIENT_H

#include <vector>

#include "icard.h"
#include "clientconnection.h"

namespace NetMauMau {

/**
 * @brief tbw
 */
namespace Client {

/**
 * @brief tbw
 */
class _EXPORT AbstractClient {
	DISALLOW_COPY_AND_ASSIGN(AbstractClient)
public:
	virtual ~AbstractClient();

	/**
	 * @brief ...
	 *
	 * @param timeout ...
	 */
	void play(timeval *timeout = 0L) throw(Common::Exception::SocketException);

	/**
	 * @brief ...
	 *
	 * @param timeout ...
	 * @return NetMauMau::Common::AbstractConnection::CAPABILITIES
	 */
	Connection::CAPABILITIES capabilities(timeval *timeout = 0L)
	throw(Common::Exception::SocketException);

protected:
	/**
	 * @brief tbw
	 */
	typedef struct {
		std::string playerName; ///< tbw
		std::size_t cardCount; ///< tbw
	} STAT;

	/**
	 * @brief tbw
	 */
	typedef std::vector<STAT> STATS;

	/**
	 * @brief tbw
	 */
	typedef std::vector<Common::ICard *> CARDS;

	/**
	 * @brief ...
	 *
	 * @param pName ...
	 * @param server ...
	 * @param port ...
	 */
	AbstractClient(const std::string &player, const std::string &server, uint16_t port);

	/**
	 * @brief ...
	 *
	 * @return std::string
	 */
	std::string getPlayerName() const;

	/**
	 * @brief ...
	 *
	 * @param msg ...
	 */
	virtual void message(const std::string &msg) = 0;

	/**
	 * @brief ...
	 *
	 * @param msg ...
	 */
	virtual void error(const std::string &msg) = 0;

	/**
	 * @brief ...
	 *
	 * @param turn ...
	 */
	virtual void turn(std::size_t turn) const = 0;

	/**
	 * @brief ...
	 *
	 * @param stats ...
	 */
	virtual void stats(const STATS &stats) const = 0;

	/**
	 * @brief ...
	 */
	virtual void gameOver() const = 0;

	/**
	 * @brief ...
	 *
	 * @param cards ...
	 * @return NetMauMau::Common::ICard*
	 */
	virtual Common::ICard *playCard(const CARDS &cards) const = 0;

	/**
	 * @brief ...
	 *
	 * @return NetMauMau::Common::ICard::SUIT
	 */
	virtual Common::ICard::SUIT getJackSuitChoice() const = 0;

	/**
	 * @brief ...
	 *
	 * @param player ...
	 */
	virtual void playerJoined(const std::string &player) const = 0;

	/**
	 * @brief ...
	 *
	 * @param player ...
	 */
	virtual void playerRejected(const std::string &player) const = 0;

	/**
	 * @brief ...
	 *
	 * @param player ...
	 */
	virtual void playerSuspends(const std::string &player) const = 0;

	/**
	 * @brief ...
	 *
	 * @param player ...
	 * @param card ...
	 */
	virtual void playedCard(const std::string &player, const Common::ICard *card) const = 0;

	/**
	 * @brief ...
	 *
	 * @param player ...
	 * @param turn ...
	 */
	virtual void playerWins(const std::string &player, std::size_t turn) const = 0;

	/**
	 * @brief ...
	 *
	 * @param player ...
	 * @param card ...
	 */
	virtual void playerPicksCard(const std::string &player, const Common::ICard *card) const = 0;

	/**
	 * @brief ...
	 *
	 * @param player ...
	 * @param count ...
	 */
	virtual void playerPicksCard(const std::string &player, std::size_t count) const = 0;

	/**
	 * @brief ...
	 *
	 * @param player ...
	 */
	virtual void nextPlayer(const std::string &player) const = 0;

	/**
	 * @brief ...
	 *
	 * @param cards ...
	 */
	virtual void cardSet(const CARDS &cards) const = 0;

	/**
	 * @brief ...
	 *
	 * @param card ...
	 */
	virtual void initialCard(const Common::ICard *card) const = 0;

	/**
	 * @brief ...
	 *
	 * @param card ...
	 * @param jackSuit ...
	 */
	virtual void openCard(const Common::ICard *card, const std::string &jackSuit) const = 0;

	/**
	 * @brief ...
	 *
	 * @param player ...
	 * @param card ...
	 */
	virtual void cardRejected(const std::string &player, const Common::ICard *card) const = 0;

	/**
	 * @brief ...
	 *
	 * @param suit ...
	 */
	virtual void jackSuit(Common::ICard::SUIT suit) = 0;

private:
	CARDS getCards(const std::vector<Common::ICard *>::const_iterator &first) const;

private:
	Connection m_connection;
	std::string m_pName;
	std::vector<Common::ICard *> m_cards;
	Common::ICard *m_openCard;
};

}

}

#endif /* NETMAUMAU_ABSTRACTCLIENT_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
