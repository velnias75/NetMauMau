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

#ifndef NETMAUMAU_CLIENTCONNECTIONIMPL_H
#define NETMAUMAU_CLIENTCONNECTIONIMPL_H

#include <deque>

#include "clientconnection.h"           // for Connection, etc

namespace NetMauMau {

namespace Client {

class ConnectionImpl {
	DISALLOW_COPY_AND_ASSIGN(ConnectionImpl)
public:
	explicit ConnectionImpl(Connection *piface, const std::string &pName, const timeval *timeout,
							uint32_t clientVersion);
	explicit ConnectionImpl(Connection *piface, const std::string &pName, const timeval *timeout,
							uint32_t clientVersion, Connection::BASE64RAII &base64);
	~ConnectionImpl();

	bool hello(uint16_t *maj = 0L, uint16_t *min = 0L) throw(Common::Exception::SocketException);

public:
	typedef std::deque<std::string::traits_type::char_type> BUFFER;
	Connection *const _piface;

	std::string m_pName;
	const timeval *m_timeout;
	uint32_t m_clientVersion;
	Connection::BASE64RAII &m_base64;
	BUFFER m_buf;
};

}

}

#endif /* NETMAUMAU_CLIENTCONNECTIONIMPL_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
