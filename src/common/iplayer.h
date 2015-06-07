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

#ifndef NETMAUMAU_PLAYER_IPLAYER_H
#define NETMAUMAU_PLAYER_IPLAYER_H

#include <vector>

#include "icard.h"

namespace NetMauMau {

class EngineContext;
class ICardCountObserver;

namespace RuleSet {
class IRuleSet;
}

namespace Player {

class IPlayer {
	DISALLOW_COPY_AND_ASSIGN(IPlayer)
public:
	typedef std::vector<NetMauMau::Common::ICardPtr> CARDS;
	typedef enum { MAUMAU, NOMATCH, SUSPEND } REASON;
	typedef enum { HUMAN, EASY, HARD } TYPE;

	virtual ~IPlayer() {}

	virtual std::string getName() const = 0;
	virtual int  getSerial() const = 0;
	virtual bool isAIPlayer() const = 0;
	virtual bool isAlive() const = 0;
	virtual TYPE getType() const = 0;

	virtual void setRuleSet(const RuleSet::IRuleSet *ruleset) = 0;
	virtual void setEngineContext(const EngineContext *engineCtx) = 0;
	virtual void setCardCountObserver(const ICardCountObserver *cco) = 0;

	virtual void receiveCard(const Common::ICardPtr &card) = 0;
	virtual void receiveCardSet(const CARDS &cards) = 0;

	virtual Common::ICardPtr requestCard(const Common::ICardPtr &uncoveredCard,
										 const Common::ICard::SUIT *jackSuit,
										 std::size_t takeCount, bool noSuspend = false) const = 0;
	virtual Common::ICard::SUIT getJackChoice(const Common::ICardPtr &uncoveredCard,
			const Common::ICardPtr &playedCard) const = 0;

	virtual bool getAceRoundChoice() const = 0;

	virtual void informAIStat(const IPlayer *player, std::size_t count, Common::ICard::SUIT lpSuit,
							  Common::ICard::RANK lpRank) = 0;

	virtual Common::ICard::SUIT getLastPlayedSuit() const = 0;
	virtual Common::ICard::RANK getLastPlayedRank() const = 0;

	virtual bool cardAccepted(const Common::ICard *playedCard) = 0;
	virtual void setNeighbourCardCount(std::size_t playerCount, std::size_t leftCount,
									   std::size_t rightCount) = 0;
	virtual void setDirChangeEnabled(bool dirChangeEnabled) = 0;
	virtual void talonShuffled() = 0;
	virtual void setNineIsSuspend(bool b) = 0;

	virtual REASON getNoCardReason(const NetMauMau::Common::ICardPtr &uncoveredCard,
								   const NetMauMau::Common::ICard::SUIT *suit) const = 0;

	virtual std::size_t getCardCount() const = 0;
	virtual std::size_t getPoints() const = 0;

	virtual void reset() = 0;

protected:
	explicit IPlayer() {}
};

}

}

#endif /* NETMAUMAU_PLAYER_IPLAYER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
