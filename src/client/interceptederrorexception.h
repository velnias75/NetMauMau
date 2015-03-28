/*
 * Copyright 2014-2015 by Heiko Schäfer <heiko@rangun.de>
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

#ifndef NETMAUMAU_INTERCEPTEDERROREXCEPTION_H
#define NETMAUMAU_INTERCEPTEDERROREXCEPTION_H

#include "socketexception.h"

namespace NetMauMau {

namespace Client {

namespace Exception {

class _EXPORT InterceptedErrorException : public Common::Exception::SocketException {
	InterceptedErrorException &operator=(const InterceptedErrorException &);
public:
	InterceptedErrorException(const InterceptedErrorException &o) throw();
	explicit InterceptedErrorException(const std::string &msg,
									   SOCKET sockfd = INVALID_SOCKET) throw();
	virtual ~InterceptedErrorException() throw();
};

}

}

}

#endif /* NETMAUMAU_INTERCEPTEDERROREXCEPTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
