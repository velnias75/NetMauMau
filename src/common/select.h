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

#ifndef NETMAUMAU_COMMON_SELECT_H
#define NETMAUMAU_COMMON_SELECT_H

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#ifndef _WIN32
#include <sys/select.h>
#endif

#include "smartsingleton.h"

#include "socketexception.h"

namespace NetMauMau {

namespace Common {

class _EXPORT Select : public SmartSingleton<Select> {
	DISALLOW_COPY_AND_ASSIGN(Select)
	friend class SmartSingleton<Select>;
public:
	virtual ~Select();

	// cppcheck-suppress functionStatic
	int perform(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
				struct timeval *timeout, bool blockall = false) const throw();

private:
	Select() throw(Exception::SocketException);

private:
#if _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
	mutable sigset_t m_sigSet;
#else
	int m_sigSet;
#endif
};

}

}

extern template class NetMauMau::Common::SmartSingleton<NetMauMau::Common::Select>;

#endif /* NETMAUMAU_COMMON_SELECT_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
