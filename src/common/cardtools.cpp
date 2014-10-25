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

#include <sstream>

#include "cardtools.h"

namespace {

#ifndef DISABLE_ANSI
const std::string ANSI_RED("\x1B[31m");
const std::string ANSI_DFT("\x1B[39m");
#else
const std::string ANSI_RED;
const std::string ANSI_DFT;
#endif

#pragma GCC diagnostic ignored "-Wunsafe-loop-optimizations"
#pragma GCC diagnostic push
const std::string SUITES[] = { "\u2666", "\u2665", "\u2660", "\u2663" };
#pragma GCC diagnostic pop

}

const std::string *NetMauMau::Common::getSuiteSymbols() {
	return SUITES;
}

std::string NetMauMau::Common::suiteToSymbol(NetMauMau::ICard::SUITE suite, bool ansi,
		bool endansi) {

	std::string d;
	d.reserve(50);

	switch(suite) {
	case ICard::DIAMOND:
		if(ansi) d.append(ANSI_RED);

		d.append("\u2666");
		break;

	case ICard::HEART:
		if(ansi) d.append(ANSI_RED);

		d.append("\u2665");
		break;

	case ICard::SPADE:
		if(ansi) d.append(ANSI_DFT);

		d.append("\u2660");
		break;

	case ICard::CLUB:
		if(ansi) d.append(ANSI_DFT);

		d.append("\u2663");
		break;
	}

#ifndef DISABLE_ANSI

	if(ansi && endansi) d.append("\x1B[39m");

#endif

	return d;
}

NetMauMau::ICard::SUITE NetMauMau::Common::symbolToSuite(const std::string &sym) {

	if(sym == "\u2666") {
		return NetMauMau::ICard::DIAMOND;
	} else if(sym == "\u2665") {
		return NetMauMau::ICard::HEART;
	} else if(sym == "\u2660") {
		return NetMauMau::ICard::SPADE;
	} else if(sym == "\u2663") {
		return NetMauMau::ICard::CLUB;
	}

	return NetMauMau::ICard::HEART;
}

std::size_t NetMauMau::Common::getCardPoints(ICard::VALUE v) {

	switch(v) {
	case ICard::SEVEN:
		return 7;

	case ICard::EIGHT:
		return 8;

	case ICard::NINE:
		return 9;

	case ICard::TEN:
	case ICard::QUEEN:
	case ICard::KING:
		return 10;

	case ICard::ACE:
		return 11;

	case ICard::JACK:
		return 20;
	}

	return 0;
}

bool NetMauMau::Common::parseCardDesc(const std::string &desc, NetMauMau::ICard::SUITE *suite,
									  NetMauMau::ICard::VALUE *value) {

	const std::string::size_type p = desc.find(' ');

	if(p == std::string::npos) return false;

	const std::string s(desc.substr(0, p));
	const std::string v(desc.substr(p + 1));

	*suite = symbolToSuite(s);

	std::istringstream is(v);
	unsigned int iVal;
	is >> iVal;

	if(::isdigit(v[0]) && iVal) {
		*value = static_cast<NetMauMau::ICard::VALUE>(iVal);
	} else {
		switch(v[0]) {
		case 'J':
			*value = NetMauMau::ICard::JACK;
			break;

		case 'Q':
			*value = NetMauMau::ICard::QUEEN;
			break;

		case 'K':
			*value = NetMauMau::ICard::KING;
			break;

		case 'A':
			*value = NetMauMau::ICard::ACE;
			break;
		}
	}

	return true;
}

std::string NetMauMau::Common::createCardDesc(ICard::SUITE s, ICard::VALUE v, bool ansi) {

	std::string d = suiteToSymbol(s, ansi);
	d.append(1, ' ');

	switch(v) {
	case ICard::JACK:
		d.append(1, 'J');
		break;

	case ICard::QUEEN:
		d.append(1, 'Q');
		break;

	case ICard::KING:
		d.append(1, 'K');
		break;

	case ICard::ACE:
		d.append(1, 'A');
		break;

	default:
		std::ostringstream n;
		n << static_cast<unsigned int>(v);
		d.append(n.str());
		break;
	}

#ifndef DISABLE_ANSI

	if(ansi) d.append("\x1B[39m");

#endif

	return d;

}

std::string NetMauMau::Common::ansiSuite(const std::string &suite) {
	return suiteToSymbol(symbolToSuite(suite), true, true);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
