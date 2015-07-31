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

#ifndef NETMAUMAU_ABSTRACTCLIENTV05IMPL_H
#define NETMAUMAU_ABSTRACTCLIENTV05IMPL_H

#include <cstddef>                      // for size_t

#include "abstractclient.h"             // for AbstractClient

namespace NetMauMau {

namespace Client {

class IBase64;

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

class AbstractClientV05Impl {
	DISALLOW_COPY_AND_ASSIGN(AbstractClientV05Impl)
	friend class AbstractClientV05;
	typedef std::vector<unsigned char> PNGDATA;
public:
	explicit AbstractClientV05Impl(const std::string &pName, const std::string &server,
								   uint16_t port, const unsigned char *pngData,
								   std::size_t pngDataLen);
	~AbstractClientV05Impl();

	AbstractClient::CARDS getCards(const AbstractClient::CARDS &mCards,
								   AbstractClient::CARDS::size_type cnt = 0);

	AbstractClient::CARDS recvPossibleCards(std::string &msg)
	throw(NetMauMau::Common::Exception::SocketException);

	void sendPlayedCard(const NetMauMau::Common::ICard **lastPlayedCard)
	throw(NetMauMau::Common::Exception::SocketException);

	static const IBase64 *getBase64() _CONST _DEPRECATED _NOUNUSED;
	static void setBase64(const IBase64 *base64) _CONST _DEPRECATED;

public:
	Connection m_connection;
	const std::string m_pName;
	PNGDATA m_pngData;
	AbstractClient::CARDS m_cards;
	const Common::ICard *m_openCard;
	bool m_disconnectNow;
	bool m_playing;
};

template<class T>
class MappedMessageProcessor {
	DISALLOW_COPY_AND_ASSIGN(MappedMessageProcessor)

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
	typedef struct _LOCAL : std::binary_function<const std::string *, const std::string *, bool> {
		inline result_type operator()(first_argument_type x, second_argument_type y) const {
			return (x && y) ? (*x < *y) : true;
		}
	} PROTOCMP;
#pragma GCC diagnostic pop

	typedef AbstractClientV05::PIRET(T::*PROTOFN)(const _playInternalParams &) const;
	typedef std::map<const std::string *const, PROTOFN, PROTOCMP> PROTOMAP;

public:
	MappedMessageProcessor(T *t, AbstractClientV05Impl *pimpl) : _t(t), _pimpl(pimpl),
		m_protoMap() {}

	~MappedMessageProcessor() {}

	void map(const std::string &key, const PROTOFN &fn);

	AbstractClient::PIRET process(const _playInternalParams &p) const
	throw(NetMauMau::Common::Exception::SocketException) {

		const typename PROTOMAP::const_iterator &f(m_protoMap.find(&p.msg));

		if(!_pimpl->m_disconnectNow && f != m_protoMap.end()) {
			return (_t->*f->second)(p);
		}

		return !_pimpl->m_disconnectNow ? AbstractClient::NOT_UNDERSTOOD : AbstractClient::OK;
	}

private:
	T *_t;
	AbstractClientV05Impl *const _pimpl;
	PROTOMAP m_protoMap;
};

template<class T>
void MappedMessageProcessor<T>::map(const std::string &key, const PROTOFN &fn) {
	m_protoMap.insert(std::make_pair(&key, fn));
}

}

}

#endif /* NETMAUMAU_ABSTRACTCLIENTV05IMPL_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
