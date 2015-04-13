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

#include <fstream>
#include <cstdio>
#include <map>

#include "cardtools.h"

namespace {

#ifndef DISABLE_ANSI
const std::string ANSI_RED("\x1B[31m");
const std::string ANSI_DFT("\x1B[39m");
#else
const std::string ANSI_RED;
const std::string ANSI_DFT;
#endif

#ifndef _WIN32
const std::string SERVER_EXE(std::string(NMM_EXE_PATH).append(1, '/').append(NMM_EXE_NAME));
#else
const std::string SERVER_EXE(NMM_EXE_NAME);
#endif

#ifndef __clang__
#pragma GCC diagnostic ignored "-Wunsafe-loop-optimizations"
#pragma GCC diagnostic push
#endif
const std::string SUIT[] = { "\u2666", "\u2665", "\u2660", "\u2663" };
#ifndef __clang__
#pragma GCC diagnostic pop
#endif

class _ICARD : public NetMauMau::Common::ICard {
	DISALLOW_COPY_AND_ASSIGN(_ICARD)
public:
	explicit _ICARD() : ICard() {}
	virtual ~_ICARD() {}

	virtual SUIT getSuit() const {
		return SUIT_ILLEGAL;
	}

	virtual RANK getRank() const {
		return RANK_ILLEGAL;
	}

	virtual std::size_t getPoints() const {
		return 0;
	}

	virtual std::string description(bool) const {
		return "ILLEGAL CARD";
	}

} ILLEGAL_CARD;

const std::map<const std::string, NetMauMau::Common::ICard::SUIT>::value_type SSP[] = {
	std::make_pair(SUIT[2], NetMauMau::Common::ICard::SPADES),
	std::make_pair(SUIT[3], NetMauMau::Common::ICard::CLUBS),
	std::make_pair(SUIT[1], NetMauMau::Common::ICard::HEARTS),
	std::make_pair(SUIT[0], NetMauMau::Common::ICard::DIAMONDS),
	std::make_pair("X", NetMauMau::Common::ICard::SUIT_ILLEGAL)
};

const std::map<const std::string, NetMauMau::Common::ICard::SUIT> suitToSymbolMap(SSP, SSP + 5);

}

const std::string *NetMauMau::Common::getSuitSymbols() {
	return SUIT;
}

std::string NetMauMau::Common::suitToSymbol(ICard::SUIT suit, bool ansi, bool endansi) {

	std::string d;
	d.reserve(50);

	switch(suit) {
	case ICard::DIAMONDS:
		if(ansi) d.append(ANSI_RED);

		d.append("\u2666");
		break;

	case ICard::HEARTS:
		if(ansi) d.append(ANSI_RED);

		d.append("\u2665");
		break;

	case ICard::SPADES:
		if(ansi) d.append(ANSI_DFT);

		d.append("\u2660");
		break;

	case ICard::CLUBS:
		if(ansi) d.append(ANSI_DFT);

		d.append("\u2663");
		break;

	case ICard::SUIT_ILLEGAL:
		d.append("X");
		break;
	}

#ifndef DISABLE_ANSI

	if(ansi && endansi) d.append("\x1B[39m");

#endif

	return d;
}

NetMauMau::Common::ICard::SUIT NetMauMau::Common::symbolToSuit(const std::string &sym) {
	const std::map<const std::string, NetMauMau::Common::ICard::SUIT>::const_iterator
	&f(suitToSymbolMap.find(sym));
	return f != suitToSymbolMap.end() ? f->second : ICard::HEARTS;
}

std::size_t NetMauMau::Common::getCardPoints(ICard::RANK v) {

	switch(v) {
	case ICard::SEVEN:
		return 1;

	case ICard::EIGHT:
		return 2;

	case ICard::NINE:
		return 3;

	case ICard::TEN:
		return 4;

	case ICard::QUEEN:
		return 5;

	case ICard::KING:
		return 6;

	case ICard::ACE:
		return 11;

	case ICard::JACK:
		return 20;

	case ICard::RANK_ILLEGAL:
	default:
		return 0;
	}

#ifndef __clang__
	return 0;
#endif
}

unsigned int NetMauMau::Common::suitOrderPosition(NetMauMau::Common::ICard::SUIT s) {

	switch(s) {
	case ICard::CLUBS:
		return 0;

	case ICard::SPADES:
		return 1;

	case ICard::HEARTS:
		return 2;

	case ICard::DIAMONDS:
		return 3;

	case ICard::SUIT_ILLEGAL:
	default:
		return 4;
	}
}

unsigned int NetMauMau::Common::rankOrderPosition(NetMauMau::Common::ICard::RANK r) {

	switch(r) {
	case NetMauMau::Common::ICard::SEVEN:
		return 0;

	case NetMauMau::Common::ICard::EIGHT:
		return 1;

	case NetMauMau::Common::ICard::NINE:
		return 2;

	case NetMauMau::Common::ICard::TEN:
		return 3;

	case NetMauMau::Common::ICard::QUEEN:
		return 4;

	case NetMauMau::Common::ICard::KING:
		return 5;

	case NetMauMau::Common::ICard::ACE:
		return 6;

	case NetMauMau::Common::ICard::JACK:
		return 7;

	default:
		return 8;
	}
}

bool NetMauMau::Common::cardEqual(const NetMauMau::Common::ICard *x,
								  const NetMauMau::Common::ICard *y) {
	return x->getSuit() == y->getSuit() && x->getRank() == y->getRank();
}

bool NetMauMau::Common::cardLess(const NetMauMau::Common::ICard *x,
								 const NetMauMau::Common::ICard *y) {

	return x->getSuit() == y->getSuit() ?
		   rankOrderPosition(x->getRank()) < rankOrderPosition(y->getRank()) :
		   suitOrderPosition(x->getSuit()) < suitOrderPosition(y->getSuit());
}

bool NetMauMau::Common::cardGreater(const NetMauMau::Common::ICard *x,
									const NetMauMau::Common::ICard *y) {

	return x->getRank() == y->getRank() ?
		   suitOrderPosition(x->getSuit()) < suitOrderPosition(y->getSuit()) :
		   rankOrderPosition(x->getRank()) < rankOrderPosition(y->getRank());
}

bool NetMauMau::Common::isSuit(const NetMauMau::Common::ICard *card,
							   NetMauMau::Common::ICard::SUIT suit) {
	return card->getSuit() == suit;
}

bool NetMauMau::Common::isRank(const NetMauMau::Common::ICard *card,
							   NetMauMau::Common::ICard::RANK rank) {
	return card->getRank() == rank;
}

NetMauMau::Common::ICard *NetMauMau::Common::getIllegalCard() {
	return &ILLEGAL_CARD;
}

bool NetMauMau::Common::parseCardDesc(const std::string &desc, ICard::SUIT *suit,
									  ICard::RANK *rank) {

	if(desc == "ILLEGAL CARD") {
		*suit = ICard::SUIT_ILLEGAL;
		*rank = ICard::RANK_ILLEGAL;
		return true;
	}

	const std::string::size_type p = desc.find(' ');

	if(p == std::string::npos || !suit || !rank) return false;

	const std::string r(desc.substr(p + 1));
	const std::string::value_type rc(r[0]);

	*suit = symbolToSuit(desc.substr(0, p));

	unsigned int iVal;

	if(::isdigit(rc) && (iVal = std::strtoul(r.c_str(), NULL, 10))) {
		*rank = static_cast<ICard::RANK>(iVal);
	} else {
		switch(rc) {
		case 'J':
			*rank = ICard::JACK;
			break;

		case 'Q':
			*rank = ICard::QUEEN;
			break;

		case 'K':
			*rank = ICard::KING;
			break;

		case 'A':
			*rank = ICard::ACE;
			break;
		}
	}

	return true;
}

std::string NetMauMau::Common::createCardDesc(ICard::SUIT s, ICard::RANK v, bool ansi) {

	std::string d(suitToSymbol(s, ansi));
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

	case ICard::SEVEN:
	case ICard::EIGHT:
	case ICard::NINE:
	case ICard::TEN:
	default:
		char n[256];
		std::snprintf(n, 255, "%u", static_cast<unsigned int>(v));
		d.append(n);
		break;
	}

#ifndef DISABLE_ANSI

	if(ansi) d.append("\x1B[39m");

#endif

	return d;

}

std::string NetMauMau::Common::ansiSuit(const std::string &suit) {
	return suitToSymbol(symbolToSuit(suit), true, true);
}

const char *NetMauMau::Common::getServerExe() {
	return SERVER_EXE.c_str();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
