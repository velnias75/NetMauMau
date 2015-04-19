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

#ifndef NETMAUMAU_COMMON_SMARTPTR_H
#define NETMAUMAU_COMMON_SMARTPTR_H

#include <iterator>

namespace NetMauMau {

namespace Common {

template<class T>
class SmartPtr {
	struct refCounter;
public:
	typedef T element_type;
	typedef element_type *element_pointer;

	explicit SmartPtr(T *p);

	explicit SmartPtr(const T *p = 0L);

	SmartPtr(const SmartPtr &o) throw() : m_refCounter(0L), m_crp(o.m_crp) {
		acquire(o.m_refCounter);
	}

	~SmartPtr() {
		release();
	}

	SmartPtr &operator=(const T *p) {
		release();
		m_crp = p;
		return *this;
	}

	SmartPtr &operator=(const SmartPtr &o);

	template<class O> friend class SmartPtr;

	template<class O> SmartPtr(const SmartPtr<O> &o) throw() : m_crp(o.m_crp) {
		acquire(o.m_refCounter);
	}

	template<class O> SmartPtr &operator=(const SmartPtr<O> &o) {

		if(this != &o) {
			release();
			acquire(o.m_refCounter);
			m_crp = o.m_crp;
		}

		return *this;
	}

	T &operator*() const throw() {
		return m_refCounter ? *m_refCounter->m_ptr : *m_crp;
	}

	const T *operator->() const throw() {
		return  m_refCounter ? m_refCounter->m_ptr : m_crp;
	}

	operator bool() const throw() {
		return this->operator const T * ();
	}

	operator T *() const throw() {
		return m_refCounter ? m_refCounter->m_ptr : 0L;
	}

	operator const T *() const throw() {
		return m_refCounter ? m_refCounter->m_ptr : (m_crp ? m_crp : 0L);
	}

	bool unique() const throw() {
		return (m_refCounter ? m_refCounter->m_count == 1ULL : true);
	}

private:
	void acquire(refCounter *c) throw() {

		m_refCounter = c;

		if(c) ++c->m_count;
	}

	void release();

private:
	struct refCounter {
		refCounter(T *p = 0L, unsigned long long int c = 1ULL) : m_ptr(p), m_count(c) {}
		T *const m_ptr;
		unsigned long long int m_count;
	} *m_refCounter;

	const T *m_crp;
};

template<class T>
SmartPtr<T>::SmartPtr(T *p) : m_refCounter(p ? new refCounter(p) : 0L), m_crp(0L) {}

template<class T>
SmartPtr<T>::SmartPtr(const T *p) : m_refCounter(0L), m_crp(p) {}

template<class T>
void SmartPtr<T>::release() {

	if(m_refCounter) {

		if(--m_refCounter->m_count == 0ULL) {
			delete m_refCounter->m_ptr;
			delete m_refCounter;
			m_refCounter = 0L;
		}
	}
}

template<class T>
SmartPtr<T> &SmartPtr<T>::operator=(const SmartPtr<T> &o) {

	if(this != &o) {
		release();
		acquire(o.m_refCounter);
		m_crp = o.m_crp;
	}

	return *this;
}

}

}

#endif /* NETMAUMAU_COMMON_SMARTPTR_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
