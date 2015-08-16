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

#ifndef NETMAUMAU_ENGINE_AI_IACTION_H
#define NETMAUMAU_ENGINE_AI_IACTION_H

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#if defined(TRACE_AI) && !defined(NDEBUG)
#include <string>
#endif

#include "linkercontrol.h"

namespace NetMauMau {

namespace Common {
template<class> class SmartPtr;
}

namespace AI {

class ICondition;
class IAIState;

class IAction {
	DISALLOW_COPY_AND_ASSIGN(IAction)
public:
	virtual ~IAction() throw() {}

	virtual const Common::SmartPtr<ICondition> &operator()(IAIState &state) const throw() = 0;

#if defined(TRACE_AI) && !defined(NDEBUG)
	virtual std::string traceLog() const throw() = 0;
#endif

protected:
	IAction() throw() {}
};

typedef Common::SmartPtr<IAction> IActionPtr;

}

}

#endif /* NETMAUMAU_ENGINE_AI_IACTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
