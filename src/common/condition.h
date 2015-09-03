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

#ifndef NETMAUMAU_COMMON_CONDITION_H
#define NETMAUMAU_COMMON_CONDITION_H

#include <pthread.h>

#include "mutex.h"

namespace NetMauMau {

namespace Common {

class _EXPORT Condition {
	Condition &operator=(const Condition &);

	template<typename M, typename P>
	void wait_internal(typename Commons::RParam<M>::Type m,
					   typename Commons::RParam<P>::Type p) {

		while(!p()) {

			const int r = pthread_cond_wait(&m_cond, m);

			if(r) throw MutexException(errorString(r));
		}
	}

public:
	Condition();
	Condition(const Condition &o) throw();
	~Condition() throw();

	template<typename M, typename P>
	inline void wait(const M &m, const P &p) {
		wait_internal<M, P>(m, p);
	}

	int signal() throw();

private:
	pthread_cond_t m_cond;
};

}

}

#endif /* NETMAUMAU_COMMON_CONDITION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
