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

#ifndef NETMAUMAU_ENGINE_AI_DECISIONTREE_H
#define NETMAUMAU_ENGINE_AI_DECISIONTREE_H

#include <cstdlib>

#include "iaction.h"
#include "iaistate.h"
#include "icondition.h"

#if defined(TRACE_AI) && !defined(NDEBUG)
#include <cstdio>
#include "cardtools.h"
#include "logger.h"
#endif

namespace NetMauMau {

namespace AI {

template<class RootCond, bool Jack = false>
class DecisionChain {
	DISALLOW_COPY_AND_ASSIGN(DecisionChain)
public:
	DecisionChain(IAIState &state, const IActionPtr &trueAct, const IActionPtr &falseAct) :
		m_rootCondition(IConditionPtr(new RootCond(trueAct, falseAct))), m_state(state) {}

	DecisionChain(IAIState &state) : m_rootCondition(IConditionPtr(new RootCond())),
		m_state(state) {}

	~DecisionChain() {}

	Common::ICardPtr getCard(bool noJack = false) const;

private:
	const IConditionPtr m_rootCondition;
	IAIState &m_state;
};

template<class RootCond, bool Jack>
Common::ICardPtr DecisionChain<RootCond, Jack>::getCard(bool noJack) const {

	IConditionPtr cond(m_rootCondition);
	IActionPtr act;

	const bool oj = m_state.isNoJack();

	m_state.setNoJack(noJack);

#if defined(TRACE_AI) && !defined(NDEBUG)

	if(!getenv("NMM_NO_TRACE")) logDebug("-> BEGIN " << (Jack ? "-jack- " : "") << "trace of AI \""
											 << m_state.getName() << "\"");

#endif

	while(cond && (act = (*cond)(m_state))) {

		cond = (*act)(m_state);

// 		if(m_state.getCard()) break;
	}

	m_state.setNoJack(oj);

#if defined(TRACE_AI) && !defined(NDEBUG)

	// cppcheck-suppress unreadVariable
	const bool ansi = isatty(fileno(stderr));
	const Common::ICardPtr c(m_state.getCard());

	if(!getenv("NMM_NO_TRACE")) logDebug("   END " << (Jack ? "-jack- " : "") << "trace of AI \""
											 << m_state.getName() << "\" -> " << (c ? (Jack ?
													 Common::suitToSymbol(c->getSuit(), ansi, ansi)
													 : c->description(ansi)) : "NO CARD") << " <-");

#endif

	return m_state.getCard();
}

}

}

#endif /* NETMAUMAU_ENGINE_AI_DECISIONTREE_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
