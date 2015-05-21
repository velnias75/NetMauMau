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

#include <popt.h>                       // for POPT_ARG_VAL, poptBadOption, etc
#include <cstdlib>                      // for NULL, EXIT_FAILURE, etc
#include <iomanip>                      // for operator<<, setw
#include <iostream>                     // for basic_ostream, operator<<, etc
#include <sstream>                      // IWYU pragma: keep
#include <stdbool.h>

#include "testclient.h"                 // for TestClient
#include "testimg.h"                    // for test_client_img

namespace {
#ifndef DISABLE_ANSI
const std::string RED_ON("\x1B[31m");
const std::string RED_OFF("\x1B[39m");
const std::string BOLD_ON("\x1B[1m");
const std::string BOLD_OFF("\x1B[22m");
#else
const std::string RED_ON;
const std::string RED_OFF;
const std::string BOLD_ON;
const std::string BOLD_OFF;
#endif

bool noImg = true;

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
	{ "player-image", 'i', POPT_ARG_VAL, &noImg, 0, "Send a test player image", NULL },
	{ "caps", 'c', POPT_ARG_VAL, &showCaps, 1, "Display the server capabilities", NULL },
	POPT_AUTOHELP
	POPT_TABLEEND
};

}

using namespace NetMauMau;

int main(int argc, const char **argv) {

	poptContext pctx = poptGetContext(NULL, argc, argv, poptOptions, 0);
	int c;

	poptSetOtherOptionHelp(pctx, "[OPTIONS]* [<server[:port]>]");

	while((c = poptGetNextOpt(pctx)) >= 0);

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
					  sizeof(test_client_img));

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
			client.play(&tv);

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
		std::cerr << RED_ON << BOLD_ON << "ERROR: " << e.what() << BOLD_OFF << RED_OFF
				  << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
