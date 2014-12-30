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

#ifndef NETMAUMAU_ABSTRACTCLIENTV05IMPL_H
#define NETMAUMAU_ABSTRACTCLIENTV05IMPL_H

#include "abstractclient.h"

namespace NetMauMau {

namespace Client {

class AbstractClientV05Impl {
	DISALLOW_COPY_AND_ASSIGN(AbstractClientV05Impl)
public:
	AbstractClientV05Impl(const std::string &pName, const std::string &server, uint16_t port,
						  const unsigned char *pngData, std::size_t pngDataLen);

	~AbstractClientV05Impl();

	AbstractClient::CARDS getCards(const AbstractClient::CARDS &mCards,
								   AbstractClient::CARDS::size_type cnt = 0);

	AbstractClient::CARDS recvPossibleCards(std::string &msg)
	throw(NetMauMau::Common::Exception::SocketException);

	void sendPlayedCard(const NetMauMau::Common::ICard **lastPlayedCard)
	throw(NetMauMau::Common::Exception::SocketException);

public:
	Connection m_connection;
	const std::string m_pName;
	unsigned char *m_pngData;
	std::size_t m_pngDataLen;
	AbstractClient::CARDS m_cards;
	const Common::ICard *m_openCard;
	bool m_disconnectNow;
};

}

}

#endif /* NETMAUMAU_ABSTRACTCLIENTV05IMPL_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
