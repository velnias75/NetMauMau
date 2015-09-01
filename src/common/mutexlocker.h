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

#ifndef NETMAUMAU_COMMON_MUTEXLOCKER_H
#define NETMAUMAU_COMMON_MUTEXLOCKER_H

#include <pthread.h>

#include "linkercontrol.h"

#define MUTEXLOCKER(mux) NetMauMau::Common::MutexLocker __mutex__locker__(mux); \
	_UNUSED(__mutex__locker__)

namespace NetMauMau {

namespace Common {

class _EXPORT MutexLocker {
	DISALLOW_COPY_AND_ASSIGN(MutexLocker)
public:
	explicit MutexLocker(pthread_mutex_t *mux) throw();
	~MutexLocker() throw();

	int unlock() const throw();

private:
	pthread_mutex_t *const m_mux;
	bool m_locked;
};

class _EXPORT ReadLock {
	DISALLOW_COPY_AND_ASSIGN(ReadLock)
public:
	explicit ReadLock(pthread_rwlock_t *mux) throw();
	~ReadLock() throw();

	int unlock() const throw();

private:
	pthread_rwlock_t *const m_mux;
	bool m_locked;
};

class _EXPORT WriteLock {
	DISALLOW_COPY_AND_ASSIGN(WriteLock)
public:
	explicit WriteLock(pthread_rwlock_t *mux) throw();
	~WriteLock() throw();

	int unlock() const throw();

private:
	pthread_rwlock_t *const m_mux;
	bool m_locked;
};

}

}

#endif /* NETMAUMAU_COMMON_MUTEXLOCKER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
