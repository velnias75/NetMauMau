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

#ifndef NETMAUMAU_COMMON_SIGNALMASK_H
#define NETMAUMAU_COMMON_SIGNALMASK_H

#include <csignal>
#include <vector>

#include "linkercontrol.h"

namespace NetMauMau {

namespace Common {

class _EXPORT SignalMask {
	DISALLOW_COPY_AND_ASSIGN(SignalMask)
public:
	typedef std::vector<int> SIGVECTOR;

	explicit SignalMask(const SIGVECTOR &delsv = SIGVECTOR());
	~SignalMask();

private:
	const SIGVECTOR &m_sigVec;

#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE
	sigset_t m_sigSet;
	sigset_t m_oldSet;
#else
	int m_sigSet;
	int m_oldSet;
#endif

	bool m_ok;
};

}

}

#endif /* NETMAUMAU_COMMON_SIGNALMASK_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
