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

#include "abstractclientv05impl.h"

#include <algorithm>                    // for find_if

#include "logger.h"
#include "protocol.h"                   // for PLAYCARDEND, SUSPEND

namespace {
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct cardEqualsDescription : std::binary_function < NetMauMau::Common::ICard *,
		std::string, bool > {
	inline result_type operator()(const first_argument_type c,
								  const second_argument_type &d) const {
		return c->description() == d;
	}
};

#pragma GCC diagnostic pop
}

using namespace NetMauMau::Client;

const MappedMessageProcessor<AbstractClientV05>::value_type
MappedMessageInitializer<AbstractClientV05>::m_data[23] _INIT_PRIO(102) = {
	std::make_pair(&NetMauMau::Common::Protocol::V15::BYE,
	&AbstractClientV05::performBye),
	std::make_pair(&NetMauMau::Common::Protocol::V15::CARDACCEPTED,
	&AbstractClientV05::performCardAccepted),
	std::make_pair(&NetMauMau::Common::Protocol::V15::CARDCOUNT,
	&AbstractClientV05::performCardCount),
	std::make_pair(&NetMauMau::Common::Protocol::V15::CARDREJECTED,
	&AbstractClientV05::performCardRejected),
	std::make_pair(&NetMauMau::Common::Protocol::V15::ERROR,
	&AbstractClientV05::performError),
	std::make_pair(&NetMauMau::Common::Protocol::V15::GETCARDS,
	&AbstractClientV05::performGetCards),
	std::make_pair(&NetMauMau::Common::Protocol::V15::INITIALCARD,
	&AbstractClientV05::performInitialCard),
	std::make_pair(&NetMauMau::Common::Protocol::V15::JACKCHOICE,
	&AbstractClientV05::performJackChoice),
	std::make_pair(&NetMauMau::Common::Protocol::V15::JACKMODEOFF,
	&AbstractClientV05::performJackModeOff),
	std::make_pair(&NetMauMau::Common::Protocol::V15::JACKSUIT,
	&AbstractClientV05::performJackSuit),
	std::make_pair(&NetMauMau::Common::Protocol::V15::MESSAGE,
	&AbstractClientV05::performMessage),
	std::make_pair(&NetMauMau::Common::Protocol::V15::NEXTPLAYER,
	&AbstractClientV05::performNextPlayer),
	std::make_pair(&NetMauMau::Common::Protocol::V15::OPENCARD,
	&AbstractClientV05::performOpenCard),
	std::make_pair(&NetMauMau::Common::Protocol::V15::PLAYCARD,
	&AbstractClientV05::performPlayCard),
	std::make_pair(&NetMauMau::Common::Protocol::V15::PLAYEDCARD,
	&AbstractClientV05::performPlayedCard),
	std::make_pair(&NetMauMau::Common::Protocol::V15::PLAYERJOINED,
	&AbstractClientV05::performPlayerJoined),
	std::make_pair(&NetMauMau::Common::Protocol::V15::PLAYERPICKSCARD,
	&AbstractClientV05::performPlayerPicksCard),
	std::make_pair(&NetMauMau::Common::Protocol::V15::PLAYERPICKSCARDS,
	&AbstractClientV05::performPlayerPicksCards),
	std::make_pair(&NetMauMau::Common::Protocol::V15::PLAYERREJECTED,
	&AbstractClientV05::performPlayerRejected),
	std::make_pair(&NetMauMau::Common::Protocol::V15::STATS,
	&AbstractClientV05::performStats),
	std::make_pair(&NetMauMau::Common::Protocol::V15::SUSPENDS,
	&AbstractClientV05::performSuspends),
	std::make_pair(&NetMauMau::Common::Protocol::V15::TALONSHUFFLED,
	&AbstractClientV05::performTalonShuffled),
	std::make_pair(&NetMauMau::Common::Protocol::V15::TURN,
	&AbstractClientV05::performTurn)
};

const MappedMessageProcessor<AbstractClientV07>::value_type
MappedMessageInitializer<AbstractClientV07>::m_data[3] _INIT_PRIO(102) = {
	std::make_pair(&NetMauMau::Common::Protocol::V15::ACEROUND,
	&AbstractClientV07::performAceround),
	std::make_pair(&NetMauMau::Common::Protocol::V15::ACEROUNDENDED,
	&AbstractClientV07::performAceroundEnded),
	std::make_pair(&NetMauMau::Common::Protocol::V15::ACEROUNDSTARTED,
	&AbstractClientV07::performAceroundStarted)
};

const MappedMessageProcessor<AbstractClientV08>::value_type
MappedMessageInitializer<AbstractClientV08>::m_data[1] _INIT_PRIO(102) = {
	std::make_pair(&NetMauMau::Common::Protocol::V15::PLAYCARD,
	&AbstractClientV08::performPlayCard)
};

const MappedMessageProcessor<AbstractClientV13>::value_type
MappedMessageInitializer<AbstractClientV13>::m_data[1] _INIT_PRIO(102) = {
	std::make_pair(&NetMauMau::Common::Protocol::V15::DIRCHANGE,
	&AbstractClientV13::performDirChange)
};

AbstractClientV05Impl::AbstractClientV05Impl(const std::string &pName, const std::string &server,
		uint16_t port, const unsigned char *pngData, std::size_t pngDataLen) : m_connection(pName,
					server, port), m_pName(pName), m_pngData(), m_cards(), m_openCard(0L),
	m_disconnectNow(false), m_playing(false) {

	if(pngData && pngDataLen) {

		if(m_pngData.max_size() >= pngDataLen) {
			m_pngData.reserve(pngDataLen);
			m_pngData.insert(m_pngData.end(), pngData, pngData + pngDataLen);
		} else {
			logDebug(__PRETTY_FUNCTION__ << " clearing m_pngData");
			PNGDATA().swap(m_pngData);
		}
	}
}

AbstractClientV05Impl::~AbstractClientV05Impl() {

	m_connection.setInterrupted(false);

	for(AbstractClient::CARDS::const_iterator
			i(m_cards.begin()); i != m_cards.end(); ++i) delete *i;

	delete m_openCard;
}

NetMauMau::Client::AbstractClient::CARDS AbstractClientV05Impl::recvPossibleCards(std::string &msg)
throw(NetMauMau::Common::Exception::SocketException) {

	const NetMauMau::Client::AbstractClient::CARDS &myCards(getCards(m_cards));
	NetMauMau::Client::AbstractClient::CARDS possCards;

	if(myCards.size() < possCards.max_size()) possCards.reserve(myCards.size());

	m_connection >> msg;

	while(msg != NetMauMau::Common::Protocol::V15::PLAYCARDEND) {

		const AbstractClient::CARDS::const_iterator &f(std::find_if(myCards.begin(),
				myCards.end(), std::bind2nd(cardEqualsDescription(), msg)));

		if(f != myCards.end()) possCards.push_back(*f);

		m_connection >> msg;
	}

	return possCards;
}

void AbstractClientV05Impl::sendPlayedCard(const NetMauMau::Common::ICard **lastPlayedCard)
throw(NetMauMau::Common::Exception::SocketException) {

	if(*lastPlayedCard) {
		m_connection << (*lastPlayedCard)->description();
	} else {
		m_connection << NetMauMau::Common::Protocol::V15::SUSPEND;
	}
}

NetMauMau::Client::AbstractClient::CARDS
AbstractClientV05Impl::getCards(const AbstractClient::CARDS &mCards,
								AbstractClient::CARDS::size_type cnt) {

	AbstractClient::CARDS cards;

	if(!mCards.empty()) {

		if(mCards.size() <= cards.max_size()) cards.reserve(mCards.size());

		AbstractClient::CARDS::const_iterator i(mCards.begin());
		std::advance(i, cnt);

		cards.insert(cards.end(), i, mCards.end());
	}

	return cards;
}

const IBase64 *AbstractClientV05Impl::getBase64() {
	return 0L;
}

void AbstractClientV05Impl::setBase64(const IBase64 *) {}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
