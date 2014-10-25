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

#ifndef NETMAUMAU_CARDTOOLS_H
#define NETMAUMAU_CARDTOOLS_H

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#include <cstdlib>

#include "icard.h"

namespace NetMauMau {

namespace Common {

template<typename T>
inline T genRandom(T ubound) {
#if HAVE_ARC4RANDOM_UNIFORM
	return ubound > 0 ? ::arc4random_uniform(ubound) : 0;
#else
	return ubound > 0 ? (std::rand() % ubound) : 0;
#endif
}

_EXPORT const std::string *getSuiteSymbols() _CONST;
_EXPORT std::string ansiSuite(const std::string &suite);
_EXPORT ICard::SUITE symbolToSuite(const std::string &sym);
_EXPORT std::string suiteToSymbol(ICard::SUITE suite, bool ansi, bool endansi = false);
_EXPORT _NOUNUSED bool parseCardDesc(const std::string &desc, NetMauMau::ICard::SUITE *suite,
									 NetMauMau::ICard::VALUE *value);
_EXPORT std::string createCardDesc(ICard::SUITE s, ICard::VALUE v, bool ansi);
_EXPORT std::size_t getCardPoints(ICard::VALUE v) _CONST;

}

}

#endif /* NETMAUMAU_CARDTOOLS_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
