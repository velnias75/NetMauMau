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

#include <algorithm>
#include <sstream>
#include <cerrno>
#include <cstring>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include "serverconnection.h"
#include "logger.h"

namespace {

const std::string AIDefaultPicture(
	"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");

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct _isPlayer : public std::binary_function < NetMauMau::Common::AbstractConnection::NAMESOCKFD,
		std::string, bool > {
	bool operator()(const NetMauMau::Common::AbstractConnection::NAMESOCKFD &nsd,
					const std::string &player) const {
		return nsd.name == player;
	}
};
#pragma GCC diagnostic pop

}

using namespace NetMauMau::Server;

Connection::Connection(uint16_t port, const char *server) : AbstractConnection(server, port),
	m_caps() {}

Connection::~Connection() {

	for(std::vector<NAMESOCKFD>::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) {

		try {
			send("BYE", 3, i->sockfd);
		} catch(const NetMauMau::Common::Exception::SocketException &) {}

		close(i->sockfd);
	}
}

bool Connection::wire(int sockfd, const struct sockaddr *addr, socklen_t addrlen) const {

	const int yes = 1;

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&yes),
				  sizeof(int)) == -1) {
		return false;
	}

	return bind(sockfd, addr, addrlen) == 0;
}

std::string Connection::wireError(const std::string &err) const {
	return std::string("could not bind") + (!err.empty() ? ": " : "") + (!err.empty() ? err : "");
}

void Connection::connect() throw(NetMauMau::Common::Exception::SocketException) {

	AbstractConnection::connect();

	if(listen(getSocketFD(), SOMAXCONN)) {
		throw NetMauMau::Common::Exception::SocketException(std::strerror(errno), getSocketFD(),
				errno);
	}
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic push
int Connection::wait(timeval *tv) const {

	fd_set rfds;

	FD_ZERO(&rfds);
	FD_SET(getSocketFD(), &rfds);

	return ::select(getSocketFD() + 1, &rfds, NULL, NULL, tv);
}
#pragma GCC diagnostic pop

Connection::ACCEPT_STATE Connection::accept(INFO &info,
		bool refuse) throw(NetMauMau::Common::Exception::SocketException) {

	ACCEPT_STATE accepted = REFUSED;

	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len = sizeof(struct sockaddr_storage);

	const int cfd = ::accept(getSocketFD(), reinterpret_cast<struct sockaddr *>(&peer_addr),
							 &peer_addr_len);

	if(cfd != -1) {

		info.sockfd = cfd;

		char host[NI_MAXHOST], service[NI_MAXSERV];

		const int err = getnameinfo(reinterpret_cast<struct sockaddr *>(&peer_addr), peer_addr_len,
									host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);

		info.port = (uint16_t)std::strtoul(service, NULL, 10);
		info.host = host;

		if(!err) {

			const std::string hello(PACKAGE_NAME);

			std::ostringstream os;
			os << hello << ' ' << MIN_MAJOR << '.' << MIN_MINOR;

			send(os.str().c_str(), os.str().length(), cfd);

			const std::string rHello = read(cfd);

			if(rHello != "CAP" && rHello.substr(0, 10) != "PLAYERLIST") {

				const std::string::size_type spc = rHello.find(' ');
				const std::string::size_type dot = rHello.find('.');

				if(isValidHello(dot, spc, rHello, hello)) {

					info.maj = getMajorFromHello(rHello, dot, spc);
					info.min = getMinorFromHello(rHello, dot);

					const uint32_t cver = (info.maj << 16u) | info.min;
					const uint32_t minver = (static_cast<uint16_t>(MIN_MAJOR) << 16u) |
											static_cast<uint16_t>(MIN_MINOR);
					const uint32_t maxver = getServerVersion();

					send(cver >= 4 ? "NAMP" : "NAME", 4, cfd);
					info.name = read(cfd);

					const std::string &sanName(info.name[0] == '+' ? info.name.substr(1) :
											   info.name);

					if(!sanName.empty()) {
						info.name = sanName;
					} else {
						refuse = true;
					}

					if(cver >= minver && cver <= maxver && !refuse) {

						std::string playerPic;

						if(cver >= 4 && info.name[0] == '+') {
							info.name = info.name.substr(1);
							playerPic = read(cfd);
						}

						const NAMESOCKFD nsf = { info.name, playerPic, cfd, cver };

						registerPlayer(nsf);
						send("OK", 2, cfd);
						accepted = PLAY;

					} else {

						try {
							send(cver <= maxver ? "NO" : "VM", 2, cfd);
						} catch(const NetMauMau::Common::Exception::SocketException &e) {
							logDebug("Sending " << (cver <= maxver ? "NO" : "VM")
									 << " to client failed: " << e.what());
						}

						shutdown(cfd, SHUT_RDWR);
						close(cfd);
						accepted = REFUSED;
					}
				} else {
					logDebug("HELLO failed: " << rHello.substr(0, std::strlen(PACKAGE_NAME))
							 << " != " << hello);

					try {
						send("NO", 2, cfd);
					} catch(const NetMauMau::Common::Exception::SocketException &e) {
						logDebug("Sending NO to client failed: " << e.what());
					}

					shutdown(cfd, SHUT_RDWR);
					close(cfd);
				}

			} else if(rHello.substr(0, 10) == "PLAYERLIST") {

				const std::string::size_type spc = rHello.find(' ');
				const std::string::size_type dot = rHello.find('.');

				const PLAYERINFOS &pi(getRegisteredPlayers());
				const uint32_t cver = rHello.length() > 10 ?
									  (getMajorFromHello(rHello, dot, spc) << 16u) |
									  getMinorFromHello(rHello, dot) : 0;

				for(PLAYERINFOS::const_iterator i(pi.begin()); i != pi.end(); ++i) {

					std::string piz(i->name);
					piz.append(1, 0);

					if(cver >= 4) {
						piz.append(i->playerPic.empty() ? "-" : i->playerPic).append(1, 0);
					}

					send(piz.c_str(), piz.length(), cfd);
				}

				for(std::vector<std::string>::const_iterator i(getAIPlayers().begin());
						i != getAIPlayers().end(); ++i) {

					std::string piz(*i);
					piz.append(1, 0);

					if(cver >= 4) piz.append(AIDefaultPicture).append(1, 0);

					send(piz.c_str(), piz.length(), cfd);
				}

				send(cver >= 4 ? "PLAYERLISTEND\0-\0" : "PLAYERLISTEND\0",
					 cver >= 4 ? 16 : 14, cfd);

				shutdown(cfd, SHUT_RDWR);
				close(cfd);

				accepted = PLAYERLIST;

			} else {

				std::ostringstream oscap;

				for(CAPABILITIES::const_iterator i(m_caps.begin()); i != m_caps.end(); ++i) {
					oscap << i->first << '=' << i->second << '\0';
				}

				oscap << "CAPEND" << '\0';

				send(oscap.str().c_str(), oscap.str().length(), cfd);

				shutdown(cfd, SHUT_RDWR);
				close(cfd);

				accepted = CAP;
			}

		} else {
			shutdown(cfd, SHUT_RDWR);
			close(cfd);

			throw NetMauMau::Common::Exception::SocketException(gai_strerror(err), -1, errno);
		}
	}

	return accepted;
}

void Connection::sendVersionedMessage(const Connection::VERSIONEDMESSAGE &vm) const
throw(NetMauMau::Common::Exception::SocketException) {

	for(std::vector<NAMESOCKFD>::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) {

		const Connection::PLAYERINFOS::const_iterator &f(std::find_if(getPlayers().begin(),
				getPlayers().end(), std::bind2nd(_isPlayer(), i->name)));

		if(f != getPlayers().end()) {

			bool vMsg = false;

			for(VERSIONEDMESSAGE::const_iterator j(vm.begin()); j != vm.end(); ++j) {
				if(j->first && f->clientVersion >= j->first) {
					std::string msg(j->second);
					write(i->sockfd, msg.substr(j->second.length() - 9) == "VM_ADDPIC" ?
						  msg.replace(j->second.length() - 9, std::string::npos,
									  i->playerPic.empty() ? "-" : i->playerPic) : msg);
					vMsg = true;
					break;
				}
			}

			if(!vMsg) write(i->sockfd, vm.find(0)->second);
		}
	}
}

Connection &Connection::operator<<(const std::string &msg)
throw(NetMauMau::Common::Exception::SocketException) {

	for(std::vector<NAMESOCKFD>::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) {
		write(i->sockfd, msg);
	}

	return *this;
}

Connection &Connection::operator>>(std::string &msg)
throw(NetMauMau::Common::Exception::SocketException) {

	for(std::vector<NAMESOCKFD>::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) {
		msg = read(i->sockfd);
	}

	return *this;
}

void Connection::intercept() throw(NetMauMau::Common::Exception::SocketException) {
	INFO info;
	accept(info, true);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
