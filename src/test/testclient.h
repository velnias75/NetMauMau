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

#ifndef NETMAUMAU_TESTCLIENT_H
#define NETMAUMAU_TESTCLIENT_H

#include "abstractclient.h"

class TestClient : public NetMauMau::Client::AbstractClient {
	DISALLOW_COPY_AND_ASSIGN(TestClient)
public:
	TestClient(const std::string &pName, const std::string &server, uint16_t port);
	virtual ~TestClient();

protected:
	virtual void message(const std::string &msg);
	virtual void error(const std::string &msg);
	virtual void turn(std::size_t turn) const;
	virtual void stats(const STATS &stats) const;
	virtual void gameOver() const;

	virtual NetMauMau::Common::ICard *playCard(const CARDS &cards) const;
	virtual NetMauMau::Common::ICard::SUIT getJackSuitChoice() const;

	virtual void playerJoined(const std::string &player) const;
	virtual void playerRejected(const std::string &player) const;
	virtual void playerSuspends(const std::string &player) const;
	virtual void playedCard(const std::string &player, const NetMauMau::Common::ICard *card) const;
	virtual void playerWins(const std::string &player, std::size_t turn) const;
	virtual void playerLost(const std::string &player, std::size_t turn) const _CONST;
	virtual void playerPicksCard(const std::string &player,
								 const NetMauMau::Common::ICard *card) const;
	virtual void playerPicksCard(const std::string &player, std::size_t count) const;
	virtual void nextPlayer(const std::string &player) const;

	virtual void cardSet(const CARDS &cards) const;
	virtual void enableSuspend(bool enable) const _CONST;
	virtual void initialCard(const NetMauMau::Common::ICard *card) const;
	virtual void openCard(const NetMauMau::Common::ICard *card, const std::string &jackSuit) const;
	virtual void cardRejected(const std::string &player,
							  const NetMauMau::Common::ICard *card) const;
	virtual void cardAccepted(const NetMauMau::Common::ICard *card) const _CONST;
	virtual void jackSuit(NetMauMau::Common::ICard::SUIT suit);
};

#endif /* NETMAUMAU_TESTCLIENT_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
