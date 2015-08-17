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
#include <algorithm>

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

	template<class, class, std::size_t> friend struct MappedMessageAllocator;
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
template<class, class, std::size_t> struct MappedMessageAllocator;

template<class T, std::size_t N>
class _LOCAL MappedMessageProcessor {
	DISALLOW_COPY_AND_ASSIGN(MappedMessageProcessor)

	template<class> friend class MappedMessageInitializer;
	template<class, class, std::size_t> friend struct MappedMessageAllocator;

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
	typedef std::pair<const std::string *const, PROTOFN> value_type;
	typedef MappedMessageAllocator<value_type, T, N> allocator_type;
	typedef std::map<const std::string *const, PROTOFN, _protoCmp, allocator_type> PROTOMAP;

public:
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

template<class T, std::size_t N>
const MappedMessageInitializer<T> MappedMessageProcessor<T, N>::m_messageMap;

template<class, class, std::size_t> struct MappedMessageAllocator;

template<class C, std::size_t N> struct MappedMessageAllocator<void, C, N> {

	typedef void value_type;
	typedef void *pointer;
	typedef const void *const_pointer;

	template <class U>
	struct rebind {
		typedef MappedMessageAllocator<U, C, N> other;
	};
};

template<class T, class C, std::size_t N>
struct MappedMessageAllocator {

	typedef T value_type;
	typedef T *pointer;
	typedef const T *const_pointer;
	typedef T &reference;
	typedef const T &const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	template<class U> struct rebind {
		typedef MappedMessageAllocator<U, C, N> other;
	};

	MappedMessageAllocator() {}

	template<class U>
	MappedMessageAllocator(const MappedMessageAllocator<U, C, N> &) {}

	~MappedMessageAllocator() {}

	pointer allocate(size_type n, MappedMessageAllocator<void, C, N> * = 0) {
		return reinterpret_cast<pointer>(::operator new(n * sizeof(value_type)));
	}

	static void deallocate(pointer p, size_type) {
		::operator delete(p);
	}

	static void construct(pointer p, const_reference val) {

		typedef MappedMessageAllocator<typename MappedMessageProcessor<C, N>::value_type, C, N> sal;

		const const_pointer &v(sal::m_data[sal::m_nxt] != val ? std::find(&sal::m_data[sal::m_nxt],
							   sal::m_data + N, val) : &sal::m_data[sal::m_nxt++]);

		std::uninitialized_copy(v, v + 1, p);
	}

	static void destroy(pointer) {}

private:
	template<class, class, std::size_t> friend struct MappedMessageAllocator;
	template<class> friend class MappedMessageInitializer;

	static const typename MappedMessageProcessor<C, N>::value_type m_data[];
	static std::size_t m_nxt;
};

template<class T, class C, std::size_t N>
std::size_t MappedMessageAllocator<T, C, N>::m_nxt = 0u;

template<> const MappedMessageProcessor<AbstractClientV05, MP_CNT_V05>::value_type
MappedMessageAllocator < MappedMessageProcessor<AbstractClientV05, MP_CNT_V05>::value_type,
					   AbstractClientV05, MP_CNT_V05 >::m_data[];

template<> const MappedMessageProcessor<AbstractClientV07, MP_CNT_V07>::value_type
MappedMessageAllocator < MappedMessageProcessor<AbstractClientV07, MP_CNT_V07>::value_type,
					   AbstractClientV07, MP_CNT_V07 >::m_data[];

template<> const MappedMessageProcessor<AbstractClientV08, MP_CNT_V08>::value_type
MappedMessageAllocator < MappedMessageProcessor<AbstractClientV08, MP_CNT_V08>::value_type,
					   AbstractClientV08, MP_CNT_V08 >::m_data[];

template<> const MappedMessageProcessor<AbstractClientV13, MP_CNT_V13>::value_type
MappedMessageAllocator < MappedMessageProcessor<AbstractClientV13, MP_CNT_V13>::value_type,
					   AbstractClientV13, MP_CNT_V13 >::m_data[];

template<class T> class MappedMessageInitializer;

template<> class _LOCAL MappedMessageInitializer<AbstractClientV05> {
	DISALLOW_COPY_AND_ASSIGN(MappedMessageInitializer<AbstractClientV05>)
	friend class MappedMessageProcessor<AbstractClientV05, MP_CNT_V05>;
public:
	// cppcheck-suppress functionStatic
	MappedMessageInitializer() : m_protoMap(
			MappedMessageProcessor<AbstractClientV05, MP_CNT_V05>::allocator_type::m_data,
			MappedMessageProcessor<AbstractClientV05, MP_CNT_V05>::allocator_type::m_data
			+ MP_CNT_V05) {}
	// cppcheck-suppress functionStatic
	~MappedMessageInitializer() {}

private:
	const MappedMessageProcessor<AbstractClientV05, MP_CNT_V05>::PROTOMAP m_protoMap;
};

template<> class _LOCAL MappedMessageInitializer<AbstractClientV07> {
	DISALLOW_COPY_AND_ASSIGN(MappedMessageInitializer<AbstractClientV07>)
	friend class MappedMessageProcessor<AbstractClientV07, MP_CNT_V07>;
public:
	// cppcheck-suppress functionStatic
	MappedMessageInitializer() : m_protoMap(
			MappedMessageProcessor<AbstractClientV07, MP_CNT_V07>::allocator_type::m_data,
			MappedMessageProcessor<AbstractClientV07, MP_CNT_V07>::allocator_type::m_data
			+ MP_CNT_V07) {}
	// cppcheck-suppress functionStatic
	~MappedMessageInitializer() {}

private:
	const MappedMessageProcessor<AbstractClientV07, MP_CNT_V07>::PROTOMAP m_protoMap;
};

template<> class _LOCAL MappedMessageInitializer<AbstractClientV08> {
	DISALLOW_COPY_AND_ASSIGN(MappedMessageInitializer<AbstractClientV08>)
	friend class MappedMessageProcessor<AbstractClientV08, MP_CNT_V08>;
public:
	// cppcheck-suppress functionStatic
	MappedMessageInitializer() : m_protoMap(
			MappedMessageProcessor<AbstractClientV08, MP_CNT_V08>::allocator_type::m_data,
			MappedMessageProcessor<AbstractClientV08, MP_CNT_V08>::allocator_type::m_data
			+ MP_CNT_V08) {}
	// cppcheck-suppress functionStatic
	~MappedMessageInitializer() {}

private:
	const MappedMessageProcessor<AbstractClientV08, MP_CNT_V08>::PROTOMAP m_protoMap;
};

template<> class _LOCAL MappedMessageInitializer<AbstractClientV13> {
	DISALLOW_COPY_AND_ASSIGN(MappedMessageInitializer<AbstractClientV13>)
	friend class MappedMessageProcessor<AbstractClientV13, MP_CNT_V13>;
public:
	// cppcheck-suppress functionStatic
	MappedMessageInitializer()
		: m_protoMap(MappedMessageProcessor<AbstractClientV13, MP_CNT_V13>::allocator_type::m_data,
					 MappedMessageProcessor<AbstractClientV13, MP_CNT_V13>::allocator_type::m_data
					 + MP_CNT_V13) {}
	// cppcheck-suppress functionStatic
	~MappedMessageInitializer() {}

private:
	const MappedMessageProcessor<AbstractClientV13, MP_CNT_V13>::PROTOMAP m_protoMap;
};

}

}

template<class T1, class T2, class C, std::size_t N>
bool operator==(const NetMauMau::Client::MappedMessageAllocator<T1, C, N> &,
				const NetMauMau::Client::MappedMessageAllocator<T2, C, N> &) {
	return true;
}

template<class T1, class T2, class C, std::size_t N>
bool operator!=(const NetMauMau::Client::MappedMessageAllocator<T1, C, N> &,
				const NetMauMau::Client::MappedMessageAllocator<T2, C, N> &) {
	return false;
}

#endif /* NETMAUMAU_ABSTRACTCLIENTV05IMPL_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
