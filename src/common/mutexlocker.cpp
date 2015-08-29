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

#include "mutexlocker.h"

#ifndef NDEBUG
#include "errorstring.h"
#include "logger.h"
#endif

using namespace NetMauMau::Common;

MutexLocker::MutexLocker(pthread_mutex_t *mux) throw() : m_mux(mux), m_locked(false) {

	int r = 0;

	// cppcheck-suppress unreadVariable
	m_locked = (r = pthread_mutex_lock(m_mux)) == 0;

#ifndef NDEBUG

	if(!m_locked) logDebug(__PRETTY_FUNCTION__ << ": " << errorString(r));

#endif
}

MutexLocker::~MutexLocker() throw() {
	unlock();
}

int MutexLocker::unlock() const throw() {

	int r = m_locked ? pthread_mutex_unlock(m_mux) : 0;

#ifndef NDEBUG

	if(r) logDebug(__PRETTY_FUNCTION__ << ": " << errorString(r));

#endif

	return r;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
