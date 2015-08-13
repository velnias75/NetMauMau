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

#include "servereventhandler.h"

#include <cstdio>                       // for snprintf
#include <stdbool.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "logger.h"                     // for BasicLogger, logError
#include "iplayer.h"                    // for IPlayer
#include "eff_map.h"
#include "protocol.h"                   // for JACKSUIT, MESSAGE, etc

using namespace NetMauMau::Server;

bool EventHandler::m_interrupt = false;

EventHandler::EventHandler(Connection &con) : DefaultEventHandler(), m_connection(con),
	m_lastMsg() {}

EventHandler::~EventHandler() {}

Connection &EventHandler::getConnection() const {
	return m_connection;
}

void EventHandler::gameAboutToStart() const {
	m_connection.clearPlayerPictures();
}

void EventHandler::gameOver() const throw(NetMauMau::Common::Exception::SocketException) {
	m_connection << NetMauMau::Common::Protocol::V15::BYE;
}

bool EventHandler::shutdown() const throw() {
	return m_interrupt;
}

void EventHandler::setInterrupted() {
	m_interrupt = true;
	Connection::setInterrupted();
}

void EventHandler::reset() throw() {
	m_lastMsg.clear();
	m_connection.reset();
	m_interrupt = false;
}

void EventHandler::message_internal(const std::string &type, const std::string &msg,
									const EXCEPTIONS &except) const
throw(NetMauMau::Common::Exception::SocketException) {

	for(Connection::PLAYERINFOS::const_iterator i(m_connection.getPlayers().begin());
			i != m_connection.getPlayers().end(); ++i) {

		if(std::find(except.begin(), except.end(), i->name) == except.end()) {
			try {
				m_connection.write(i->sockfd, type);
				m_connection.write(i->sockfd, msg);
			} catch(const NetMauMau::Common::Exception::SocketException &) {
				logError(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Couldn't send \""
						 << type << "\" to \"" << i->name << "\"");
			}
		}
	}
}

void EventHandler::message(const std::string &msg, const EXCEPTIONS &except) const
throw(NetMauMau::Common::Exception::SocketException) {
	if(msg != m_lastMsg) {
		message_internal(NetMauMau::Common::Protocol::V15::MESSAGE, m_lastMsg = msg, except);
	}
}

void EventHandler::error(const std::string &msg, const std::vector<std::string> &except) const
throw(NetMauMau::Common::Exception::SocketException) {
	message_internal(NetMauMau::Common::Protocol::V15::ERROR, msg, except);
}

void EventHandler::directionChange() const throw(NetMauMau::Common::Exception::SocketException) {

	Connection::VERSIONEDMESSAGE versionedMessage;

	std::ostringstream vm_old;
	vm_old << NetMauMau::Common::Protocol::V15::MESSAGE << '\0' << "Direction has changed";

	NetMauMau::Common::efficientAddOrUpdate(versionedMessage, 0u, vm_old.str());
	NetMauMau::Common::efficientAddOrUpdate(versionedMessage, 13u,
											NetMauMau::Common::Protocol::V15::DIRCHANGE);

	m_connection.sendVersionedMessage(versionedMessage);
}

void EventHandler::playerAdded(const NetMauMau::Player::IPlayer *player) const
throw(NetMauMau::Common::Exception::SocketException) {

	Connection::VERSIONEDMESSAGE versionedMessage;
	std::ostringstream vm_old, vm_new;

	vm_old << NetMauMau::Common::Protocol::V15::PLAYERJOINED << '\0' << player->getName();
	vm_new << vm_old.str() << '\0' << NetMauMau::Common::Protocol::V15::VM_ADDPIC;

	NetMauMau::Common::efficientAddOrUpdate(versionedMessage, 0u, vm_old.str());
	NetMauMau::Common::efficientAddOrUpdate(versionedMessage, 4u, vm_new.str());

	m_connection.sendVersionedMessage(versionedMessage);
}

void EventHandler::playerRejected(const NetMauMau::Player::IPlayer *player) const
throw(NetMauMau::Common::Exception::SocketException) {
	if(player->getSerial() != -1) {
		m_connection.write(player->getSerial(), NetMauMau::Common::Protocol::V15::PLAYERREJECTED);
		m_connection.write(player->getSerial(), player->getName());
	}
}

void EventHandler::initialCard(const NetMauMau::Common::ICard *ic) const
throw(NetMauMau::Common::Exception::SocketException) {
	m_connection << NetMauMau::Common::Protocol::V15::INITIALCARD << ic->description();
}

void EventHandler::uncoveredCard(const NetMauMau::Common::ICard *uc) const
throw(NetMauMau::Common::Exception::SocketException) {
	m_connection << NetMauMau::Common::Protocol::V15::OPENCARD;
	m_connection << uc->description();
}

void EventHandler::talonEmpty(bool empty) const
throw(NetMauMau::Common::Exception::SocketException) {
	m_connection << std::string(NetMauMau::Common::Protocol::V15::SUSPEND)
				 .append(1, ' ').append(empty ?
										NetMauMau::Common::Protocol::V15::OFF :
										NetMauMau::Common::Protocol::V15::ON);
}

void EventHandler::turn(std::size_t t) const throw(NetMauMau::Common::Exception::SocketException) {

	char cc[256];
#ifndef _WIN32
	std::snprintf(cc, 255, "%zu", t);
#else
	std::snprintf(cc, 255, "%lu", (unsigned long)t);
#endif

	m_connection << NetMauMau::Common::Protocol::V15::TURN << cc;
}

void EventHandler::stats(const NetMauMau::Engine::PLAYERS &m_players) const
throw(NetMauMau::Common::Exception::SocketException) {

	for(Connection::PLAYERINFOS::const_iterator i(m_connection.getPlayers().begin());
			i != m_connection.getPlayers().end(); ++i) {

		try {
			m_connection.write(i->sockfd, NetMauMau::Common::Protocol::V15::STATS);
		} catch(const NetMauMau::Common::Exception::SocketException &) {
			logError(NetMauMau::Common::Logger::time(TIMEFORMAT) << "Could send stats to \""
					 << i->name << "\"");
			break;
		}

		for(NetMauMau::Engine::PLAYERS::const_iterator j(m_players.begin()); j != m_players.end();
				++j) {

			const NetMauMau::Engine::PLAYERS::value_type p = *j;

			if(p->getName() != i->name) {

				char cc[256];
#ifndef _WIN32
				std::snprintf(cc, 255, "%zu", p->getCardCount());
#else
				std::snprintf(cc, 255, "%lu", (unsigned long)p->getCardCount());
#endif

				try {
					m_connection.write(i->sockfd, p->getName());
					m_connection.write(i->sockfd, cc);
				} catch(const NetMauMau::Common::Exception::SocketException &) {
					try {
						m_connection.write(i->sockfd, "0");
					} catch(const NetMauMau::Common::Exception::SocketException &) {}
				}
			}
		}

		m_connection.write(i->sockfd, NetMauMau::Common::Protocol::V15::ENDSTATS);
	}
}

void EventHandler::playerWins(const NetMauMau::Player::IPlayer *player, std::size_t,
							  bool ultimate) const
throw(NetMauMau::Common::Exception::SocketException) {

	std::string pw(NetMauMau::Common::Protocol::V15::PLAYERWINS);

	if(ultimate) pw.append(1, '+');

	m_connection << pw << player->getName();

}

std::size_t EventHandler::playerLost(const NetMauMau::Player::IPlayer *player, std::size_t,
									 std::size_t pointFactor) const
throw(NetMauMau::Common::Exception::SocketException) {

	Connection::VERSIONEDMESSAGE versionedMessage;

	std::ostringstream vm_old, vm_new;
	vm_old << NetMauMau::Common::Protocol::V15::PLAYERLOST << '\0' << player->getName();
	vm_new << vm_old.str() << '\0' << (player->getPoints() * pointFactor);

	NetMauMau::Common::efficientAddOrUpdate(versionedMessage, 0u, vm_old.str());
	NetMauMau::Common::efficientAddOrUpdate(versionedMessage, 3u, vm_new.str());

	m_connection.sendVersionedMessage(versionedMessage);

	return player->getPoints() * pointFactor;
}

void EventHandler::playerPlaysCard(const NetMauMau::Player::IPlayer *player,
								   const NetMauMau::Common::ICard *playedCard,
								   const NetMauMau::Common::ICard *) const
throw(NetMauMau::Common::Exception::SocketException) {
	m_connection << NetMauMau::Common::Protocol::V15::PLAYEDCARD << player->getName()
				 << playedCard->description();
}

void EventHandler::cardRejected(NetMauMau::Player::IPlayer *player,
								const NetMauMau::Common::ICard *,
								const NetMauMau::Common::ICard *playedCard) const
throw(NetMauMau::Common::Exception::SocketException) {

	for(Connection::PLAYERINFOS::const_iterator i(m_connection.getPlayers().begin());
			i != m_connection.getPlayers().end(); ++i) {

		if(player->getName() == i->name) {
			m_connection.write(i->sockfd, NetMauMau::Common::Protocol::V15::CARDREJECTED);
			m_connection.write(i->sockfd, player->getName());
			m_connection.write(i->sockfd, playedCard->description());
		}
	}
}

void EventHandler::playerSuspends(const NetMauMau::Player::IPlayer *player,
								  const NetMauMau::Common::ICard *) const
throw(NetMauMau::Common::Exception::SocketException) {
	m_connection << NetMauMau::Common::Protocol::V15::SUSPENDS << player->getName();
}

void EventHandler::playerPicksCard(const NetMauMau::Player::IPlayer *player,
								   const NetMauMau::Common::ICard *card) const
throw(NetMauMau::Common::Exception::SocketException) {

	for(Connection::PLAYERINFOS::const_iterator i(m_connection.getPlayers().begin());
			i != m_connection.getPlayers().end(); ++i) {

		m_connection.write(i->sockfd, NetMauMau::Common::Protocol::V15::PLAYERPICKSCARD);
		m_connection.write(i->sockfd, player->getName());

		if(card && i->name == player->getName()) {
			m_connection.write(i->sockfd, NetMauMau::Common::Protocol::V15::CARDTAKEN);
			m_connection.write(i->sockfd, card->description());
		} else {
			m_connection.write(i->sockfd, NetMauMau::Common::Protocol::V15::HIDDENCARDTAKEN);
		}
	}
}

void EventHandler::playerPicksCards(const NetMauMau::Player::IPlayer *player,
									std::size_t cardCount) const
throw(NetMauMau::Common::Exception::SocketException) {

	char cc[256];
#ifndef _WIN32
	std::snprintf(cc, 255, "%zu", cardCount);
#else
	std::snprintf(cc, 255, "%lu", (unsigned long)cardCount);
#endif

	m_connection << NetMauMau::Common::Protocol::V15::PLAYERPICKSCARDS << player->getName()
				 << NetMauMau::Common::Protocol::V15::TAKECOUNT << cc;
}

void EventHandler::playerChooseJackSuit(const NetMauMau::Player::IPlayer *,
										NetMauMau::Common::ICard::SUIT suit) const
throw(NetMauMau::Common::Exception::SocketException) {

	Connection::VERSIONEDMESSAGE vm;
	std::ostringstream vm_old, vm_new;

	vm_old << NetMauMau::Common::Protocol::V15::JACKSUIT << '\0'
		   << NetMauMau::Common::suitToSymbol(suit == NetMauMau::Common::ICard::SUIT_ILLEGAL ?
				   NetMauMau::Common::ICard::HEARTS : suit, false);
	vm_new << NetMauMau::Common::Protocol::V15::JACKSUIT << '\0'
		   << NetMauMau::Common::suitToSymbol(suit, false);

	NetMauMau::Common::efficientAddOrUpdate(vm, 0u, vm_old.str());
	NetMauMau::Common::efficientAddOrUpdate(vm, 4u, vm_new.str());

	m_connection.sendVersionedMessage(vm);
}

void EventHandler::setJackModeOff() const throw(NetMauMau::Common::Exception::SocketException) {

	Connection::VERSIONEDMESSAGE vm;

	NetMauMau::Common::efficientAddOrUpdate(vm, 15u, NetMauMau::Common::Protocol::V15::JACKMODEOFF);

	m_connection.sendVersionedMessage(vm);
}

void EventHandler::nextPlayer(const NetMauMau::Player::IPlayer *player) const
throw(NetMauMau::Common::Exception::SocketException) {
	for(Connection::PLAYERINFOS::const_iterator i(m_connection.getPlayers().begin());
			i != m_connection.getPlayers().end(); ++i) {
		if(player->getName() != i->name) {
			m_connection.write(i->sockfd, NetMauMau::Common::Protocol::V15::NEXTPLAYER);
			m_connection.write(i->sockfd, player->getName());
		}
	}
}

void EventHandler::aceRoundStarted(const NetMauMau::Player::IPlayer *player)
throw(NetMauMau::Common::Exception::SocketException) {
	m_connection << NetMauMau::Common::Protocol::V15::ACEROUNDSTARTED << player->getName();
}

void EventHandler::aceRoundEnded(const NetMauMau::Player::IPlayer *player)
throw(NetMauMau::Common::Exception::SocketException) {
	m_connection << NetMauMau::Common::Protocol::V15::ACEROUNDENDED << player->getName();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
