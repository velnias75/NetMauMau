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

#ifndef NETMAUMAU_ENGINE_AI_ABSTRACTCONDITION_H
#define NETMAUMAU_ENGINE_AI_ABSTRACTCONDITION_H

#include "decisionbase.h"               // for DecisionBase
#include "icondition.h"                 // for ICondition, IConditionPtr

#include "iaction.h"                    // for IActionPtr

namespace NetMauMau {

namespace AI {

class AbstractCondition : public ICondition, protected DecisionBase {
	DISALLOW_COPY_AND_ASSIGN(AbstractCondition)
public:
	virtual ~AbstractCondition();

	virtual IActionPtr operator()(const IAIState &state) const;

protected:
	AbstractCondition();

	virtual IActionPtr perform(const IAIState &state,
							   const Player::IPlayer::CARDS &cards) const = 0;

	IActionPtr createNextAction(const IConditionPtr &cond) const;
	static const IActionPtr &getNullAction() _CONST;
};

}

}

#endif /* NETMAUMAU_ENGINE_AI_ABSTRACTCONDITION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
