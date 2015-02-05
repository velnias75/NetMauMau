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

#ifndef NETMAUMAU_STDRULESET_H
#define NETMAUMAU_STDRULESET_H

#include "iruleset.h"

namespace NetMauMau {

class IAceRoundListener;

namespace RuleSet {

class StdRuleSet : public IRuleSet {
	DISALLOW_COPY_AND_ASSIGN(StdRuleSet)
public:
	StdRuleSet(bool dirChangePossible, const IAceRoundListener *l = 0L);
	virtual ~StdRuleSet();

	virtual void checkInitial(const Player::IPlayer *player, const Common::ICard *playedCard);
	virtual bool checkCard(const Player::IPlayer *player, const Common::ICard *uncoveredCard,
						   const Common::ICard *playedCard, bool ai);
	virtual bool checkCard(const Common::ICard *uncoveredCard,
						   const Common::ICard *playedCard) const;

	virtual std::size_t lostPointFactor(const Common::ICard *uncoveredCard) const;

	virtual bool hasToSuspend() const _PURE;
	virtual void hasSuspended();

	virtual std::size_t initialCardCount() const _CONST;
	virtual bool suspendIfNoMatchingCard() const _CONST;
	virtual bool takeIfLost() const _CONST;

	virtual std::size_t takeCardCount() const _PURE;
	virtual std::size_t takeCards(const Common::ICard *playedCard) const;
	virtual void hasTakenCards();

	virtual bool hasDirChange() const _PURE;
	virtual void dirChanged();
	virtual bool getDirChangeIsSuspend() const _PURE;
	virtual void setDirChangeIsSuspend(bool suspend);

	virtual bool isAceRoundPossible() const _PURE;
	virtual Common::ICard::RANK getAceRoundRank() const;

	virtual bool isAceRound() const;
	virtual bool isJackMode() const _PURE;
	virtual Common::ICard::SUIT getJackSuit() const _PURE;
	virtual void setJackModeOff();

	virtual std::size_t getMaxPlayers() const _CONST;
	virtual void setCurPlayers(std::size_t players);

	virtual void reset() throw();

protected:
	virtual bool isDirChange(const Common::ICard *playedCard) const;

private:
	bool m_hasToSuspend;
	bool m_hasSuspended;
	std::size_t m_takeCardCount;
	bool m_jackMode;
	Common::ICard::SUIT m_jackSuit;
	bool m_aceRound;
	const Player::IPlayer *m_aceRoundPlayer;
	const IAceRoundListener *m_arl;
	std::size_t m_curPlayers;
	bool m_dirChange;
	bool m_dirChangeIsSuspend;
	bool m_dirChangePossible;
};

}

}

#endif /* NETMAUMAU_STDRULESET_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
