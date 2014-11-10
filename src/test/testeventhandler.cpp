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

#include "testeventhandler.h"

#include "iplayer.h"
#include "cardtools.h"

namespace {
#ifndef DISABLE_ANSI
const std::string BLUE_ON("\x1B[1m\x1B[34m");
const std::string BLUE_OFF("\x1B[39m\x1B[22m");
const std::string BOLD_P_ON("Player \x1B[1m");
const std::string BOLD_ON("\x1B[1m");
const std::string BOLD_OFF("\x1B[22m");
#else
const std::string BLUE_ON;
const std::string BLUE_OFF;
const std::string BOLD_P_ON("Player ");
const std::string BOLD_ON;
const std::string BOLD_OFF;
#endif
}

TestEventHandler::TestEventHandler() : DefaultEventHandler() {}

TestEventHandler::~TestEventHandler() {}

void TestEventHandler::playerAdded(const NetMauMau::Player::IPlayer *player)
throw(NetMauMau::Common::Exception::SocketException) {
	std::cerr << "Added player \"" << player->getName() << "\"" << std::endl;
}

void TestEventHandler::cardsDistributed(const NetMauMau::Player::IPlayer *player,
										const std::vector<NetMauMau::Common::ICard *> &cards)
throw(NetMauMau::Common::Exception::SocketException) {

	std::cerr << "Distributing cards to player \"" << player->getName() << "\"" << std::endl;

	std::vector<NetMauMau::Common::ICard *>::const_iterator i(cards.begin());
	const std::vector<NetMauMau::Common::ICard *>::const_iterator &e(cards.end());

	for(; i != e; ++i) {
		std::cerr << "\tgets " << (*i)->description(true) << std::endl;
	}
}

void TestEventHandler::initialCard(const NetMauMau::Common::ICard *ic)
throw(NetMauMau::Common::Exception::SocketException) {
	std::cerr << "Uncovered card: " << ic->description(true) << std::endl;
}

void TestEventHandler::cardsAlreadyDistributed()
throw(NetMauMau::Common::Exception::SocketException) {
	std::cerr << "NOT distributing cards again" << std::endl;
}

void TestEventHandler::playerPicksCard(const NetMauMau::Player::IPlayer *player,
									   const NetMauMau::Common::ICard */*card*/)
throw(NetMauMau::Common::Exception::SocketException) {

	std::cerr << BOLD_P_ON << player->getName() << BOLD_OFF << "\ttakes a card from talon!\t("
			  << player->getCardCount() << ")" << std::endl;
}

void TestEventHandler::playerPicksCards(const NetMauMau::Player::IPlayer *player,
										std::size_t cardCount)
throw(NetMauMau::Common::Exception::SocketException) {

	std::cerr << BOLD_P_ON << player->getName() << BOLD_OFF << "\tmust take " << cardCount
			  << " cards!\t\t(" << player->getCardCount() << ")" << std::endl;
}

void TestEventHandler::playerSuspends(const NetMauMau::Player::IPlayer *player,
									  const NetMauMau::Common::ICard *dueCard)
throw(NetMauMau::Common::Exception::SocketException) {

	if(!dueCard) {
		std::cerr << BOLD_P_ON << player->getName() << BOLD_OFF << "\tsuspends this turn!\t\t("
				  << player->getCardCount() << ")" << std::endl;
	} else {
		std::cerr << BOLD_P_ON << player->getName() << BOLD_OFF << "\tsuspends this turn due to "
				  << dueCard->description(true)  << "\t(" << player->getCardCount() << ")"
				  << std::endl;
	}
}

void TestEventHandler::playerPlaysCard(const NetMauMau::Player::IPlayer *player,
									   const NetMauMau::Common::ICard *playedCard,
									   const NetMauMau::Common::ICard *unvoredCard)
throw(NetMauMau::Common::Exception::SocketException) {

	std::cerr << BOLD_P_ON << player->getName() << BOLD_OFF << "\tplays "
			  << playedCard->description(true) << " over " << unvoredCard->description(true)
			  << "\t\t(" << player->getCardCount() << ")" << std::endl;
}

void TestEventHandler::playerChooseJackSuit(const NetMauMau::Player::IPlayer *player,
		NetMauMau::Common::ICard::SUIT suit)
throw(NetMauMau::Common::Exception::SocketException) {

	std::cerr << BOLD_P_ON << player->getName() << BOLD_OFF << "\thas chosen "
			  << NetMauMau::Common::suitToSymbol(suit, true, true) << std::endl;
}

void TestEventHandler::playerWins(const NetMauMau::Player::IPlayer *player, std::size_t t, bool)
throw(NetMauMau::Common::Exception::SocketException) {

	std::cerr << BOLD_P_ON << player->getName() << BOLD_OFF << "\t" << BLUE_ON << "wins in turn #"
			  << t << " :-)" << BLUE_OFF << std::endl;
}

void TestEventHandler::playerLost(const NetMauMau::Player::IPlayer *player, std::size_t)
throw(NetMauMau::Common::Exception::SocketException) {

	std::cerr << BOLD_P_ON << player->getName() << BOLD_OFF << "\t" << BOLD_ON << "has lost with "
			  << player->getPoints() << " points in hand :-(" << BOLD_OFF << std::endl;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
