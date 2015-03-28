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

/**
 * @file
 * @author Heiko Schäfer <heiko@rangun.de>
 */

#ifndef NETMAUMAU_GAMERUNNINGEXCEPTION_H
#define NETMAUMAU_GAMERUNNINGEXCEPTION_H

#include "socketexception.h"

namespace NetMauMau {

namespace Client {

namespace Exception {

/**
 * @brief There is already a game running on the server
 * @since 0.15
 */
class _EXPORT GameRunningException : public Common::Exception::SocketException {
	GameRunningException &operator=(const GameRunningException &);
public:
	GameRunningException(const GameRunningException &o) throw();
	explicit GameRunningException(const std::string &msg, SOCKET sockfd = INVALID_SOCKET) throw();
	virtual ~GameRunningException() throw();
};

}

}

}

#endif /* NETMAUMAU_GAMERUNNINGEXCEPTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
