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

#ifndef NETMAUMAU_NULLRULESET_H
#define NETMAUMAU_NULLRULESET_H

#include "iruleset.h"
#include "smartptr.h"

namespace NetMauMau {

namespace RuleSet {

class NullRuleSet : public IRuleSet {
	DISALLOW_COPY_AND_ASSIGN(NullRuleSet)
public:
	typedef Common::SmartPtr<NullRuleSet> NullRuleSetPtr;

	~NullRuleSet();

	static NullRuleSetPtr getInstance();

	virtual bool isNull() const throw() _CONST;

	virtual void checkInitial(const Player::IPlayer *player,
							  const Common::ICardPtr &playedCard) _CONST;
	virtual bool checkCard(const Player::IPlayer *player, const Common::ICardPtr &uncoveredCard,
						   const Common::ICardPtr &playedCard, bool ai) _CONST;
	virtual bool checkCard(const Common::ICardPtr &uncoveredCard,
						   const Common::ICardPtr &playedCard) const _CONST;

	virtual std::size_t lostPointFactor(const Common::ICardPtr &uncoveredCard) const _CONST;

	virtual bool hasToSuspend() const _CONST;
	virtual void hasSuspended() _CONST;
	virtual void hasTakenCards() _CONST;
	virtual std::size_t takeCardCount() const _CONST;
	virtual std::size_t takeCards(const Common::ICard *playedCard) const _CONST;

	virtual std::size_t initialCardCount() const _CONST;
	virtual bool takeAfterSevenIfNoMatch() const _CONST;
	virtual bool takeIfLost() const _CONST;

	virtual bool isAceRoundPossible() const _CONST;
	virtual Common::ICard::RANK getAceRoundRank() const _CONST;

	virtual bool hasDirChange() const _CONST;
	virtual void dirChanged() _CONST;
	virtual bool getDirChangeIsSuspend() const _CONST;
	virtual void setDirChangeIsSuspend(bool suspend) _CONST;

	virtual bool isAceRound() const _CONST;
	virtual bool isJackMode() const _CONST;
	virtual Common::ICard::SUIT getJackSuit() const _CONST;
	virtual void setJackModeOff() _CONST;

	virtual std::size_t getMaxPlayers() const _CONST;
	virtual void setCurPlayers(std::size_t players) _CONST;

	virtual void reset() throw() _CONST;

private:
	NullRuleSet();

private:
	static NullRuleSetPtr m_instance;
};

}

}

#endif /* NETMAUMAU_NULLRULESET_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
