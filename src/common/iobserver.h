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

#ifndef NETMAUMAU_COMMON_IOBSERVER_H
#define NETMAUMAU_COMMON_IOBSERVER_H

#include "linkercontrol.h"

namespace NetMauMau {

namespace Common {

template<class SourceType>
class IObserver {
	DISALLOW_COPY_AND_ASSIGN(IObserver)
public:
	typedef SourceType source_type;
	typedef typename SourceType::what_type what_type;

	virtual ~IObserver() {}

	virtual void setSource(const source_type *s) = 0;
	virtual void update(const what_type &what) = 0;

protected:
	IObserver() {}
};

}

}

#endif /* NETMAUMAU_COMMON_IOBSERVER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
