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

#include "mutex.h"

#define MUTEXLOCKER(mux) NetMauMau::Common::MutexLocker __mutex__locker__(mux); \
	_UNUSED(__mutex__locker__)

namespace NetMauMau {

namespace Common {

template < typename M, int (&Locker)(typename M::mutex_type *),
		 int (&Unlocker)(typename M::mutex_type *) >
class MutexLockerBase {
	DISALLOW_COPY_AND_ASSIGN(MutexLockerBase)
public:
	typedef M mutex_type;

	explicit MutexLockerBase(typename Commons::RParam<mutex_type>::Type mux);
	~MutexLockerBase() throw();

	int unlock() throw();
	bool isLocked() const throw() _PURE;

private:
	typename mutex_type::mutex_type *const m_mux;
	bool m_locked;
};

template < typename M, int (&Locker)(typename M::mutex_type *),
		 int (&Unlocker)(typename M::mutex_type *) >
MutexLockerBase<M, Locker, Unlocker>::MutexLockerBase(typename
		Commons::RParam<mutex_type>::Type mux) : m_mux(mux), m_locked(true) {

	const int r = Locker(m_mux);

	if(r) throw MutexException(errorString(r));
}

template < typename M, int (&Locker)(typename M::mutex_type *),
		 int (&Unlocker)(typename M::mutex_type *) >
MutexLockerBase<M, Locker, Unlocker>::~MutexLockerBase() throw() {
	unlock();
}

template < typename M, int (&Locker)(typename M::mutex_type *),
		 int (&Unlocker)(typename M::mutex_type *) >
int MutexLockerBase<M, Locker, Unlocker>::unlock() throw() {
	return m_locked ? (m_locked = (Unlocker(m_mux) == 0)) : 0;
}

template < typename M, int (&Locker)(typename M::mutex_type *),
		 int (&Unlocker)(typename M::mutex_type *) >
bool MutexLockerBase<M, Locker, Unlocker>::isLocked() const throw() {
	return m_locked;
}

typedef MutexLockerBase<Mutex,  pthread_mutex_lock,    pthread_mutex_unlock>  MutexLocker;
typedef MutexLockerBase<RWLock, pthread_rwlock_rdlock, pthread_rwlock_unlock> ReadLock;
typedef MutexLockerBase<RWLock, pthread_rwlock_wrlock, pthread_rwlock_unlock> WriteLock;

extern template class _EXPORT MutexLockerBase<Mutex,  pthread_mutex_lock,    pthread_mutex_unlock>;
extern template class _EXPORT MutexLockerBase<RWLock, pthread_rwlock_rdlock, pthread_rwlock_unlock>;
extern template class _EXPORT MutexLockerBase<RWLock, pthread_rwlock_wrlock, pthread_rwlock_unlock>;

}

}

#endif /* NETMAUMAU_COMMON_MUTEXLOCKER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
