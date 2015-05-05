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
#include "config.h"
#endif

#include <algorithm>
#include <cstring>
#include <cerrno>
#include <cstdio>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(HAVE_SYS_STAT_H) && defined(HAVE_SYS_TYPES_H)
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include "serverconnection.h"

#include "defaultplayerimage.h"
#include "errorstring.h"
#include "pngcheck.h"
#include "base64.h"
#include "logger.h"
#include "sqlite.h"

namespace {
const std::string aiBase64
(NetMauMau::Common::base64_encode(reinterpret_cast<const NetMauMau::Common::BYTE *>
								  (NetMauMau::Common::DefaultPlayerImage.c_str()),
								  NetMauMau::Common::DefaultPlayerImage.length()));

#if defined(HAVE_MAGIC_H) && defined(HAVE_LIBMAGIC)
const char *NOPNG = " or no PNG image";
#else
const char *NOPNG = "";
#endif

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct _isPlayer : public std::binary_function < NetMauMau::Common::IConnection::NAMESOCKFD,
		std::string, bool > {

	bool operator()(const NetMauMau::Common::IConnection::NAMESOCKFD &nsd,
					const std::string &player) const {
		return nsd.name == player;
	}
};

#if 0
struct _playerClientversionLess :
	public std::binary_function < NetMauMau::Common::IConnection::PLAYERINFOS::value_type,
		NetMauMau::Common::IConnection::PLAYERINFOS::value_type, bool > {
	bool operator()(const NetMauMau::Common::IConnection::PLAYERINFOS::value_type &x,
					const NetMauMau::Common::IConnection::PLAYERINFOS::value_type &y) const {
		return x.clientVersion < y.clientVersion;
	}
};

struct _playerClientversionLess2 :
	public std::binary_function < NetMauMau::Common::IConnection::PLAYERINFOS::value_type,
		uint32_t, bool > {
	bool operator()(const NetMauMau::Common::IConnection::PLAYERINFOS::value_type &x,
					uint32_t y) const {
		return x.clientVersion < y;
	}
};
#endif
#pragma GCC diagnostic pop

}

using namespace NetMauMau::Server;

Connection::Connection(uint32_t minVer, bool inetd, uint16_t port, const char *server)
	: AbstractConnection(server, port), m_caps(), m_clientMinVer(minVer), m_inetd(inetd),
	  m_aiPlayerImages(new(std::nothrow) const std::string*[4]()) {

#if !defined(_WIN32) && (defined(HAVE_SYS_STAT_H) && defined(HAVE_SYS_TYPES_H))

	struct stat s;

	const std::string dataDir(PKGDATADIR"/ai_img");

	for(int i = 0; i < 4; ++i) {

		m_aiPlayerImages[i] = 0L;

		char *fname = strdup(std::string(dataDir).append(1, 0x30 + i).append(".png").c_str());

		if(stat(fname, &s) != -1) {

			FILE *in = std::fopen(fname, "rb");

			if(in) {

				NetMauMau::Common::BYTE *picData =
					new(std::nothrow) NetMauMau::Common::BYTE[s.st_size]();

				NetMauMau::Common::BYTE *ptr = picData;

				if(picData) {

					std::size_t r;

					while((r = std::fread(ptr, static_cast<std::size_t>(s.st_size),
										  sizeof(NetMauMau::Common::BYTE), in))) {
						ptr += r;
					}

					if(std::feof(in)) {

						if(NetMauMau::Common::checkPNG(picData,
													   static_cast<std::size_t>(s.st_size))) {
							m_aiPlayerImages[i] = new(std::nothrow)
							std::string(NetMauMau::Common::base64_encode(picData,
										static_cast<std::size_t>(s.st_size)));
						} else {
							logWarning("Image for AI player " << i << ": " << fname
									   << " is not a PNG image; discarding it");
						}

					} else {
						logWarning("Error reading image for AI player " << i << ": " << fname
								   << ": " << std::strerror(errno));
					}

					delete [] picData;
				}

				std::fclose(in);

			} else {
				logWarning("Couldn't read image for AI player " << i << ": " << fname);
			}

		} else if(errno != ENOENT) {
			logWarning("Can't stat " << fname << ": " << std::strerror(errno));
		}

		free(fname);
	}

#else

	for(int i = 0; i < 4; ++i) m_aiPlayerImages[i] = 0L;

#endif
}

Connection::~Connection() {

	for(PLAYERINFOS::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) {

		NetMauMau::DB::SQLite::getInstance()->logOutPlayer(*i);

		try {
			send("BYE", 3, i->sockfd);
		} catch(const NetMauMau::Common::Exception::SocketException &) {}

		shutdown(i->sockfd);
	}

	for(int i = 0; i < 4; ++i) delete m_aiPlayerImages[i];

	delete [] m_aiPlayerImages;
}

bool Connection::wire(SOCKET sockfd, const struct sockaddr *addr, socklen_t addrlen) const {

	const int yes = 1;

#ifndef _WIN32

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&yes),
				  sizeof(int)) == -1) {
#else

	if(setsockopt(sockfd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char *>(&yes),
				  sizeof(int)) == -1) {
#endif
		return false;
	}

	return bind(sockfd, addr, addrlen) == 0;
}

std::string Connection::wireError(const std::string &err) const {
	return std::string("could not bind") + (!err.empty() ? ": " : "") + (!err.empty() ? err : "");
}

void Connection::connect(bool inetd) throw(NetMauMau::Common::Exception::SocketException) {

	AbstractConnection::connect(inetd);

	if(listen(getSocketFD(), SOMAXCONN)) {
		throw NetMauMau::Common::Exception::SocketException(NetMauMau::Common::errorString(),
				getSocketFD(), errno);
	}
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic push
int Connection::wait(timeval *tv) {

	if(tv) {
		for(PLAYERINFOS::const_iterator i(getRegisteredPlayers().begin());
				i != getRegisteredPlayers().end(); ++i) {

#ifdef _WIN32
			fd_set rfds;

			FD_ZERO(&rfds);
			FD_SET(i->sockfd, &rfds);

			int nRet, err = 0;
			timeval tv = { 0, 0 };

			if((nRet = select(0, &rfds, NULL, NULL, &tv)) == SOCKET_ERROR) {
				err = -2;
			}

			if(nRet > 0 && FD_ISSET(i->sockfd, &rfds)) {
				err = -2;
			}

			if(err == -2) {
				logDebug("Lost connection to player \"" << i->name << "\"");
				removePlayer(i->sockfd);
				return err;
			}

#else

			char buffer[32];

			if(!::recv(i->sockfd, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT)) {
				logDebug("Lost connection to player \"" << i->name << "\"");
				removePlayer(i->sockfd);
				return -2;
			}

#endif
		}
	}

	fd_set rfds;

	FD_ZERO(&rfds);
	FD_SET(getSocketFD(), &rfds);

	return ::select(getSocketFD() + 1, &rfds, NULL, NULL, tv);
}
#pragma GCC diagnostic pop

Connection::ACCEPT_STATE Connection::accept(INFO &info,
		bool gameRunning) throw(NetMauMau::Common::Exception::SocketException) {

	bool refuse = gameRunning;

	ACCEPT_STATE accepted = REFUSED;

	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len = sizeof(struct sockaddr_storage);

	const SOCKET cfd = ::accept(getSocketFD(), reinterpret_cast<struct sockaddr *>(&peer_addr),
								&peer_addr_len);

	if(cfd != INVALID_SOCKET) {

		try {

			info.sockfd = cfd;

			char host[NI_MAXHOST], service[NI_MAXSERV];

			const int err = getnameinfo(reinterpret_cast<struct sockaddr *>(&peer_addr),
										peer_addr_len, host, NI_MAXHOST, service, NI_MAXSERV,
										NI_NUMERICSERV);

			info.port = (uint16_t)std::strtoul(service, NULL, 10);
			info.host = host;

			if(!err) {

				const std::string hello(PACKAGE_NAME);

				std::ostringstream os;
				os << hello << ' ' << MIN_MAJOR << '.' << MIN_MINOR;

				send(os.str().c_str(), os.str().length(), cfd);

				const std::string rHello = read(cfd);

				if(rHello != "CAP" && rHello.substr(0, 10) != "PLAYERLIST" &&
						rHello.substr(0, 6) != "SCORES") {

					const std::string::size_type spc = rHello.find(' ');
					const std::string::size_type dot = rHello.find('.');

					if(isValidHello(dot, spc, rHello, hello)) {

						info.maj = getMajorFromHello(rHello, dot, spc);
						info.min = getMinorFromHello(rHello, dot);

						const uint32_t cver = MAKE_VERSION(info.maj, info.min);
						const uint32_t minver = getMinClientVersion();
						const uint32_t maxver = getServerVersion();

						send(cver >= 4 ? "NAMP" : "NAME", 4, cfd);

						std::string namePic;

						try {
							namePic.reserve(MAXPICBYTES);
						} catch(const std::bad_alloc &) {}

						try {
							namePic = read(cfd, cver >= 4 ? MAXPICBYTES : 1024);
						} catch(const NetMauMau::Common::Exception::SocketException &e) {
							if(e.error() == ENOMEM) {

								try {
									namePic = read(cfd);
								} catch(const NetMauMau::Common::Exception::SocketException &) {
									throw;
								}

							} else {
								throw;
							}
						}

						std::size_t left = namePic.length();

						info.name = namePic.substr(0, namePic.find('\0'));

						if(namePic.empty() || namePic == "+") refuse = true;

						if(cver >= minver && cver <= maxver && !refuse) {

							std::string playerPic, picLength;

							if(cver >= 4 && info.name[0] == '+') {

								try {

									left += info.name.length() + 1;

									info.name = info.name.substr(1);

									picLength = namePic.substr(namePic.find('\0') + 1);
									picLength = picLength.substr(0, picLength.find('\0'));

									left += picLength.length() + 1;

									std::size_t pl;
									(std::istringstream(picLength)) >> pl;

									try {

										std::size_t v = pl - std::min(pl, left);
										playerPic = namePic.substr(namePic.rfind('\0') + 1);

										while(v) {
											playerPic.reserve(playerPic.size() + v);
											playerPic.append(read(cfd, v));
											v = pl - playerPic.length();
										}

										char cc[20] = "0\0";

										if(pl > MAXPICBYTES || !isPNG(playerPic)) {

											send(cc, 20, cfd);
											recv(cc, 2, cfd);

											logInfo("Player picture for \"" << info.name
													<< "\" rejected (too large" << NOPNG << ")");
											std::string().swap(playerPic);

										} else {
#ifndef _WIN32
											std::snprintf(cc, 20, "%zu", playerPic.length());
#else
											std::snprintf(cc, 20, "%lu",
														  (unsigned long)playerPic.length());
#endif
											send(cc, 20, cfd);
											recv(cc, 2, cfd);

											if(!(cc[0] == 'O' && cc[1] == 'K')) {
												logWarning("Player picture transmission for \""
														   << info.name << "\" failed: got "
														   << playerPic.length()
														   << " bytes; expected " << pl
														   << " bytes)");
												std::string().swap(playerPic);
											} else {
												logInfo("Player picture transmission for \""
														<< info.name << "\" successful ("
														<< playerPic.length() << " bytes)");
											}
										}

									} catch(const std::bad_alloc &) {

										char cc[20] = "0\0";

										send(cc, 20, cfd);
										recv(cc, 2, cfd);
										logInfo("Player picture for \"" << info.name
												<< "\" rejected (out of memory)");

										std::string().swap(playerPic);
									}

								} catch(const NetMauMau::Common::Exception::SocketException &) {
									std::string().swap(playerPic);
								}
							}

							const NAMESOCKFD nsf(info.name, playerPic, cfd, cver);
							const bool isOk = registerPlayer(nsf, getAIPlayers());

							send(isOk ? "OK" : "IN", 2, cfd);

							if(isOk) {
								accepted = PLAY;
								NetMauMau::DB::SQLite::getInstance()->addPlayer(info);
							} else {
								shutdown(cfd);
								accepted = REFUSED;
							}

						} else {

							try {
								send(cver <= maxver ? (gameRunning ? "GR" : "NO") : "VM", 2, cfd);
							} catch(const NetMauMau::Common::Exception::SocketException &e) {
								logDebug("Sending " << (cver <= maxver ? "NO" : "VM")
										 << " to client failed: " << e.what());
							}

							shutdown(cfd);
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

						shutdown(cfd);
					}

				} else if(rHello.substr(0, 10) == "PLAYERLIST") {

					const std::string::size_type spc = rHello.find(' ');
					const std::string::size_type dot = rHello.find('.');

					const PLAYERINFOS &pi(getRegisteredPlayers());
					const uint32_t cver = rHello.length() > 10 ?
										  MAKE_VERSION(getMajorFromHello(rHello, dot, spc),
													   getMinorFromHello(rHello, dot)) : 0;

					for(PLAYERINFOS::const_iterator i(pi.begin()); i != pi.end(); ++i) {

						std::string piz(i->name);
						piz.append(1, 0);

						if(cver >= 4) {
							piz.reserve(piz.length() + i->playerPic.length() + 1);
							piz.append(i->playerPic.empty() ? "-" : i->playerPic).append(1, 0);
						}

						send(piz.c_str(), piz.length(), cfd);
					}

					std::size_t j = 0;

					for(std::vector<std::string>::const_iterator i(getAIPlayers().begin());
							i != getAIPlayers().end(); ++i, ++j) {

						std::string piz(*i);
						piz.append(1, 0);

						if(cver >= 4) {

							piz.reserve(piz.length() + (m_aiPlayerImages[j] &&
														!m_aiPlayerImages[j]->empty() ?
														m_aiPlayerImages[j]->length() :
														aiBase64.length())
										+ 1);

							piz.append(m_aiPlayerImages[j] && !m_aiPlayerImages[j]->empty() ?
									   (*m_aiPlayerImages[j]) : aiBase64).append(1, 0);
						}

						send(piz.c_str(), piz.length(), cfd);
					}

					send(cver >= 4 ? "PLAYERLISTEND\0-\0" : "PLAYERLISTEND\0",
						 cver >= 4 ? 16 : 14, cfd);

					shutdown(cfd);
					accepted = PLAYERLIST;

				} else if(rHello.substr(0, 6) == "SCORES") {

					const NetMauMau::DB::SQLite::SCORE_TYPE st =
						rHello.substr(7, rHello.find(' ', 7) - 7) == "ABS" ?
						NetMauMau::DB::SQLite::ABS : NetMauMau::DB::SQLite::NORM;

					const std::size_t limit = std::strtoul(rHello.substr(rHello.rfind(' ')).c_str(),
														   NULL, 10);

					const
					NetMauMau::DB::SQLite::SCORES &scores(NetMauMau::DB::SQLite::getInstance()->
														  getScores(st, limit));

					std::ostringstream osscores;

					for(NetMauMau::DB::SQLite::SCORES::const_iterator i(scores.begin());
							i != scores.end(); ++i) {
						osscores << i->name << '=' << i->score << '\0';
					}

					osscores << "SCORESEND" << '\0';

					send(osscores.str().c_str(), osscores.str().length(), cfd);

					shutdown(cfd);
					accepted = SCORES;

				} else {

					std::ostringstream oscap;

					for(CAPABILITIES::const_iterator i(m_caps.begin()); i != m_caps.end(); ++i) {
						oscap << i->first << '=' << i->second << '\0';
					}

					oscap << "CAPEND" << '\0';

					send(oscap.str().c_str(), oscap.str().length(), cfd);

					shutdown(cfd);
					accepted = CAP;
				}

			} else {
				shutdown(cfd);
				throw NetMauMau::Common::Exception::SocketException(gai_strerror(err), -1, errno);
			}

		} catch(const NetMauMau::Common::Exception::SocketException &) {
			shutdown(cfd);
			throw;
		}
	}

	return accepted;
}

void Connection::removePlayer(const NetMauMau::Common::IConnection::INFO &info) {
	NetMauMau::DB::SQLite::getInstance()->logOutPlayer(NAMESOCKFD(info.name, "", info.sockfd,
			MAKE_VERSION(info.maj, info.min)));
	NetMauMau::Common::AbstractConnection::removePlayer(info);
}

void Connection::removePlayer(SOCKET sockfd) {
	NetMauMau::DB::SQLite::getInstance()->logOutPlayer(getPlayerInfo(sockfd));
	NetMauMau::Common::AbstractConnection::removePlayer(sockfd);
}

NetMauMau::Common::IConnection::NAMESOCKFD
Connection::getPlayerInfo(const std::string &name) const {

	const PLAYERINFOS::const_iterator &f(std::find_if(getRegisteredPlayers().begin(),
										 getRegisteredPlayers().end(), std::bind2nd(_isPlayer(),
												 name)));

	return f != getRegisteredPlayers().end() ? *f : NAMESOCKFD();
}

void Connection::sendVersionedMessage(const Connection::VERSIONEDMESSAGE &vm) const
throw(NetMauMau::Common::Exception::SocketException) {

	for(PLAYERINFOS::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) {

		const Connection::PLAYERINFOS::const_iterator &f(std::find_if(getPlayers().begin(),
				getPlayers().end(), std::bind2nd(_isPlayer(), i->name)));

		if(f != getPlayers().end()) {

			bool vMsg = false;

			for(VERSIONEDMESSAGE::const_iterator j(vm.begin()); j != vm.end(); ++j) {

				if(j->first && f->clientVersion >= j->first) {

					std::string msg(j->second);

					const bool wantPic = msg.substr(j->second.length() - 9) == "VM_ADDPIC";

					const Connection::PLAYERINFOS::const_iterator
					&pp(wantPic ? std::find_if(getPlayers().begin(), getPlayers().end(),
											   std::bind2nd(_isPlayer(), msg.substr(13,
															msg.length() - 23))) :
						getPlayers().end());

					if(wantPic && pp != getPlayers().end()) {
						write(i->sockfd, msg.replace(j->second.length() - 9, std::string::npos,
													 pp->playerPic.empty() ? "-" : pp->playerPic));
					} else {
						write(i->sockfd, msg);
					}

					vMsg = true;
					break;
				}
			}

			const VERSIONEDMESSAGE::const_iterator &nullV(vm.find(0));

			if(!vMsg && nullV != vm.end()) write(i->sockfd, nullV->second);
		}
	}
}

void Connection::clearPlayerPictures() const {
	for(PLAYERINFOS::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) {
		std::string().swap(i->playerPic);
	}
}

Connection &Connection::operator<<(const std::string &msg)
throw(NetMauMau::Common::Exception::SocketException) {

	for(PLAYERINFOS::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) {
		write(i->sockfd, msg);
	}

	return *this;
}

Connection &Connection::operator>>(std::string &msg)
throw(NetMauMau::Common::Exception::SocketException) {

	for(PLAYERINFOS::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) {
		msg = read(i->sockfd);
	}

	return *this;
}

void Connection::intercept() throw(NetMauMau::Common::Exception::SocketException) {
	INFO info;
	accept(info, true);
}

bool Connection::isPNG(const std::string &pic) {
	const std::vector<NetMauMau::Common::BYTE> &pngData(NetMauMau::Common::base64_decode(pic));
	return !(pngData.empty() || !NetMauMau::Common::checkPNG(pngData.data(), pngData.size()));
}

void Connection::reset() throw() {

	for(PLAYERINFOS::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) {
		NetMauMau::DB::SQLite::getInstance()->logOutPlayer(*i);
	}

	AbstractConnection::reset();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
