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

#ifndef NETMAUMAU_ENGINECONFIG_H
#define NETMAUMAU_ENGINECONFIG_H

#include "icard.h"

namespace NetMauMau {

class IAceRoundListener;

namespace Event {
class IEventHandler;
}

namespace RuleSet {
class IRuleSet;
}

class EngineConfig {
	EngineConfig &operator=(const EngineConfig &);
public:
	explicit EngineConfig(const EngineConfig &);
	explicit EngineConfig(Event::IEventHandler &eventHandler, bool dirChange, long aiDelay = 1000L,
						  bool nextMessage = true, char aceRound = 0, std::size_t factor = 1,
						  std::size_t initialCardCount = 5);
	~EngineConfig();

	Event::IEventHandler &getEventHandler() const _PURE;
	bool getDirChange() const _PURE;
	long getAIDelay() const _PURE;
	bool getNextMessage() const _PURE;
	void setNextMessage(bool b);
	char getAceRound() const _PURE;
	Common::ICard::RANK getAceRoundRank() const _PURE;
	RuleSet::IRuleSet *getRuleSet(const NetMauMau::IAceRoundListener *arl = 0L) const;
	std::size_t getTalonFactor() const _PURE;

private:
	static std::string getLuaScriptPath();

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

#endif /* NETMAUMAU_ENGINECONFIG_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
