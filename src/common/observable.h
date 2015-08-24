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

#ifndef NETMAUMAU_COMMON_OBSERVERABLE_H
#define NETMAUMAU_COMMON_OBSERVERABLE_H

#include <algorithm>
#include <vector>

#include "iobserver.h"

namespace NetMauMau {

namespace Common {

template<class SourceType, class WhatType>
class Observable {
	DISALLOW_COPY_AND_ASSIGN(Observable)
public:
	typedef WhatType what_type;

	virtual ~Observable();

	void addObserver(IObserver<SourceType> *o) {

		if(o) {
			m_observers.push_back(o);
			o->setSource(dynamic_cast<SourceType *>(this));
		}
	}

	void notify(const what_type what);

protected:
	Observable() : m_observers() {}

private:
	std::vector<IObserver<SourceType> *> m_observers;
};

template<class SourceType, class WhatType>
Observable<SourceType, WhatType>::~Observable() {}

template<class SourceType, class WhatType>
void Observable<SourceType, WhatType>::notify(const what_type what) {

#if GCC_VERSION < 40300

	for(typename std::vector<IObserver<SourceType> *>::const_iterator iter(m_observers.begin());
			iter != m_observers.end(); ++iter) {
		(*iter)->update(what);
	}

#else
	std::for_each(m_observers.begin(), m_observers.end(),
				  std::bind2nd(std::mem_fun(&IObserver<SourceType>::update), what));
#endif

}

}

}

#endif /* NETMAUMAU_COMMON_OBSERVERABLE_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
