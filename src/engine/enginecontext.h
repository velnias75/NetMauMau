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

#ifndef NETMAUMAU_ENGINECONTEXT_H
#define NETMAUMAU_ENGINECONTEXT_H

#include <vector>
#include <cstddef>                      // for size_t

#include "cardtools.h"
#include "luaexception.h"
#include "nullaceroundlistener.h"

namespace NetMauMau {

namespace Event {
class IEventHandler;
}

namespace RuleSet {
class IRuleSet;
}

class EngineContext {
	EngineContext &operator=(const EngineContext &);
public:
	explicit EngineContext(const EngineContext &);
	explicit EngineContext(Event::IEventHandler &eventHandler, bool dirChange, long aiDelay = 1000L,
						   bool nextMessage = true, char aceRound = 0,
						   const Common::CARDCONFIG &cc = Common::CARDCONFIG());
	~EngineContext();

	inline Event::IEventHandler &getEventHandler() const {
		return m_eventHandler;
	}

	inline bool getDirChange() const {
		return m_dirChange;
	}

	inline long getAIDelay() const {
		return m_aiDelay;
	}

	inline bool getNextMessage() const {
		return m_nextMessage;
	}

	inline void setNextMessage(bool b) {
		m_nextMessage = b;
	}

	inline char getAceRound() const {
		return m_aceRound;
	}

	inline Common::ICard::RANK getAceRoundRank() const {
		return m_aceRoundRank;
	}

	RuleSet::IRuleSet *getRuleSet(const NetMauMau::IAceRoundListener *arl =
									  NullAceRoundListener::getInstance()) const
	throw(Lua::Exception::LuaException) _NONNULL_ALL;

	inline std::size_t getTalonFactor() const {
		return m_talonFactor;
	}

private:
	static std::vector<std::string> getLuaScriptPaths();

private:
	Event::IEventHandler &m_eventHandler;
	const bool m_dirChange;
	const long m_aiDelay;
	bool m_nextMessage;
	const Common::ICard::RANK m_aceRoundRank;
	mutable RuleSet::IRuleSet *m_ruleset;
	const char m_aceRound;
	const std::size_t m_talonFactor;
	const std::size_t m_initialCardCount;
};

}

#endif /* NETMAUMAU_ENGINECONTEXT_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
