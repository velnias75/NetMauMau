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

#ifndef NETMAUMAU_CLIENT_EXCEPTION_REMOTEPLAYEREXCEPTION_H
#define NETMAUMAU_CLIENT_EXCEPTION_REMOTEPLAYEREXCEPTION_H

#include "socketexception.h"            // for INVALID_SOCKET, SOCKET, etc

namespace NetMauMau {

namespace Client {

namespace Exception {

/**
 * @ingroup exceptions
 * @brief Error in communication wit a remote player
 * @since 0.23.6
 */
class RemotePlayerException : public Common::Exception::SocketException {
	RemotePlayerException &operator=(const RemotePlayerException &);
public:
	RemotePlayerException(const RemotePlayerException &o) throw();
	RemotePlayerException(const std::string &player, const std::string &msg) throw();
	virtual ~RemotePlayerException() throw();

	std::string player() const throw();

private:
	std::string m_player;
};

}

}

}

#endif /* NETMAUMAU_CLIENT_EXCEPTION_REMOTEPLAYEREXCEPTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
