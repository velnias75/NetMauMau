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

#ifndef NETMAUMAU_ENGINE_AI_BINARYCONDITION_H
#define NETMAUMAU_ENGINE_AI_BINARYCONDITION_H

#include "abstractcondition.h"

namespace NetMauMau {

namespace AI {

class BinaryCondition : public AbstractCondition {
	DISALLOW_COPY_AND_ASSIGN(BinaryCondition)
public:
	virtual ~BinaryCondition() throw() {}

protected:
	BinaryCondition(const IActionPtr &actTrue, const IActionPtr &actFalse) throw()
		: AbstractCondition(), m_trueAction(actTrue), m_falseAction(actFalse) {}

	inline IActionPtr getTrueAction() const throw() {
		return m_trueAction;
	}

	inline IActionPtr getFalseAction() const throw() {
		return m_falseAction;
	}

private:
	const IActionPtr m_trueAction;
	const IActionPtr m_falseAction;
};

}

}

#endif /* NETMAUMAU_ENGINE_AI_BINARYCONDITION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
