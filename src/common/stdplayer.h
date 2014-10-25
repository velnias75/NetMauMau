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

#ifndef NETMAUMAU_STDPLAYER_H
#define NETMAUMAU_STDPLAYER_H

#include "iplayer.h"

namespace NetMauMau {

namespace Player {

class _EXPORT StdPlayer : public IPlayer {
	DISALLOW_COPY_AND_ASSIGN(StdPlayer)
public:
	StdPlayer(const std::string &name);
	virtual ~StdPlayer();

	virtual std::string getName() const;
	virtual int getSerial() const _CONST;

	virtual void receiveCard(ICard *card);
	virtual void receiveCardSet(const std::vector<ICard *> &cards);

	virtual ICard *requestCard(const ICard *uncoveredCard, const ICard::SUITE *jackSuite) const;
	virtual ICard::SUITE getJackChoice(const ICard *uncoveredCard, const ICard *playedCard) const;

	virtual bool cardAccepted(const ICard *playedCard);

	virtual REASON getNoCardReason() const _PURE;

	virtual std::size_t getCardCount() const _PURE;
	virtual std::size_t getPoints() const;

	virtual void reset();

	static void resetJackState();

protected:
	const std::vector<ICard *> &getPlayerCards() const _CONST;

private:
	ICard *findBestCard(const NetMauMau::ICard *uc, const NetMauMau::ICard::SUITE *js,
						bool noJack) const;

private:
	std::string m_name;
	mutable std::vector<ICard *> m_cards;
	static bool m_jackPlayed;
};

}

}

#endif /* NETMAUMAU_STDPLAYER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
