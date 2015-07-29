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
#include "config.h"                     // for PACKAGE_NAME
#endif

#include "enginecontext.h"

#ifdef _WIN32
#include "pathtools.h"
#endif

#include <sys/stat.h>                   // for stat
#include <cstring>

#include "logger.h"
#include "luaruleset.h"                 // for LuaRuleSet

namespace {
#ifndef _WIN32
const char *STDRULESLUA = "/stdrules.lua";
#else
const char *STDRULESLUA = "stdrules";
#endif
}

using namespace NetMauMau;

EngineContext::EngineContext(Event::IEventHandler &eventHandler, bool dirChange, long aiDelay,
							 bool nextMessage, char aceRound, const Common::CARDCONFIG &cc) :
	m_eventHandler(eventHandler), m_dirChange(dirChange), m_aiDelay(aiDelay),
	m_nextMessage(nextMessage), m_aceRoundRank(aceRound == 'A' ? Common::ICard::ACE :
			(aceRound == 'Q' ? Common::ICard::QUEEN : (aceRound == 'K' ?
					Common::ICard::KING : Common::ICard::RANK_ILLEGAL))), m_ruleset(0L),
	m_aceRound(aceRound), m_talonFactor(cc.decks),
	m_initialCardCount(cc.initialCards) {}

EngineContext::EngineContext(const EngineContext &o) : m_eventHandler(o.m_eventHandler),
	m_dirChange(o.m_dirChange), m_aiDelay(o.m_aiDelay), m_nextMessage(o.m_nextMessage),
	m_aceRoundRank(o.m_aceRoundRank), m_ruleset(o.m_ruleset), m_aceRound(o.m_aceRound),
	m_talonFactor(o.m_talonFactor), m_initialCardCount(o.m_initialCardCount) {}

EngineContext::~EngineContext() {
	delete m_ruleset;
}

RuleSet::IRuleSet *EngineContext::getRuleSet(const NetMauMau::IAceRoundListener *arl) const
throw(Lua::Exception::LuaException) {
	return m_ruleset ? m_ruleset : (m_ruleset = new RuleSet::LuaRuleSet(getLuaScriptPaths(),
			m_dirChange, m_initialCardCount, m_aceRound ? arl :
			NullAceRoundListener::getInstance()));
}

std::vector<std::string> EngineContext::getLuaScriptPaths() {

	char *luaDir = std::getenv("NETMAUMAU_RULES");

	struct stat ls;

	if(luaDir) return std::vector<std::string>(1, std::string(luaDir));

	std::vector<std::string> addPaths;

	addPaths.reserve(3);

#ifndef _WIN32
	luaDir = strdup(SYSCONFDIR);
#endif

#ifndef _WIN32

	logDebug("Searching \"" << LUADIR << "\" for main Lua rules file…");

	if(!stat((std::string(LUADIR) + STDRULESLUA).c_str(), &ls)) {
		logDebug(" found \"" << (std::string(LUADIR) + STDRULESLUA) << "\"");
		addPaths.push_back(std::string(LUADIR) + STDRULESLUA);
	}

	logDebug("Searching \"" << SYSCONFDIR << "/" << PACKAGE
			 << "\" for additional Lua rules file…");

	if(!stat((std::string(SYSCONFDIR) + "/" + PACKAGE + STDRULESLUA).c_str(), &ls)) {
		logDebug(" found \"" << (std::string(SYSCONFDIR) + "/" + PACKAGE + STDRULESLUA) << "\"");
		addPaths.push_back(std::string(SYSCONFDIR) +  "/" + PACKAGE + STDRULESLUA);
#else

	if(!stat(NetMauMau::Common::getModulePath(NetMauMau::Common::USER, STDRULESLUA,
			 "lua").c_str(), &ls)) {
		return std::vector<std::string>
			   (1, NetMauMau::Common::getModulePath(NetMauMau::Common::PKGDATA, STDRULESLUA,
					   "lua"));
#endif
	}

#ifndef _WIN32

	free(luaDir);

	luaDir = getenv("HOME");

	logDebug("Searching \"" << luaDir << "/." << PACKAGE_NAME << "\" for user Lua rules file…");

	if(luaDir && !stat((std::string(luaDir) + "/." + PACKAGE_NAME + STDRULESLUA).c_str(), &ls)) {
		logDebug(" found \"" << (std::string(luaDir) + "/." + PACKAGE_NAME + STDRULESLUA) << "\"");
		addPaths.push_back(std::string(luaDir) + "/." + PACKAGE_NAME + STDRULESLUA);
	}

	return addPaths;

#else
	else {
		logInfo("To use your own custom rules you can place your rules to \""
				<< NetMauMau::Common::getModulePath(NetMauMau::Common::USER, STDRULESLUA, "lua")
				<< "\"");
	}

	return std::vector<std::string>(1, NetMauMau::Common::getModulePath(NetMauMau::Common::BINDIR,
									STDRULESLUA, "lua"));
#endif
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
