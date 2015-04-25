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

#ifndef NETMAUMAU_ENGINE_AIDT_DECISIONTREE_H
#define NETMAUMAU_ENGINE_AIDT_DECISIONTREE_H

#include "iaistate.h"
#include "icondition.h"

namespace NetMauMau {

namespace AIDT {

template<class RootCond>
class DecisionTree {
	DISALLOW_COPY_AND_ASSIGN(DecisionTree)
public:
	DecisionTree(IAIState &state, const IActionPtr &trueAct, const IActionPtr &falseAct) :
		m_rootCondition(IConditionPtr(new RootCond(trueAct, falseAct))), m_state(state) {}

	DecisionTree(IAIState &state) : m_rootCondition(IConditionPtr(new RootCond())),
		m_state(state) {}

	~DecisionTree() {}

	Common::ICardPtr getCard() const;

private:
	const IConditionPtr m_rootCondition;
	IAIState &m_state;
};

template<class RootCond>
Common::ICardPtr DecisionTree<RootCond>::getCard() const {

	IConditionPtr cond(m_rootCondition);
	IActionPtr act;

	while(cond && (act = (*cond)(m_state))) cond = (*act)(m_state);

	return m_state.getCard();
}

}

}

#endif /* NETMAUMAU_ENGINE_AIDT_DECISIONTREE_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
