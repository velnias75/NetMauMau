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

#ifdef ENABLE_THREADS
#include "mutex.h"
#endif

#include "abstractconnection.h"         // for AbstractConnection, etc
#include "observable.h"

struct timeval;

#define MIN_MAJOR 0
#define MIN_MINOR 2

#define WAIT_ERROR -2

namespace NetMauMau {

namespace Server {

class Connection : public Common::AbstractConnection,
	public Common::Observable<Connection, std::pair<std::string, std::string> > {
	DISALLOW_COPY_AND_ASSIGN(Connection)
public:
	using Common::AbstractConnection::wait;
	using Common::AbstractConnection::getPlayerInfo;

#ifdef ENABLE_THREADS
	typedef struct _playerThreadData {

		inline _playerThreadData(const _playerThreadData &o) : get(o.get), gmx(o.gmx), eat(o.eat),
			emx(o.emx), nfd(o.nfd), tid(o.tid), msg(o.msg), stp(o.stp), con(o.con), exc(o.exc) {}

		_playerThreadData(const NAMESOCKFD &n, Connection &c);
		~_playerThreadData();

		inline _playerThreadData &operator=(const _playerThreadData &o) _PURE {

			if(this != &o) {
				_playerThreadData tmp(o);
				std::swap(tmp, *this);
			}

			return *this;
		}

		pthread_cond_t    get;
		NetMauMau::Common::Mutex gmx;
		pthread_cond_t    eat;
		NetMauMau::Common::Mutex emx;
		const NAMESOCKFD &nfd;
		pthread_t         tid;
		std::string       msg;
		volatile bool     stp;
		Connection       &con;

		Common::Exception::SocketException *exc;

	} PLAYERTHREADDATA;

	typedef std::vector<NetMauMau::Server::Connection::PLAYERTHREADDATA *> PTD;
#endif

	typedef enum { NONE, PLAY, CAP, REFUSED, PLAYERLIST, SCORES } ACCEPT_STATE;
	typedef std::map<uint32_t, std::string, std::greater<uint32_t> > VERSIONEDMESSAGE;

	explicit Connection(uint32_t minVer, bool inetd, uint16_t port = SERVER_PORT,
						const char *server = NULL);
	virtual ~Connection();

	virtual void connect(bool inetd) throw(Common::Exception::SocketException);

	int wait(timeval *tv = NULL);

	virtual void removePlayer(const INFO &info);
	virtual void removePlayer(SOCKET sockfd);

	inline const PLAYERINFOS &getPlayers() const {
		return getRegisteredPlayers();
	}

	NAMESOCKFD getPlayerInfo(const std::string &name) const;

#ifdef ENABLE_THREADS
	void createThreads();
#endif

	void sendVersionedMessage(const VERSIONEDMESSAGE &vm) const
	throw(Common::Exception::SocketException);

	Connection &operator<<(const std::string &msg) throw(Common::Exception::SocketException);

	ACCEPT_STATE
	accept(INFO &v, bool gameRunning = false) throw(Common::Exception::SocketException);

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

#ifdef ENABLE_THREADS
	friend class EventHandler;
	static void signalMessage(PTD &data, SOCKET fd, const std::string &msg);
	void waitPlayerThreads() const throw(Common::Exception::SocketException);
	void removeThread(SOCKET fd);

	inline PTD &getData() const {
		return m_data;
	}

#endif

private:
	static bool isPNG(const std::string &pic);

#ifdef ENABLE_THREADS
	void shutdownThreads() throw();
#endif

private:
	CAPABILITIES m_caps;
	const uint32_t m_clientMinVer;
	const bool m_inetd;
	const std::string **const m_aiPlayerImages;

#ifdef ENABLE_THREADS
	mutable PTD m_data;
	pthread_attr_t m_attr;
#endif
};

}

}

#endif /* NETMAUMAU_SERVER_SERVERCONNECTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
