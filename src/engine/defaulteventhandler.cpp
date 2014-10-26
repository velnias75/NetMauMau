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

#include "defaulteventhandler.h"

using namespace NetMauMau::Event;

DefaultEventHandler::DefaultEventHandler() : IEventHandler() {}

DefaultEventHandler::~DefaultEventHandler() {}

NetMauMau::Common::AbstractConnection *DefaultEventHandler::getConnection() const {
	return 0L;
}

bool DefaultEventHandler::shutdown() const {
	return false;
}

void DefaultEventHandler::reset() {}

void DefaultEventHandler::message(const std::string &, const std::vector<std::string> &)
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::error(const std::string &, const std::vector<std::string> &)
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::acceptingPlayers() throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::stats(const NetMauMau::Engine::PLAYERS &)
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerAdded(const NetMauMau::Player::IPlayer *)
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerRejected(const NetMauMau::Player::IPlayer *)
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::cardsDistributed(const NetMauMau::Player::IPlayer *,
		const std::vector<NetMauMau::Common::ICard *> &)
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::initialCard(const NetMauMau::Common::ICard *)
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::cardsAlreadyDistributed()
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::turn(std::size_t) throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerPicksCard(const NetMauMau::Player::IPlayer *,
		const NetMauMau::Common::ICard *) throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerPicksCards(const NetMauMau::Player::IPlayer *,
		std::size_t) throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerSuspends(const NetMauMau::Player::IPlayer *,
		const NetMauMau::Common::ICard *) throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerPlaysCard(const NetMauMau::Player::IPlayer *,
		const NetMauMau::Common::ICard *,
		const NetMauMau::Common::ICard *) throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::cardRejected(NetMauMau::Player::IPlayer *,
									   const NetMauMau::Common::ICard *,
									   const NetMauMau::Common::ICard *)
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerChooseJackSuit(const NetMauMau::Player::IPlayer *,
		NetMauMau::Common::ICard::SUIT) throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerWins(const NetMauMau::Player::IPlayer *, std::size_t)
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::playerLost(const NetMauMau::Player::IPlayer *, std::size_t)
throw(NetMauMau::Common::Exception::SocketException) {}

void DefaultEventHandler::nextPlayer(const NetMauMau::Player::IPlayer *)
throw(NetMauMau::Common::Exception::SocketException) {}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
