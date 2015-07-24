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

#include <cstddef>                      // for size_t
#include <vector>                       // for vector

#include "iruleset.h"                   // for IRuleSet
#include "luaexception.h"               // for LuaException

namespace NetMauMau {

class IAceRoundListener;

namespace RuleSet {

class LuaRuleSet : public IRuleSet {
	DISALLOW_COPY_AND_ASSIGN(LuaRuleSet)
public:
	explicit LuaRuleSet(const std::vector<std::string> &luafiles, bool dirChangePossible,
						std::size_t initialCardCount = 5,
						const IAceRoundListener *l = 0L) throw(Lua::Exception::LuaException);
	virtual ~LuaRuleSet() _CONST;

	virtual bool isNull() const _CONST;

	virtual void checkInitial(const Player::IPlayer *player,
							  const Common::ICardPtr &playedCard) throw(Lua::Exception::LuaException);
	virtual bool checkCard(const Player::IPlayer *player, const Common::ICardPtr &uncoveredCard,
						   const Common::ICardPtr &playedCard,
						   bool ai) throw(Lua::Exception::LuaException);
	virtual bool checkCard(const Common::ICardPtr &uncoveredCard,
						   const Common::ICardPtr &playedCard) const
	throw(Lua::Exception::LuaException);

	virtual std::size_t lostPointFactor(const Common::ICardPtr &uncoveredCard) const
	throw(Lua::Exception::LuaException);

	virtual bool hasToSuspend() const throw(Lua::Exception::LuaException);
	virtual void hasSuspended() throw(Lua::Exception::LuaException);
	virtual std::size_t takeCardCount() const throw(Lua::Exception::LuaException);
	virtual std::size_t takeCards(const Common::ICard *playedCard) const
	throw(Lua::Exception::LuaException);
	virtual void hasTakenCards() throw(Lua::Exception::LuaException);

	virtual std::size_t initialCardCount() const throw(Lua::Exception::LuaException);
	virtual bool takeAfterSevenIfNoMatch() const throw(Lua::Exception::LuaException);
	virtual bool takeIfLost() const throw(Lua::Exception::LuaException);

	virtual bool isAceRoundPossible() const throw(Lua::Exception::LuaException);
	virtual Common::ICard::RANK getAceRoundRank() const throw(Lua::Exception::LuaException);

	virtual bool hasDirChange() const throw(Lua::Exception::LuaException);
	virtual void dirChanged() throw(Lua::Exception::LuaException);
	virtual bool getDirChangeIsSuspend() const throw(Lua::Exception::LuaException);
	virtual void setDirChangeIsSuspend(bool suspend) throw(Lua::Exception::LuaException);

	virtual bool isAceRound() const throw(Lua::Exception::LuaException);
	virtual bool isJackMode() const throw(Lua::Exception::LuaException);
	virtual Common::ICard::SUIT getJackSuit() const throw(Lua::Exception::LuaException);
	virtual void setJackModeOff() throw(Lua::Exception::LuaException);

	virtual std::size_t getMaxPlayers() const throw(Lua::Exception::LuaException);
	virtual void setCurPlayers(std::size_t players) throw(Lua::Exception::LuaException);

	virtual void reset() throw();

private:
	static std::vector<const char *> checkInterface();
};

}

}

#endif /* #define NETMAUMAU_RULESET_LUARULESET_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
