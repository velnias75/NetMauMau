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

#include "decisiontree.h"

#include "cardtools.h"
#include "jackonlycondition.h"

using namespace NetMauMau::Engine::AIDT;

DecisionTree::DecisionTree(const AIState &state) :
	m_rootCondition(NetMauMau::Common::SmartPtr<ICondition>(new JackOnlyCondition())),
	m_state(state) {}

DecisionTree::~DecisionTree() {}

const NetMauMau::Common::ICardPtr &DecisionTree::getCard() const {

	(*m_rootCondition)(m_state);

	static NetMauMau::Common::ICardPtr IC(const_cast<const NetMauMau::Common::ICard *>
										  (NetMauMau::Common::getIllegalCard()));
	return IC;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
