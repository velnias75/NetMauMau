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

#ifndef NETMAUMAU_ABSTRACTSOCKETIMPL_H
#define NETMAUMAU_ABSTRACTSOCKETIMPL_H

#include <stdint.h>                     // for uint16_t

#include "socketexception.h"            // for SOCKET

struct addrinfo;

namespace NetMauMau {

namespace Common {

class AbstractSocketImpl {
	DISALLOW_COPY_AND_ASSIGN(AbstractSocketImpl)
public:
	explicit AbstractSocketImpl(const char *server, uint16_t port, bool sockopt_env = false);
	explicit AbstractSocketImpl(const char *server, uint16_t port, unsigned char sockopts);
	~AbstractSocketImpl();

	_EXPORT static int getAddrInfo(const char *server, uint16_t port, struct addrinfo **result,
								   bool ipv4 = false);
	
	static unsigned char setSocketOptions(SOCKET fd, unsigned char what) _NOUNUSED;
	static void logErrSocketOptions(unsigned char what);
	
	inline bool getSockoptEnv() const {
		return m_sockoptEnv;
	}
	
	inline unsigned char getSockOpts() const {
		return m_sopt;
	}

public:
	const std::string m_server;
	const uint16_t m_port;
	SOCKET m_sfd;
	std::string m_wireError;
	const bool m_sockoptEnv;
	const unsigned char m_sopt;
};

}

}

#endif /* NETMAUMAU_ABSTRACTSOCKETIMPL_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
