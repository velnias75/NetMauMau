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

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#if defined(IN_IDE_PARSER)
#define PKGDATADIR ""
#endif

#include <fstream>
#include <cstring>

#include <stdint.h>
#include <sys/types.h>

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifndef _WIN32
#define MHD_PLATFORM_H 1
#endif

#include <microhttpd.h>

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include "httpd.h"
#include "base64.h"
#include "logger.h"
#include "helpers.h"
#include "iplayer.h"
#include "mimemagic.h"
#include "pathtools.h"
#include "cachepolicyfactory.h"
#include "defaultplayerimage.h"
#include "abstractsocketimpl.h"

#ifdef HAVE_ZLIB_H
#include "zstreambuf.h"
#include "zlibexception.h"
#endif

#ifndef _WIN32
#define TIMEFORMAT "%T - "
#else
#define TIMEFORMAT "%H:%M:%S - "
#endif

namespace {

std::string LKFIM;
const char *B2TOP = "<a href=\"#top\">Back to top</a>";

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct scoresTable : std::unary_function<NetMauMau::DB::SQLite::SCORES::value_type, void> {
	inline explicit scoresTable(std::ostream &o) : os(o), pos(0) {}
	inline result_type operator()(const argument_type &s) const {
		os << "<tr><td align=\"right\">" << ++pos << "&nbsp;</td><td>&nbsp;" << s.name
		   << "</td><td align=\"center\">" << (s.score < 0 ? "<font color=\"red\">" : "")
		   << s.score << (s.score < 0 ? "</font>" : "") << "</td></tr>";
	}

private:
	std::ostream &os;
	mutable std::size_t pos;
};

struct capaTable :
		std::unary_function<NetMauMau::Common::AbstractConnection::CAPABILITIES::value_type, void> {
	inline explicit capaTable(std::ostream &o) : os(o) {}
	inline result_type operator()(const argument_type &p) const {
		const bool isWurl = p.first == "WEBSERVER_URL";
		os << "<tr><td>&nbsp;" << p.first << "</td><td>&nbsp;" << (isWurl ? "<a href=\"" : "")
		   << p.second << (isWurl ? "\">" : "") << (isWurl ? p.second : "")
		   << (isWurl ? "</a>" : "") << "</td></tr>";
	}

private:
	std::ostream &os;
};

struct listPlayers : std::unary_function<NetMauMau::Server::Httpd::PLAYERS::value_type, void> {
	inline explicit listPlayers(std::ostream &o) : os(o), pos(0u) {}
	inline result_type operator()(const argument_type &p) const {
		os << "<tr><td align=\"right\">&nbsp;" << ++pos
		   << ".&nbsp;</td><td align=\"center\">&nbsp;<a href=\"/images/" << p->getName()
		   << "\"><img height=\"30\" src=\"/images/" << p->getName() << "\">"
		   << "</a><b>&nbsp;</td><td>&nbsp;" << p->getName() << "</b>&nbsp;<i>("
		   << (p->getType() == NetMauMau::Player::IPlayer::HUMAN ? "human player" :
			   (p->getType() == NetMauMau::Player::IPlayer::HARD ? "hard AI" : "easy AI"))
		   << ")</i>&nbsp;</td></tr>";
	}

private:
	std::ostream &os;
	mutable std::size_t pos;
};
#pragma GCC diagnostic pop

#if MHD_VERSION <= 0x00000200
char *unquoteUrl(const char *url) {

	char *myUrl = strdup(url);
	char *p = myUrl;

	while(*p) {

		if(*p == '%' && *(p + 1) && *(p + 2)) {

			char hex[3] = { *(p + 1), *(p + 2), 0 };
			char c = static_cast<char>(std::strtol(hex, (char **) NULL, 16));

			if(c) {
				*p = c;
				std::strcpy(p + 1, p + 3);
			}
		}

		++p;

	}

	return myUrl;
}
#endif

#if MHD_VERSION > 0x00000200
void panic(void */*cls*/, const char *file, unsigned int line, const char *reason) {

	logFatal(NetMauMau::Common::Logger::time(TIMEFORMAT) << "webserver: PANIC: "
			 << file << ":" << line << (reason ? ": " : "") << (reason ? reason : ""));

#ifdef HAVE_RAISE
	std::raise(SIGTERM);
#else
	std::abort();
#endif
}
#endif

int processRequestHeader(void *cls, enum MHD_ValueKind /*kind*/,
						 const char *key, const char *value) {
	reinterpret_cast<NetMauMau::Server::Httpd *>(cls)->insertReqHdrPair(key, value);
	return MHD_YES;
}

int answer_to_connection(void *cls, struct MHD_Connection *connection, const char *url,
						 const char */*method*/, const char */*version*/,
						 const char */*upload_data*/,
#if MHD_VERSION > 0x00000200
						 size_t */*upload_data_size*/,
#else
						 unsigned int */*upload_data_size*/,
#endif
						 void **/*con_cls*/) {

	NetMauMau::Server::Httpd *httpd = reinterpret_cast<NetMauMau::Server::Httpd *>(cls);
	const bool havePlayers = !httpd->getPlayers().empty();

	httpd->clearReqHdrMap();
	MHD_get_connection_values(connection, MHD_HEADER_KIND, processRequestHeader, cls);

#if MHD_VERSION > 0x00000200
	// logging disabled for older MHD, maybe we should use the AcceptPolicy callback?
	const MHD_ConnectionInfo *info =
		MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS);

	char hbuf[NI_MAXHOST];

	if(getnameinfo(reinterpret_cast<sockaddr *>(info->client_addr), sizeof(sockaddr_in), hbuf,
				   sizeof(hbuf), NULL, 0, NI_NUMERICSERV)) std::strncpy(hbuf, "<unknown>", 9);

	logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "webserver: request from \'"
			<< hbuf << "\' to resource \'" << url << "\'");
#endif

	NetMauMau::Server::CachePolicyFactory::ICachePolicyPtr cp;
	std::vector<std::string::traits_type::char_type> bin;

#ifdef HAVE_ZLIB_H
	const NetMauMau::Server::Httpd::REQHEADERMAP::const_iterator
	&accEnc(httpd->getReqHdrMap().find("Accept-Encoding"));

	bool gzipReq = accEnc != httpd->getReqHdrMap().end() && accEnc->second.find("deflate") !=
				   NetMauMau::Server::Httpd::REQHEADERMAP::mapped_type::npos;
#else
	const bool gzipReq = false;
#endif

	char *contentType = 0L;
	bool binGzip = false, binary = false;

	std::ostringstream oss;
	oss.unsetf(std::ios_base::skipws);

	std::streambuf *sb = 0L;

#ifdef HAVE_ZLIB_H

	try {

		sb = gzipReq ? new NetMauMau::Common::Zstreambuf(oss, Z_BEST_COMPRESSION, true) : 0L;
#endif

		std::ostream os(sb ? sb : oss.rdbuf());

		if(!std::strncmp("/images/", url, 8)) {

			cp = NetMauMau::Server::CachePolicyFactory::getInstance()->
				 createPrivateCachePolicy(1800L);

			binary = true;

#if MHD_VERSION > 0x00000200
			const char *name = std::strrchr(url, '/');
#else
			char *myUrl = unquoteUrl(url);
			const char *name = std::strrchr(myUrl, '/');
#endif

			if(name && *(name + 1)) {

				const NetMauMau::Server::Httpd::IMAGES::const_iterator
				&f(httpd->getImages().find(name + 1));

				if(f != httpd->getImages().end()) {
					bin.assign(f->second.begin(), f->second.end());
				} else {
					bin.assign(NetMauMau::Common::DefaultPlayerImage.begin(),
							   NetMauMau::Common::DefaultPlayerImage.end());
				}

			} else {
				bin.assign(NetMauMau::Common::DefaultPlayerImage.begin(),
						   NetMauMau::Common::DefaultPlayerImage.end());
			}

			const std::string &mime(NetMauMau::Common::MimeMagic::getInstance()->
									getMime(reinterpret_cast<const unsigned char *>(bin.data()),
											bin.size()));

			contentType = !mime.empty() ? strdup((mime + "; charset=binary").c_str()) :
						  strdup("image/png; charset=binary");

#if MHD_VERSION <= 0x00000200
			free(myUrl);
#endif

		} else if(!std::strncmp("/robots.txt", url, 11)) {

			cp = NetMauMau::Server::CachePolicyFactory::getInstance()->createNoCachePolicy();
			contentType = strdup("text/plain");
			os << "User-agent: *\nDisallow: /\n";

		} else if(!std::strncmp("/favicon.ico", url, 8)) {

			cp = NetMauMau::Server::CachePolicyFactory::getInstance()->createPublicCachePolicy();

			binary = true;
			LKFIM = contentType = strdup("image/vnd.microsoft.icon; charset=binary");

			std::ifstream fav(NetMauMau::Common::getModulePath(NetMauMau::Common::PKGDATA,
							  "netmaumau", "ico").c_str(), std::ios::binary);

			if(!fav.fail()) {

#ifdef HAVE_ZLIB_H

				if(gzipReq) {

					std::streambuf *fisb = 0L;

					try {

						binGzip = true;

						std::ostringstream fioss;
						fioss.unsetf(std::ios_base::skipws);

						fisb = new NetMauMau::Common::Zstreambuf(fioss, Z_BEST_COMPRESSION, true);

						std::ostream foss(fisb);

						std::copy
						(std::istreambuf_iterator<std::string::traits_type::char_type>(fav),
						 std::istreambuf_iterator<std::string::traits_type::char_type>(),
						 std::ostreambuf_iterator<std::string::traits_type::char_type>(foss));

						foss.flush();
						delete fisb;

						const std::string &v(fioss.str());
						bin.assign(v.begin(), v.end());

					} catch(const NetMauMau::Common::Exception::ZLibException &) {
						binGzip = false;
						delete fisb;
					}

				} else {
#endif
					bin.assign(std::istreambuf_iterator<std::string::traits_type::char_type>(fav),
							   std::istreambuf_iterator<std::string::traits_type::char_type>());
#ifdef HAVE_ZLIB_H
				}

#endif

				if(!gzipReq) {
					const std::string &mime(NetMauMau::Common::MimeMagic::getInstance()->
											getMime(reinterpret_cast<const unsigned char *>
													(bin.data()), bin.size()));

					if(!mime.empty()) {
						LKFIM = contentType = strdup((mime + "; charset=binary").c_str());
					}
				}

			} else {
				logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
						   << "Failed to open favicon file: \"" <<
						   NetMauMau::Common::getModulePath(NetMauMau::Common::PKGDATA, "netmaumau",
								   "ico") << "\"");
			}

		} else {

			cp = NetMauMau::Server::CachePolicyFactory::getInstance()->createNoCachePolicy();

			contentType = strdup("text/html; charset=utf-8");

			const NetMauMau::DB::SQLite::SCORES &sc(httpd->getCapabilities().find("HAVE_SCORES") !=
													httpd->getCapabilities().end() ?
													NetMauMau::DB::SQLite::getInstance()->
													getScores(NetMauMau::DB::SQLite::NORM) :
													NetMauMau::DB::SQLite::SCORES());

			os << "<html><head>"
			   << "<link rel=\"shortcut icon\" type=\""
			   << (!LKFIM.empty() ? LKFIM.c_str() : "image/vnd.microsoft.icon")
			   << "\" href=\"/favicon.ico\" />"
			   << "<link rel=\"icon\" type=\""
			   << (!LKFIM.empty() ? LKFIM.c_str() : "image/vnd.microsoft.icon")
			   << "\" href=\"/favicon.ico\" />"
			   << "<title>" << PACKAGE_STRING << " ("
#ifndef _WIN32
			   << BUILD_TARGET
#else
			   << "Windows [" << BUILD_TARGET << "]"
#endif
			   << ")</title>";

			os << "<style>"
			   << "table, td, th { background-color:white; border: thin solid black; "
			   << "border-spacing: 0; border-collapse: collapse; }"
			   << "pre { background-color:white; }"
			   << "a { text-decoration:none; }"
			   << "img { border:none; }"
			   << "</style></head>";

			os << "<body bgcolor=\"#eeeeee\"><a name=\"top\"><font face=\"Sans-Serif\">"
			   << "<h1 align=\"center\">" << PACKAGE_STRING << "</h1></a><hr />";

			os << "<p><ul>";

			if(havePlayers) os << "<li><a href=\"#players\">Players online</a></li>";

			if(!sc.empty()) os << "<li><a href=\"#scores\">Hall of Fame</a></li>";

			os << "<li><a href=\"#capa\">Server capabilities</a></li>";
			os << "<li><a href=\"#dump\">Server dump</a></li>";

			os << "</ul></p><hr />";

			if(havePlayers) {
				os << "<a name=\"players\"><h2 align=\"center\">Players online <i>("
				   << (httpd->isWaiting() ?  "waiting" : "running")
				   << ")</i></h2><p align=\"center\"><table>";

				std::for_each(httpd->getPlayers().begin(), httpd->getPlayers().end(),
							  listPlayers(os));

				os << "</table></p></a>" << B2TOP << "<hr />";
			}

			if(!sc.empty()) {
				os << "<a name=\"scores\"><center><h2>Hall of Fame</h2><table width=\"50%\">"
				   << "<tr><th>&nbsp;</th><th>PLAYER</th><th>SCORE</th></tr>";

				std::for_each(sc.begin(), sc.end(), scoresTable(os));

				os << "</table></center></a>" << B2TOP << "<hr />";
			}

			os << "<a name=\"capa\"><center><h2>Server capabilities</h2><table width=\"50%\">"
			   << "<tr><th>NAME</th><th>VALUE</th></tr>";

			std::for_each(httpd->getCapabilities().begin(), httpd->getCapabilities().end(),
						  capaTable(os));

			os << "</table></center></a>" << B2TOP << "<hr /><a name=\"dump\">"
			   << "<h2 align=\"center\">Server dump</h2><tt><pre>";

			NetMauMau::dump(os);

			os << "== Version ==\n";

			NetMauMau::version(os, true);

			os << "</pre></a></tt><hr />" << B2TOP << "</font></body></html>";
		}

		os.flush();

#ifdef HAVE_ZLIB_H
	} catch(const NetMauMau::Common::Exception::ZLibException &e) {

		oss << e.what();
		gzipReq = false;

		if(!cp) cp = NetMauMau::Server::CachePolicyFactory::getInstance()->createNoCachePolicy();
	}

#endif

	delete sb;

	void *data = 0L;

	if(binary) {

		if((data = malloc(bin.size()))) {
			std::memcpy(data, bin.data(), bin.size());
		}

#ifdef HAVE_ZLIB_H
	} else if(gzipReq) {

		if((data = std::malloc(oss.str().size()))) {
			std::memcpy(data, oss.str().data(), oss.str().size());
		}

#endif
	} else {
#ifdef HAVE_STRNDUP
		data = static_cast<void *>(const_cast<char *>(strndup(oss.str().c_str(),
								   oss.str().length())));
#else
		data = static_cast<void *>(const_cast<char *>(strdup(oss.str().c_str())));
#endif
	}

	const std::size_t len = binary ? (data ? bin.size() : 0u) : gzipReq ? oss.str().size() :
								oss.str().length();

	MHD_Response *response = MHD_create_response_from_data(len, data, true, false);
	MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, contentType);

#ifdef HAVE_ZLIB_H

	if(gzipReq && (!binary || binGzip)) MHD_add_response_header(response,
				MHD_HTTP_HEADER_CONTENT_ENCODING, "deflate");

#endif

	if(cp->expires()) MHD_add_response_header(response, MHD_HTTP_HEADER_EXPIRES,
				cp->getExpiryDate());

	MHD_add_response_header(response, MHD_HTTP_HEADER_CACHE_CONTROL, cp->getCacheControl());

	const int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);

	MHD_destroy_response(response);

	free(contentType);

	return ret;
}

}

using namespace NetMauMau::Server;

Httpd::Httpd() : Common::IObserver<Game>(), Common::IObserver<Engine>(),
	Common::IObserver<Connection>(), Common::SmartSingleton<Httpd>(), m_daemon(0L), m_reqHdrMap(),
	m_gameSource(0L), m_engineSource(0L), m_connectionSource(0L), m_players(), m_images(), m_caps(),
	m_gameRunning(false), m_waiting(true), m_url() {

#if MHD_VERSION > 0x00000200
	MHD_set_panic_func(panic, this);
#endif

	struct addrinfo *ai = NULL;

	if(NetMauMau::httpd && !NetMauMau::Common::AbstractSocketImpl::getAddrInfo(NetMauMau::host &&
			*NetMauMau::host ? NetMauMau::host : 0L, static_cast<uint16_t>(NetMauMau::hport), &ai,
			true) && ai && !(m_daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
										static_cast<unsigned short>(NetMauMau::hport), NULL, NULL,
										&answer_to_connection, this,
										MHD_OPTION_CONNECTION_LIMIT, 10,
#if MHD_VERSION > 0x00000200
										MHD_OPTION_SOCK_ADDR, ai->ai_addr,
										MHD_OPTION_PER_IP_CONNECTION_LIMIT, 10,
										MHD_OPTION_THREAD_POOL_SIZE, 5,
#endif
										MHD_OPTION_END))) {

		logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
				   << "Failed to start webserver at http://"
#if MHD_VERSION > 0x00000200
				   << (ai && ai->ai_canonname ? ai->ai_canonname : "localhost")
#else
				   << "localhost"
#endif
				   << ":" << NetMauMau::hport);

	} else if(NetMauMau::httpd && ai && m_daemon) {
		logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Started webserver at http://"
#if MHD_VERSION > 0x00000200
				<< (ai && ai->ai_canonname ? ai->ai_canonname : "localhost")
#else
				<< "localhost"
#endif
				<< ":" << NetMauMau::hport);

		std::ostringstream uos;

		uos << "http://"
#if MHD_VERSION > 0x00000200
			<< (ai && ai->ai_canonname ? ai->ai_canonname : "localhost") << ':'
#else
			<< "localhost:"
#endif
			<< NetMauMau::hport;

		m_url = uos.str();
	}

	if(ai) freeaddrinfo(ai);
}

Httpd::~Httpd() throw() {
	if(m_daemon) MHD_stop_daemon(m_daemon);
}

void Httpd::setSource(const NetMauMau::Common::IObserver<Connection>::source_type *s) {
	m_connectionSource = s;
}

void Httpd::setSource(const NetMauMau::Common::IObserver<NetMauMau::Engine>::source_type *s) {
	m_engineSource = s;
}

void Httpd::setSource(const NetMauMau::Common::IObserver<Game>::source_type *s) {
	m_gameSource = s;
}

void Httpd::update(const NetMauMau::Common::IObserver<Connection>::what_type &what) {

	const std::vector<NetMauMau::Common::BYTE> &b64(NetMauMau::Common::base64_decode(what.second));

	if(b64.empty()) {
		NetMauMau::Common::efficientAddOrUpdate(m_images, what.first,
												NetMauMau::Common::DefaultPlayerImage);
	} else {
		NetMauMau::Common::efficientAddOrUpdate(m_images, what.first,
												std::string(b64.begin(), b64.end()));
	}
}

void Httpd::update(const NetMauMau::Common::IObserver<NetMauMau::Engine>::what_type &what) {
	m_players = what;
}

void Httpd::update(const NetMauMau::Common::IObserver<Game>::what_type &what) {

	switch(what) {
	case PLAYERADDED:
	case PLAYERREMOVED:
		break;

	case READY:
		m_waiting = false;
		break;

	case GAMESTARTED:
		m_gameRunning = true;
		m_waiting = false;
		break;

	case GAMEENDED:
		m_players.clear();
		m_images.clear();
		m_waiting = true;
		m_gameRunning = false;
		break;
	}
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
