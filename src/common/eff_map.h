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

/**
 * @file eff_map.h
 * @brief An efficient std::map adder and updater based on Scott Meyer's Effective STL
 * @author Heiko Schäfer <heiko@rangun.de4>
 */

#ifndef NETMAUMAU_COMMON_EFF_MAP_H
#define NETMAUMAU_COMMON_EFF_MAP_H

#include <map>

namespace NetMauMau {

namespace Common {

template<typename MapType, typename KeyArgType, typename ValueArgType>
typename MapType::iterator efficientAddOrUpdate(MapType &m, const KeyArgType &k,
		const ValueArgType &v, const typename MapType::iterator &lb) {

	if(lb != m.end() && !(m.key_comp()(k, lb->first))) {
		lb->second = v;
		return lb;
	} else {
		typedef typename MapType::value_type MVT;
		return m.insert(lb, MVT(k, v));
	}
}

template<typename MapType, typename KeyArgType, typename ValueArgType>
typename MapType::iterator efficientAddOrUpdate(MapType &m, const KeyArgType &k,
		const ValueArgType &v) {
	return efficientAddOrUpdate(m, k, v, m.lower_bound(k));
}

template<typename MapType>
struct key {
	typename MapType::value_type::first_type &operator()
	(const typename MapType::value_type &v) const {
		return v.first;
	}
};

template<typename MapType>
struct value {
	typename MapType::value_type::second_type &operator()
	(const typename MapType::value_type &v) const {
		return v.second;
	}
};

}

}

#endif /* NETMAUMAU_COMMON_EFF_MAP_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
