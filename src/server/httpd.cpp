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

#include <cstring>
#include <cstdarg>

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MHD_PLATFORM_H 1

#include <microhttpd.h>
#include <netdb.h>

#include "httpd.h"
#include "logger.h"
#include "helpers.h"
#include "abstractsocketimpl.h"

#ifndef _WIN32
#define TIMEFORMAT "%T - "
#else
#define TIMEFORMAT "%H:%M:%S - "
#endif

namespace {

const char *B2TOP = "<a href=\"#top\">Back to top</a>";

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct scoresTable : std::unary_function<NetMauMau::DB::SQLite::SCORES::value_type, void> {

	inline explicit scoresTable(std::ostringstream &o) : os(o), pos(0) {}
	inline result_type operator()(const argument_type &s) const {
		os << "<tr><td align=\"right\">" << ++pos << "&nbsp;</td><td>&nbsp;" << s.name
		   << "</td><td align=\"center\">" << (s.score < 0 ? "<font color=\"red\">" : "") << s.score
		   << (s.score < 0 ? "</font>" : "") << "</td></tr>";
	}

private:
	std::ostringstream &os;
	mutable std::size_t pos;
};

struct capaTable :
		std::unary_function<NetMauMau::Common::AbstractConnection::CAPABILITIES::value_type, void> {

	inline explicit capaTable(std::ostringstream &o) : os(o) {}
	inline result_type operator()(const argument_type &p) const {
		os << "<tr><td>&nbsp;" << p.first << "</td><td>&nbsp;" << p.second << "</td></tr>";
	}

private:
	std::ostringstream &os;
};
#pragma GCC diagnostic pop

int answer_to_connection(void *cls, struct MHD_Connection *connection, const char */*url*/,
						 const char */*method*/, const char */*version*/,
						 const char */*upload_data*/, size_t */*upload_data_size*/,
						 void **/*con_cls*/) {

	const NetMauMau::Server::Httpd *httpd = reinterpret_cast<NetMauMau::Server::Httpd *>(cls);
	const bool haveScores = httpd->getCapabilities().find("HAVE_SCORES") !=
							httpd->getCapabilities().end();

	std::ostringstream os;

	os << "<html><head><title>" << PACKAGE_STRING << " (" << BUILD_TARGET << ")</title>";

	os << "<style>"
	   << "table, td, th { background-color:white; border: thin solid black; "
	   << "border-spacing: 0; border-collapse: collapse; }"
	   << "pre { background-color:white; }"
	   << "a { text-decoration:none; }"
	   << "</style>";

	os << "<body bgcolor=\"#eeeeee\"><a name=\"top\"><font face=\"Sans-Serif\">"
	   << "<h1 align=\"center\">" << PACKAGE_STRING << "</h1></a><hr />";

	os << "<p><ul>";

	if(haveScores) os << "<li><a href=\"#scores\">Hall of Fame</a></li>";

	os << "<li><a href=\"#capa\">Server capabilities</a></li>";
	os << "<li><a href=\"#dump\">Server dump</a></li>";

	os << "</ul></p><hr />";

	if(haveScores) {
		const NetMauMau::DB::SQLite::SCORES
		&sc(NetMauMau::DB::SQLite::getInstance()->getScores(NetMauMau::DB::SQLite::NORM));

		os << "<a name=\"scores\"><center><h2>Hall of Fame</h2><table width=\"50%\">"
		   << "<tr><th>&nbsp;</th><th>PLAYER</th><th>SCORE</th></tr>";

		std::for_each(sc.begin(), sc.end(), scoresTable(os));

		os << "</table></center></a>" << B2TOP << "<hr />";
	}

	os << "<a name=\"capa\"><center><h2>Server capabilities</h2><table width=\"50%\">"
	   << "<tr><th>NAME</th><th>VALUE</th></tr>";

	std::for_each(httpd->getCapabilities().begin(), httpd->getCapabilities().end(), capaTable(os));

	os << "</table></center></a>" << B2TOP << "<hr /><a name=\"dump\">"
	   << "<h2 align=\"center\">Server dump</h2><tt><pre>";

	NetMauMau::dump(os);

	os << "</pre></a></tt><hr />" << B2TOP << "</font></body></html>";

	MHD_Response *response = MHD_create_response_from_data(os.str().length(),
							 static_cast<void *>(const_cast<char *>(strdup(os.str().c_str()))),
							 true, false);

	const int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);

	MHD_destroy_response(response);

	return ret;
}

}

using namespace NetMauMau::Server;

HttpdPtr Httpd::m_instance;

Httpd::Httpd() : m_daemon(0L), m_source(0L), m_caps() {

	struct addrinfo *ai = NULL;

	if(NetMauMau::httpd && !NetMauMau::Common::AbstractSocketImpl::getAddrInfo(NetMauMau::host &&
			*NetMauMau::host ? NetMauMau::host : 0L, static_cast<uint16_t>(NetMauMau::hport), &ai,
			true) && ai && !(m_daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
										static_cast<unsigned short>(NetMauMau::hport), NULL, NULL,
										&answer_to_connection, this,
										MHD_OPTION_SOCK_ADDR, ai->ai_addr,
										MHD_OPTION_PER_IP_CONNECTION_LIMIT, 10,
										MHD_OPTION_CONNECTION_LIMIT, 10,
										MHD_OPTION_THREAD_POOL_SIZE, 5, MHD_OPTION_END))) {

		logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
				   << "Failed to start webserver at http://"
				   << (ai && ai->ai_canonname ? ai->ai_canonname : "localhost")
				   << ":" << NetMauMau::hport);

	} else if(NetMauMau::httpd) {
		logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Started webserver at http://"
				<< (ai && ai->ai_canonname ? ai->ai_canonname : "localhost")
				<< ":" << NetMauMau::hport);
	}

	freeaddrinfo(ai);
}

Httpd::~Httpd() {
	if(m_daemon) MHD_stop_daemon(m_daemon);
}

Httpd *Httpd::getInstance() {

	if(!m_instance) m_instance = HttpdPtr(new Httpd());

	return m_instance;
}

void Httpd::setSource(const Common::IObserver<Game>::source_type *s) {
	m_source = s;
}

void Httpd::update(what_type what) {

	switch(what) {
	case PLAYERADDED:
		logDebug("Received PLAYERADDED notification");
		break;

	case PLAYERREMOVED:
		logDebug("Received PLAYERREMOVED notification");
		break;

	case GAMESTARTED:
		logDebug("Received GAMESTARTED notification");
		break;

	case GAMEENDED:
		logDebug("Received GAMEENDED notification");
		break;
	}
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
