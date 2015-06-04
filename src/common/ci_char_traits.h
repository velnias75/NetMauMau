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

#ifndef NETMAUMAU_COMMON_CI_CHAR_TRAITS_H
#define NETMAUMAU_COMMON_CI_CHAR_TRAITS_H

#include <string>

#include "linkercontrol.h"

namespace NetMauMau {

namespace Common {

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct _EXPORT ci_char_traits : public std::char_traits<char> {
	static bool eq(char_type c1, char_type c2) _PURE;
	static bool lt(char_type c1, char_type c2) _PURE;
	static int compare(const char_type *s1, const char_type *s2, size_t n) _PURE;
	static const char_type *find(const char_type *s, size_t n, char_type a) _PURE;
};
#pragma GCC diagnostic pop

typedef std::basic_string<char, ci_char_traits> ci_string;

}

}

#endif /* NETMAUMAU_COMMON_CI_CHAR_TRAITS_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
