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

#ifndef NETMAUMAU_SERVER_HTTPD_H
#define NETMAUMAU_SERVER_HTTPD_H

#include "game.h"
#include "abstractconnection.h"

struct MHD_Daemon;

namespace NetMauMau {

namespace Server {

class Httpd;

typedef Common::SmartPtr<Httpd> HttpdPtr;

class Httpd : public Common::IObserver<Game>, public Common::IObserver<Engine> {
	DISALLOW_COPY_AND_ASSIGN(Httpd)
public:
	virtual ~Httpd();

	static Httpd *getInstance();

	virtual void setSource(const Common::IObserver<Engine>::source_type *s);
	virtual void setSource(const Common::IObserver<Game>::source_type *s);

	virtual void update(const Common::IObserver<Engine>::what_type &what);
	virtual void update(const Common::IObserver<Game>::what_type &what);

	inline std::string getWebServerURL() const {
		return m_url;
	}

	inline bool isWaiting() const {
		return m_waiting;
	}

	inline const NetMauMau::Common::IObserver<NetMauMau::Engine>::what_type &getPlayers() const {
		return m_players;
	}

	inline void setCapabilities(const Common::AbstractConnection::CAPABILITIES &caps) {
		m_caps = caps;
	}

	inline const Common::AbstractConnection::CAPABILITIES &getCapabilities() const {
		return m_caps;
	}

private:
	Httpd();

private:
	static HttpdPtr m_instance;

	MHD_Daemon *m_daemon;
	const Common::IObserver<Game>::source_type *m_gameSource;
	const Common::IObserver<Engine>::source_type *m_engineSource;
	NetMauMau::Common::IObserver<NetMauMau::Engine>::what_type m_players;
	Common::AbstractConnection::CAPABILITIES m_caps;
	bool m_gameRunning;
	bool m_waiting;
	std::string m_url;
};

}

}

#endif /* NETMAUMAU_SERVER_HTTPD_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
