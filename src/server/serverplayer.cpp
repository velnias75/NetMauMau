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

#include <sstream>

#include "serverplayer.h"
#include "serverconnection.h"
#include "cardtools.h"
#include "engine.h"

using namespace NetMauMau::Server;

Player::Player(const std::string &name, int sockfd, Connection &con) : StdPlayer(name),
	m_connection(con), m_sockfd(sockfd) {}

Player::~Player() {}

int Player::getSerial() const {
	return m_sockfd;
}

void Player::receiveCard(NetMauMau::Common::ICard *card) {
	receiveCardSet(std::vector<NetMauMau::Common::ICard *>(1, card));
}

void Player::receiveCardSet(const std::vector<NetMauMau::Common::ICard *> &cards) {

	NetMauMau::Player::StdPlayer::receiveCardSet(cards);

	m_connection.write(m_sockfd, "GETCARDS");

	for(std::vector<NetMauMau::Common::ICard *>::const_iterator i(cards.begin());
			i != cards.end(); ++i) {
		m_connection.write(m_sockfd, (*i)->description());
	}

	m_connection.write(m_sockfd, "CARDSGOT");
}

NetMauMau::Common::ICard *Player::requestCard(const NetMauMau::Common::ICard *uncoveredCard,
		const NetMauMau::Common::ICard::SUIT *) const {

	m_connection.write(m_sockfd, "OPENCARD");
	m_connection.write(m_sockfd, uncoveredCard->description());;
	m_connection.write(m_sockfd, "PLAYCARD");

	const std::string offeredCard = m_connection.read(m_sockfd);

	if(offeredCard == "SUSPEND") {
		return 0L;
	}

	return findCard(offeredCard);
}

NetMauMau::Common::ICard *Player::findCard(const std::string &offeredCard) const {

	NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::HEARTS;
	NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::ACE;

	if(NetMauMau::Common::parseCardDesc(offeredCard, &s, &r)) {
		const std::vector<NetMauMau::Common::ICard *> &pc(getPlayerCards());

		for(std::vector<NetMauMau::Common::ICard *>::const_iterator i(pc.begin());
				i != pc.end(); ++i) {
			if((*i)->getSuit() == s && (*i)->getRank() == r) return *i;
		}
	}

	return 0L;
}

bool Player::cardAccepted(const NetMauMau::Common::ICard *playedCard) {

	NetMauMau::Player::StdPlayer::cardAccepted(playedCard);

	m_connection.write(m_sockfd, "CARDACCEPTED");
	m_connection.write(m_sockfd, playedCard->description());

	return !getCardCount();
}

Player::IPlayer::REASON Player::getNoCardReason() const {
	return NetMauMau::Player::IPlayer::SUSPEND;
}

std::size_t Player::getCardCount() const {

	m_connection.write(m_sockfd, "CARDCOUNT");

	std::size_t cc = 0;

	try {
		(std::istringstream(m_connection.read(m_sockfd))) >> cc;
	} catch(const NetMauMau::Common::Exception::SocketException &) {}

	return cc;
}

NetMauMau::Common::ICard::SUIT Player::getJackChoice(const NetMauMau::Common::ICard *,
		const NetMauMau::Common::ICard *) const {
	m_connection.write(m_sockfd, "JACKCHOICE");
	return NetMauMau::Common::symbolToSuit(m_connection.read(m_sockfd));
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
