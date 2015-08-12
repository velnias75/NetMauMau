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

#include <popt.h>                       // for POPT_ARG_VAL, poptBadOption, etc
#include <cstdlib>                      // for NULL, EXIT_FAILURE, etc
#include <iomanip>                      // for operator<<, setw
#include <iostream>                     // for basic_ostream, operator<<, etc
#include <sstream>                      // IWYU pragma: keep
#include <ctime>
#include <stdbool.h>

#include "testclient.h"                 // for TestClient
#include "testimg.h"                    // for test_client_img
#include "logger.h"

namespace {
bool noImg = true;
bool autoPlay = false;

int delay = 1;

#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic push
char *pName = "TestClient";
#pragma GCC diagnostic pop

bool showCaps = false;

poptOption poptOptions[] = {
	{
		"name", 'n', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &pName,
		0, "Set the name of the player", "NAME"
	},
	{ "player-image", 'i', POPT_ARG_NONE, NULL, 'i', "Send a test player image", NULL },
	{
		"autoplay", 'a', POPT_ARG_NONE, NULL,
		'A', "Automatically plays the first possible choice", NULL
	},
#ifdef HAVE_UNISTD_H
	{
		"delay", 'D', POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &delay,
		0, "In auoplay mode delay before turn", "SECONDS"
	},
#endif
	{ "caps", 'c', POPT_ARG_NONE, NULL, 'c', "Display the server capabilities", NULL },
	POPT_AUTOHELP
	POPT_TABLEEND
};

}

using namespace NetMauMau;

int main(int argc, const char **argv) {

	poptContext pctx = poptGetContext(NULL, argc, argv, poptOptions, 0);
	int c;

	poptSetOtherOptionHelp(pctx, "[OPTIONS]* [<server[:port]>]");

	while((c = poptGetNextOpt(pctx)) >= 0) {

		switch(c) {
		case 'A':
			autoPlay = true;
			break;

		case 'i':
			noImg = false;
			break;

		case 'c':
			showCaps = true;
			break;
		}
	}

	const char *lo = poptGetArg(pctx);
	std::string server(lo ? lo : "localhost");

	if(c < -1) {
		std::cerr << poptBadOption(pctx, POPT_BADOPTION_NOALIAS) << ": " << poptStrerror(c)
				  << std::endl;;
		return EXIT_FAILURE;
	}

	poptFreeContext(pctx);

	uint16_t port = TestClient::getDefaultPort();
	const std::string::size_type p = server.find(':');

	if(p != std::string::npos) {
		(std::istringstream(server.substr(p + 1))) >> port;
		server = server.substr(0, p);

		if(server.empty()) server = "localhost";
	}

	TestClient client(pName, server, port, noImg ? 0L : test_client_img, noImg ? 0 :
					  sizeof(test_client_img), autoPlay, delay);

	try {
		struct timeval tv = { 120, 0 };

		if(showCaps) {
			const Client::Connection::CAPABILITIES &caps(client.capabilities(&tv));

			std::cout << "Server-caps:" << std::endl;

			for(Client::Connection::CAPABILITIES::const_iterator i(caps.begin()); i != caps.end();
					++i) {
				std::cout << i->first << '=' << i->second << std::endl;
			}

		} else {

			const std::time_t stime = std::time(0L);

			client.play(&tv);

			const std::time_t etime = std::time(0L) - stime;

			std::cout << std::setfill('0') << std::endl << "Playing time: " << std::setw(2)
					  << (etime / 60) << ":" << std::setw(2) << (etime % 60) << std::endl
					  << std::setfill(' ');

			const Client::Connection::SCORES &scores(client.getScores());

			if(!scores.empty()) {

				std::cout << std::endl << "Top ten hall of fame:" << std::endl;

				int j = 1;

				for(Client::Connection::SCORES::const_iterator i(scores.begin()); i != scores.end();
						++i, ++j) {

					std::cout << std::setw(2) << j << std::setw(0) << ") " << BOLD_ON
							  << std::setw(25) << i->name << std::setw(0) << ": " << BOLD_OFF
							  << i->score << std::endl;
				}
			}
		}

	} catch(const Common::Exception::SocketException &e) {
		logger(RED_ON << BOLD_ON << "ERROR: " << typeid(e) << ": " << e.what() << BOLD_OFF
			   << RED_OFF);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
