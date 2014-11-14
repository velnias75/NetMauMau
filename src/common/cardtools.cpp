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
const std::string SUIT[] = { "\u2666", "\u2665", "\u2660", "\u2663" };
#pragma GCC diagnostic pop

class _ICARD : public NetMauMau::Common::ICard {
	DISALLOW_COPY_AND_ASSIGN(_ICARD)
public:
	_ICARD() : ICard() {}
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

	if(sym == "\u2666") {
		return ICard::DIAMONDS;
	} else if(sym == "\u2665") {
		return ICard::HEARTS;
	} else if(sym == "\u2660") {
		return ICard::SPADES;
	} else if(sym == "\u2663") {
		return ICard::CLUBS;
	} else if(sym == "X") {
		return ICard::SUIT_ILLEGAL;
	}

	return ICard::HEARTS;
}

std::size_t NetMauMau::Common::getCardPoints(ICard::RANK v) {

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

	default:
		return 0;
	}

	return 0;
}

unsigned int NetMauMau::Common::suitOrderPosition(NetMauMau::Common::ICard::SUIT s) {

	switch(s) {
	case NetMauMau::Common::ICard::CLUBS:
		return 0;

	case NetMauMau::Common::ICard::SPADES:
		return 1;

	case NetMauMau::Common::ICard::HEARTS:
		return 2;

	case NetMauMau::Common::ICard::DIAMONDS:
		return 3;

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
	return x->description() == y->description();
}

bool NetMauMau::Common::cardLess(const NetMauMau::Common::ICard *x,
								 const NetMauMau::Common::ICard *y) {

	return x->getSuit() == y->getSuit() ?
		   rankOrderPosition(x->getRank()) < rankOrderPosition(y->getRank()) :
		   suitOrderPosition(x->getSuit()) < suitOrderPosition(y->getSuit());
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

	const std::string s(desc.substr(0, p));
	const std::string r(desc.substr(p + 1));

	*suit = symbolToSuit(s);

	unsigned int iVal;
	(std::istringstream(r)) >> iVal;

	if(::isdigit(r[0]) && iVal) {
		*rank = static_cast<ICard::RANK>(iVal);
	} else {
		switch(r[0]) {
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

	std::string d = suitToSymbol(s, ansi);
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

std::string NetMauMau::Common::ansiSuit(const std::string &suit) {
	return suitToSymbol(symbolToSuit(suit), true, true);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
