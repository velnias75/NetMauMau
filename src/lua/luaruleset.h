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

#ifndef NETMAUMAU_RULESET_LUARULESET_H
#define NETMAUMAU_RULESET_LUARULESET_H

#include "iruleset.h"

namespace NetMauMau {

class IAceRoundListener;

namespace RuleSet {

class LuaRuleSet : public IRuleSet {
	DISALLOW_COPY_AND_ASSIGN(LuaRuleSet)
public:
	LuaRuleSet(bool dirChangePossible, std::size_t initialCardCount = 5,
			   const IAceRoundListener *l = 0L);
	virtual ~LuaRuleSet();

	virtual void checkInitial(const Player::IPlayer *player, const Common::ICard *playedCard);
	virtual bool checkCard(const Player::IPlayer *player, const Common::ICard *uncoveredCard,
						   const Common::ICard *playedCard, bool ai);
	virtual bool checkCard(const Common::ICard *uncoveredCard,
						   const Common::ICard *playedCard) const;

	virtual std::size_t lostPointFactor(const Common::ICard *uncoveredCard) const;

	virtual bool hasToSuspend() const;
	virtual void hasSuspended();
	virtual std::size_t takeCardCount() const;
	virtual std::size_t takeCards(const Common::ICard *playedCard) const;
	virtual void hasTakenCards();

	virtual std::size_t initialCardCount() const;
	virtual bool suspendIfNoMatchingCard() const;
	virtual bool takeIfLost() const;

	virtual bool isAceRoundPossible() const;
	virtual Common::ICard::RANK getAceRoundRank() const;

	virtual bool hasDirChange() const;
	virtual void dirChanged();
	virtual bool getDirChangeIsSuspend() const;
	virtual void setDirChangeIsSuspend(bool suspend);

	virtual bool isAceRound() const;
	virtual bool isJackMode() const;
	virtual Common::ICard::SUIT getJackSuit() const;
	virtual void setJackModeOff();

	virtual std::size_t getMaxPlayers() const;
	virtual void setCurPlayers(std::size_t players);

	virtual void reset() throw();
};

}

}

#endif /* #define NETMAUMAU_RULESET_LUARULESET_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
