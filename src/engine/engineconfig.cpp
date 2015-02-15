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

#include "engineconfig.h"

#include "stdruleset.h"

using namespace NetMauMau;

EngineConfig::EngineConfig(Event::IEventHandler &eventHandler, long aiDelay,
						   RuleSet::IRuleSet *ruleset, bool nextMessage)
	: m_eventHandler(eventHandler), m_dirChange(false), m_aiDelay(aiDelay),
	  m_nextMessage(nextMessage), m_aceRoundRank(Common::ICard::RANK_ILLEGAL), m_ruleset(ruleset),
	  m_delRuleSet(false), m_aceRound(0) {}

EngineConfig::EngineConfig(Event::IEventHandler &eventHandler, bool dirChange, long aiDelay,
						   bool nextMessage, char aceRound) : m_eventHandler(eventHandler),
	m_dirChange(dirChange), m_aiDelay(aiDelay), m_nextMessage(nextMessage),
	m_aceRoundRank(aceRound == 'A' ? Common::ICard::ACE : (aceRound == 'Q' ? Common::ICard::QUEEN :
				   (aceRound == 'K' ? Common::ICard::KING : Common::ICard::RANK_ILLEGAL))),
	m_ruleset(0L), m_delRuleSet(true), m_aceRound(aceRound) {}

EngineConfig::EngineConfig(const EngineConfig &o) : m_eventHandler(o.m_eventHandler),
	m_dirChange(o.m_dirChange), m_aiDelay(o.m_aiDelay), m_nextMessage(o.m_nextMessage),
	m_aceRoundRank(o.m_aceRoundRank), m_ruleset(o.m_ruleset), m_delRuleSet(o.m_delRuleSet),
	m_aceRound(o.m_aceRound) {}

EngineConfig::~EngineConfig() {
	if(m_delRuleSet) delete m_ruleset;
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
	return m_ruleset ? m_ruleset : (m_ruleset = new RuleSet::StdRuleSet(m_dirChange, m_aceRound ?
			arl : 0L));
}

char EngineConfig::getAceRound() const {
	return m_aceRound;
}

Common::ICard::RANK EngineConfig::getAceRoundRank() const {
	return m_aceRoundRank;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
