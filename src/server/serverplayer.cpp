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

#include <cstdio>

#include "serverplayer.h"

#include "serverplayerexception.h"
#include "serverconnection.h"
#include "cardtools.h"
#include "iruleset.h"
#include "engine.h"

using namespace NetMauMau::Server;

Player::Player(const std::string &name, int sockfd, Connection &con) : StdPlayer(name),
	m_connection(con), m_sockfd(sockfd) {}

Player::~Player() {}

int Player::getSerial() const {
	return m_sockfd;
}

bool Player::isAIPlayer() const {
	return false;
}

bool Player::isAlive() const {

	try {
		NetMauMau::Common::AbstractSocket::checkSocket(m_sockfd);
	} catch(const Exception::ServerPlayerException &) {
		return false;
	}

	return true;
}

void Player::receiveCard(NetMauMau::Common::ICard *card) {

	try {
		if(card) receiveCardSet(CARDS(1, card));
	} catch(const NetMauMau::Common::Exception::SocketException &) {
		throw Exception::ServerPlayerException(__FUNCTION__);
	}
}

void Player::receiveCardSet(const CARDS &cards)
throw(NetMauMau::Common::Exception::SocketException) {

	NetMauMau::Player::StdPlayer::receiveCardSet(cards);

	try {
		m_connection.write(m_sockfd, "GETCARDS");

		for(CARDS::const_iterator i(cards.begin()); i != cards.end(); ++i) {
			m_connection.write(m_sockfd, (*i)->description());
		}

		m_connection.write(m_sockfd, "CARDSGOT");

	} catch(const NetMauMau::Common::Exception::SocketException &) {
		throw Exception::ServerPlayerException(__FUNCTION__);
	}
}

void Player::shuffleCards() {}

NetMauMau::Common::ICard *Player::requestCard(const NetMauMau::Common::ICard *uncoveredCard,
		const NetMauMau::Common::ICard::SUIT *s, std::size_t takeCount) const {

	try {

		m_connection.write(m_sockfd, "OPENCARD");
		m_connection.write(m_sockfd, uncoveredCard->description());;
		m_connection.write(m_sockfd, "PLAYCARD");

		for(CARDS::const_iterator i(getPlayerCards().begin()); i != getPlayerCards().end(); ++i) {

			if(s && (*i)->getSuit() != *s) continue;

			const bool accepted = getRuleSet()->isAceRound() ?
								  (*i)->getRank() == getRuleSet()->getAceRoundRank() :
								  getRuleSet()->checkCard(*i, uncoveredCard);

			const bool jack = ((*i)->getRank() == NetMauMau::Common::ICard::JACK &&
							   uncoveredCard->getRank() != NetMauMau::Common::ICard::JACK) &&
							  !getRuleSet()->isAceRound();

			if(accepted || jack) {
				m_connection.write(m_sockfd, (*i)->description());
			}
		}

		m_connection.write(m_sockfd, "PLAYCARDEND");

		if(m_connection.getPlayerInfo(getName()).clientVersion >= 8) {

			char cc[20];

#ifndef _WIN32
			std::snprintf(cc, 20, "%zu", takeCount);
#else
			std::snprintf(cc, 20, "%lu", (unsigned long)takeCount);
#endif

			m_connection.write(m_sockfd, cc);
		}

		const std::string offeredCard = m_connection.read(m_sockfd);

		if(offeredCard == "SUSPEND") {
			return 0L;
		} else if(offeredCard == "ILLEGAL CARD") {
			return NetMauMau::Common::getIllegalCard();
		}

		return findCard(offeredCard);

	} catch(const NetMauMau::Common::Exception::SocketException &e) {
		throw Exception::ServerPlayerException(__FUNCTION__);
	}
}

NetMauMau::Common::ICard *Player::findCard(const std::string &offeredCard) const {

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::HEARTS;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::ACE;

	if(NetMauMau::Common::parseCardDesc(offeredCard, &s, &r)) {

		const CARDS &pc(getPlayerCards());

		for(CARDS::const_iterator i(pc.begin()); i != pc.end(); ++i) {
			if((*i)->getSuit() == s && (*i)->getRank() == r) return *i;
		}
	}

	return 0L;
}

bool Player::cardAccepted(const NetMauMau::Common::ICard *playedCard)
throw(NetMauMau::Common::Exception::SocketException) {

	NetMauMau::Player::StdPlayer::cardAccepted(playedCard);

	try {

		m_connection.write(m_sockfd, "CARDACCEPTED");
		m_connection.write(m_sockfd, playedCard->description());

		return !getCardCount();

	} catch(const NetMauMau::Common::Exception::SocketException &) {
		throw Exception::ServerPlayerException(__FUNCTION__);
	}
}

void Player::talonShuffled() throw(NetMauMau::Common::Exception::SocketException) {
	NetMauMau::Player::StdPlayer::talonShuffled();
	m_connection.write(m_sockfd, "TALONSHUFFLED");
}

Player::IPlayer::REASON Player::getNoCardReason() const {

	try {

		if(m_connection.getPlayerInfo(getName()).clientVersion >= 15) {

			m_connection.write(m_sockfd, "NOCARDREASON");
			return m_connection.read(m_sockfd) == "NOMATCH" ?
				   NetMauMau::Player::IPlayer::NOMATCH :
				   NetMauMau::Player::IPlayer::SUSPEND;
		}

	} catch(const NetMauMau::Common::Exception::SocketException &) {}

	return NetMauMau::Player::IPlayer::SUSPEND;
}

std::size_t Player::getCardCount() const throw(NetMauMau::Common::Exception::SocketException) {

	std::size_t cc = 0;

	try {
		m_connection.write(m_sockfd, "CARDCOUNT");
		cc = std::strtoul(m_connection.read(m_sockfd).c_str(), NULL, 10);
	} catch(const NetMauMau::Common::Exception::SocketException &) {
		throw Exception::ServerPlayerException(__FUNCTION__);
	}

	return cc;
}

NetMauMau::Common::ICard::SUIT Player::getJackChoice(const NetMauMau::Common::ICard *,
		const NetMauMau::Common::ICard *) const
throw(NetMauMau::Common::Exception::SocketException) {

	try {
		m_connection.write(m_sockfd, "JACKCHOICE");
		return NetMauMau::Common::symbolToSuit(m_connection.read(m_sockfd));
	} catch(const NetMauMau::Common::Exception::SocketException &) {
		throw Exception::ServerPlayerException(__FUNCTION__);
	}
}

bool Player::getAceRoundChoice() const throw(NetMauMau::Common::Exception::SocketException) {

	if(isAceRoundAllowed()) {
		try {
			m_connection.write(m_sockfd, "ACEROUND");
			return m_connection.read(m_sockfd) == "TRUE";
		} catch(const NetMauMau::Common::Exception::SocketException &) {
			throw Exception::ServerPlayerException(__FUNCTION__);
		}
	}

	return false;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
