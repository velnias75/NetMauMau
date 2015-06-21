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

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"                     // for PACKAGE_NAME, etc
#endif

#include <algorithm>
#include <cstring>                      // for strncmp, memcpy
#include <sstream>                      // for operator<<, ostringstream, etc

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include "base64.h"
#include "abstractclient.h"             // for AbstractClient
#include "capabilitiesexception.h"      // for CapabilitiesException
#include "clientconnectionimpl.h"       // for ConnectionImpl
#include "connectionrejectedexception.h"
#include "gamerunningexception.h"       // for GameRunningException
#include "interceptederrorexception.h"  // for InterceptedErrorException
#include "nonetmaumauserverexception.h" // for NoNetMauMauServerException
#include "playerlistexception.h"        // for PlayerlistException
#include "protocolerrorexception.h"     // for ProtocolErrorException
#include "scoresexception.h"            // for ScoresException
#include "versionmismatchexception.h"   // for VersionMismatchException
#include "protocol.h"                   // for ERROR

#define MAX_PNAME 1024

using namespace NetMauMau::Client;

Connection::Connection(const std::string &pName, const std::string &server, uint16_t port)
	: AbstractConnection(server.c_str(), port), _pimpl(new ConnectionImpl(this, pName, 0L, 2)) {
	init();
}

Connection::Connection(const std::string &pName, const std::string &server, uint16_t port,
					   BASE64RAII &) : AbstractConnection(server.c_str(), port),
	_pimpl(new ConnectionImpl(this, pName, 0L, 2)) {
	init();
}

Connection::~Connection() {
	shutdown(getSocketFD());
	delete _pimpl;
}

void Connection::init() {
	if(_pimpl->m_pName.length() > MAX_PNAME - 1) {
		_pimpl->m_pName.substr(0, MAX_PNAME - 1).swap(_pimpl->m_pName);
	}
}

void Connection::setClientVersion(uint32_t clientVersion) {
	_pimpl->m_clientVersion = clientVersion;
}

void Connection::setTimeout(struct timeval *timeout) {
	_pimpl->m_timeout = timeout;
}

Connection::PLAYERINFOS Connection::playerList(const IPlayerPicListener *hdl, bool playerPNG)
throw(NetMauMau::Common::Exception::SocketException) {

	PLAYERINFOS plv;

	if(_pimpl->hello()) {

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

			const std::vector<unsigned char> &pp(NetMauMau::Common::base64_decode(pic));

			if(playerPNG) hdl->endReceivePlayerPicture(pl);

			unsigned char *ppd = 0L;

			if(!pic.empty() && pic != "-") {

				ppd = new(std::nothrow) unsigned char[pp.size()];

				if(ppd) {
					std::memcpy(ppd, pp.data(), pp.size() * sizeof(unsigned char));
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

	if(_pimpl->hello()) {

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

Connection::SCORES Connection::getScores(SCORE_TYPE::_scoreType type, std::size_t limit)
throw(NetMauMau::Common::Exception::SocketException) {

	try {

		const CAPABILITIES &c(capabilities());
		const CAPABILITIES::const_iterator &f(c.find("HAVE_SCORES"));

		if(f != c.end() && f->second == "true") {

			SCORES scores;

			if(_pimpl->hello()) {

				std::ostringstream os;
				os << "SCORES ";

				switch(type) {
				case SCORE_TYPE::NORM:
					os << "NORM ";
					break;

				default:
					os << "ABS ";
					break;
				}

				os << limit;

				send(os.str().c_str(), os.str().length(), getSocketFD());

				std::string score;
				*this >> score;

				while(score != "SCORESEND") {

					const std::string::size_type p = score.find('=');

					if(p != std::string::npos) {

						SCORE sc = {
							score.substr(0, p),
							std::strtoll(score.substr(p + 1).c_str(), NULL, 10)
						};

						scores.push_back(sc);
					}

					*this >> score;
				}

			} else {
				throw Exception::ScoresException("Unable to get scores", getSocketFD());
			}

			return scores;
		}

	} catch(const Exception::ScoresException &) {
		throw;
	} catch(const NetMauMau::Common::Exception::SocketException &e) {
		throw Exception::ScoresException(e.what(), getSocketFD());
	}

	return SCORES();
}

void Connection::connect(const IPlayerPicListener *l, const unsigned char *data, std::size_t len)
throw(NetMauMau::Common::Exception::SocketException) {

	uint16_t maj = 0, min = 0;

	if(_pimpl->hello(&maj, &min)) {

		const uint32_t sver = MAKE_VERSION(maj, min);
		const uint32_t mver = NetMauMau::Client::AbstractClient::getClientProtocolVersion();

		if(mver >= sver) {

			std::ostringstream os;

			os << PACKAGE_NAME << ' '
			   << static_cast<uint16_t>(_pimpl->m_clientVersion << 16u) << '.'
			   << static_cast<uint16_t>(_pimpl->m_clientVersion);

			send(os.str().c_str(), os.str().length(), getSocketFD());

			char name[4] = { 0 };
			recv(name, 4, getSocketFD());

			if(!std::strncmp(name, "NAME", 4)) {
				send(_pimpl->m_pName.c_str(), _pimpl->m_pName.length(), getSocketFD());
			} else if(!std::strncmp(name, "NAMP", 4)) {

				const std::string &base64png(NetMauMau::Common::base64_encode(data, len));

				if(base64png.empty()) {

					send(_pimpl->m_pName.c_str(), _pimpl->m_pName.length(), getSocketFD());

					if(data || len) l->uploadFailed(_pimpl->m_pName);

				} else {

					try {

						std::ostringstream osp;

						osp << "+" << _pimpl->m_pName << '\0' << base64png.length() << '\0'
							<< base64png;

						send(osp.str().c_str(), osp.str().length(), getSocketFD());

						char ack[1024];
						std::memset(ack, '0', 1023);
						ack[1023] = 0;

						recv(ack, 1023, getSocketFD());

						if(std::strtoul(ack, NULL, 10) == base64png.length()) {
							send("OK", 2, getSocketFD());
							l->uploadSucceded(_pimpl->m_pName);
						} else {
							send("NO", 2, getSocketFD());
							l->uploadFailed(_pimpl->m_pName);
						}

					} catch(const NetMauMau::Common::Exception::SocketException &) {
						l->uploadFailed(_pimpl->m_pName);
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
					MAKE_VERSION(SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR),
					MAKE_VERSION(maj, min), getSocketFD());
			} else if((status[0] == 'I' && status[1] == 'N')) {
				throw Exception::PlayerlistException(_pimpl->m_pName, getSocketFD());
			} else if((status[0] == 'G' && status[1] == 'R')) {
				throw Exception::GameRunningException("There is already a game running " \
													  "on this server", getSocketFD());
			} else if(!(status[0] == 'O' && status[1] == 'K')) {
				throw NetMauMau::Common::Exception::SocketException("Connection rejected due " \
						"to unknown reason", getSocketFD());
			}

		} else {
			throw Exception::VersionMismatchException(
				MAKE_VERSION(maj, min), _pimpl->m_clientVersion, getSocketFD());
		}

	} else {
		throw Exception::NoNetMauMauServerException("Remote server seems not to be " \
				"a " PACKAGE_NAME " server", getSocketFD());
	}
}

bool Connection::wire(SOCKET sockfd, const struct sockaddr *addr, socklen_t addrlen) const {

	int ret = -1;

	if((ret = ::connect(sockfd, addr, addrlen)) == -1) shutdown(sockfd);

	return ret != -1;
}

std::string Connection::wireError(const std::string &err) const {
	return std::string(!err.empty() ? err : "could not connect");
}

Connection &Connection::operator>>(std::string &msg)
throw(NetMauMau::Common::Exception::SocketException) {

	std::string str;
	char  buf[1024] = { 0 };

	ConnectionImpl::BUFFER::iterator f;

	while(_pimpl->m_buf.empty() || (!_pimpl->m_buf.empty() && (f = std::find(_pimpl->m_buf.begin(),
									_pimpl->m_buf.end(), '\0')) == _pimpl->m_buf.end())) {
		std::size_t l;

		if((l = recv(buf, 1024, getSocketFD())) > 0) {
			_pimpl->m_buf.insert(_pimpl->m_buf.end(), buf, buf + l);
		}
	}

	str.insert(str.begin(), _pimpl->m_buf.begin(), f);

	_pimpl->m_buf.erase(_pimpl->m_buf.begin(), f + 1);

	str.swap(msg);

	if(msg.empty()) {
		throw Exception::InterceptedErrorException("Lost connection to server", getSocketFD());
	} else if(msg == NetMauMau::Common::Protocol::V15::ERROR) {

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

Connection::_base64RAII::_base64RAII() : m_base64(0L) {}

Connection::_base64RAII::_base64RAII(const IBase64 *) : m_base64(0L) {}

Connection::_base64RAII::~_base64RAII() {}

Connection::_base64RAII::operator const IBase64 *() {
	return 0L;
}

Connection::_base64RAII &Connection::_base64RAII::operator=(const IBase64 *) {
	return *this;
}

bool NetMauMau::Client::operator<(const std::string &x, const Connection::PLAYERINFO &y) {
	return x < y.name;
}

bool NetMauMau::Client::operator<(const Connection::PLAYERINFO &x, const std::string &y) {
	return x.name < y;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
