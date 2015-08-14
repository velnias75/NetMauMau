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

#include <cstdio>
#include <cassert>
#include <iomanip>
#include <iostream>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "testeventhandler.h"

#include "iplayer.h"
#include "cardtools.h"

namespace {
#ifndef DISABLE_ANSI
#define OFF (m_isatty ? 10 : 0)
const std::string BLUE_ON("\x1B[1m\x1B[34m");
const std::string BLUE_OFF("\x1B[39m\x1B[22m");
const std::string BOLD_P_ON("Player \x1B[1m");
const std::string BOLD_ON("\x1B[1m");
const std::string BOLD_OFF("\x1B[22m");
#else
#define OFF 0
const std::string BLUE_ON;
const std::string BLUE_OFF;
const std::string BOLD_P_ON("Player ");
const std::string BOLD_ON;
const std::string BOLD_OFF;
#endif
}

#define PLAYER (m_isatty ? BOLD_P_ON : "Player ") << std::setw(8) << std::left \
		<< player->getName() << (m_isatty ? BOLD_OFF : "")

#define SETWIDTH std::setw(29)

TestEventHandler::TestEventHandler() : DefaultEventHandler(), m_isatty(isatty(fileno(stderr))) {}

TestEventHandler::~TestEventHandler() {}

void TestEventHandler::playerAdded(const NetMauMau::Player::IPlayer *player) const
throw(NetMauMau::Common::Exception::SocketException) {
	std::cerr << "Added player \"" << player->getName() << "\"" << std::endl;
}

void TestEventHandler::cardsDistributed(const NetMauMau::Player::IPlayer *player,
										const CARDS &cards) const
throw(NetMauMau::Common::Exception::SocketException) {

	std::cerr << "Distributing cards to player \"" << player->getName() << "\"" << std::endl;

	CARDS::const_iterator i(cards.begin());
	const CARDS::const_iterator &e(cards.end());

	for(; i != e; ++i) {
		std::cerr << "\tgets " << (*i)->description(m_isatty) << std::endl;
	}
}

void TestEventHandler::initialCard(const NetMauMau::Common::ICard *ic) const
throw(NetMauMau::Common::Exception::SocketException) {
	std::cerr << "Uncovered card: " << ic->description(m_isatty) << std::endl;
}

void TestEventHandler::cardsAlreadyDistributed() const
throw(NetMauMau::Common::Exception::SocketException) {
	std::cerr << "NOT distributing cards again" << std::endl;
}

void TestEventHandler::playerPicksCard(const NetMauMau::Player::IPlayer *player,
									   const NetMauMau::Common::ICard *) const
throw(NetMauMau::Common::Exception::SocketException) {

	std::cerr << PLAYER << std::setw(30) << "takes a card from talon!" << "("
			  << player->getCardCount() << ")" << std::endl;
}

void TestEventHandler::playerPicksCards(const NetMauMau::Player::IPlayer *player,
										std::size_t cardCount) const
throw(NetMauMau::Common::Exception::SocketException) {

	std::cerr << PLAYER << "must take " << cardCount << std::setw(19) << " cards!" << "("
			  << player->getCardCount() << ")" << std::endl;
}

void TestEventHandler::playerSuspends(const NetMauMau::Player::IPlayer *player,
									  const NetMauMau::Common::ICard *dueCard) const
throw(NetMauMau::Common::Exception::SocketException) {

	if(!dueCard) {
		std::cerr << PLAYER << SETWIDTH << "suspends this turn!" << " ("
				  << player->getCardCount() << ")" << std::endl;
	} else {
		std::cerr << PLAYER << "suspends this turn due to " << std::setw(6 + OFF)
				  << dueCard->description(m_isatty) << "(" << player->getCardCount() << ")"
				  << std::endl;
	}
}

void TestEventHandler::playerPlaysCard(const NetMauMau::Player::IPlayer *player,
									   const NetMauMau::Common::ICard *playedCard,
									   const NetMauMau::Common::ICard *uc) const
throw(NetMauMau::Common::Exception::SocketException) {

	std::cerr << PLAYER << "plays " << std::setw(6 + OFF) << playedCard->description(m_isatty)
			  << " over " << std::setw(16 + OFF) << uc->description(m_isatty) << "("
			  << player->getCardCount() << ")" << std::endl;
}

void TestEventHandler::playerChooseJackSuit(const NetMauMau::Player::IPlayer *player,
		NetMauMau::Common::ICard::SUIT suit) const
throw(NetMauMau::Common::Exception::SocketException) {

	assert(suit != NetMauMau::Common::ICard::SUIT_ILLEGAL);

	std::cerr << PLAYER << "has chosen "
			  << NetMauMau::Common::suitToSymbol(suit, m_isatty, m_isatty) << std::endl;
}

void TestEventHandler::playerWins(const NetMauMau::Player::IPlayer *player, std::size_t t,
								  bool) const throw(NetMauMau::Common::Exception::SocketException) {

	std::cerr << PLAYER << (m_isatty ? BLUE_ON : "") << "wins in turn #" << t << " :-)"
			  << (m_isatty ? BLUE_OFF : "") << std::endl;
}

std::size_t TestEventHandler::playerLost(const NetMauMau::Player::IPlayer *player, std::size_t,
		std::size_t pointFactor) const
throw(NetMauMau::Common::Exception::SocketException) {

	std::cerr << PLAYER << (m_isatty ? BOLD_ON : "") << "has lost with "
			  << (player->getPoints() * pointFactor) << " points in hand :-("
			  << (m_isatty ? BOLD_OFF : "") << std::endl;

	return player->getPoints() * pointFactor;
}

void TestEventHandler::setJackModeOff() const
throw(NetMauMau::Common::Exception::SocketException) {}

void TestEventHandler::aceRoundStarted(const NetMauMau::Player::IPlayer *player)
throw(NetMauMau::Common::Exception::SocketException) {
	std::cerr << PLAYER << (m_isatty ? BOLD_ON : "") << "started" << (m_isatty ? BOLD_OFF : "")
			  << " an " << (m_isatty ? BLUE_ON : "") << "Ace round" << (m_isatty ? BLUE_OFF : "")
			  << std::endl;
}

void TestEventHandler::aceRoundEnded(const NetMauMau::Player::IPlayer *player)
throw(NetMauMau::Common::Exception::SocketException) {
	std::cerr << PLAYER << (m_isatty ? BOLD_ON : "") << "ended" << (m_isatty ? BOLD_OFF : "")
			  << " an " << (m_isatty ? BLUE_ON : "") << "Ace round" << (m_isatty ? BLUE_OFF : "")
			  << std::endl;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
