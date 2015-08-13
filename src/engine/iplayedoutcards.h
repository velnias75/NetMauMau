/*
 * Copyright 2015 by Heiko Schäfer <heiko@rangun.de>
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

#ifndef NETMAUMAU_IPLAYEDOUTCARDS_H
#define NETMAUMAU_IPLAYEDOUTCARDS_H

#include <limits>
#include <vector>

#include "cardtools.h"
#include "smartptr.h"

namespace NetMauMau {

template<class>	struct CardsAllocator;

template<> struct CardsAllocator<void> {

	typedef void value_type;
	typedef void *pointer;
	typedef const void *const_pointer;

	template <class U>
	struct rebind {
		typedef CardsAllocator<U> other;
	};
};

template<class T>
struct CardsAllocator {

	typedef T value_type;
	typedef T *pointer;
	typedef const T *const_pointer;
	typedef T &reference;
	typedef const T &const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	template<class U> struct rebind {
		typedef CardsAllocator<U> other;
	};

	CardsAllocator() {}

	template<class U> CardsAllocator(const CardsAllocator<U> &) {}

	~CardsAllocator() {}

	pointer allocate(size_type n, CardsAllocator<void> * = 0) {
		return reinterpret_cast<pointer>(::operator new(n * sizeof(value_type)));
	}

	static void deallocate(pointer p, size_type) {
		::operator delete(p);
	}

	static void construct(pointer p, const_reference val) {

		const value_type &v(NetMauMau::Common::find(val, m_deck, m_deck + 32));

		if(&v != m_deck + 32) {
			std::uninitialized_copy(&v, &v + 1, p);
		} else {
			new(static_cast<void *>(p)) value_type(val);
		}
	}

	static void destroy(pointer p) {
		(static_cast<pointer>(p))->~value_type();
	}

	static size_type max_size() {
		return std::numeric_limits<size_type>::max();
	}

private:
	friend class Talon;
	static const value_type m_deck[];
};

template<>
const CardsAllocator<Common::ICardPtr>::value_type CardsAllocator<Common::ICardPtr>::m_deck[32];

class IPlayedOutCards {
	DISALLOW_COPY_AND_ASSIGN(IPlayedOutCards)
public:
	typedef std::vector<Common::ICardPtr, CardsAllocator<Common::ICardPtr> > CARDS;

	virtual ~IPlayedOutCards() {}

	virtual const CARDS &getCards() const = 0;

protected:
	explicit IPlayedOutCards() {}
};

}

template<class T1, class T2>
bool operator==(const NetMauMau::CardsAllocator<T1> &, const NetMauMau::CardsAllocator<T2> &) {
	return true;
}

template<class T1, class T2>
bool operator!=(const NetMauMau::CardsAllocator<T1> &, const NetMauMau::CardsAllocator<T2> &) {
	return false;
}

#endif /* NETMAUMAU_IPLAYEDOUTCARDS_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
