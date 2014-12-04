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

#include <iostream>

#include "testclient.h"

#include "cardtools.h"

namespace {
#ifndef DISABLE_ANSI
const std::string RED_ON("\x1B[31m");
const std::string RED_OFF("\x1B[39m");
const std::string BLUE_ON("\x1B[1m\x1B[34m");
const std::string BLUE_OFF("\x1B[39m\x1B[22m");
const std::string BOLD_ON("\x1B[1m");
const std::string BOLD_P_ON("Player \x1B[1m");
const std::string BOLD_OFF("\x1B[22m");
#else
const std::string RED_ON;
const std::string RED_OFF;
const std::string BLUE_ON;
const std::string BLUE_OFF;
const std::string BOLD_ON;
const std::string BOLD_P_ON("Player ");
const std::string BOLD_OFF;
#endif
}

TestClient::TestClient(const std::string &pName, const std::string &server, uint16_t port)
	: AbstractClient(pName, server, port) {}

TestClient::~TestClient() {}

void TestClient::message(const std::string &msg) const {
	std::cout << msg << std::endl;
}

void TestClient::error(const std::string &msg) const {
	message(BOLD_ON + RED_ON + "ERROR: " + msg + RED_OFF + BOLD_OFF);
}

void TestClient::unknownServerMessage(std::string msg) const {
	message(std::string("Unknown server message: \"") + msg + "\"");
}

void TestClient::gameOver() const {
	std::cout << "Game over!" << std::endl;
}

void TestClient::turn(std::size_t t) const {
	std::cout << "Turn #" << t << std::endl;
}

void TestClient::stats(const STATS &s) const {

	bool mau = false;

	for(STATS::const_iterator i(s.begin()); i != s.end(); ++i) {
		mau = i->cardCount == 1;
		std::cout << BOLD_P_ON << i->playerName << BOLD_OFF << " has " <<
				  (mau ? BOLD_ON : "") << i->cardCount << (mau ? BOLD_OFF : "")
				  << " cards in hand" << std::endl;
	}

	if(mau) std::cout << "\a";
}

NetMauMau::Common::ICard *TestClient::playCard(const CARDS &cards) const {

	std::size_t pos;

	do {

		pos = 0;

		std::cout << "Choose card:" << std::endl;
		std::cout << "0) Suspend turn" << std::endl;

		for(CARDS::const_iterator i(cards.begin()); i != cards.end(); ++i, ++pos) {
			std::cout << (pos + 1) << ") " << (*i)->description(true) << std::endl;
		}

		std::cout << "Choose: ";
		std::cin >> pos;

	} while(pos > cards.size());

	if(pos) return cards.at(pos - 1);

	return 0L;
}

NetMauMau::Common::ICard::SUIT TestClient::getJackSuitChoice() const {

	std::size_t pos;

	do {

		pos = 0;

		std::cout << "Choose suit:" << std::endl;

		for(std::size_t i = 0; i < 4; ++i, ++pos) {
			std::cout << (pos + 1) << ") "
					  << NetMauMau::Common::ansiSuit(NetMauMau::Common::getSuitSymbols()[i])
					  << std::endl;
		}

		std::cout << "Choose: ";
		std::cin >> pos;

	} while(pos < 1 || pos > 4);

	return NetMauMau::Common::symbolToSuit(NetMauMau::Common::getSuitSymbols()[pos - 1]);
}

void TestClient::playerJoined(const std::string &player, const unsigned char *,
							  std::size_t) const {
	std::cout << BOLD_P_ON << player << BOLD_OFF << " joins the game" << std::endl;
}

void TestClient::playerRejected(const std::string &) const {
	std::cout << BOLD_ON << "You" << BOLD_OFF << " are refused to join the game" << std::endl;
}

void TestClient::playerSuspends(const std::string &player) const {
	std::cout << BOLD_P_ON << player << BOLD_OFF << " suspends" << std::endl;
}

void TestClient::playedCard(const std::string &player, const NetMauMau::Common::ICard *card) const {
	std::cout << BOLD_P_ON << player << BOLD_OFF << " plays card: " << card->description(true)
			  << std::endl;
}

void TestClient::playerWins(const std::string &player, std::size_t t) const {
	std::cout << BOLD_P_ON << player << BOLD_OFF << BLUE_ON << " wins in turn #"
			  << t << " :-)" << BLUE_OFF << std::endl;
}

void TestClient::playerLost(const std::string &, std::size_t, std::size_t) const {}

void TestClient::cardSet(const CARDS &cards) const {
	for(CARDS::const_iterator i(cards.begin()); i != cards.end(); ++i) {
		std::cout << BOLD_P_ON << getPlayerName() << BOLD_OFF << " gets card: "
				  << (*i)->description(true) << std::endl;
	}
}

void TestClient::enableSuspend(bool) const {}

void TestClient::initialCard(const NetMauMau::Common::ICard *card) const {
	std::cout << "Initial card is " << card->description(true) << std::endl;
}

void TestClient::openCard(const NetMauMau::Common::ICard *card, const std::string &js) const {

	std::cout << "Open card: " << card->description(true);

	if(!js.empty()) {
		std::cout << " (current Jack suit " << NetMauMau::Common::ansiSuit(js) << ")";
	}

	std::cout << std::endl;
}

void TestClient::talonShuffled() const {}

void TestClient::cardRejected(const std::string &player,
							  const NetMauMau::Common::ICard *card) const {
	std::cout << BOLD_P_ON << player << BOLD_OFF << "'s card " << card->description(true)
			  << " is REJECTED" << std::endl;
}

void TestClient::cardAccepted(const NetMauMau::Common::ICard *) const {}

void TestClient::jackSuit(NetMauMau::Common::ICard::SUIT suit) const {
	std::cout << "Suit chosen by Jack is " << NetMauMau::Common::suitToSymbol(suit, true, true)
			  << std::endl;
}

void TestClient::playerPicksCard(const std::string &player,
								 const NetMauMau::Common::ICard *card) const {

	std::cout << BOLD_P_ON << player << BOLD_OFF << " picks up a card";

	if(card) std::cout << ": " << card->description(true);

	std::cout << std::endl;
}

void TestClient::playerPicksCard(const std::string &player, std::size_t count) const {
	std::cout << BOLD_P_ON << player << BOLD_OFF << " picks up " << count << " cards"
			  << std::endl;
}

void TestClient::nextPlayer(const std::string &player) const {
	std::cout << "Waiting for turn of player " << BOLD_ON << player << BOLD_OFF << std::endl;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
