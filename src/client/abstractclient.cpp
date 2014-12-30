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
#include "abstractclientv05impl.h"
#include "clientcardfactory.h"
#include "cardtools.h"
#include "pngcheck.h"
#include "base64.h"
#include "logger.h"

namespace {

}

using namespace NetMauMau::Client;

AbstractClientV08::AbstractClientV08(const std::string &player, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV07(player, server, port) {
	_pimpl->m_connection.setClientVersion(clientVersion);
}

AbstractClientV08::AbstractClientV08(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV07(player, pngData, pngDataLen, server, port) {
	_pimpl->m_connection.setClientVersion(clientVersion);
}

AbstractClientV08::~AbstractClientV08() {}

AbstractClientV07::AbstractClientV07(const std::string &player, const std::string &server,
									 uint16_t port) : AbstractClientV05(player, server, port) {
	_pimpl->m_connection.setClientVersion(7);
}

AbstractClientV07::AbstractClientV07(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port) : AbstractClientV05(player, pngData, pngDataLen,
												 server, port) {
	_pimpl->m_connection.setClientVersion(7);
}

AbstractClientV07::~AbstractClientV07() {}

AbstractClientV05::AbstractClientV05(const std::string &pName, const unsigned char *data,
									 std::size_t len, const std::string &server, uint16_t port)
	: IPlayerPicListener(), _pimpl(new AbstractClientV05Impl(pName, server, port, data, len)) {}

AbstractClientV05::AbstractClientV05(const std::string &pName, const std::string &server,
									 uint16_t port) : IPlayerPicListener(),
	_pimpl(new AbstractClientV05Impl(pName, server, port, 0L, 0)) {}

AbstractClientV05::~AbstractClientV05() {
	delete _pimpl;
}

std::string AbstractClientV05::getPlayerName() const {
	return _pimpl->m_pName;
}

const char *AbstractClientV05::getDefaultAIName() {
	return AI_NAME;
}

void AbstractClientV05::disconnect() {
	_pimpl->m_disconnectNow = true;
	_pimpl->m_connection.setInterrupted(true, true);
}

Connection::CAPABILITIES AbstractClientV05::capabilities(timeval *timeout)
throw(NetMauMau::Common::Exception::SocketException) {
	_pimpl->m_connection.setTimeout(timeout);
	return _pimpl->m_connection.capabilities();
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
	_pimpl->m_connection.setTimeout(timeout);
	return _pimpl->m_connection.playerList(this, playerPNG);
}

void AbstractClientV05::play(timeval *timeout)
throw(NetMauMau::Common::Exception::SocketException) {

	_pimpl->m_connection.setTimeout(timeout);
	_pimpl->m_connection.connect(this, _pimpl->m_pngData, _pimpl->m_pngDataLen);

	delete [] _pimpl->m_pngData;
	_pimpl->m_pngDataLen = 0;
	_pimpl->m_pngData = 0L;

	const NetMauMau::Common::ICard *lastPlayedCard = 0L;
	bool initCardShown = false;
	std::string msg, cjackSuit;
	std::size_t cturn = 0;

	while(!_pimpl->m_disconnectNow) {

		try {

			_pimpl->m_connection >> msg;

			if(!_pimpl->m_disconnectNow && !msg.empty()) {

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

			if(!_pimpl->m_disconnectNow) error(e.what());

			break;

		} catch(const NetMauMau::Common::Exception::SocketException &) {
			if(!_pimpl->m_disconnectNow) throw;
		}
	}

	_pimpl->m_disconnectNow = false;
}

AbstractClientV05::PIRET AbstractClientV07::playInternal(std::string &msg, std::size_t *cturn,
		bool *initCardShown, std::string &cjackSuit,
		const NetMauMau::Common::ICard **lastPlayedCard)
throw(NetMauMau::Common::Exception::SocketException) {

	PIRET ret = AbstractClientV05::playInternal(msg, cturn, initCardShown, cjackSuit,
				lastPlayedCard);

	if(ret == NOT_UNDERSTOOD) {
		if(!_pimpl->m_disconnectNow && msg == "ACEROUND") {
			_pimpl->m_connection << (getAceRoundChoice() ? "TRUE" : "FALSE");
		} else if(!_pimpl->m_disconnectNow && msg == "ACEROUNDSTARTED") {
			_pimpl->m_connection >> msg;
			aceRoundStarted(msg);
		} else if(!_pimpl->m_disconnectNow && msg == "ACEROUNDENDED") {
			_pimpl->m_connection >> msg;
			aceRoundEnded(msg);
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

	if(!_pimpl->m_disconnectNow && msg == "MESSAGE") {
		_pimpl->m_connection >> msg;
		message(msg);
	} else if(!_pimpl->m_disconnectNow && msg == "ERROR") {
		_pimpl->m_connection >> msg;
		error(msg);
		return BREAK;
	} else if(!_pimpl->m_disconnectNow && msg == "TURN") {

		_pimpl->m_connection >> msg;

		*cturn = std::strtoul(msg.c_str(), NULL, 10);

		turn(*cturn);

	} else if(!_pimpl->m_disconnectNow && msg == "NEXTPLAYER") {
		_pimpl->m_connection >> msg;
		nextPlayer(msg);
	} else if(!_pimpl->m_disconnectNow && msg == "STATS") {

		_pimpl->m_connection >> msg;

		STATS cstats;

		while(msg != "ENDSTATS") {

			std::string cntS;
			_pimpl->m_connection >> cntS;

			const STAT stat = { msg, std::strtoul(cntS.c_str(), NULL, 10) };
			cstats.push_back(stat);

			_pimpl->m_connection >> msg;
		}

		stats(cstats);

	} else if(!_pimpl->m_disconnectNow && msg == "PLAYERJOINED") {

		std::string plPic;

		_pimpl->m_connection >> msg;

		beginReceivePlayerPicture(msg);

		_pimpl->m_connection >> plPic;

		const std::vector<NetMauMau::Common::BYTE>
		&plPicPng(NetMauMau::Common::base64_decode(plPic));

		endReceivePlayerPicture(msg);

		const bool hasPlPic = !(plPic == "-" || plPicPng.empty());

		playerJoined(msg, hasPlPic ? plPicPng.data() : 0L,
					 hasPlPic ? plPicPng.size() : 0);

	} else if(!_pimpl->m_disconnectNow && msg == "PLAYERREJECTED") {
		_pimpl->m_connection >> msg;
		playerRejected(msg);
		return BREAK;
	} else if(!_pimpl->m_disconnectNow && msg.substr(0, 10) == "PLAYERWINS") {

		const bool ultimate = msg.length() > 10 && msg[10] == '+';

		_pimpl->m_connection >> msg;
		playerWins(msg, *cturn);

		if(!ultimate) {
			gameOver();
			return BREAK;
		}

	} else if(!_pimpl->m_disconnectNow && msg.substr(0, 10) == "PLAYERLOST") {
		std::string pl, pc;
		_pimpl->m_connection >> pl >> pc;
		playerLost(pl, *cturn, std::strtoul(pc.c_str(), NULL, 10));
	} else if(!_pimpl->m_disconnectNow && msg == "GETCARDS") {

		_pimpl->m_connection >> msg;

		const CARDS::size_type cnt = _pimpl->m_cards.empty() ? 0 : _pimpl->m_cards.size();

		while(msg != "CARDSGOT") {
			_pimpl->m_cards.push_back((NetMauMau::Client::CardFactory(msg)).create());
			_pimpl->m_connection >> msg;
		}

		cardSet(_pimpl->getCards(_pimpl->m_cards, cnt));

	} else if(!_pimpl->m_disconnectNow && msg == "INITIALCARD") {

		_pimpl->m_connection >> msg;
		const NetMauMau::Common::ICard *ic = (NetMauMau::Client::CardFactory(msg))
											 .create();

		if(ic->getRank() == NetMauMau::Common::ICard::JACK ||
				ic->getRank() == NetMauMau::Common::ICard::EIGHT) {
			initialCard(ic);
			*initCardShown = true;
		}

		delete ic;

	} else if(!_pimpl->m_disconnectNow && msg == "TALONSHUFFLED") {
		talonShuffled();
	} else if(!_pimpl->m_disconnectNow && msg == "OPENCARD") {

		_pimpl->m_connection >> msg;
		delete _pimpl->m_openCard;
		_pimpl->m_openCard = (NetMauMau::Client::CardFactory(msg)).create();

		if(!*initCardShown) {

			assert(NetMauMau::Common::symbolToSuit(cjackSuit)
				   != NetMauMau::Common::ICard::SUIT_ILLEGAL);

			openCard(_pimpl->m_openCard, cjackSuit);

		} else {
			*initCardShown = false;
		}

	} else if(!_pimpl->m_disconnectNow && msg == "PLAYCARD") {
		*lastPlayedCard = playCard(_pimpl->recvPossibleCards(msg));
		_pimpl->sendPlayedCard(lastPlayedCard);
	} else if(!_pimpl->m_disconnectNow && !msg.compare(0, 8, "SUSPEND ")) {
		enableSuspend(!msg.compare(8, std::string::npos, "ON"));
	} else if(!_pimpl->m_disconnectNow && msg == "SUSPENDS") {
		_pimpl->m_connection >> msg;
		playerSuspends(msg);
	} else if(!_pimpl->m_disconnectNow && msg == "CARDACCEPTED") {

		_pimpl->m_connection >> msg;

		if(*lastPlayedCard) {
			const CARDS::iterator
			&f(std::find_if(_pimpl->m_cards.begin(), _pimpl->m_cards.end(),
							std::bind2nd(std::ptr_fun(NetMauMau::Common::cardEqual),
										 *lastPlayedCard)));

			if(f != _pimpl->m_cards.end()) {
				cardAccepted(*f);
				delete *f;
				_pimpl->m_cards.erase(f);
			}
		}

	} else if(!_pimpl->m_disconnectNow && msg == "CARDREJECTED") {

		std::string player;
		_pimpl->m_connection >> player >> msg;

		const NetMauMau::Common::ICard *c = (NetMauMau::Client::CardFactory(msg))
											.create();
		cardRejected(player, c);
		delete c;

	} else if(!_pimpl->m_disconnectNow && msg == "CARDCOUNT") {

		char cc[256];
#ifndef _WIN32
		std::snprintf(cc, 255, "%zu", _pimpl->m_cards.size());
#else
		std::snprintf(cc, 255, "%lu", (unsigned long)_pimpl->m_cards.size());
#endif

		_pimpl->m_connection << cc;

	} else if(!_pimpl->m_disconnectNow && msg == "PLAYEDCARD") {

		std::string player;
		_pimpl->m_connection >> player >> msg;

		const NetMauMau::Common::ICard *c = (NetMauMau::Client::CardFactory(msg))
											.create();
		playedCard(player, c);
		delete c;

		cjackSuit.clear();

	} else if(!_pimpl->m_disconnectNow && msg == "JACKSUIT") {

		_pimpl->m_connection >> msg;
		cjackSuit = msg;

		assert(NetMauMau::Common::symbolToSuit(cjackSuit)
			   != NetMauMau::Common::ICard::SUIT_ILLEGAL);

		jackSuit(NetMauMau::Common::symbolToSuit(cjackSuit));

	} else if(!_pimpl->m_disconnectNow && msg == "JACKCHOICE") {

		const NetMauMau::Common::ICard::SUIT s = getJackSuitChoice();

		assert(s != NetMauMau::Common::ICard::SUIT_ILLEGAL);

		_pimpl->m_connection << NetMauMau::Common::suitToSymbol(s, false);

	} else if(!_pimpl->m_disconnectNow && msg == "PLAYERPICKSCARD") {

		std::string player, extra;
		_pimpl->m_connection >> player >> extra;

		if(extra == "CARDTAKEN") {
			_pimpl->m_connection >> msg;
			const NetMauMau::Common::ICard *c = (NetMauMau::Client::CardFactory(msg))
												.create();
			playerPicksCard(player, c);
			delete c;
		} else {
			playerPicksCard(player, static_cast<NetMauMau::Common::ICard *>(0L));
		}

	} else if(!_pimpl->m_disconnectNow && msg == "PLAYERPICKSCARDS") {

		std::string player;
		_pimpl->m_connection >> player >> msg;
		_pimpl->m_connection >> msg;

		playerPicksCard(player, std::strtoul(msg.c_str(), NULL, 10));

	} else if(!_pimpl->m_disconnectNow && msg == "BYE") {
		gameOver();
		return BREAK;
	} else {
		return NOT_UNDERSTOOD;
	}

	return OK;
}

AbstractClientV05::PIRET AbstractClientV08::playInternal(std::string &msg, std::size_t *cturn,
		bool *initCardShown, std::string &cjackSuit,
		const NetMauMau::Common::ICard **lastPlayedCard)
throw(NetMauMau::Common::Exception::SocketException) {

	if(!_pimpl->m_disconnectNow && msg == "PLAYCARD") {

		const NetMauMau::Client::AbstractClient::CARDS &possCards(_pimpl->recvPossibleCards(msg));

		std::string tc;
		_pimpl->m_connection >> tc;

		*lastPlayedCard = playCard(possCards, std::strtoul(tc.c_str(), NULL, 10));
		_pimpl->sendPlayedCard(lastPlayedCard);

	} else {
		return NetMauMau::Client::AbstractClientV07::playInternal(msg, cturn, initCardShown,
				cjackSuit, lastPlayedCard);
	}

	return OK;
}

uint32_t AbstractClientV05::getClientProtocolVersion() {
	return MAKE_VERSION(SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR);
}

uint32_t AbstractClientV05::parseProtocolVersion(const std::string &ver) {

	const std::string::size_type p = ver.find('.');

	if(p != std::string::npos) {
		return MAKE_VERSION(std::strtoul(ver.substr(0, p).c_str(), NULL, 10),
							std::strtoul(ver.substr(p + 1).c_str(), NULL, 10));
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

NetMauMau::Common::ICard *AbstractClientV08::playCard(const AbstractClientV05::CARDS &cards) const {
	return playCard(cards, 0);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
