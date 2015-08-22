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
#include "tcpopt_cork.h"
#include "tcpopt_nodelay.h"
#include "abstractclientv05impl.h"      // for AbstractClientV05Impl
#include "logger.h"                     // for BasicLogger, logDebug
#include "capabilitiesexception.h"      // for CapabilitiesException
#include "cardtools.h"                  // for symbolToSuit, suitToSymbol, etc
#include "clientcardfactory.h"          // for CardFactory
#include "interceptederrorexception.h"  // for InterceptedErrorException
#include "pngcheck.h"                   // for checkPNG
#include "shutdownexception.h"
#include "remoteplayerexception.h"
#include "lostconnectionexception.h"
#include "protocol.h"                   // for PLAYCARD, ACEROUND, etc

using namespace NetMauMau::Client;

AbstractClientV13::AbstractClientV13(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV11(player, pngData, pngDataLen, server, port, clientVersion),
	  m_mmp(new MappedMessageProcessor<AbstractClientV13, 1u>(*this, *_pimpl)) {}

AbstractClientV13::AbstractClientV13(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion, unsigned char sockopts)
	: AbstractClientV11(player, pngData, pngDataLen, server, port, clientVersion, sockopts),
	  m_mmp(new MappedMessageProcessor<AbstractClientV13, 1u>(*this, *_pimpl)) {}

AbstractClientV13::AbstractClientV13(const std::string &player, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV11(player, server, port, clientVersion),
	  m_mmp(new MappedMessageProcessor<AbstractClientV13, 1u>(*this, *_pimpl)) {}

AbstractClientV13::AbstractClientV13(const std::string &player, const std::string &server,
									 uint16_t port, uint32_t clientVersion, unsigned char sockopts)
	: AbstractClientV11(player, server, port, clientVersion, sockopts),
	  m_mmp(new MappedMessageProcessor<AbstractClientV13, 1u>(*this, *_pimpl)) {}

AbstractClientV13::AbstractClientV13(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion, const IBase64 *)
	: AbstractClientV11(player, pngData, pngDataLen, server, port, clientVersion,
						static_cast<const IBase64 *>(0L)),
	m_mmp(new MappedMessageProcessor<AbstractClientV13, 1u>(*this, *_pimpl)) {}

AbstractClientV13::AbstractClientV13(const std::string &player, const std::string &server,
									 uint16_t port, uint32_t clientVersion, const IBase64 *)
	: AbstractClientV11(player, server, port, clientVersion, static_cast<const IBase64 *>(0L)),
	  m_mmp(new MappedMessageProcessor<AbstractClientV13, 1u>(*this, *_pimpl)) {}

AbstractClientV13::~AbstractClientV13() {
	delete m_mmp;
}

AbstractClientV11::AbstractClientV11(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV09(player, pngData, pngDataLen, server, port, clientVersion) {}

AbstractClientV11::AbstractClientV11(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion, unsigned char sockopts)
	: AbstractClientV09(player, pngData, pngDataLen, server, port, clientVersion, sockopts) {}

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

AbstractClientV11::AbstractClientV11(const std::string &player, const std::string &server,
									 uint16_t port, uint32_t clientVersion, unsigned char sockopts)
	: AbstractClientV09(player, server, port, clientVersion, sockopts) {}

AbstractClientV11::~AbstractClientV11() {}

AbstractClientV09::AbstractClientV09(const std::string &player, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV08(player, server, port, clientVersion) {}

AbstractClientV09::AbstractClientV09(const std::string &player, const std::string &server,
									 uint16_t port, uint32_t clientVersion, unsigned char sockopts)
	: AbstractClientV08(player, server, port, clientVersion, sockopts) {}

AbstractClientV09::AbstractClientV09(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV08(player, pngData, pngDataLen, server, port, clientVersion) {}

AbstractClientV09::AbstractClientV09(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion, unsigned char sockopts)
	: AbstractClientV08(player, pngData, pngDataLen, server, port, clientVersion, sockopts) {}

AbstractClientV09::~AbstractClientV09() {}

AbstractClientV08::AbstractClientV08(const std::string &player, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV07(player, server, port),
	  m_mmp(new MappedMessageProcessor<AbstractClientV08, 1u>(*this, *_pimpl)) {
	_pimpl->m_connection.setClientVersion(clientVersion);
}

AbstractClientV08::AbstractClientV08(const std::string &player, const std::string &server,
									 uint16_t port, uint32_t clientVersion, unsigned char sockopts)
	: AbstractClientV07(player, server, port, sockopts),
	  m_mmp(new MappedMessageProcessor<AbstractClientV08, 1u>(*this, *_pimpl)) {
	_pimpl->m_connection.setClientVersion(clientVersion);
}

AbstractClientV08::AbstractClientV08(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion)
	: AbstractClientV07(player, pngData, pngDataLen, server, port),
	  m_mmp(new MappedMessageProcessor<AbstractClientV08, 1u>(*this, *_pimpl)) {
	_pimpl->m_connection.setClientVersion(clientVersion);
}

AbstractClientV08::AbstractClientV08(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, uint32_t clientVersion, unsigned char sockopts)
	: AbstractClientV07(player, pngData, pngDataLen, server, port, sockopts),
	  m_mmp(new MappedMessageProcessor<AbstractClientV08, 1u>(*this, *_pimpl)) {
	_pimpl->m_connection.setClientVersion(clientVersion);
}

AbstractClientV08::~AbstractClientV08() {
	delete m_mmp;
}

AbstractClientV07::AbstractClientV07(const std::string &player, const std::string &server,
									 uint16_t port) : AbstractClientV05(player, server, port),
	m_mmp(new MappedMessageProcessor<AbstractClientV07, 3u>(*this, *_pimpl)) {
	_pimpl->m_connection.setClientVersion(7);
}

AbstractClientV07::AbstractClientV07(const std::string &player, const std::string &server,
									 uint16_t port, unsigned char sockopts)
	: AbstractClientV05(player, server, port, sockopts),
	  m_mmp(new MappedMessageProcessor<AbstractClientV07, 3u>(*this, *_pimpl)) {
	_pimpl->m_connection.setClientVersion(7);
}

AbstractClientV07::AbstractClientV07(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port) : AbstractClientV05(player, pngData, pngDataLen,
												 server, port),
	m_mmp(new MappedMessageProcessor<AbstractClientV07, 3u>(*this, *_pimpl)) {
	_pimpl->m_connection.setClientVersion(7);
}

AbstractClientV07::AbstractClientV07(const std::string &player, const unsigned char *pngData,
									 std::size_t pngDataLen, const std::string &server,
									 uint16_t port, unsigned char sockopts)
	: AbstractClientV05(player, pngData, pngDataLen, server, port, sockopts),
	  m_mmp(new MappedMessageProcessor<AbstractClientV07, 3u>(*this, *_pimpl)) {
	_pimpl->m_connection.setClientVersion(7);
}

AbstractClientV07::~AbstractClientV07() {
	delete m_mmp;
}

AbstractClientV05::AbstractClientV05(const std::string &pName, const unsigned char *data,
									 std::size_t len, const std::string &server, uint16_t port)
	: IPlayerPicListener(), _pimpl(new AbstractClientV05Impl(pName, server, port, data, len,
								   SOCKOPT_ALL)),
	m_mmp(new MappedMessageProcessor<AbstractClientV05, 23u>(*this, *_pimpl)) {}

AbstractClientV05::AbstractClientV05(const std::string &pName, const unsigned char *data,
									 std::size_t len, const std::string &server, uint16_t port,
									 unsigned char sockopts) : IPlayerPicListener(),
	_pimpl(new AbstractClientV05Impl(pName, server, port, data, len, sockopts)),
	m_mmp(new MappedMessageProcessor<AbstractClientV05, 23u>(*this, *_pimpl)) {}


AbstractClientV05::AbstractClientV05(const std::string &pName, const std::string &server,
									 uint16_t port) : IPlayerPicListener(),
	_pimpl(new AbstractClientV05Impl(pName, server, port, 0L, 0, SOCKOPT_ALL)),
	m_mmp(new MappedMessageProcessor<AbstractClientV05, 23u>(*this, *_pimpl)) {}

AbstractClientV05::AbstractClientV05(const std::string &pName, const std::string &server,
									 uint16_t port, unsigned char sockopts) : IPlayerPicListener(),
	_pimpl(new AbstractClientV05Impl(pName, server, port, 0L, 0, sockopts)),
	m_mmp(new MappedMessageProcessor<AbstractClientV05, 23u>(*this, *_pimpl)) {}

AbstractClientV05::~AbstractClientV05() {
	delete m_mmp;
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

	{
		TCPOPT_CORK(_pimpl->m_connection.getSocketFD());

		_pimpl->m_connection.setTimeout(timeout);
		_pimpl->m_connection.connect(this, _pimpl->m_pngData.data(), _pimpl->m_pngData.size());
	}

	TCPOPT_NODELAY(_pimpl->m_connection.getSocketFD());

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

AbstractClient::PIRET AbstractClientV13::performDirChange(const _playInternalParams &) const {
	directionChanged();
	return OK;
}

AbstractClientV05::PIRET AbstractClientV13::playInternal(const _playInternalParams &p)
throw(NetMauMau::Common::Exception::SocketException) {
	PIRET  ret = AbstractClientV11::playInternal(p);
	return ret == NOT_UNDERSTOOD ? m_mmp->process(p) : ret;
}

AbstractClient::PIRET AbstractClientV07::performAceround(const _playInternalParams &) const {

	_pimpl->m_connection << (getAceRoundChoice() ? NetMauMau::Common::Protocol::V15::TRUE :
							 NetMauMau::Common::Protocol::V15::FALSE);

	return OK;
}

AbstractClient::PIRET
AbstractClientV07::performAceroundStarted(const _playInternalParams &p) const {

	_pimpl->m_connection >> p.msg;
	aceRoundStarted(p.msg);

	return OK;
}

AbstractClient::PIRET AbstractClientV07::performAceroundEnded(const _playInternalParams &p) const {

	_pimpl->m_connection >> p.msg;
	aceRoundEnded(p.msg);

	return OK;
}

AbstractClientV05::PIRET AbstractClientV07::playInternal(const _playInternalParams &p)
throw(NetMauMau::Common::Exception::SocketException) {
	PIRET  ret = AbstractClientV05::playInternal(p);
	return ret == NOT_UNDERSTOOD ? m_mmp->process(p) : ret;
}

AbstractClientV05::PIRET AbstractClientV05::performMessage(const _playInternalParams &p) const {

	_pimpl->m_connection >> p.msg;
	message(p.msg);

	return OK;
}

AbstractClientV05::PIRET AbstractClientV05::performError(const _playInternalParams &p) const
throw(NetMauMau::Common::Exception::SocketException) {

	_pimpl->m_connection >> p.msg;
	checkedError(p.msg);

	return BREAK;
}

AbstractClientV05::PIRET AbstractClientV05::performTurn(const _playInternalParams &p) const {

	_pimpl->m_connection >> p.msg;
	*p.cturn = std::strtoul(p.msg.c_str(), NULL, 10);
	turn(*p.cturn);

	return OK;
}

AbstractClientV05::PIRET AbstractClientV05::performNextPlayer(const _playInternalParams &p) const {

	_pimpl->m_connection >> p.msg;
	nextPlayer(p.msg);

	return OK;
}

AbstractClientV05::PIRET AbstractClientV05::performStats(const _playInternalParams &p) const {

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

	return OK;
}

AbstractClientV05::PIRET
AbstractClientV05::performPlayerJoined(const _playInternalParams &p) const {

	std::string plPic;

	_pimpl->m_connection >> p.msg;

	beginReceivePlayerPicture(p.msg);

	_pimpl->m_connection >> plPic;

	const std::vector<unsigned char> &plPicPng(NetMauMau::Common::base64_decode(plPic));

	endReceivePlayerPicture(p.msg);

	const bool hasPlPic = (!plPicPng.empty() && plPic != '-');

	playerJoined(p.msg, hasPlPic ? plPicPng.data() : 0L,  hasPlPic ? plPicPng.size() : 0);

	return OK;
}

AbstractClientV05::PIRET
AbstractClientV05::performPlayerRejected(const _playInternalParams &p) const {

	_pimpl->m_connection >> p.msg;
	playerRejected(p.msg);

	return BREAK;
}

AbstractClientV05::PIRET AbstractClientV05::performGetCards(const _playInternalParams &p) const {

	_pimpl->m_connection >> p.msg;

	const CARDS::size_type cnt = _pimpl->m_cards.empty() ? 0 : _pimpl->m_cards.size();

	while(p.msg != NetMauMau::Common::Protocol::V15::CARDSGOT) {
		_pimpl->m_cards.push_back((NetMauMau::Client::CardFactory(p.msg)).create());
		_pimpl->m_connection >> p.msg;
	}

	cardSet(_pimpl->getCards(_pimpl->m_cards, cnt));

	return OK;
}

AbstractClientV05::PIRET AbstractClientV05::performInitialCard(const _playInternalParams &p) const {

	_pimpl->m_connection >> p.msg;

	const NetMauMau::Common::ICard *ic = (NetMauMau::Client::CardFactory(p.msg)).create();

	if(ic == NetMauMau::Common::ICard::JACK || ic == NetMauMau::Common::ICard::EIGHT) {
		initialCard(ic);
		*p.initCardShown = true;
	}

	delete ic;

	return OK;
}

AbstractClientV05::PIRET
AbstractClientV05::performTalonShuffled(const _playInternalParams &) const {
	talonShuffled();
	return OK;
}

AbstractClientV05::PIRET AbstractClientV05::performOpenCard(const _playInternalParams &p) const {

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

	return OK;
}

AbstractClientV05::PIRET AbstractClientV05::performPlayCard(const _playInternalParams &p) const
throw(NetMauMau::Common::Exception::SocketException) {

	*p.lastPlayedCard = playCard(_pimpl->recvPossibleCards(p.msg));

	_pimpl->sendPlayedCard(p.lastPlayedCard);

	return OK;
}

AbstractClientV05::PIRET AbstractClientV05::performSuspends(const _playInternalParams &p) const {

	_pimpl->m_connection >> p.msg;
	playerSuspends(p.msg);

	return OK;
}

AbstractClientV05::PIRET
AbstractClientV05::performCardAccepted(const _playInternalParams &p) const {

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

	return OK;
}

AbstractClientV05::PIRET
AbstractClientV05::performCardRejected(const _playInternalParams &p) const {

	std::string player;
	_pimpl->m_connection >> player >> p.msg;

	const NetMauMau::Common::ICard *c = (NetMauMau::Client::CardFactory(p.msg)).create();
	cardRejected(player, c);
	delete c;

	return OK;
}

AbstractClientV05::PIRET AbstractClientV05::performCardCount(const _playInternalParams &) const {

	char cc[256];
#ifndef _WIN32
	std::snprintf(cc, 255, "%zu", _pimpl->m_cards.size());
#else
	std::snprintf(cc, 255, "%lu", (unsigned long)_pimpl->m_cards.size());
#endif

	_pimpl->m_connection << cc;

	return OK;
}

AbstractClientV05::PIRET AbstractClientV05::performPlayedCard(const _playInternalParams &p) const {

	std::string player;
	_pimpl->m_connection >> player >> p.msg;

	const NetMauMau::Common::ICard *c = (NetMauMau::Client::CardFactory(p.msg)).create();
	playedCard(player, c);
	delete c;

	p.cjackSuit.clear();

	return OK;
}

AbstractClientV05::PIRET AbstractClientV05::performJackSuit(const _playInternalParams &p) const {

	_pimpl->m_connection >> p.msg;
	p.cjackSuit = p.msg;

	assert(NetMauMau::Common::symbolToSuit(p.cjackSuit)
		   != NetMauMau::Common::ICard::SUIT_ILLEGAL);

	jackSuit(NetMauMau::Common::symbolToSuit(p.cjackSuit));

	return OK;
}

AbstractClientV05::PIRET AbstractClientV05::performJackModeOff(const _playInternalParams &) const {
	jackSuit(NetMauMau::Common::ICard::SUIT_ILLEGAL);
	return OK;
}

AbstractClientV05::PIRET AbstractClientV05::performJackChoice(const _playInternalParams &) const {

	const NetMauMau::Common::ICard::SUIT s = getJackSuitChoice();

	assert(s != NetMauMau::Common::ICard::SUIT_ILLEGAL);

	_pimpl->m_connection << NetMauMau::Common::suitToSymbol(s, false);

	return OK;

}

AbstractClientV05::PIRET
AbstractClientV05::performPlayerPicksCard(const _playInternalParams &p) const {

	std::string player, extra;
	_pimpl->m_connection >> player >> extra;

	if(extra.compare(NetMauMau::Common::Protocol::V15::CARDTAKEN) == 0) {
		_pimpl->m_connection >> p.msg;
		const NetMauMau::Common::ICard *c = (NetMauMau::Client::CardFactory(p.msg)).create();
		playerPicksCard(player, c);
		delete c;
	} else {
		playerPicksCard(player, static_cast<NetMauMau::Common::ICard *>(0L));
	}

	return OK;
}

AbstractClientV05::PIRET
AbstractClientV05::performPlayerPicksCards(const _playInternalParams &p) const {

	std::string player;
	_pimpl->m_connection >> player >> p.msg;
	_pimpl->m_connection >> p.msg;

	playerPicksCard(player, std::strtoul(p.msg.c_str(), NULL, 10));

	return OK;
}

AbstractClientV05::PIRET AbstractClientV05::performBye(const _playInternalParams &) const {
	gameOver();
	return BREAK;
}

AbstractClientV05::PIRET AbstractClientV05::playInternal(const _playInternalParams &p)
throw(NetMauMau::Common::Exception::SocketException) {

	PIRET ret = m_mmp->process(p);

	if(ret == NOT_UNDERSTOOD) {

		if(!_pimpl->m_disconnectNow &&
				p.msg.compare(0, 10, NetMauMau::Common::Protocol::V15::PLAYERWINS) == 0) {

			const bool ultimate = p.msg.length() > 10 && p.msg[10] == '+';

			_pimpl->m_connection >> p.msg;
			playerWins(p.msg, *p.cturn);

			if(!ultimate) return performBye(p);

		} else if(!_pimpl->m_disconnectNow && p.msg.compare(0, 10,
				  NetMauMau::Common::Protocol::V15::PLAYERLOST) == 0) {

			std::string pl, pc;
			_pimpl->m_connection >> pl >> pc;
			playerLost(pl, *p.cturn, std::strtoul(pc.c_str(), NULL, 10));

		} else if(!_pimpl->m_disconnectNow && !p.msg.compare(0, 8,
				  std::string(NetMauMau::Common::Protocol::V15::SUSPEND).append(1, ' '))) {
			enableSuspend(!p.msg.compare(8, std::string::npos,
										 NetMauMau::Common::Protocol::V15::ON));

		} else if(!_pimpl->m_disconnectNow) {
			return NOT_UNDERSTOOD;
		}

		return OK;
	}

	return ret;
}

AbstractClientV05::PIRET AbstractClientV08::performPlayCard(const _playInternalParams &p) const
throw(NetMauMau::Common::Exception::SocketException) {

	const NetMauMau::Client::AbstractClient::CARDS &possCards(_pimpl->recvPossibleCards(p.msg));

	std::string tc;
	_pimpl->m_connection >> tc;

	*p.lastPlayedCard = playCard(possCards, std::strtoul(tc.c_str(), NULL, 10));

	_pimpl->sendPlayedCard(p.lastPlayedCard);

	return OK;
}

AbstractClientV05::PIRET AbstractClientV08::playInternal(const _playInternalParams &p)
throw(NetMauMau::Common::Exception::SocketException) {
	return m_mmp->process(p) == NOT_UNDERSTOOD ? AbstractClientV07::playInternal(p) : OK;
}

bool AbstractClientV05::isLostConnMsg(const std::string &msg) {
	return (msg.find(NetMauMau::Common::Protocol::V15::ERR_TO_EXC_LOSTCONN) != std::string::npos) ||
		   (msg.find(NetMauMau::Common::Protocol::V15::ERR_TO_EXC_LOSTCONNNAMED) !=
			std::string::npos);
}

bool AbstractClientV05::isShutdownMsg(const std::string &msg) {
	return msg.find(NetMauMau::Common::Protocol::V15::ERR_TO_EXC_SHUTDOWNMSG) != std::string::npos;
}

bool AbstractClientV05::isMisconfgMsg(const std::string &msg) {
	return msg.find(NetMauMau::Common::Protocol::V15::ERR_TO_EXC_MISCONFIGURED) !=
		   std::string::npos;
}

std::string::size_type AbstractClientV05::isRemotePlMsg(const std::string &msg) {
	return msg.find(NetMauMau::Common::Protocol::V15::ERR_TO_EXC_PLAYER);
}

void AbstractClientV05::checkedError(const std::string &msg) const
throw(NetMauMau::Common::Exception::SocketException) {

	logDebug("Client library: " << __PRETTY_FUNCTION__ << ": " << msg);

	const std::string::size_type rplexc = isRemotePlMsg(msg);

	if(rplexc != std::string::npos) {

		const std::string::size_type msgLen =
			NetMauMau::Common::Protocol::V15::ERR_TO_EXC_PLAYER.length();

		throw NetMauMau::Client::Exception::RemotePlayerException(msg.substr(rplexc + msgLen,
				msg.find(": ") - msgLen), msg.substr(msg.find(": ") + 2));
	}

	if(isMisconfgMsg(msg)) throw NetMauMau::Common::Exception::SocketException(msg);

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

	return 0u;
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
	_pimpl->m_connection.setTimeout(timeout);
	return _pimpl->m_connection.getScores(type, limit);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
