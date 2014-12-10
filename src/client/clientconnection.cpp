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

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cerrno>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include "clientconnection.h"

#include "base64.h"
#include "abstractclient.h"
#include "timeoutexception.h"
#include "shutdownexception.h"
#include "playerlistexception.h"
#include "capabilitiesexception.h"
#include "protocolerrorexception.h"
#include "versionmismatchexception.h"
#include "interceptederrorexception.h"
#include "nonetmaumauserverexception.h"
#include "connectionrejectedexception.h"

#define MAX_PNAME 1024

using namespace NetMauMau::Client;

Connection::Connection(const std::string &pName, const std::string &server, uint16_t port) :
	AbstractConnection(server.c_str(), port), m_pName(pName), m_timeout(0L) {
	if(m_pName.length() > MAX_PNAME - 1) m_pName = m_pName.substr(0, MAX_PNAME - 1);
}

Connection::~Connection() {
	shutdown(getSocketFD(), SHUT_RDWR);
	close(getSocketFD());
}

void Connection::setTimeout(struct timeval *timeout) {
	m_timeout = timeout;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic push
bool Connection::hello(uint16_t *maj, uint16_t *min)
throw(NetMauMau::Common::Exception::SocketException) {

	NetMauMau::Common::AbstractConnection::connect();

	fd_set rfds;
	int pret = -1;

	FD_ZERO(&rfds);
	FD_SET(getSocketFD(), &rfds);

	timeval tv = { m_timeout ? m_timeout->tv_sec : 0, m_timeout ? m_timeout->tv_usec : 0 };

	if(!m_timeout || (pret = ::select(getSocketFD() + 1, &rfds, NULL, NULL, &tv)) > 0) {

		const std::string rHello = read(getSocketFD());
		const std::string::size_type spc = rHello.find(' ');
		const std::string::size_type dot = rHello.find('.');

		const bool isServerHello = isHello(dot, spc);

		if(isServerHello && maj) *maj = getMajorFromHello(rHello, dot, spc);

		if(isServerHello && min) *min = getMinorFromHello(rHello, dot);

		return isValidHello(dot, spc, rHello, PACKAGE_NAME);

	} else if(pret == -1 && errno != EINTR) {
		throw NetMauMau::Common::Exception::SocketException(strerror(errno), getSocketFD(), errno);
	} else if(pret == -1 && errno == EINTR) {
		throw Exception::ShutdownException("Server is shutting down", getSocketFD());
	} else {
		throw Exception::TimeoutException("Timeout while connecting to server", getSocketFD());
	}
}
#pragma GCC diagnostic pop

Connection::PLAYERINFOS Connection::playerList(const IPlayerPicListener *hdl, bool playerPNG)
throw(NetMauMau::Common::Exception::SocketException) {

	PLAYERINFOS plv;

	if(hello()) {

		if(playerPNG) {
			std::ostringstream os;
			os << "PLAYERLIST" << ' ' << SERVER_VERSION_MAJOR << '.' << SERVER_VERSION_MINOR;

			send(os.str().c_str(), os.str().length(), getSocketFD());
		} else {
			send("PLAYERLIST", 10, getSocketFD());
		}

		std::string pl, pic;

		*this >> pl;

		if(playerPNG) {
			hdl->beginReceivePlayerPicture(pl);
			*this >> pic;
		}

		while(pl != "PLAYERLISTEND") {

			const std::vector<NetMauMau::Common::BYTE> &pp(NetMauMau::Common::base64_decode(pic));

			if(playerPNG) hdl->endReceivePlayerPicture(pl);

			NetMauMau::Common::BYTE *ppd = 0L;

			if(!pic.empty() && pic != "-") {

				ppd = new(std::nothrow) NetMauMau::Common::BYTE[pp.size()];

				if(ppd) {
					std::memcpy(ppd, pp.data(), pp.size() * sizeof(NetMauMau::Common::BYTE));
				} else {
					pic = "-";
				}
			}

			PLAYERINFO pInfo = { pl, ppd, (pic.empty() || pic != "-") ? pp.size() : 0 };

			plv.push_back(pInfo);

			*this >> pl;

			const bool ple = (pl == "PLAYERLISTEND");

			if(playerPNG) {
				if(!ple) hdl->beginReceivePlayerPicture(pl);

				*this >> pic;
			}
		}

	} else {
		throw Exception::PlayerlistException("Unable to get player list", getSocketFD());
	}

	return plv;
}

Connection::CAPABILITIES Connection::capabilities()
throw(NetMauMau::Common::Exception::SocketException) {

	Connection::CAPABILITIES caps;

	if(hello()) {

		send("CAP", 3, getSocketFD());

		std::string cap;
		*this >> cap;

		while(cap != "CAPEND") {

			const std::string::size_type p = cap.find('=');

			if(p != std::string::npos) {
				caps.insert(std::make_pair(cap.substr(0, p), cap.substr(p + 1)));
			}

			*this >> cap;
		}

	} else {
		throw Exception::CapabilitiesException("Unable to get capabilities", getSocketFD());
	}

	return caps;
}

void Connection::connect(const IPlayerPicListener *l, const unsigned char *data, std::size_t len)
throw(NetMauMau::Common::Exception::SocketException) {

	uint16_t maj = 0, min = 0;

	if(hello(&maj, &min)) {

		const uint32_t sver = (maj << 16u) | min;
		const uint32_t mver = AbstractClient::getClientProtocolVersion();

		if(mver >= sver) {

			std::ostringstream os;
			os << PACKAGE_NAME << ' ' << SERVER_VERSION_MAJOR << '.' << SERVER_VERSION_MINOR;

			send(os.str().c_str(), os.str().length(), getSocketFD());

			char name[4] = { 0 };
			recv(name, 4, getSocketFD());

			if(!strncmp(name, "NAME", 4)) {
				send(m_pName.c_str(), m_pName.length(), getSocketFD());
			} else if(!strncmp(name, "NAMP", 4)) {
				if(!(data && len)) {
					send(m_pName.c_str(), m_pName.length(), getSocketFD());
				} else {
					try {

						const std::string &base64png(NetMauMau::Common::base64_encode(data, len));

						if(!base64png.empty()) {

							std::ostringstream osp;
							osp << "+" << m_pName << '\0' << base64png.length() << '\0'
								<< base64png;

							send(osp.str().c_str(), osp.str().length(), getSocketFD());

							char ack[1024] = "0";
							recv(ack, 1023, getSocketFD());

							if(std::strtoul(ack, NULL, 10) == base64png.length()) {
								send("OK", 2, getSocketFD());
								l->uploadSucceded(m_pName);
							} else {
								send("NO", 2, getSocketFD());
								l->uploadFailed(m_pName);
							}

						} else {
							send(m_pName.c_str(), m_pName.length(), getSocketFD());
							l->uploadFailed(m_pName);
						}

					} catch(const NetMauMau::Common::Exception::SocketException &) {
						l->uploadFailed(m_pName);
					}
				}
			} else {
				throw Exception::ProtocolErrorException("Protocol error", getSocketFD());
			}

			char status[2];
			recv(status, 2, getSocketFD());

			if((status[0] == 'N' && status[1] == 'O')) {
				throw Exception::ConnectionRejectedException("Remote server rejected " \
						"the connection", getSocketFD());
			} else if((status[0] == 'V' && status[1] == 'M')) {
				throw Exception::VersionMismatchException(
					(static_cast<uint16_t>(maj) << 16u) | static_cast<uint16_t>(min),
					(static_cast<uint16_t>(SERVER_VERSION_MAJOR) << 16u) |
					static_cast<uint16_t>(SERVER_VERSION_MINOR), getSocketFD());
			}

		} else {
			throw Exception::VersionMismatchException(
				(static_cast<uint16_t>(maj) << 16u) | static_cast<uint16_t>(min),
				(static_cast<uint16_t>(SERVER_VERSION_MAJOR) << 16u) |
				static_cast<uint16_t>(SERVER_VERSION_MINOR), getSocketFD());
		}

	} else {
		throw Exception::NoNetMauMauServerException("Remote server seems not to be " \
				"a " PACKAGE_NAME " server", getSocketFD());
	}
}

bool Connection::wire(int sockfd, const struct sockaddr *addr, socklen_t addrlen) const {

	int ret = -1;

	if((ret = ::connect(sockfd, addr, addrlen)) == -1) {
		shutdown(sockfd, SHUT_RDWR);
		close(sockfd);
	}

	return ret != -1;
}

std::string Connection::wireError(const std::string &err) const {
	return std::string(!err.empty() ? err : "could not connect");
}

Connection &Connection::operator>>(std::string &msg)
throw(NetMauMau::Common::Exception::SocketException) {

	char c;
	std::string str;

	while(recv(&c, 1, getSocketFD()) > 0 && c != '\0') str.append(1, c);

	msg = str;

	if(msg.empty()) {
		throw Exception::InterceptedErrorException("Lost connection to server",
				getSocketFD());
	} else if(msg == "ERROR") {

		std::string errMsg;
		*this >> errMsg;

		throw Exception::InterceptedErrorException(errMsg, getSocketFD());
	}

	return *this;
}

Connection &Connection::operator<<(const std::string &msg)
throw(NetMauMau::Common::Exception::SocketException) {
	send(msg.c_str(), msg.length(), getSocketFD());
	return *this;
}

bool NetMauMau::Client::operator<(const std::string &x, const Connection::PLAYERINFO &y) {
	return x < y.name;
}

bool NetMauMau::Client::operator<(const Connection::PLAYERINFO &x, const std::string &y) {
	return x.name < y;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
