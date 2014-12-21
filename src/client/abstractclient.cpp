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
#include <cassert>
#include <cstring>

#include "abstractclient.h"

#include "interceptederrorexception.h"
#include "clientcardfactory.h"
#include "cardtools.h"
#include "pngcheck.h"
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

AbstractClientV07::AbstractClientV07(const std::string &player, const std::string &server,
									 uint16_t port) : AbstractClientV05(player, server, port) {}

AbstractClientV07::AbstractClientV07(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port) : AbstractClientV05(player, pngData, pngDataLen,
												 server, port) {}

AbstractClientV05::AbstractClientV05(const std::string &pName, const unsigned char *data,
									 std::size_t len, const std::string &server, uint16_t port)
	: IPlayerPicListener(), m_connection(pName, server, port), m_pName(pName),
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

AbstractClientV05::AbstractClientV05(const std::string &pName, const std::string &server,
									 uint16_t port) : IPlayerPicListener(),
	m_connection(pName, server, port), m_pName(pName), m_pngData(0L), m_pngDataLen(0), m_cards(),
	m_openCard(0L), m_disconnectNow(false) {}

AbstractClientV05::~AbstractClientV05() {

	for(CARDS::const_iterator i(m_cards.begin()); i != m_cards.end(); ++i) delete *i;

	delete m_openCard;
	delete [] m_pngData;

	m_connection.setInterrupted(false);
}

std::string AbstractClientV05::getPlayerName() const {
	return m_pName;
}

const char *AbstractClientV05::getDefaultAIName() {
	return AI_NAME;
}

void AbstractClientV05::disconnect() {
	m_disconnectNow = true;
	m_connection.setInterrupted(true, true);
}

Connection::CAPABILITIES AbstractClientV05::capabilities(timeval *timeout)
throw(NetMauMau::Common::Exception::SocketException) {
	m_connection.setTimeout(timeout);
	return m_connection.capabilities();
}

Connection::PLAYERLIST AbstractClientV05::playerList(timeval *timeout)
throw(NetMauMau::Common::Exception::SocketException) {

	const PLAYERINFOS &pi(playerList(false, timeout));
	PLAYERLIST pl;

	for(PLAYERINFOS::const_iterator i(pi.begin()); i != pi.end(); ++i) {
		pl.push_back(i->name);
	}

	return pl;
}

Connection::PLAYERINFOS AbstractClientV05::playerList(bool playerPNG, timeval *timeout)
throw(NetMauMau::Common::Exception::SocketException) {
	m_connection.setTimeout(timeout);
	return m_connection.playerList(this, playerPNG);
}

void AbstractClientV05::play(timeval *timeout)
throw(NetMauMau::Common::Exception::SocketException) {

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

				bool brk = false;

				switch(playInternal(msg, &cturn, &initCardShown, cjackSuit, &lastPlayedCard)) {
				case BREAK:
					brk = true;
					break;

				case NOT_UNDERSTOOD:
					logDebug("Client library: " << __PRETTY_FUNCTION__ << ": " << msg);
					unknownServerMessage(msg);
					break;

				default:
					break;
				}

				if(brk) break;
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

AbstractClientV05::PIRET AbstractClientV07::playInternal(std::string &msg, std::size_t *cturn,
		bool *initCardShown, std::string &cjackSuit,
		const NetMauMau::Common::ICard **lastPlayedCard)
throw(NetMauMau::Common::Exception::SocketException) {

	PIRET ret = AbstractClientV05::playInternal(msg, cturn, initCardShown, cjackSuit,
				lastPlayedCard);

	if(ret == NOT_UNDERSTOOD) {
		if(!m_disconnectNow && msg == "ACEROUND") {
			m_connection << (getAceRoundChoice() ? "TRUE" : "FALSE");
		} else if(!m_disconnectNow && msg == "ACEROUNDSTARTED") {
			aceRoundStarted();
		} else if(!m_disconnectNow && msg == "ACEROUNDENDED") {
			aceRoundEnded();
		} else {
			return NOT_UNDERSTOOD;
		}

		return OK;
	}

	return ret;
}

AbstractClientV05::PIRET AbstractClientV05::playInternal(std::string &msg, std::size_t *cturn,
		bool *initCardShown, std::string &cjackSuit,
		const NetMauMau::Common::ICard **lastPlayedCard)
throw(NetMauMau::Common::Exception::SocketException) {

	if(!m_disconnectNow && msg == "MESSAGE") {
		m_connection >> msg;
		message(msg);
	} else if(!m_disconnectNow && msg == "ERROR") {
		m_connection >> msg;
		error(msg);
		return BREAK;
	} else if(!m_disconnectNow && msg == "TURN") {

		m_connection >> msg;

		*cturn = std::strtoul(msg.c_str(), NULL, 10);

		turn(*cturn);

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
		return BREAK;
	} else if(!m_disconnectNow && msg.substr(0, 10) == "PLAYERWINS") {

		const bool ultimate = msg.length() > 10 && msg[10] == '+';

		m_connection >> msg;
		playerWins(msg, *cturn);

		if(!ultimate) {
			gameOver();
			return BREAK;
		}

	} else if(!m_disconnectNow && msg.substr(0, 10) == "PLAYERLOST") {
		std::string pl, pc;
		m_connection >> pl >> pc;
		playerLost(pl, *cturn, std::strtoul(pc.c_str(), NULL, 10));
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
			*initCardShown = true;
		}

		delete ic;

	} else if(!m_disconnectNow && msg == "TALONSHUFFLED") {
		talonShuffled();
	} else if(!m_disconnectNow && msg == "OPENCARD") {

		m_connection >> msg;
		delete m_openCard;
		m_openCard = (NetMauMau::Client::CardFactory(msg)).create();

		if(!*initCardShown) {

			assert(NetMauMau::Common::symbolToSuit(cjackSuit)
				   != NetMauMau::Common::ICard::SUIT_ILLEGAL);

			openCard(m_openCard, cjackSuit);

		} else {
			*initCardShown = false;
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

		*lastPlayedCard = playCard(possCards);

		if(*lastPlayedCard) {
			m_connection << (*lastPlayedCard)->description();
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

		if(*lastPlayedCard) {
			const CARDS::iterator
			&f(std::find_if(m_cards.begin(), m_cards.end(),
							std::bind2nd(std::ptr_fun(NetMauMau::Common::cardEqual),
										 *lastPlayedCard)));

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

		assert(NetMauMau::Common::symbolToSuit(cjackSuit)
			   != NetMauMau::Common::ICard::SUIT_ILLEGAL);

		jackSuit(NetMauMau::Common::symbolToSuit(cjackSuit));

	} else if(!m_disconnectNow && msg == "JACKCHOICE") {

		const NetMauMau::Common::ICard::SUIT s = getJackSuitChoice();

		assert(s != NetMauMau::Common::ICard::SUIT_ILLEGAL);

		m_connection << NetMauMau::Common::suitToSymbol(s, false);

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
		return BREAK;
	} else {
		return NOT_UNDERSTOOD;
	}

	return OK;
}

uint32_t AbstractClientV05::getClientProtocolVersion() {
	return (static_cast<uint16_t>(SERVER_VERSION_MAJOR) << 16u) |
		   static_cast<uint16_t>(SERVER_VERSION_MINOR);
}

uint32_t AbstractClientV05::parseProtocolVersion(const std::string &ver) {

	const std::string::size_type p = ver.find('.');

	if(p != std::string::npos) {

		const uint16_t maj = std::strtoul(ver.substr(0, p).c_str(), NULL, 10);
		const uint16_t min = std::strtoul(ver.substr(p + 1).c_str(), NULL, 10);

		return (maj << 16u) | min;
	}

	return 0;
}

uint16_t AbstractClientV05::getDefaultPort() {
	return SERVER_PORT;
}

bool AbstractClientV05::isPlayerImageUploadable(const unsigned char *pngData,
		std::size_t pngDataLen) {
	const std::string &base64png(NetMauMau::Common::base64_encode(pngData, pngDataLen));
	return !base64png.empty() && base64png.size() <= MAXPICBYTES &&
		   NetMauMau::Common::checkPNG(pngData, pngDataLen);
}

void AbstractClientV05::beginReceivePlayerPicture(const std::string &) const throw() {}
void AbstractClientV05::endReceivePlayerPicture(const std::string &) const throw() {}
void AbstractClientV05::uploadSucceded(const std::string &) const throw() {}
void AbstractClientV05::uploadFailed(const std::string &) const throw() {}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
