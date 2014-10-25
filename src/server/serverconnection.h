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

#ifndef NETMAUMAU_SERVERCONNECTION_H
#define NETMAUMAU_SERVERCONNECTION_H

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#include "abstractconnection.h"

#define MIN_MAJOR 0
#define MIN_MINOR 2

namespace NetMauMau {

namespace Server {

class Connection : public Common::AbstractConnection {
	DISALLOW_COPY_AND_ASSIGN(Connection)
public:
	typedef enum { NONE, PLAY, CAP, REFUSED } ACCEPT_STATE;

	Connection(uint16_t port = SERVER_PORT, const char *server = NULL);
	virtual ~Connection();

	virtual void connect() throw(Common::Exception::SocketException);

	int wait(timeval *tv = NULL) const;

	const PLAYERINFOS &getPlayers() const _CONST;

	Connection &operator<<(const std::string &msg) throw(Common::Exception::SocketException);
	Connection &operator>>(std::string &msg) throw(Common::Exception::SocketException);

	ACCEPT_STATE accept(INFO &v, bool refuse = false) throw(Common::Exception::SocketException);
	void setCapabilities(const CAPABILITIES &caps);

protected:
	virtual bool wire(int sockfd, const struct sockaddr *addr, socklen_t addrlen) const;
	virtual std::string wireError(const std::string &err) const;
	virtual void intercept();

private:
	CAPABILITIES m_caps;
};

}

}

#endif /* NETMAUMAU_SERVERCONNECTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
