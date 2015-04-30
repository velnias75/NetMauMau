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

#include "defaulteventhandler.h"

#include "nullconnection.h"

using namespace NetMauMau::Event;

DefaultEventHandler::DefaultEventHandler() : IEventHandler() {}

DefaultEventHandler::~DefaultEventHandler() {}

NetMauMau::Common::IConnection &DefaultEventHandler::getConnection() const {
	return NetMauMau::NullConnection::getInstance();
}

void DefaultEventHandler::gameAboutToStart() const {}

void DefaultEventHandler::gameOver() const throw(NetMauMau::Common::Exception::SocketException) {}

bool DefaultEventHandler::shutdown() const throw() {
	return false;
}

void DefaultEventHandler::reset() throw() {}

void DefaultEventHandler::message(const std::string &, const std::vector<std::string> &) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::error(const std::string &, const std::vector<std::string> &) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::directionChange() const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::acceptingPlayers() const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::stats(const NetMauMau::Engine::PLAYERS &) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerAdded(const NetMauMau::Player::IPlayer *) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerRejected(const NetMauMau::Player::IPlayer *) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::cardsDistributed(const NetMauMau::Player::IPlayer *, const CARDS &) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::initialCard(const NetMauMau::Common::ICard *) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::uncoveredCard(const NetMauMau::Common::ICard *) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::talonEmpty(bool) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::cardsAlreadyDistributed() const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::turn(std::size_t) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerPicksCard(const NetMauMau::Player::IPlayer *,
		const NetMauMau::Common::ICard *) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerPicksCards(const NetMauMau::Player::IPlayer *,
		std::size_t) const throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerSuspends(const NetMauMau::Player::IPlayer *,
		const NetMauMau::Common::ICard *) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerPlaysCard(const NetMauMau::Player::IPlayer *,
		const NetMauMau::Common::ICard *, const NetMauMau::Common::ICard *) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::cardRejected(NetMauMau::Player::IPlayer *,
									   const NetMauMau::Common::ICard *,
									   const NetMauMau::Common::ICard *) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerChooseJackSuit(const NetMauMau::Player::IPlayer *,
		NetMauMau::Common::ICard::SUIT) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerWins(const NetMauMau::Player::IPlayer *, std::size_t, bool) const
throw(NetMauMau::Common::Exception::SocketException) {}

std::size_t DefaultEventHandler::playerLost(const NetMauMau::Player::IPlayer *, std::size_t,
		std::size_t) const throw(NetMauMau::Common::Exception::SocketException) {
	return 0;
}

void DefaultEventHandler::nextPlayer(const NetMauMau::Player::IPlayer *) const
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::aceRoundStarted(const NetMauMau::Player::IPlayer *)
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::aceRoundEnded(const NetMauMau::Player::IPlayer *)
throw(NetMauMau::Common::Exception::SocketException) {}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
