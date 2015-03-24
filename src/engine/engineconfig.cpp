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

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#include <cstdlib>

#include <sys/stat.h>

#include "engineconfig.h"

#include "luaruleset.h"

namespace {
#ifndef _WIN32
const char *STDRULESLUA = "/stdrules.lua";
#else
const char *STDRULESLUA = "\\stdrules.lua";
#endif
}

using namespace NetMauMau;

EngineConfig::EngineConfig(Event::IEventHandler &eventHandler, bool dirChange, long aiDelay,
						   bool nextMessage, char aceRound, std::size_t factor,
						   std::size_t initialCardCount) : m_eventHandler(eventHandler),
	m_dirChange(dirChange), m_aiDelay(aiDelay), m_nextMessage(nextMessage),
	m_aceRoundRank(aceRound == 'A' ? Common::ICard::ACE : (aceRound == 'Q' ? Common::ICard::QUEEN :
				   (aceRound == 'K' ? Common::ICard::KING : Common::ICard::RANK_ILLEGAL))),
	m_ruleset(0L), m_aceRound(aceRound), m_talonFactor(factor),
	m_initialCardCount(initialCardCount) {}

EngineConfig::EngineConfig(const EngineConfig &o) : m_eventHandler(o.m_eventHandler),
	m_dirChange(o.m_dirChange), m_aiDelay(o.m_aiDelay), m_nextMessage(o.m_nextMessage),
	m_aceRoundRank(o.m_aceRoundRank), m_ruleset(o.m_ruleset), m_aceRound(o.m_aceRound),
	m_talonFactor(o.m_talonFactor), m_initialCardCount(o.m_initialCardCount) {}

EngineConfig::~EngineConfig() {
	delete m_ruleset;
}

long EngineConfig::getAIDelay() const {
	return m_aiDelay;
}

bool EngineConfig::getDirChange() const {
	return m_dirChange;
}

Event::IEventHandler &EngineConfig::getEventHandler() const {
	return m_eventHandler;
}

bool EngineConfig::getNextMessage() const {
	return m_nextMessage;
}

void EngineConfig::setNextMessage(bool b) {
	m_nextMessage = b;
}

RuleSet::IRuleSet *EngineConfig::getRuleSet(const NetMauMau::IAceRoundListener *arl) const {
	return m_ruleset ? m_ruleset : (m_ruleset = new RuleSet::LuaRuleSet(getLuaScriptPath(),
			m_dirChange, m_initialCardCount, m_aceRound ? arl : 0L));
}

char EngineConfig::getAceRound() const {
	return m_aceRound;
}

Common::ICard::RANK EngineConfig::getAceRoundRank() const {
	return m_aceRoundRank;
}

std::size_t EngineConfig::getTalonFactor() const {
	return m_talonFactor;
}
#include "logger.h"
std::string EngineConfig::getLuaScriptPath() {

	char *luaDir = std::getenv("NETMAUMAU_RULESDIR");

	struct stat ls;

	if(!(luaDir && !stat((std::string(luaDir) + STDRULESLUA).c_str(), &ls))) {

		luaDir = std::getenv("HOME");

		if(!(luaDir && !stat((std::string(luaDir) + "/." +
							  PACKAGE_NAME + STDRULESLUA).c_str(), &ls))) {
			return std::string(LUADIR) + STDRULESLUA;
		}
	}

	return std::string(luaDir) + STDRULESLUA;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
