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

/**
 * @NOTE keeping API compatibility:
 * https://techbase.kde.org/Policies/Binary_Compatibility_Issues_With_C++#Adding_new_virtual_functions_to_leaf_classes
 * http://static.coldattic.info/restricted/science/syrcose09/cppbincomp.pdf
 */

#include <cstdio>
#include <cstring>

#include "abstractclient.h"

#include "interceptederrorexception.h"
#include "clientcardfactory.h"
#include "cardtools.h"
#include "base64.h"
#include "logger.h"

namespace {

NetMauMau::Client::AbstractClient::CARDS
getCards(const NetMauMau::Client::AbstractClient::CARDS &mCards,
		 const NetMauMau::Client::AbstractClient::CARDS::size_type cnt = 0) {

	NetMauMau::Client::AbstractClient::CARDS cards;

	if(!mCards.empty()) {

		cards.reserve(mCards.size());

		NetMauMau::Client::AbstractClient::CARDS::const_iterator i(mCards.begin());
		std::advance(i, cnt);

		cards.insert(cards.end(), i, mCards.end());
	}

	return cards;
}

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct cardEqualsDescription : public std::binary_function < NetMauMau::Common::ICard *,
		std::string, bool > {
	bool operator()(const NetMauMau::Common::ICard *c, const std::string &d) const {
		return c->description() == d;
	}
};
#pragma GCC diagnostic pop

}

using namespace NetMauMau::Client;

AbstractClient::AbstractClient(const std::string &pName, const unsigned char *data, std::size_t len,
							   const std::string &server, uint16_t port) : IPlayerPicListener(),
	m_connection(pName, server, port), m_pName(pName),
	m_pngData(new(std::nothrow) unsigned char[len]()), m_pngDataLen(len), m_cards(),
	m_openCard(0L), m_disconnectNow(false) {

	if(m_pngData && m_pngDataLen) {
		std::memcpy(m_pngData, data, len);
	} else {
		delete [] m_pngData;
		m_pngData = 0L;
		m_pngDataLen = 0;
	}
}

AbstractClient::AbstractClient(const std::string &pName, const std::string &server, uint16_t port)
	: IPlayerPicListener(), m_connection(pName, server, port), m_pName(pName), m_pngData(0L),
	  m_pngDataLen(0), m_cards(), m_openCard(0L), m_disconnectNow(false) {}

AbstractClient::~AbstractClient() {

	for(CARDS::const_iterator i(m_cards.begin()); i != m_cards.end(); ++i) delete *i;

	delete m_openCard;
	delete [] m_pngData;

	m_connection.setInterrupted(false);
}

std::string AbstractClient::getPlayerName() const {
	return m_pName;
}

const char *AbstractClient::getDefaultAIName() {
	return AI_NAME;
}

void AbstractClient::disconnect() {
	m_disconnectNow = true;
	m_connection.setInterrupted(true, true);
}

Connection::CAPABILITIES AbstractClient::capabilities(timeval *timeout)
throw(NetMauMau::Common::Exception::SocketException) {
	m_connection.setTimeout(timeout);
	return m_connection.capabilities();
}

Connection::PLAYERLIST AbstractClient::playerList(timeval *timeout)
throw(NetMauMau::Common::Exception::SocketException) {

	const PLAYERINFOS &pi(playerList(false, timeout));
	PLAYERLIST pl;

	for(PLAYERINFOS::const_iterator i(pi.begin()); i != pi.end(); ++i) {
		pl.push_back(i->name);
	}

	return pl;
}

Connection::PLAYERINFOS AbstractClient::playerList(bool playerPNG, timeval *timeout)
throw(NetMauMau::Common::Exception::SocketException) {
	m_connection.setTimeout(timeout);
	return m_connection.playerList(this, playerPNG);
}

void AbstractClient::play(timeval *timeout) throw(NetMauMau::Common::Exception::SocketException) {

	m_connection.setTimeout(timeout);
	m_connection.connect(this, m_pngData, m_pngDataLen);

	delete [] m_pngData;
	m_pngDataLen = 0;
	m_pngData = 0L;

	const NetMauMau::Common::ICard *lastPlayedCard = 0L;
	bool initCardShown = false;
	std::string msg, cjackSuit;
	std::size_t cturn = 0;

	while(!m_disconnectNow) {

		try {

			m_connection >> msg;

			if(!m_disconnectNow && !msg.empty()) {

				if(!m_disconnectNow && msg == "MESSAGE") {
					m_connection >> msg;
					message(msg);
				} else if(!m_disconnectNow && msg == "ERROR") {
					m_connection >> msg;
					error(msg);
					break;
				} else if(!m_disconnectNow && msg == "TURN") {

					m_connection >> msg;

					cturn = std::strtoul(msg.c_str(), NULL, 10);

					turn(cturn);

				} else if(!m_disconnectNow && msg == "NEXTPLAYER") {
					m_connection >> msg;
					nextPlayer(msg);
				} else if(!m_disconnectNow && msg == "STATS") {

					m_connection >> msg;

					STATS cstats;

					while(msg != "ENDSTATS") {

						std::string cntS;
						m_connection >> cntS;

						const STAT stat = { msg, std::strtoul(cntS.c_str(), NULL, 10) };
						cstats.push_back(stat);

						m_connection >> msg;
					}

					stats(cstats);

				} else if(!m_disconnectNow && msg == "PLAYERJOINED") {

					std::string plPic;

					m_connection >> msg;

					beginReceivePlayerPicture(msg);

					m_connection >> plPic;

					const std::vector<NetMauMau::Common::BYTE>
					&plPicPng(NetMauMau::Common::base64_decode(plPic));

					endReceivePlayerPicture(msg);

					const bool hasPlPic = !(plPic == "-" || plPicPng.empty());

					playerJoined(msg, hasPlPic ? plPicPng.data() : 0L,
								 hasPlPic ? plPicPng.size() : 0);

				} else if(!m_disconnectNow && msg == "PLAYERREJECTED") {
					m_connection >> msg;
					playerRejected(msg);
					break;
				} else if(!m_disconnectNow && msg.substr(0, 10) == "PLAYERWINS") {

					const bool ultimate = msg.length() > 10 && msg[10] == '+';

					m_connection >> msg;
					playerWins(msg, cturn);

					if(!ultimate) {
						gameOver();
						break;
					}

				} else if(!m_disconnectNow && msg.substr(0, 10) == "PLAYERLOST") {
					std::string pl, pc;
					m_connection >> pl >> pc;
					playerLost(pl, cturn, std::strtoul(pc.c_str(), NULL, 10));
				} else if(!m_disconnectNow && msg == "GETCARDS") {

					m_connection >> msg;

					const CARDS::size_type cnt = m_cards.empty() ? 0 : m_cards.size();

					while(msg != "CARDSGOT") {
						m_cards.push_back((NetMauMau::Client::CardFactory(msg)).create());
						m_connection >> msg;
					}

					cardSet(getCards(m_cards, cnt));

				} else if(!m_disconnectNow && msg == "INITIALCARD") {

					m_connection >> msg;
					const NetMauMau::Common::ICard *ic = (NetMauMau::Client::CardFactory(msg))
														 .create();

					if(ic->getRank() == NetMauMau::Common::ICard::JACK ||
							ic->getRank() == NetMauMau::Common::ICard::EIGHT) {
						initialCard(ic);
						initCardShown = true;
					}

					delete ic;

				} else if(!m_disconnectNow && msg == "TALONSHUFFLED") {
					talonShuffled();
				} else if(!m_disconnectNow && msg == "OPENCARD") {

					m_connection >> msg;
					delete m_openCard;
					m_openCard = (NetMauMau::Client::CardFactory(msg)).create();

					if(!initCardShown) {
						openCard(m_openCard, cjackSuit);
					} else {
						initCardShown = false;
					}

				} else if(!m_disconnectNow && msg == "PLAYCARD") {

					const AbstractClient::CARDS &myCards(getCards(m_cards));
					AbstractClient::CARDS possCards;

					possCards.reserve(myCards.size());

					m_connection >> msg;

					while(msg != "PLAYCARDEND") {

						const AbstractClient::CARDS::const_iterator &f(std::find_if(myCards.begin(),
								myCards.end(), std::bind2nd(cardEqualsDescription(), msg)));

						if(f != myCards.end()) possCards.push_back(*f);

						m_connection >> msg;
					}

					lastPlayedCard = playCard(possCards);

					if(lastPlayedCard) {
						m_connection << lastPlayedCard->description();
					} else {
						m_connection << "SUSPEND";
					}

				} else if(!m_disconnectNow && !msg.compare(0, 8, "SUSPEND ")) {
					enableSuspend(!msg.compare(8, std::string::npos, "ON"));
				} else if(!m_disconnectNow && msg == "SUSPENDS") {
					m_connection >> msg;
					playerSuspends(msg);
				} else if(!m_disconnectNow && msg == "CARDACCEPTED") {

					m_connection >> msg;

					if(lastPlayedCard) {
						const CARDS::iterator
						&f(std::find_if(m_cards.begin(), m_cards.end(),
										std::bind2nd(std::ptr_fun(NetMauMau::Common::cardEqual),
													 lastPlayedCard)));

						if(f != m_cards.end()) {
							cardAccepted(*f);
							delete *f;
							m_cards.erase(f);
						}
					}

				} else if(!m_disconnectNow && msg == "CARDREJECTED") {

					std::string player;
					m_connection >> player >> msg;

					const NetMauMau::Common::ICard *c = (NetMauMau::Client::CardFactory(msg))
														.create();
					cardRejected(player, c);
					delete c;

				} else if(!m_disconnectNow && msg == "CARDCOUNT") {

					char cc[256];
#ifndef _WIN32
					std::snprintf(cc, 255, "%zu", m_cards.size());
#else
					std::snprintf(cc, 255, "%lu", (unsigned long)m_cards.size());
#endif

					m_connection << cc;

				} else if(!m_disconnectNow && msg == "PLAYEDCARD") {

					std::string player;
					m_connection >> player >> msg;

					const NetMauMau::Common::ICard *c = (NetMauMau::Client::CardFactory(msg))
														.create();
					playedCard(player, c);
					delete c;

					cjackSuit.clear();

				} else if(!m_disconnectNow && msg == "JACKSUIT") {
					m_connection >> msg;
					cjackSuit = msg;
					jackSuit(NetMauMau::Common::symbolToSuit(cjackSuit));
				} else if(!m_disconnectNow && msg == "JACKCHOICE") {
					m_connection << NetMauMau::Common::suitToSymbol(getJackSuitChoice(), false);
				} else if(!m_disconnectNow && msg == "PLAYERPICKSCARD") {

					std::string player, extra;
					m_connection >> player >> extra;

					if(extra == "CARDTAKEN") {
						m_connection >> msg;
						const NetMauMau::Common::ICard *c = (NetMauMau::Client::CardFactory(msg))
															.create();
						playerPicksCard(player, c);
						delete c;
					} else {
						playerPicksCard(player, static_cast<NetMauMau::Common::ICard *>(0L));
					}

				} else if(!m_disconnectNow && msg == "PLAYERPICKSCARDS") {

					std::string player;
					m_connection >> player >> msg;
					m_connection >> msg;

					playerPicksCard(player, std::strtoul(msg.c_str(), NULL, 10));

				} else if(!m_disconnectNow && msg == "BYE") {
					gameOver();
					break;
				} else if(!m_disconnectNow) {
					logDebug("Client library: " << __PRETTY_FUNCTION__ << ": " << msg);
					unknownServerMessage(msg);
				}
			}

		} catch(const Exception::InterceptedErrorException &e) {

			if(!m_disconnectNow) error(e.what());

			break;

		} catch(const NetMauMau::Common::Exception::SocketException &) {
			if(!m_disconnectNow) throw;
		}
	}

	m_disconnectNow = false;
}

uint32_t AbstractClient::getClientProtocolVersion() {
	return (static_cast<uint16_t>(SERVER_VERSION_MAJOR) << 16u) |
		   static_cast<uint16_t>(SERVER_VERSION_MINOR);
}

uint32_t AbstractClient::parseProtocolVersion(const std::string &ver) {

	const std::string::size_type p = ver.find('.');

	if(p != std::string::npos) {

		const uint16_t maj = std::strtoul(ver.substr(0, p).c_str(), NULL, 10);
		const uint16_t min = std::strtoul(ver.substr(p + 1).c_str(), NULL, 10);

		return (maj << 16u) | min;
	}

	return 0;
}

uint16_t AbstractClient::getDefaultPort() {
	return SERVER_PORT;
}

bool AbstractClient::isPlayerImageUploadable(const unsigned char *pngData, std::size_t pngDataLen) {
	const std::string &base64png(NetMauMau::Common::base64_encode(pngData, pngDataLen));
	return !base64png.empty() && base64png.size() <= MAXPICBYTES;
}

void AbstractClient::beginReceivePlayerPicture(const std::string &) const throw() {}
void AbstractClient::endReceivePlayerPicture(const std::string &) const throw() {}
void AbstractClient::uploadSucceded(const std::string &) const throw() {}
void AbstractClient::uploadFailed(const std::string &) const throw() {}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
