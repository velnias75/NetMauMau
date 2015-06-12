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

#include "nullconnection.h"

using namespace NetMauMau;

NullConnection::NullConnection() : Common::IConnection() {}

NullConnection::~NullConnection() {}

NullConnection &NullConnection::getInstance() {
	static NullConnection instance;
	return instance;
}

void NullConnection::addAIPlayers(const PLAYERNAMES &) {}

Common::IConnection::NAMESOCKFD NullConnection::getPlayerInfo(SOCKET) const {
	return Common::IConnection::NAMESOCKFD();
}

std::string NullConnection::getPlayerName(SOCKET) const {
	return std::string();
}

bool NullConnection::hasHumanPlayers() const {
	return false;
}

void NullConnection::removePlayer(SOCKET) {}

void NullConnection::wait(long) throw(Common::Exception::SocketException) {}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
