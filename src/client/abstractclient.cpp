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

#include <cassert>                      // for assert
#include <cstdio>                       // for NULL, snprintf
#include <stdbool.h>

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include "base64.h"
#include "abstractclientv05impl.h"      // for AbstractClientV05Impl
#include "logger.h"                     // for BasicLogger, logDebug
#include "capabilitiesexception.h"      // for CapabilitiesException
#include "cardtools.h"                  // for symbolToSuit, suitToSymbol, etc
#include "clientcardfactory.h"          // for CardFactory
#include "interceptederrorexception.h"  // for InterceptedErrorException
#include "pngcheck.h"                   // for checkPNG
#include "scoresexception.h"            // for ScoresException
#include "shutdownexception.h"
#include "lostconnectionexception.h"
#include "protocol.h"                   // for PLAYCARD, ACEROUND, etc

namespace NetMauMau {

namespace Client {

struct _LOCAL _playInternalParams {
	inline _playInternalParams(std::string &m, std::size_t *t, bool *ics, std::string &js,
							   const Common::ICard **lpc) : msg(m), cturn(t), initCardShown(ics),
		cjackSuit(js), lastPlayedCard(lpc) {}

	std::string &msg;
	std::size_t *cturn;
	bool *initCardShown;
	std::string &cjackSuit;
	const Common::ICard **lastPlayedCard;
};

}

}

using namespace NetMauMau::Client;

AbstractClientV13::AbstractClientV13(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV11(player, pngData, pngDataLen, server, port, clientVersion) {}

AbstractClientV13::AbstractClientV13(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion, const IBase64 *)
	: AbstractClientV11(player, pngData, pngDataLen, server, port, clientVersion, 0L) {}

AbstractClientV13::AbstractClientV13(const std::string &player, const std::string &server,
									 uint16_t port, uint32_t clientVersion, const IBase64 *)
	: AbstractClientV11(player, server, port, clientVersion, 0L) {}

AbstractClientV13::AbstractClientV13(const std::string &player, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV11(player, server, port, clientVersion) {}

AbstractClientV13::~AbstractClientV13() {}

AbstractClientV11::AbstractClientV11(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV09(player, pngData, pngDataLen, server, port, clientVersion) {}

AbstractClientV11::AbstractClientV11(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion, const IBase64 *)
	: AbstractClientV09(player, pngData, pngDataLen, server, port, clientVersion) {}

AbstractClientV11::AbstractClientV11(const std::string &player, const std::string &server,
									 uint16_t port, uint32_t clientVersion, const IBase64 *)
	: AbstractClientV09(player, server, port, clientVersion) {}

AbstractClientV11::AbstractClientV11(const std::string &player, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV09(player, server, port, clientVersion) {}

AbstractClientV11::~AbstractClientV11() {}

AbstractClientV09::AbstractClientV09(const std::string &player, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV08(player, server, port, clientVersion) {}

AbstractClientV09::AbstractClientV09(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV08(player, pngData, pngDataLen, server, port, clientVersion) {}

AbstractClientV09::~AbstractClientV09() {}

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

	if(_pimpl->m_playing) {
		throw Exception::CapabilitiesException("attempt to get capabilities in running game");
	}

	_pimpl->m_connection.setTimeout(timeout);
	return _pimpl->m_connection.capabilities();
}

Connection::PLAYERLIST AbstractClientV05::playerList(timeval *timeout)
throw(NetMauMau::Common::Exception::SocketException) {

	const PLAYERINFOS &pi(playerList(false, timeout));
	PLAYERLIST pl;

	if(pi.size() <= pl.max_size()) pl.reserve(pi.size());

	std::transform(pi.begin(), pi.end(), std::back_inserter(pl),
				   std::mem_fun_ref(&PLAYERINFOS::value_type::getName));

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
	_pimpl->m_connection.connect(this, _pimpl->m_pngData.data(), _pimpl->m_pngData.size());

	AbstractClientV05Impl::PNGDATA().swap(_pimpl->m_pngData);

	std::size_t cturn = 0u;
	bool initCardShown = false;
	std::string msg, cjackSuit;
	const NetMauMau::Common::ICard *lastPlayedCard = 0;

	_playInternalParams pip(msg, &cturn, &initCardShown, cjackSuit, &lastPlayedCard);

	_pimpl->m_playing = true;

	while(!_pimpl->m_disconnectNow) {

		try {

			_pimpl->m_connection >> pip.msg;

			if(!_pimpl->m_disconnectNow && !pip.msg.empty()) {

				bool brk = false;

				switch(playInternal(pip)) {
				case BREAK:
					brk = true;
					break;

				case NOT_UNDERSTOOD:
					logDebug("Client library: " << __PRETTY_FUNCTION__ << ": " << pip.msg);
					unknownServerMessage(pip.msg);
					break;

				default:
					break;
				}

				if(brk) break;
			}

		} catch(const Exception::InterceptedErrorException &e) {

			if(!_pimpl->m_disconnectNow) checkedError(e.what());

			break;

		} catch(const NetMauMau::Common::Exception::SocketException &) {
			if(!_pimpl->m_disconnectNow) throw;
		}
	}

	_pimpl->m_playing = false;
	_pimpl->m_disconnectNow = false;
}

AbstractClientV05::PIRET AbstractClientV13::playInternal(const _playInternalParams &p)
throw(NetMauMau::Common::Exception::SocketException) {

	PIRET ret = AbstractClientV11::playInternal(p);

	if(ret == NOT_UNDERSTOOD) {
		if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::DIRCHANGE) {
			directionChanged();
		} else {
			return NOT_UNDERSTOOD;
		}

		return OK;
	}

	return ret;
}

AbstractClientV05::PIRET AbstractClientV07::playInternal(const _playInternalParams &p)
throw(NetMauMau::Common::Exception::SocketException) {

	PIRET ret = AbstractClientV05::playInternal(p);

	if(ret == NOT_UNDERSTOOD) {
		if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::ACEROUND) {
			_pimpl->m_connection << (getAceRoundChoice() ? NetMauMau::Common::Protocol::V15::TRUE :
									 NetMauMau::Common::Protocol::V15::FALSE);
		} else if(!_pimpl->m_disconnectNow && p.msg ==
				  NetMauMau::Common::Protocol::V15::ACEROUNDSTARTED) {
			_pimpl->m_connection >> p.msg;
			aceRoundStarted(p.msg);
		} else if(!_pimpl->m_disconnectNow && p.msg ==
				  NetMauMau::Common::Protocol::V15::ACEROUNDENDED) {
			_pimpl->m_connection >> p.msg;
			aceRoundEnded(p.msg);
		} else {
			return NOT_UNDERSTOOD;
		}

		return OK;
	}

	return ret;
}

AbstractClientV05::PIRET AbstractClientV05::playInternal(const _playInternalParams &p)
throw(NetMauMau::Common::Exception::SocketException) {

	if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::MESSAGE) {
		_pimpl->m_connection >> p.msg;
		message(p.msg);
	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::ERROR) {
		_pimpl->m_connection >> p.msg;
		checkedError(p.msg);
		return BREAK;
	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::TURN) {

		_pimpl->m_connection >> p.msg;

		*p.cturn = std::strtoul(p.msg.c_str(), NULL, 10);

		turn(*p.cturn);

	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::NEXTPLAYER) {
		_pimpl->m_connection >> p.msg;
		nextPlayer(p.msg);
	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::STATS) {

		_pimpl->m_connection >> p.msg;

		STATS cstats;

		while(p.msg != NetMauMau::Common::Protocol::V15::ENDSTATS) {

			std::string cntS;
			_pimpl->m_connection >> cntS;

			const STAT stat = { p.msg, std::strtoul(cntS.c_str(), NULL, 10) };
			cstats.push_back(stat);

			_pimpl->m_connection >> p.msg;
		}

		stats(cstats);

	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::PLAYERJOINED) {

		std::string plPic;

		_pimpl->m_connection >> p.msg;

		beginReceivePlayerPicture(p.msg);

		_pimpl->m_connection >> plPic;

		const std::vector<unsigned char> &plPicPng(NetMauMau::Common::base64_decode(plPic));

		endReceivePlayerPicture(p.msg);

		const bool hasPlPic = (!plPicPng.empty() && plPic != '-');

		playerJoined(p.msg, hasPlPic ? plPicPng.data() : 0L,
					 hasPlPic ? plPicPng.size() : 0);

	} else if(!_pimpl->m_disconnectNow && p.msg ==
			  NetMauMau::Common::Protocol::V15::PLAYERREJECTED) {
		_pimpl->m_connection >> p.msg;
		playerRejected(p.msg);
		return BREAK;
	} else if(!_pimpl->m_disconnectNow && p.msg.substr(0, 10) ==
			  NetMauMau::Common::Protocol::V15::PLAYERWINS) {

		const bool ultimate = p.msg.length() > 10 && p.msg[10] == '+';

		_pimpl->m_connection >> p.msg;
		playerWins(p.msg, *p.cturn);

		if(!ultimate) {
			gameOver();
			return BREAK;
		}

	} else if(!_pimpl->m_disconnectNow && p.msg.substr(0, 10) ==
			  NetMauMau::Common::Protocol::V15::PLAYERLOST) {
		std::string pl, pc;
		_pimpl->m_connection >> pl >> pc;
		playerLost(pl, *p.cturn, std::strtoul(pc.c_str(), NULL, 10));
	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::GETCARDS) {

		_pimpl->m_connection >> p.msg;

		const CARDS::size_type cnt = _pimpl->m_cards.empty() ? 0 : _pimpl->m_cards.size();

		while(p.msg != NetMauMau::Common::Protocol::V15::CARDSGOT) {
			_pimpl->m_cards.push_back((NetMauMau::Client::CardFactory(p.msg)).create());
			_pimpl->m_connection >> p.msg;
		}

		cardSet(_pimpl->getCards(_pimpl->m_cards, cnt));

	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::INITIALCARD) {

		_pimpl->m_connection >> p.msg;

		const NetMauMau::Common::ICard *ic = (NetMauMau::Client::CardFactory(p.msg)).create();

		if(ic == NetMauMau::Common::ICard::JACK || ic == NetMauMau::Common::ICard::EIGHT) {
			initialCard(ic);
			*p.initCardShown = true;
		}

		delete ic;

	} else if(!_pimpl->m_disconnectNow && p.msg ==
			  NetMauMau::Common::Protocol::V15::TALONSHUFFLED) {
		talonShuffled();
	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::OPENCARD) {

		_pimpl->m_connection >> p.msg;
		delete _pimpl->m_openCard;
		_pimpl->m_openCard = (NetMauMau::Client::CardFactory(p.msg)).create();

		if(!*p.initCardShown) {

			assert(NetMauMau::Common::symbolToSuit(p.cjackSuit)
				   != NetMauMau::Common::ICard::SUIT_ILLEGAL);

			openCard(_pimpl->m_openCard, p.cjackSuit);

		} else {
			*p.initCardShown = false;
		}

	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::PLAYCARD) {
		*p.lastPlayedCard = playCard(_pimpl->recvPossibleCards(p.msg));
		_pimpl->sendPlayedCard(p.lastPlayedCard);
	} else if(!_pimpl->m_disconnectNow && !p.msg.compare(0, 8,
			  std::string(NetMauMau::Common::Protocol::V15::SUSPEND).append(1, ' '))) {
		enableSuspend(!p.msg.compare(8, std::string::npos, NetMauMau::Common::Protocol::V15::ON));
	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::SUSPENDS) {
		_pimpl->m_connection >> p.msg;
		playerSuspends(p.msg);
	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::CARDACCEPTED) {

		_pimpl->m_connection >> p.msg;

		if(*p.lastPlayedCard) {
			const CARDS::iterator &f(std::find_if(_pimpl->m_cards.begin(), _pimpl->m_cards.end(),
												  std::bind2nd(NetMauMau::Common::equalTo
														  <CARDS::iterator::value_type>(),
														  *p.lastPlayedCard)));

			if(f != _pimpl->m_cards.end()) {
				cardAccepted(*f);
				delete *f;
				_pimpl->m_cards.erase(f);
			}
		}

	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::CARDREJECTED) {

		std::string player;
		_pimpl->m_connection >> player >> p.msg;

		const NetMauMau::Common::ICard *c = (NetMauMau::Client::CardFactory(p.msg)).create();
		cardRejected(player, c);
		delete c;

	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::CARDCOUNT) {

		char cc[256];
#ifndef _WIN32
		std::snprintf(cc, 255, "%zu", _pimpl->m_cards.size());
#else
		std::snprintf(cc, 255, "%lu", (unsigned long)_pimpl->m_cards.size());
#endif

		_pimpl->m_connection << cc;

	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::PLAYEDCARD) {

		std::string player;
		_pimpl->m_connection >> player >> p.msg;

		const NetMauMau::Common::ICard *c = (NetMauMau::Client::CardFactory(p.msg)).create();
		playedCard(player, c);
		delete c;

		p.cjackSuit.clear();

	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::JACKSUIT) {

		_pimpl->m_connection >> p.msg;
		p.cjackSuit = p.msg;

		assert(NetMauMau::Common::symbolToSuit(p.cjackSuit)
			   != NetMauMau::Common::ICard::SUIT_ILLEGAL);

		jackSuit(NetMauMau::Common::symbolToSuit(p.cjackSuit));

	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::JACKMODEOFF) {
		jackSuit(NetMauMau::Common::ICard::SUIT_ILLEGAL);
	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::JACKCHOICE) {

		const NetMauMau::Common::ICard::SUIT s = getJackSuitChoice();

		assert(s != NetMauMau::Common::ICard::SUIT_ILLEGAL);

		_pimpl->m_connection << NetMauMau::Common::suitToSymbol(s, false);

	} else if(!_pimpl->m_disconnectNow && p.msg ==
			  NetMauMau::Common::Protocol::V15::PLAYERPICKSCARD) {

		std::string player, extra;
		_pimpl->m_connection >> player >> extra;

		if(extra == NetMauMau::Common::Protocol::V15::CARDTAKEN) {
			_pimpl->m_connection >> p.msg;
			const NetMauMau::Common::ICard *c = (NetMauMau::Client::CardFactory(p.msg)).create();
			playerPicksCard(player, c);
			delete c;
		} else {
			playerPicksCard(player, static_cast<NetMauMau::Common::ICard *>(0L));
		}

	} else if(!_pimpl->m_disconnectNow && p.msg ==
			  NetMauMau::Common::Protocol::V15::PLAYERPICKSCARDS) {

		std::string player;
		_pimpl->m_connection >> player >> p.msg;
		_pimpl->m_connection >> p.msg;

		playerPicksCard(player, std::strtoul(p.msg.c_str(), NULL, 10));

	} else if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::BYE) {
		gameOver();
		return BREAK;
	} else {
		return NOT_UNDERSTOOD;
	}

	return OK;
}

AbstractClientV05::PIRET AbstractClientV08::playInternal(const _playInternalParams &p)
throw(NetMauMau::Common::Exception::SocketException) {

	if(!_pimpl->m_disconnectNow && p.msg == NetMauMau::Common::Protocol::V15::PLAYCARD) {

		const NetMauMau::Client::AbstractClient::CARDS &possCards(_pimpl->recvPossibleCards(p.msg));

		std::string tc;
		_pimpl->m_connection >> tc;

		*p.lastPlayedCard = playCard(possCards, std::strtoul(tc.c_str(), NULL, 10));
		_pimpl->sendPlayedCard(p.lastPlayedCard);

	} else {
		return NetMauMau::Client::AbstractClientV07::playInternal(p);
	}

	return OK;
}

bool AbstractClientV05::isLostConnMsg(const std::string &msg) {
	return (msg.find(NetMauMau::Common::Protocol::V15::ERR_TO_EXC_LOSTCONN) != std::string::npos) ||
		   (msg.find(NetMauMau::Common::Protocol::V15::ERR_TO_EXC_LOSTCONNNAMED) !=
			std::string::npos);
}

bool AbstractClientV05::isShutdownMsg(const std::string &msg) {
	return (msg.find(NetMauMau::Common::Protocol::V15::ERR_TO_EXC_SHUTDOWNMSG) != std::string::npos)
		   || (msg.find(NetMauMau::Common::Protocol::V15::ERR_TO_EXC_MISCONFIGURED) !=
			   std::string::npos);
}

void AbstractClientV05::checkedError(const std::string &msg) const
throw(NetMauMau::Common::Exception::SocketException) {

	logDebug("Client library: " << __PRETTY_FUNCTION__ << ": " << msg);

	if(isShutdownMsg(msg)) throw NetMauMau::Client::Exception::ShutdownException(msg);

	if(isLostConnMsg(msg)) throw NetMauMau::Client::Exception::LostConnectionException(msg);

	error(msg);
}

uint32_t AbstractClientV05::getClientProtocolVersion() {
	return MAKE_VERSION(SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR);
}

uint32_t AbstractClientV05::getClientLibraryVersion() {
	return MAKE_VERSION_REL(SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR, SERVER_VERSION_RELEASE);
}

uint32_t AbstractClientV05::parseProtocolVersion(const std::string &ver) {

	const std::string::size_type p1 = ver.find('.');
	const std::string::size_type p2 = ver.find('.', p1 + 1);

	if(p1 != std::string::npos && p2 == std::string::npos) {
		return MAKE_VERSION(std::strtoul(ver.substr(0, p1).c_str(), NULL, 10),
							std::strtoul(ver.substr(p1 + 1).c_str(), NULL, 10));
	} else if(p1 != std::string::npos) {
		return MAKE_VERSION_REL(std::strtoul(ver.substr(0, p1).c_str(), NULL, 10),
								std::strtoul(ver.substr(p1 + 1, p2).c_str(), NULL, 10),
								std::strtoul(ver.substr(p2 + 1).c_str(), NULL, 10));
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

bool AbstractClientV11::isPlayerImageUploadable(const unsigned char *pngData,
		std::size_t pngDataLen, const IBase64 *) {
	return AbstractClientV05::isPlayerImageUploadable(pngData, pngDataLen);
}

void AbstractClientV05::beginReceivePlayerPicture(const std::string &) const throw() {}
void AbstractClientV05::endReceivePlayerPicture(const std::string &) const throw() {}
void AbstractClientV05::uploadSucceded(const std::string &) const throw() {}
void AbstractClientV05::uploadFailed(const std::string &) const throw() {}

NetMauMau::Common::ICard *AbstractClientV08::playCard(const AbstractClientV05::CARDS &cards) const {
	return playCard(cards, 0);
}

AbstractClientV09::SCORES AbstractClientV09::getScores(timeval *timeout)
throw(NetMauMau::Common::Exception::SocketException) {
	return getScores(SCORE_TYPE::ABS, 10, timeout);
}

AbstractClientV09::SCORES AbstractClientV09::getScores(SCORE_TYPE::_scoreType type,
		std::size_t limit, timeval *timeout) throw(NetMauMau::Common::Exception::SocketException) {

	if(_pimpl->m_playing) throw Exception::ScoresException("Attempt to get scores in running game");

	_pimpl->m_connection.setTimeout(timeout);
	return _pimpl->m_connection.getScores(type, limit);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
