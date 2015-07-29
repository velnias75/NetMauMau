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

#ifndef NETMAUMAU_COMMON_SMARTSINGLETON_H
#define NETMAUMAU_COMMON_SMARTSINGLETON_H

#include "linkercontrol.h"
#include "smartptr.h"

namespace NetMauMau {

namespace Common {

template<class T>
class SmartSingleton {
	DISALLOW_COPY_AND_ASSIGN(SmartSingleton)
public:
	typedef Common::SmartPtr<T> smartptr_type;

	virtual ~SmartSingleton() {}

	static const smartptr_type &getInstance();

	static inline typename smartptr_type::element_pointer getInstancePtr() {
		return getInstance();
	}

protected:
	SmartSingleton() {}

private:
	static smartptr_type m_instance;
};

template<class T>
typename SmartSingleton<T>::smartptr_type SmartSingleton<T>::m_instance;

template<class T>
const typename SmartSingleton<T>::smartptr_type &SmartSingleton<T>::getInstance() {

	if(!m_instance) m_instance = smartptr_type(new typename smartptr_type::element_type());

	return m_instance;
}

}

}

#endif /* NETMAUMAU_COMMON_SMARTSINGLETON_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
