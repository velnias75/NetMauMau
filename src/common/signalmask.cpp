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

#include <algorithm>

#include "signalmask.h"

#include "logger.h"

using namespace NetMauMau::Common;

SignalMask::SignalMask(const SignalMask::SIGVECTOR &delsv) : m_sigVec(delsv), m_sigSet(),
	m_oldSet(), m_ok(false) {

#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE

	if(!sigemptyset(&m_sigSet) && !sigfillset(&m_sigSet)) {
		std::for_each(m_sigVec.begin(), m_sigVec.end(),
					  std::bind1st(std::ptr_fun(sigdelset), &m_sigSet));
		m_ok = sigprocmask(SIG_BLOCK, &m_sigSet, &m_oldSet) == 0;
	}

#endif
}

SignalMask::~SignalMask() {
#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE

	if(m_ok) sigprocmask(SIG_SETMASK, &m_oldSet, NULL);

#endif
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
