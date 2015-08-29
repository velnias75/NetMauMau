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

#include "serverconnection.h"

#ifdef HAVE_NETDB_H
#include <netdb.h>                      // for NI_MAXHOST, NI_MAXSERV, etc
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/stat.h>                   // for stat
#include <cerrno>                       // for errno, ENOENT, ENOMEM
#include <cstdio>                       // for NULL, fclose, feof, fopen, etc
#include <cstring>                      // for strerror, strdup, strlen

#ifdef ENABLE_THREADS
#include "mutexlocker.h"
#endif

#include "sqlite.h"
#include "base64.h"                     // for BYTE, base64_encode, etc
#include "logger.h"                     // for BasicLogger, logWarning, etc
#include "defaultplayerimage.h"         // for DefaultPlayerImage
#include "errorstring.h"                // for errorString
#include "pngcheck.h"                   // for checkPNG
#include "select.h"
#include "tcpopt_cork.h"
#include "tcpopt_nodelay.h"
#include "protocol.h"                   // for BYE, VM_ADDPIC

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY
#endif

namespace {
#ifdef ENABLE_THREADS
pthread_mutex_t initMux = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t consumedMux = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  consumedCnd = PTHREAD_COND_INITIALIZER;

pthread_mutexattr_t muxAttr;

void *playerThread(void *arg) throw() {

	NetMauMau::Server::Connection::PLAYERTHREADDATA *ptd =
		static_cast<NetMauMau::Server::Connection::PLAYERTHREADDATA *>(arg);

	do {

		NetMauMau::Common::MutexLocker mlp(&ptd->gmx);
		_UNUSED(mlp);

		while(!(ptd->stp || !ptd->msg.empty())) pthread_cond_wait(&ptd->get, &ptd->gmx);

		if(!ptd->stp) {

			try {
				logDebug(ptd->nfd.name << ": START write");
				ptd->con.write(ptd->nfd.sockfd, ptd->msg);
				logDebug(ptd->nfd.name << ": SLEEP");
				sleep(10);
				logDebug(ptd->nfd.name << ": END write");
			} catch(const NetMauMau::Common::Exception::SocketException &e) {
				logWarning("Exception in thread of " << ptd->nfd.name << ": " << e);
			}

			MUTEXLOCKER(&consumedMux);
			std::string().swap(ptd->msg);
			pthread_cond_signal(&consumedCnd);
		}

	} while(!ptd->stp);

	return NULL;
}

struct _socketCmp : std::unary_function<NetMauMau::Server::Connection::PLAYERTHREADDATA *, bool> {
	inline _socketCmp(SOCKET s) : fd(s) {}
	inline result_type operator()(Commons::RParam<argument_type>::Type ptd) const {
		return ptd->nfd.sockfd == fd;
	}
private:
	SOCKET fd;
};

struct _playerThreadShutter :
		std::unary_function<NetMauMau::Server::Connection::PLAYERTHREADDATA *, void> {
	inline result_type operator()(Commons::RParam<argument_type>::Type ptd) const {

		int r;

		{
			MUTEXLOCKER(&ptd->gmx);
			ptd->stp = true;
			pthread_cond_signal(&ptd->get);
		}

		if((r = pthread_join(ptd->tid, NULL))) {
			logDebug("pthread_join:" << NetMauMau::Common::errorString(r));
		}

		delete ptd;
	}
};

struct _playerThreadCreator : std::unary_function<NetMauMau::Server::Connection::NAMESOCKFD, void> {

	inline _playerThreadCreator(NetMauMau::Server::Connection &c,
								NetMauMau::Server::Connection::PTD &d) : con(c), data(d) {}

	inline result_type operator()(Commons::RParam<argument_type>::Type nfd) const {

		pthread_t tid;

		NetMauMau::Server::Connection::PLAYERTHREADDATA *ptd =
			new NetMauMau::Server::Connection::PLAYERTHREADDATA(nfd, con);

		if(!pthread_create(&tid, NULL, playerThread, static_cast<void *>(ptd))) {
			ptd->tid = tid;
			data.push_back(ptd);
		} else {
			delete ptd;
		}
	}

private:
	NetMauMau::Server::Connection &con;
	NetMauMau::Server::Connection::PTD &data;
};
#endif

_NORETURN void throwPlayerDisconnect(const NetMauMau::Common::IConnection::INFO &info)
throw(NetMauMau::Common::Exception::SocketException) {
	throw NetMauMau::Common::Exception::SocketException(std::string("Player \"").append(info.name).
			append(" has disconnected"), info.sockfd);
}

const std::string HELLO(PACKAGE_NAME);

const std::string aiBase64
(NetMauMau::Common::base64_encode(reinterpret_cast<const NetMauMau::Common::BYTE *>
								  (NetMauMau::Common::DefaultPlayerImage.c_str()),
								  NetMauMau::Common::DefaultPlayerImage.length()));

const char *TRANSMISSION  = "Player picture transmission for \"";

#if !(defined(WIN32) || defined(NDEBUG))
const char *LOSTCONPLAYER = "Lost connection to player \"";
#endif

#if defined(HAVE_MAGIC_H) && defined(HAVE_LIBMAGIC)
const char *NOPNG = " or no PNG image";
#else
const char *NOPNG = "";
#endif

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct _isPlayer : std::binary_function < NetMauMau::Common::IConnection::NAMESOCKFD, std::string,
		bool > {
	inline result_type operator()(const first_argument_type &nsf,
								  const second_argument_type &player) const {
		return nsf.name.compare(player) == 0;
	}
};

struct _logOutPlayer : std::unary_function<NetMauMau::Common::IConnection::NAMESOCKFD, void> {
	inline result_type operator()(const argument_type &nsf) const {
		NetMauMau::DB::SQLite::getInstance()->logOutPlayer(nsf);
	}
};
#pragma GCC diagnostic pop

}

using namespace NetMauMau::Server;

#ifdef ENABLE_THREADS
Connection::_playerThreadData::_playerThreadData(const NAMESOCKFD &n, Connection &c) : get(), gmx(),
	nfd(n), tid(), msg(), stp(false), con(c) {

	MUTEXLOCKER(&initMux);

	pthread_cond_init(&get, NULL);
	pthread_mutex_init(&gmx, &muxAttr);
}

Connection::_playerThreadData::~_playerThreadData() {

	MUTEXLOCKER(&initMux);

	pthread_mutex_destroy(&gmx);
	pthread_cond_destroy(&get);
}
#endif

Connection::Connection(uint32_t minVer, bool inetd, uint16_t port, const char *server)
	: AbstractConnection(server, port, true), m_caps(), m_clientMinVer(minVer), m_inetd(inetd),
	  m_aiPlayerImages(new(std::nothrow) const std::string*[4]()) {

#ifdef ENABLE_THREADS
	pthread_mutexattr_init(&muxAttr);
	pthread_mutexattr_settype(&muxAttr, PTHREAD_MUTEX_ERRORCHECK);
#endif

#if !defined(_WIN32) && (defined(HAVE_SYS_STAT_H) && defined(HAVE_SYS_TYPES_H))

	struct stat s;

	const std::string dataDir(PKGDATADIR "/ai_img");

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
										  sizeof(NetMauMau::Common::BYTE), in))) ptr += r;

					if(std::feof(in)) {

						if(NetMauMau::Common::checkPNG(picData,
													   static_cast<std::size_t>(s.st_size))) {
							m_aiPlayerImages[i] = new(std::nothrow)
							std::string(NetMauMau::Common::base64_encode(picData,
										static_cast<std::size_t>(s.st_size)));
						} else {
							logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
									   << "Image for AI player " << i << ": " << fname
									   << " is not a PNG image; discarding it");
						}

					} else {
						logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
								   << "Error reading image for AI player " << i << ": " << fname
								   << ": " << std::strerror(errno));
					}

					delete [] picData;
				}

				std::fclose(in);

			} else {
				logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
						   << "Couldn't read image for AI player " << i << ": " << fname);
			}

		} else if(errno != ENOENT) {
			logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Can't stat " << fname << ": "
					   << std::strerror(errno));
		}

		free(fname);
	}

#else

	for(int i = 0; i < 4; ++i) m_aiPlayerImages[i] = 0L;

#endif
}

Connection::~Connection() {

	TCPOPT_NODELAY(getSocketFD());

	for(PLAYERINFOS::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) {

		NetMauMau::DB::SQLite::getInstance()->logOutPlayer(*i);

		try {

			const NetMauMau::Common::TCPOptNodelay nd(i->sockfd);
			_UNUSED(nd);

#ifdef ENABLE_THREADS
			signalMessage(i->sockfd, NetMauMau::Common::Protocol::V15::BYE);
#else
			send(NetMauMau::Common::Protocol::V15::BYE.c_str(), 3, i->sockfd);
#endif

		} catch(const NetMauMau::Common::Exception::SocketException &) {}
	}

#ifdef ENABLE_THREADS
	shutdownThreads();
	pthread_mutexattr_destroy(&muxAttr);
#endif

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

	return ::bind(sockfd, addr, addrlen) == 0;
}

std::string Connection::wireError(const std::string &err) const {
	return std::string("could not bind") + (!err.empty() ? ": " : "") + (!err.empty() ? err : "");
}

void Connection::connect(bool inetd) throw(NetMauMau::Common::Exception::SocketException) {

	AbstractConnection::connect(inetd);

	if(::listen(getSocketFD(), SOMAXCONN)) {
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

			if((nRet = NetMauMau::Common::Select::getInstance()->perform(0, &rfds, NULL, NULL,
					   &tv)) == SOCKET_ERROR) {
				err = WAIT_ERROR;
			}

			if(nRet > 0 && FD_ISSET(i->sockfd, &rfds)) {
				err = WAIT_ERROR;
			}

			if(err == WAIT_ERROR) {
				logDebug(NetMauMau::Common::Logger::time(TIMEFORMAT) << LOSTCONPLAYER
						 << i->name << "\"");
				removePlayer(i->sockfd);
				return err;
			}

#else

			char buffer[32];

			if(!TEMP_FAILURE_RETRY(::recv(i->sockfd, buffer, sizeof(buffer), MSG_PEEK |
										  MSG_DONTWAIT))) {
				logDebug(NetMauMau::Common::Logger::time(TIMEFORMAT) << LOSTCONPLAYER << i->name
						 << "\"");
				removePlayer(i->sockfd);
				return WAIT_ERROR;
			}

#endif
		}
	}

	fd_set rfds;

	FD_ZERO(&rfds);
	FD_SET(getSocketFD(), &rfds);

	return NetMauMau::Common::Select::getInstance()->perform(getSocketFD() + 1, &rfds,
			NULL, NULL, tv);
}
#pragma GCC diagnostic pop

Connection::ACCEPT_STATE Connection::accept(INFO &info,
		bool gameRunning) throw(NetMauMau::Common::Exception::SocketException) {

	bool refuse = gameRunning;

	ACCEPT_STATE accepted = REFUSED;

	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len = sizeof(struct sockaddr_storage);

	const SOCKET cfd = TEMP_FAILURE_RETRY(::accept(getSocketFD(),
										  reinterpret_cast<struct sockaddr *>(&peer_addr),
										  &peer_addr_len));

	if(cfd != INVALID_SOCKET) {

		try {

			info.sockfd = cfd;

			char host[NI_MAXHOST], service[NI_MAXSERV];

			const int err = getnameinfo(reinterpret_cast<struct sockaddr *>(&peer_addr),
										peer_addr_len, host, NI_MAXHOST, service, NI_MAXSERV,
										NI_NUMERICSERV);

			info.port = static_cast<uint16_t>(std::strtoul(service, NULL, 10));
			info.host = host;

			if(!err) {

				std::ostringstream os;
				os << HELLO << ' ' << MIN_MAJOR << '.' << MIN_MINOR;

				{
					const NetMauMau::Common::TCPOptNodelay hello_nd(cfd);
					_UNUSED(hello_nd);

					send(os.str().c_str(), os.str().length(), cfd);
				}

				const std::string rHello = read(cfd);

				if(rHello != NetMauMau::Common::Protocol::V15::CAP &&
						rHello.compare(0, NetMauMau::Common::Protocol::V15::PLAYERLIST.length(),
									   NetMauMau::Common::Protocol::V15::PLAYERLIST) != 0 &&
						rHello.compare(0, NetMauMau::Common::Protocol::V15::SCORES.length(),
									   NetMauMau::Common::Protocol::V15::SCORES) != 0) {

					const std::string::size_type spc = rHello.find(' ');
					const std::string::size_type dot = rHello.find('.');

					if(isValidHello(dot, spc, rHello, HELLO)) {

						info.maj = getMajorFromHello(rHello, dot, spc);
						info.min = getMinorFromHello(rHello, dot);

						const uint32_t cver = MAKE_VERSION(info.maj, info.min);
						const uint32_t minver = getMinClientVersion();
						const uint32_t maxver = getServerVersion();

						send(cver >= 4 ? "NAMP" : "NAME", 4, cfd);

						std::string namePic;

						if(MAXPICBYTES <= namePic.max_size()) {

							try {
								namePic.reserve(MAXPICBYTES);
							} catch(const std::bad_alloc &) {}
						}

						try {

							const NetMauMau::Common::TCPOptCork np_cork(cver >= 4 ? cfd :
									INVALID_SOCKET);
							_UNUSED(np_cork);

							namePic = read(cfd, cver >= 4 ? MAXPICBYTES : 1024);

						} catch(const NetMauMau::Common::Exception::SocketException &e) {

							if(e.error() == ENOMEM) {

								try {

									const NetMauMau::Common::TCPOptCork npnm_cork(cver >= 4 ? cfd :
											INVALID_SOCKET);
									_UNUSED(npnm_cork);

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

						if(!(!namePic.empty() && namePic != '+')) refuse = true;

						if(cver >= minver && cver <= maxver && !refuse) {

							std::string playerPic, picLength;

							if(cver >= 4 && info.name[0] == '+') {

								try {

									left += info.name.length() + 1;

									info.name = info.name.substr(1);

									picLength = namePic.substr(namePic.find('\0') + 1);
									picLength = picLength.substr(0, picLength.find('\0'));

									left += picLength.length() + 1;

									const std::size_t pl = std::strtoul(picLength.c_str(),
																		NULL, 10);

									try {

										std::size_t v = pl - std::min(pl, left);
										playerPic = namePic.substr(namePic.rfind('\0') + 1);

										{
											const NetMauMau::Common::TCPOptCork nprm_cork(cfd);
											_UNUSED(nprm_cork);

											while(v) {

												const std::string::size_type resPp =
													playerPic.size() + v;

												if(resPp <= playerPic.max_size()) {
													playerPic.reserve(resPp);
												}

												playerPic.append(read(cfd, v));
												v = pl - playerPic.length();
											}
										}

										char cc[20] = "0\0";

										if(pl > MAXPICBYTES || !isPNG(playerPic)) {

											const NetMauMau::Common::TCPOptNodelay imgtl_nd(cfd);
											_UNUSED(imgtl_nd);

											std::string().swap(playerPic);

											send(cc, 20, cfd);

											if(recv(cc, 2, cfd)) {

												logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT)
														<< "Player picture for \"" << info.name
														<< "\" rejected (too large" << NOPNG
														<< ")");

											} else {
												throwPlayerDisconnect(info);
											}

										} else {

											const NetMauMau::Common::TCPOptNodelay imgsucc_nd(cfd);
											_UNUSED(imgsucc_nd);
#ifndef _WIN32
											std::snprintf(cc, 20, "%zu", playerPic.length());
#else
											std::snprintf(cc, 20, "%lu",
														  (unsigned long)playerPic.length());
#endif
											send(cc, 20, cfd);

											if(!recv(cc, 2, cfd)) throwPlayerDisconnect(info);

											if(!(cc[0] == 'O' && cc[1] == 'K')) {
												logWarning(
													NetMauMau::Common::Logger::time(TIMEFORMAT)
													<< TRANSMISSION << info.name
													<< "\" failed: got " << playerPic.length()
													<< " bytes; expected " << pl << " bytes)");
												std::string().swap(playerPic);
											} else {
												logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT)
														<< TRANSMISSION << info.name
														<< "\" successful ("
														<< playerPic.length() << " bytes)");
											}
										}

									} catch(const std::bad_alloc &) {

										const NetMauMau::Common::TCPOptNodelay imgba_nd(cfd);
										_UNUSED(imgba_nd);

										std::string().swap(playerPic);

										char cc[20] = "0\0";
										send(cc, 20, cfd);

										if(recv(cc, 2, cfd)) {
											logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT)
													<< "Player picture for \"" << info.name
													<< "\" rejected (out of memory)");
										} else {
											throwPlayerDisconnect(info);
										}
									}

								} catch(const NetMauMau::Common::Exception::SocketException &e) {

									const NetMauMau::Common::TCPOptNodelay imgfail_nd(cfd);
									_UNUSED(imgfail_nd);

									std::string().swap(playerPic);

									char cc[20] = "0\0";
									send(cc, 20, cfd);

									if(recv(cc, 2, cfd)) {
										logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
												   << TRANSMISSION << info.name << "\" failed ("
												   << e << ")");
									} else {
										throwPlayerDisconnect(info);
									}
								}

							} else {
								std::string().swap(playerPic);
							}

							const NAMESOCKFD nsf(info.name, playerPic, cfd, cver);
							const bool isOk = registerPlayer(nsf, getAIPlayers());

							if(isOk) notify(std::make_pair(info.name, playerPic));

							const NetMauMau::Common::TCPOptNodelay okin_nd(cfd);
							_UNUSED(okin_nd);

							send(isOk ? "OK" : "IN", 2, cfd);

							if(isOk) {
								accepted = PLAY;
								NetMauMau::DB::SQLite::getInstance()->addPlayer(info);
							} else {
								if(!gameRunning) shutdown(cfd);

								accepted = REFUSED;
							}

						} else {

							const NetMauMau::Common::TCPOptNodelay stat_nd(cfd);
							_UNUSED(stat_nd);

							try {
								send(cver <= maxver ? (gameRunning ? "GR" : "NO") : "VM", 2, cfd);
							} catch(const NetMauMau::Common::Exception::SocketException &e) {
								logDebug(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Sending "
										 << (cver <= maxver ? "NO" : "VM") << " to client failed: "
										 << e.what());
							}

							if(!gameRunning) shutdown(cfd);

							accepted = REFUSED;
						}

					} else {

						const NetMauMau::Common::TCPOptNodelay fail_nd(cfd);
						_UNUSED(fail_nd);

						logDebug(NetMauMau::Common::Logger::time(TIMEFORMAT) << "HELLO failed: "
								 << rHello.substr(0, std::strlen(PACKAGE_NAME)) << " != " << HELLO);

						try {
							send("NO", 2, cfd);
						} catch(const NetMauMau::Common::Exception::SocketException &e) {
							logDebug(NetMauMau::Common::Logger::time(TIMEFORMAT)
									 << "Sending NO to client failed: " << e.what());
						}

						if(!gameRunning) shutdown(cfd);
					}

				} else if(rHello.compare(0, NetMauMau::Common::Protocol::V15::PLAYERLIST.length(),
										 NetMauMau::Common::Protocol::V15::PLAYERLIST) == 0) {

					const NetMauMau::Common::TCPOptCork sc_pl(cfd);
					_UNUSED(sc_pl);

					const std::string::size_type spc = rHello.find(' ');
					const std::string::size_type dot = rHello.find('.');

					const PLAYERINFOS pi(getRegisteredPlayers());
					const uint32_t cver = rHello.length() > 10 ?
										  MAKE_VERSION(getMajorFromHello(rHello, dot, spc),
													   getMinorFromHello(rHello, dot)) : 0;

					for(PLAYERINFOS::const_iterator i(pi.begin()); i != pi.end(); ++i) {

						std::string piz(i->name);
						piz.append(1, 0);

						if(cver >= 4) {

							const std::string::size_type resPiz = piz.length() +
																  i->playerPic.length() + 1;

							if(resPiz <= piz.max_size()) piz.reserve(resPiz);

							piz.append(i->playerPic.empty() ? "-" : i->playerPic).append(1, 0);
						}

						send(piz.c_str(), piz.length(), cfd);
					}

					std::size_t j = 0;

					for(PLAYERNAMES::const_iterator i(getAIPlayers().begin());
							i != getAIPlayers().end(); ++i, ++j) {

						std::string piz(*i);
						piz.append(1, 0);

						if(cver >= 4) {

							if(j >= 4) j = 0;

							const std::string::size_type resPiz = piz.length() +
																  (m_aiPlayerImages[j] &&
																   !m_aiPlayerImages[j]->empty() ?
																   m_aiPlayerImages[j]->length() :
																   aiBase64.length()) + 1;

							if(resPiz <= piz.max_size()) piz.reserve(resPiz);

							piz.append(m_aiPlayerImages[j] && !m_aiPlayerImages[j]->empty() ?
									   (*m_aiPlayerImages[j]) : aiBase64).append(1, 0);

							notify(std::make_pair(*i, (m_aiPlayerImages[j] &&
													   !m_aiPlayerImages[j]->empty()) ?
												  (*m_aiPlayerImages[j]) : aiBase64));
						}

						send(piz.c_str(), piz.length(), cfd);
					}

					std::vector<std::string::traits_type::char_type>
					hdbvs(NetMauMau::Common::Protocol::V15::PLAYERLISTEND.begin(),
						  NetMauMau::Common::Protocol::V15::PLAYERLISTEND.end());

					hdbvs.push_back('\0');

					if(cver >= 4) {
						hdbvs.push_back('-');
						hdbvs.push_back('\0');
					}

					send(hdbvs.data(), hdbvs.size(), cfd);

					if(!gameRunning) shutdown(cfd);

					accepted = PLAYERLIST;

				} else if(rHello.compare(0, NetMauMau::Common::Protocol::V15::SCORES.length(),
										 NetMauMau::Common::Protocol::V15::SCORES) == 0) {

					const NetMauMau::Common::TCPOptCork sc_ck(cfd);
					_UNUSED(sc_ck);

					const NetMauMau::DB::SQLite::SCORE_TYPE st =
						rHello.compare(7, rHello.find(' ', 7) - 7, "ABS") == 0 ?
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

					osscores << NetMauMau::Common::Protocol::V15::SCORESEND << '\0';

					send(osscores.str().c_str(), osscores.str().length(), cfd);

					if(!gameRunning) shutdown(cfd);

					accepted = SCORES;

				} else {

					NetMauMau::Common::TCPOptCork cap_ck(cfd);
					_UNUSED(cap_ck);

					std::ostringstream oscap;

					for(CAPABILITIES::const_iterator i(m_caps.begin()); i != m_caps.end(); ++i) {
						oscap << i->first << '=' << i->second << '\0';
					}

					oscap << NetMauMau::Common::Protocol::V15::CAPEND << '\0';

					send(oscap.str().c_str(), oscap.str().length(), cfd);

					if(!gameRunning) shutdown(cfd);

					accepted = CAP;
				}

			} else {
				if(!gameRunning) shutdown(cfd);

				throw NetMauMau::Common::Exception::SocketException
				(NetMauMau::Common::errorString(err, true), INVALID_SOCKET, errno);
			}

		} catch(const NetMauMau::Common::Exception::SocketException &) {
			if(!gameRunning) shutdown(cfd);

			throw;
		}
	}

	return accepted;
}

void Connection::removePlayer(const NetMauMau::Common::IConnection::INFO &info) {
	NetMauMau::DB::SQLite::getInstance()->logOutPlayer(NAMESOCKFD(info.name, "", info.sockfd,
			MAKE_VERSION(info.maj, info.min)));
	NetMauMau::Common::AbstractConnection::removePlayer(info);

#ifdef ENABLE_THREADS
	removeThread(info.sockfd);
#endif
}

void Connection::removePlayer(SOCKET sockfd) {
	NetMauMau::DB::SQLite::getInstance()->logOutPlayer(getPlayerInfo(sockfd));
	NetMauMau::Common::AbstractConnection::removePlayer(sockfd);

#ifdef ENABLE_THREADS
	removeThread(sockfd);
#endif
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

	TCPOPT_NODELAY(getSocketFD());

	for(PLAYERINFOS::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) {

		const NetMauMau::Common::TCPOptNodelay nd(i->sockfd);
		_UNUSED(nd);

		const Connection::PLAYERINFOS::const_iterator &f(std::find_if(getPlayers().begin(),
				getPlayers().end(), std::bind2nd(_isPlayer(), i->name)));

		if(f != getPlayers().end()) {

			bool vMsg = false;

			for(VERSIONEDMESSAGE::const_iterator j(vm.begin()); j != vm.end(); ++j) {

				if(j->first && f->clientVersion >= j->first) {

					std::string msg(j->second);

					const bool wantPic = msg.substr(j->second.length() - 9) ==
										 NetMauMau::Common::Protocol::V15::VM_ADDPIC;

					const Connection::PLAYERINFOS::const_iterator
					&pp(wantPic ? std::find_if(getPlayers().begin(), getPlayers().end(),
											   std::bind2nd(_isPlayer(), msg.substr(13,
															msg.length() - 23))) :
						getPlayers().end());

					if(wantPic && pp != getPlayers().end()) {

						const std::string &smsg(msg.replace(j->second.length() - 9,
															std::string::npos,
															pp->playerPic.empty() ? "-" :
															pp->playerPic));
#ifdef ENABLE_THREADS
						const_cast<Connection *>(this)->signalMessage(i->sockfd, smsg);
#else
						write(i->sockfd, smsg);
#endif
					} else {
#ifdef ENABLE_THREADS
						const_cast<Connection *>(this)->signalMessage(i->sockfd, msg);
#else
						write(i->sockfd, msg);
#endif
					}

					vMsg = true;
					break;
				}
			}

			const VERSIONEDMESSAGE::const_iterator &nullV(vm.find(0));

			if(!vMsg && nullV != vm.end()) {
#ifdef ENABLE_THREADS
				const_cast<Connection *>(this)->signalMessage(i->sockfd, nullV->second);
#else
				write(i->sockfd, nullV->second);
#endif
			}
		}
	}
}

void Connection::clearPlayerPictures() const {
	for(PLAYERINFOS::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) std::string().swap(i->playerPic);
}

Connection &Connection::operator<<(const std::string &msg)
throw(NetMauMau::Common::Exception::SocketException) {

	TCPOPT_NODELAY(getSocketFD());

	for(PLAYERINFOS::const_iterator i(getRegisteredPlayers().begin());
			i != getRegisteredPlayers().end(); ++i) {

		const NetMauMau::Common::TCPOptNodelay nd(i->sockfd);
		_UNUSED(nd);

#ifdef ENABLE_THREADS
		signalMessage(i->sockfd, msg);
#else
		write(i->sockfd, msg);
#endif
	}

	return *this;
}

void Connection::intercept() throw(NetMauMau::Common::Exception::SocketException) {

	INFO info;

	info.sockfd = INVALID_SOCKET;

	try {
		switch(accept(info, true)) {
		case NONE:
			logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Connection from "
					<< info.host << ":" << info.port);
			break;

		case PLAY:
			logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Play request from "
					<< info.host << ":" << info.port);
			break;

		case CAP:
			logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Capabilities request from "
					<< info.host << ":" << info.port);
			break;

		case REFUSED:
			logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Refused join game request from "
					<< info.host << ":" << info.port);
			break;

		case PLAYERLIST:
			logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Player list request from "
					<< info.host << ":" << info.port);
			break;

		case SCORES:
			logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Scores request from "
					<< info.host << ":" << info.port);
			break;
		}

	} catch(const NetMauMau::Common::Exception::SocketException &e) {
#ifndef _NDEBUG

		if(info.sockfd != INVALID_SOCKET) {
			logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT) <<
					   "Error in intercepted connection from " << info.host << ":" << info.port
					   << ": " << e);
		} else {
			logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
					   << "Error in intercepted connection: " << e);
		}

#endif
	}

	shutdown(info.sockfd);
}

bool Connection::isPNG(const std::string &pic) {
	const std::vector<NetMauMau::Common::BYTE> &pngData(NetMauMau::Common::base64_decode(pic));
	return !(pngData.empty() || !NetMauMau::Common::checkPNG(pngData.data(), pngData.size()));
}

void Connection::reset() throw() {
	std::for_each(getRegisteredPlayers().begin(), getRegisteredPlayers().end(), _logOutPlayer());

#ifdef ENABLE_THREADS
	shutdownThreads();
#endif

	AbstractConnection::reset();

}

#ifdef ENABLE_THREADS
void Connection::shutdownThreads() {
	waitPlayerThreads();
	std::for_each(m_data.begin(), m_data.end(), _playerThreadShutter());
	PTD().swap(m_data);
}

void Connection::createThreads() {
	std::for_each(getRegisteredPlayers().begin(), getRegisteredPlayers().end(),
				  _playerThreadCreator(*this, m_data));
}

void Connection::removeThread(SOCKET fd) {

	const PTD::iterator &f(std::find_if(m_data.begin(), m_data.end(), _socketCmp(fd)));

	if(f != m_data.end()) {
		_playerThreadShutter()(*f);
		m_data.erase(std::remove(m_data.begin(), m_data.end(), *f), m_data.end());
	}
}

void Connection::waitPlayerThreads() const {

	for(PTD::const_iterator i(m_data.begin()); i != m_data.end(); ++i) {

		MUTEXLOCKER(&consumedMux);

		if(!(*i)->msg.empty()) while(!(*i)->msg.empty()) pthread_cond_wait(&consumedCnd,
						&consumedMux);
	}
}

void Connection::signalMessage(SOCKET fd, const std::string &msg) {

	const PTD::iterator &f(std::find_if(m_data.begin(), m_data.end(), _socketCmp(fd)));

	if(f != m_data.end()) {

		MUTEXLOCKER(&(*f)->gmx);
		(*f)->msg = msg;
		pthread_cond_signal(&(*f)->get);

	} else {
		write(fd, msg);
	}
}
#endif

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
