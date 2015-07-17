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
#include <cstdlib>
#include <sstream>
#include <iterator>

#include <stdint.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

#define MHD_PLATFORM_H 1

#include <microhttpd.h>

#include "httpd.h"
#include "logger.h"
#include "helpers.h"

#ifndef _WIN32
#define TIMEFORMAT "%T - "
#else
#define TIMEFORMAT "%H:%M:%S - "
#endif

namespace {

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct scoresTable : std::unary_function<NetMauMau::DB::SQLite::SCORES::value_type, void> {

	inline explicit scoresTable(std::ostringstream &o) : os(o) {}
	inline result_type operator()(const argument_type &s) const {
		os << "<tr><td>" << s.name << "</td><td>"
		   << (s.score < 0 ? "<font color=\"red\">" : "") << s.score
		   << (s.score < 0 ? "</font>" : "") << "</td></tr>";
	}

private:
	std::ostringstream &os;
};

struct capaTable :
		std::unary_function<NetMauMau::Common::AbstractConnection::CAPABILITIES::value_type, void> {

	inline explicit capaTable(std::ostringstream &o) : os(o) {}
	inline result_type operator()(const argument_type &p) const {
		os << "<tr><td>" << p.first << "</td><td>" << p.second << "</td></tr>";
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

	std::ostringstream os;

	os << "<html><head><title>" << PACKAGE_STRING << " (" << BUILD_TARGET << ")</title>";

	os << "<style>"
	   << "table, td, th { background-color:white; border: thin solid black; "
	   << "border-spacing: 0; border-collapse: collapse; }"
	   << "pre { background-color:white; }"
	   << "</style>";

	os << "<body bgcolor=\"#eeeeee\"><font face=\"Sans-Serif\"><h1 align=\"center\">"
	   << PACKAGE_STRING << "</h1><hr />";

	if(httpd->getCapabilities().find("HAVE_SCORES") != httpd->getCapabilities().end()) {
		const NetMauMau::DB::SQLite::SCORES
		&sc(NetMauMau::DB::SQLite::getInstance()->getScores(NetMauMau::DB::SQLite::NORM));

		os << "<center><h2>Hall of Fame</h2><table width=\"50%\">"
		   << "<tr><th>NAME</th><th>SCORE</th></tr>";

		std::for_each(sc.begin(), sc.end(), scoresTable(os));

		os << "</table></center><hr />";
	}

	os << "<center><h2>Server capabilities</h2><table width=\"50%\">"
	   << "<tr><th>NAME</th><th>VALUE</th></tr>";

	std::for_each(httpd->getCapabilities().begin(), httpd->getCapabilities().end(), capaTable(os));

	os << "</table></center><hr /><h2 align=\"center\">Server dump</h2><tt><pre>";

	NetMauMau::dump(os);

	os << "</pre></tt></font></body></html>";

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

Httpd::Httpd() : m_daemon(NetMauMau::httpd ? MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
							  static_cast<unsigned short>(NetMauMau::hport), NULL, NULL,
							  &answer_to_connection, this, MHD_OPTION_END) : 0L), m_source(0L),
	m_caps() {

	if(NetMauMau::httpd && !m_daemon) {
		logWarning(NetMauMau::Common::Logger::time(TIMEFORMAT)
				   << "Failed to start webserver at port " << NetMauMau::hport);
	} else if(NetMauMau::httpd) {
		logInfo(NetMauMau::Common::Logger::time(TIMEFORMAT)
				<< "Started webserver at port " << NetMauMau::hport);
	}
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
