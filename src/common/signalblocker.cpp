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

#include <cerrno>
#include <algorithm>

#include "signalblocker.h"

#include "errorstring.h"
#include "logger.h"

namespace {
const char *FAIL = "Failed to temporary block signals: ";
}

using namespace NetMauMau::Common;

SignalBlocker::SignalBlocker() : m_sigSet(), m_oldSet(), m_ok(init(0u, 0L)) {}

SignalBlocker::SignalBlocker(std::size_t numsv, int *ignsv) : m_sigSet(), m_oldSet(),
	m_ok(init(numsv, ignsv)) {}

SignalBlocker::~SignalBlocker() {
#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE

	if(m_ok) sigprocmask(SIG_SETMASK, &m_oldSet, NULL);

#endif
}

bool SignalBlocker::init(std::size_t numsv, int *ignsv) {
#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE

	if(!sigemptyset(&m_sigSet) && !sigfillset(&m_sigSet)) {

		if(ignsv && numsv) std::for_each(ignsv, ignsv + numsv,
											 std::bind1st(std::ptr_fun(sigdelset), &m_sigSet));

		const bool r = sigprocmask(SIG_BLOCK, &m_sigSet, &m_oldSet) == 0;

		if(!r) logWarning(FAIL << NetMauMau::Common::errorString(errno));

		return r;

	} else {
		logWarning(FAIL << NetMauMau::Common::errorString(errno));
	}

#elif !defined(_WIN32)
#warning "Signal blocking isn't supported on this platform"
#endif

	return false;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
