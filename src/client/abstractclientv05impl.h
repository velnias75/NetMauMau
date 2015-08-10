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

class _LOCAL AbstractClientV05Impl {
	DISALLOW_COPY_AND_ASSIGN(AbstractClientV05Impl)
	friend class AbstractClientV05;
	typedef std::vector<unsigned char> PNGDATA;
public:
	explicit AbstractClientV05Impl(const std::string &pName, const std::string &server,
								   uint16_t port, const unsigned char *pngData,
								   std::size_t pngDataLen, unsigned char sockopts);
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

template<class> class MappedMessageInitializer;

template<class T>
class _LOCAL MappedMessageProcessor {
	DISALLOW_COPY_AND_ASSIGN(MappedMessageProcessor)

	template<class> friend class MappedMessageInitializer;

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
	struct _LOCAL _protoCmp : std::binary_function<const std::string *, const std::string *, bool> {
		inline result_type operator()(first_argument_type x,
									  second_argument_type y) const _NONNULL_ALL {
			return *x < *y;
		}
	};
#pragma GCC diagnostic pop

	typedef AbstractClientV05::PIRET(T::*PROTOFN)(const _playInternalParams &) const;
	typedef std::map<const std::string *const, PROTOFN, _protoCmp> PROTOMAP;

public:
	typedef typename PROTOMAP::value_type value_type;

	MappedMessageProcessor(const T &t, const AbstractClientV05Impl &pimpl)
		: _t(t), _pimpl(pimpl) {}

	~MappedMessageProcessor() {}

	AbstractClient::PIRET process(const _playInternalParams &p) const
	throw(NetMauMau::Common::Exception::SocketException) {

		const typename PROTOMAP::const_iterator &f(m_messageMap.m_protoMap.find(&p.msg));

		if(!_pimpl.m_disconnectNow && f != m_messageMap.m_protoMap.end()) return (_t.*f->second)(p);

		return !_pimpl.m_disconnectNow ? AbstractClient::NOT_UNDERSTOOD : AbstractClient::OK;
	}

private:
	static const MappedMessageInitializer<T> m_messageMap;

	const T &_t;
	const AbstractClientV05Impl &_pimpl;
};

template<class T> const MappedMessageInitializer<T> MappedMessageProcessor<T>::m_messageMap;

template<class T> class MappedMessageInitializer;

template<> class _LOCAL MappedMessageInitializer<AbstractClientV05> {
	DISALLOW_COPY_AND_ASSIGN(MappedMessageInitializer<AbstractClientV05>)
	friend class MappedMessageProcessor<AbstractClientV05>;
public:
	MappedMessageInitializer() : m_protoMap(m_data, m_data + 23u) {}
	~MappedMessageInitializer() {}

private:
	static const MappedMessageProcessor<AbstractClientV05>::value_type m_data[];
	const MappedMessageProcessor<AbstractClientV05>::PROTOMAP m_protoMap;
};

template<> class _LOCAL MappedMessageInitializer<AbstractClientV07> {
	DISALLOW_COPY_AND_ASSIGN(MappedMessageInitializer<AbstractClientV07>)
	friend class MappedMessageProcessor<AbstractClientV07>;
public:
	MappedMessageInitializer() : m_protoMap(m_data, m_data + 3u) {}
	~MappedMessageInitializer() {}

private:
	static const MappedMessageProcessor<AbstractClientV07>::value_type m_data[];
	const MappedMessageProcessor<AbstractClientV07>::PROTOMAP m_protoMap;
};

template<> class _LOCAL MappedMessageInitializer<AbstractClientV08> {
	DISALLOW_COPY_AND_ASSIGN(MappedMessageInitializer<AbstractClientV08>)
	friend class MappedMessageProcessor<AbstractClientV08>;
public:
	MappedMessageInitializer() : m_protoMap(m_data, m_data + 1u) {}
	~MappedMessageInitializer() {}

private:
	static const MappedMessageProcessor<AbstractClientV08>::value_type m_data[];
	const MappedMessageProcessor<AbstractClientV08>::PROTOMAP m_protoMap;
};

template<> class _LOCAL MappedMessageInitializer<AbstractClientV13> {
	DISALLOW_COPY_AND_ASSIGN(MappedMessageInitializer<AbstractClientV13>)
	friend class MappedMessageProcessor<AbstractClientV13>;
public:
	MappedMessageInitializer() : m_protoMap(m_data, m_data + 1u) {}
	~MappedMessageInitializer() {}

private:
	static const MappedMessageProcessor<AbstractClientV13>::value_type m_data[];
	const MappedMessageProcessor<AbstractClientV13>::PROTOMAP m_protoMap;
};

}

}

#endif /* NETMAUMAU_ABSTRACTCLIENTV05IMPL_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
