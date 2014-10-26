/*
 * Copyright 2014 by Heiko Schäfer <heiko@rangun.de>
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

/**
 * @file
 * @author Heiko Schäfer <heiko@rangun.de>
 */

#ifndef NETMAUMAU_TIMEOUTEXCEPTION_H
#define NETMAUMAU_TIMEOUTEXCEPTION_H

#include "socketexception.h"

namespace NetMauMau {

namespace Client {

/**
 * @brief Exceptions thrown by clients
 */
namespace Exception {

/**
 * @brief A connection timed out
 */
class _EXPORT TimeoutException : public Common::Exception::SocketException {
	TimeoutException &operator=(const TimeoutException &);
public:
	TimeoutException(const TimeoutException &o) throw();
	TimeoutException(const std::string &msg, int sockfd = -1) throw();
	virtual ~TimeoutException() throw();
};

}

}

}

#endif /* NETMAUMAU_TIMEOUTEXCEPTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
