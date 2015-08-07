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

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "ci_string.h"

using namespace NetMauMau::Common;

bool ci_char_traits::eq(char_type c1, char_type c2) {
	return std::toupper(c1) == std::toupper(c2);
}

bool ci_char_traits::lt(char_type c1, char_type c2) {
	return std::toupper(c1) <  std::toupper(c2);
}

int ci_char_traits::compare(const char_type *s1, const char_type *s2, size_t n) {

#ifdef HAVE_STRNCASECMP
	return strncasecmp(s1, s2, n);
#else

	while(n-- != 0) {

		if(std::toupper(*s1) < std::toupper(*s2)) return -1;

		if(std::toupper(*s1) > std::toupper(*s2)) return 1;

		++s1;
		++s2;
	}

	return 0;

#endif
}

const ci_char_traits::char_type *ci_char_traits::find(const char_type *s, size_t n, char_type a) {

	const int ua(std::toupper(a));

	while(n-- != 0) {

		if(std::toupper(*s) == ua) return s;

		++s;
	}

	return 0L;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
