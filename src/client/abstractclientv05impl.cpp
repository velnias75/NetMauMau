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
#include <cstring>                      // for memcpy

#if defined(_WIN32)
#undef TRUE
#undef FALSE
#undef ERROR
#endif

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

Connection::BASE64RAII AbstractClientV05Impl::m_base64;

AbstractClientV05Impl::AbstractClientV05Impl(const std::string &pName, const std::string &server,
		uint16_t port, const unsigned char *pngData, std::size_t pngDataLen) :
	m_connection(pName, server, port, m_base64), m_pName(pName),
	m_pngData(new(std::nothrow) unsigned char[pngDataLen]()), m_pngDataLen(pngDataLen),
	m_cards(), m_openCard(0L), m_disconnectNow(false), m_playing(false) {

	if(m_pngData && m_pngDataLen) {
		std::memcpy(m_pngData, pngData, pngDataLen);
	} else {
		delete [] m_pngData;
		m_pngData = 0L;
		m_pngDataLen = 0;
	}
}

AbstractClientV05Impl::~AbstractClientV05Impl() {

	m_connection.setInterrupted(false);

	for(AbstractClient::CARDS::const_iterator
			i(m_cards.begin()); i != m_cards.end(); ++i) delete *i;

	delete m_openCard;

	delete [] m_pngData;
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
	return m_base64;
}

void AbstractClientV05Impl::setBase64(const IBase64 *base64) {
	m_base64 = base64;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
