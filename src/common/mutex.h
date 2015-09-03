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

#ifndef NETMAUMAU_COMMON_MUTEXTRAITS_H
#define NETMAUMAU_COMMON_MUTEXTRAITS_H

#include <string>
#include <exception>

#include <pthread.h>

#include "errorstring.h"
#include "tmp.h"

namespace NetMauMau {

namespace Common {

class _EXPORT MutexException : public std::exception {
	MutexException &operator=(const MutexException &);
public:
	MutexException(const std::string &msg) throw();
	virtual ~MutexException() throw();
	virtual const char *what() const throw() _PURE;

private:
	const std::string m_msg;
};

template<typename T, typename A, int (*Init)(T *, const A *), int (*Destroy)(T *)>
struct MutexTraits;

template<>
struct MutexTraits < pthread_mutex_t, pthread_mutexattr_t, pthread_mutex_init,
		pthread_mutex_destroy > {
	typedef pthread_mutex_t mutex_type;
	typedef pthread_mutexattr_t attr_type;
};

template<>
struct MutexTraits < pthread_rwlock_t, pthread_rwlockattr_t, pthread_rwlock_init,
		pthread_rwlock_destroy > {
	typedef pthread_rwlock_t mutex_type;
	typedef pthread_rwlockattr_t attr_type;
};

template<typename T, typename A, int (*Init)(T *, const A *), int (*Destroy)(T *)>
class MutexBase : public MutexTraits<T, A, Init, Destroy> {
	MutexBase &operator=(const MutexBase &);
public:
	MutexBase();
	MutexBase(const MutexBase &o) throw();
	MutexBase(typename Commons::RParam<T>::Type o) throw();
	~MutexBase() throw();

	operator T *() const throw() _CONST;

private:
	T m_mutex;
};

template<typename T, typename A, int (*Init)(T *, const A *), int (*Destroy)(T *)>
MutexBase<T, A, Init, Destroy>::MutexBase() : m_mutex() {

	const int r = Init(&m_mutex, NULL);

	if(r) throw MutexException(NetMauMau::Common::errorString(r));
}

template<typename T, typename A, int (*Init)(T *, const A *), int (*Destroy)(T *)>
MutexBase<T, A, Init, Destroy>::MutexBase(typename Commons::RParam<T>::Type o) throw()
	: m_mutex(o) {}

template<typename T, typename A, int (*Init)(T *, const A *), int (*Destroy)(T *)>
MutexBase<T, A, Init, Destroy>::MutexBase(const MutexBase &o) throw() : m_mutex(o.m_mutex) {}

template<typename T, typename A, int (*Init)(T *, const A *), int (*Destroy)(T *)>
MutexBase<T, A, Init, Destroy>::~MutexBase() throw() {
	Destroy(&m_mutex);
}

template<typename T, typename A, int (*Init)(T *, const A *), int (*Destroy)(T *)>
MutexBase<T, A, Init, Destroy>::operator T *() const throw() {
	return const_cast<T *>(&m_mutex);
}

typedef MutexBase < pthread_mutex_t, pthread_mutexattr_t, pthread_mutex_init,
		pthread_mutex_destroy > Mutex;
typedef MutexBase < pthread_rwlock_t, pthread_rwlockattr_t, pthread_rwlock_init,
		pthread_rwlock_destroy > RWLock;

extern template class _EXPORT MutexBase < pthread_mutex_t, pthread_mutexattr_t,
	   pthread_mutex_init, pthread_mutex_destroy >;
extern template class _EXPORT MutexBase < pthread_rwlock_t, pthread_rwlockattr_t,
	   pthread_rwlock_init, pthread_rwlock_destroy >;
}

}

#endif /* NETMAUMAU_COMMON_MUTEXTRAITS_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
