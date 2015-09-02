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

#include <string>
#include <exception>

#include <pthread.h>

#include "errorstring.h"

#define MUTEXLOCKER(mux) NetMauMau::Common::MutexLocker __mutex__locker__(mux); \
	_UNUSED(__mutex__locker__)

namespace NetMauMau {

namespace Common {

class _EXPORT MutexLockerException : public std::exception {
	MutexLockerException &operator=(const MutexLockerException &);
public:
	MutexLockerException(const std::string &msg) throw();
	virtual ~MutexLockerException() throw();

	virtual const char *what() const throw() _PURE;

private:
	const std::string m_msg;
};

template<typename Mutex, int (&Locker)(Mutex *), int (&Unlocker)(Mutex *)>
class MutexLockerBase {
	DISALLOW_COPY_AND_ASSIGN(MutexLockerBase)
public:
	typedef Mutex mutex_type;

	MutexLockerBase(Mutex *mux);
	~MutexLockerBase() throw();

	int unlock() throw();
	bool isLocked() const throw();

private:
	Mutex *m_mux;
	bool m_locked;
};

template<typename Mutex, int (&Locker)(Mutex *), int (&Unlocker)(Mutex *)>
MutexLockerBase<Mutex, Locker, Unlocker>::MutexLockerBase(Mutex *mux) : m_mux(mux), m_locked(true) {

	const int r = Locker(m_mux);

	if(r) throw MutexLockerException(errorString(r));
}

template<typename Mutex, int (&Locker)(Mutex *), int (&Unlocker)(Mutex *)>
MutexLockerBase<Mutex, Locker, Unlocker>::~MutexLockerBase() throw() {
	unlock();
}

template<typename Mutex, int (&Locker)(Mutex *), int (&Unlocker)(Mutex *)>
int MutexLockerBase<Mutex, Locker, Unlocker>::unlock() throw() {
	return m_locked ? (m_locked = (Unlocker(m_mux) == 0)) : 0;
}

template<typename Mutex, int (&Locker)(Mutex *), int (&Unlocker)(Mutex *)>
bool MutexLockerBase<Mutex, Locker, Unlocker>::isLocked() const throw() {
	return m_locked;
}

typedef MutexLockerBase<pthread_mutex_t,  pthread_mutex_lock,    pthread_mutex_unlock>  MutexLocker;
typedef MutexLockerBase<pthread_rwlock_t, pthread_rwlock_rdlock, pthread_rwlock_unlock> ReadLock;
typedef MutexLockerBase<pthread_rwlock_t, pthread_rwlock_wrlock, pthread_rwlock_unlock> WriteLock;

extern template
class _EXPORT MutexLockerBase<pthread_mutex_t,  pthread_mutex_lock,    pthread_mutex_unlock>;
extern template
class _EXPORT MutexLockerBase<pthread_rwlock_t, pthread_rwlock_rdlock, pthread_rwlock_unlock>;
extern template
class _EXPORT MutexLockerBase<pthread_rwlock_t, pthread_rwlock_wrlock, pthread_rwlock_unlock>;

}

}

#endif /* NETMAUMAU_COMMON_MUTEXLOCKER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
