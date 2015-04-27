/*
 * Copyright 2015 by Heiko Schäfer <heiko@rangun.de>
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

#ifndef NETMAUMAU_ENGINE_AIDT_IAISTATE_H
#define NETMAUMAU_ENGINE_AIDT_IAISTATE_H

#include "iplayer.h"

namespace NetMauMau {

namespace AIDT {

class IAIState {
	DISALLOW_COPY_AND_ASSIGN(IAIState)
public:
	typedef std::vector<std::string> PLAYEDOUTCARDS;

	virtual ~IAIState() {}

	virtual std::string getName() const = 0;

	virtual void setCard(const Common::ICardPtr &card) = 0;

	virtual Common::ICardPtr getCard() const = 0;

	virtual std::size_t getCardCount() const = 0;

	virtual Common::ICardPtr getPlayedCard() const = 0;

	virtual const Player::IPlayer::CARDS &getPlayerCards() const = 0;

	virtual Common::ICardPtr getUncoveredCard() const = 0;

	virtual const RuleSet::IRuleSet *getRuleSet() const = 0;

	virtual const std::vector<std::string> &getPlayedOutCards() const = 0;

	virtual bool hasPlayerFewCards() const = 0;

	virtual Common::ICard::SUIT *getJackSuit() const = 0;

	virtual void clearJackSuit() = 0;

	virtual bool isNoJack() const = 0;

	virtual void setNoJack(bool b) = 0;

	virtual bool hasTakenCards() const = 0;

	virtual void setCardsTaken(bool b) = 0;

	virtual std::size_t getTalonFactor() const = 0;

	virtual std::size_t getPlayerCount() const = 0;

	virtual std::size_t getLeftCount() const = 0;

	virtual std::size_t getRightCount() const = 0;

	virtual bool nineIsEight() const = 0;

	virtual bool isDirChgEnabled() const = 0;

	virtual bool tryAceRound() const = 0;

	virtual void setTryAceRound(bool b) = 0;

	virtual Common::ICard::SUIT getPowerSuit() const = 0;

	virtual void setPowerSuit(Common::ICard::SUIT suit) = 0;

	virtual bool isPowerPlay() const = 0;

	virtual void setPowerPlay(bool b) = 0;

protected:
	IAIState() {}
};

}

}

#endif /* NETMAUMAU_ENGINE_AIDT_IAISTATE_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
