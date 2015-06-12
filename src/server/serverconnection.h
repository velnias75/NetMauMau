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

#ifndef NETMAUMAU_SERVER_SERVERCONNECTION_H
#define NETMAUMAU_SERVER_SERVERCONNECTION_H

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"                     // for SERVER_PORT, etc
#endif

#include <cstddef>                      // for NULL
#include <functional>                   // for greater

#include "abstractconnection.h"         // for AbstractConnection, etc

struct timeval;

#define MIN_MAJOR 0
#define MIN_MINOR 2

#define WAIT_ERROR -2

namespace NetMauMau {

namespace Server {

class Connection : public Common::AbstractConnection {
	DISALLOW_COPY_AND_ASSIGN(Connection)
public:
	using Common::AbstractConnection::wait;
	using Common::AbstractConnection::getPlayerInfo;

	typedef enum { NONE, PLAY, CAP, REFUSED, PLAYERLIST, SCORES } ACCEPT_STATE;
	typedef std::map<uint32_t, std::string, std::greater<uint32_t> > VERSIONEDMESSAGE;

	explicit Connection(uint32_t minVer, bool inetd, uint16_t port = SERVER_PORT, const char *server = NULL);
	virtual ~Connection();

	virtual void connect(bool inetd) throw(Common::Exception::SocketException);

	int wait(timeval *tv = NULL);

	virtual void removePlayer(const INFO &info);
	virtual void removePlayer(SOCKET sockfd);

	inline const PLAYERINFOS &getPlayers() const {
		return getRegisteredPlayers();
	}

	NAMESOCKFD getPlayerInfo(const std::string &name) const;

	void sendVersionedMessage(const VERSIONEDMESSAGE &vm) const
	throw(Common::Exception::SocketException);

	Connection &operator<<(const std::string &msg) throw(Common::Exception::SocketException);
	Connection &operator>>(std::string &msg) throw(Common::Exception::SocketException);

	ACCEPT_STATE accept(INFO &v,
						bool gameRunning = false) throw(Common::Exception::SocketException);

	inline void setCapabilities(const CAPABILITIES &caps) {
		m_caps = caps;
	}

	inline static uint32_t getServerVersion() {
		return MAKE_VERSION(SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR);
	}

	inline uint32_t getMinClientVersion() const {
		return m_clientMinVer;
	}

	void clearPlayerPictures() const;

	virtual void reset() throw();

protected:
	virtual bool wire(SOCKET sockfd, const struct sockaddr *addr, socklen_t addrlen) const;
	virtual std::string wireError(const std::string &err) const;
	virtual void intercept() throw(Common::Exception::SocketException);

private:
	static bool isPNG(const std::string &pic);

private:
	CAPABILITIES m_caps;
	const uint32_t m_clientMinVer;
	const bool m_inetd;
	const std::string **const m_aiPlayerImages;
};

}

}

#endif /* NETMAUMAU_SERVER_SERVERCONNECTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
