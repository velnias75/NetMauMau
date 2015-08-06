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

#include "luaruleset.h"

#include <algorithm>                    // for max, copy, for_each
#include <iterator>                     // for ostream_iterator
#include <limits>                       // for numeric_limits

#include "logger.h"                     // for BasicLogger, logWarning
#include "iplayer.h"                    // for IPlayer
#include "luafatalexception.h"          // for LuaFatalException
#include "serverplayerexception.h"
#include "luastate.h"                   // for LuaState

#ifndef _WIN32
#define ELLIPSIS "…"
#else
#define ELLIPSIS "..."
#endif

namespace {

const std::string WRONGTYPE("Wrong return type; expecting ");

const NetMauMau::Lua::LuaState &l = NetMauMau::Lua::LuaState::getInstance();

const char *FUNCTIONS[] = {
	"checkCard",
	"dirChanged",
	"getAceRoundRank",
	"getDirChangeIsSuspend",
	"getJackSuit",
	"hasDirChange",
	"hasSuspended",
	"hasTakenCards",
	"hasToSuspend",
	"init",
	"initialCardCount",
	"isAceRound",
	"isAceRoundPossible",
	"isJackMode",
	"lostPointFactor",
	"setCurPlayers",
	"setDirChangeIsSuspend",
	"setJackModeOff",
	"takeCardCount",
	"takeCards",
	"takeIfLost",
	"takeAfterSevenIfNoMatch",
	"getMaxPlayers",
};

enum FUNCTIONNAMES {
	CHECKCARD = 0,
	DIRCHANGED,
	GETACEROUNDRANK,
	GETDIRCHANGEISSUSPEND,
	GETJACKSUIT,
	HASDIRCHANGE,
	HASSUSPENDED,
	HASTAKENCARDS,
	HASTOSUSPEND,
	INIT,
	INITIALCARDCOUNT,
	ISACEROUND,
	ISACEROUNDPOSSIBLE,
	ISJACKMODE,
	LOSTPOINTFACTOR,
	SETCURPLAYERS,
	SETDIRCHANGEISSUSPEND,
	SETJACKMODEOFF,
	TAKECARDCOUNT,
	TAKECARDS,
	TAKEIFLOST,
	TAKEAFTERSEVENIFNOMATCH,
	ENDFUNCTIONS,
	GETMAXPLAYERS = ENDFUNCTIONS
};

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct _checkMissing : public std::unary_function<const char *, void> {

	inline explicit _checkMissing() : missing() {
		missing.reserve(sizeof(FUNCTIONS));
	}

	inline result_type operator()(argument_type fn) {
		if(fn && !exists(fn)) missing.push_back(fn);
	}

	std::vector<const char *> missing;

private:
	inline bool exists(const char *fn) const {

		lua_pushstring(l, fn);
		lua_rawget(l, LUA_GLOBALSINDEX);
		const bool ex = static_cast<bool>(lua_isfunction(l, -1));
		lua_pop(l, 1);

		return ex;
	}
};
#pragma GCC diagnostic pop

template < typename T, typename LuaType = lua_Integer,
		 LuaType(*CONV)(lua_State *, int) = lua_tointeger >
struct LuaTypeCheckerBase {

	inline static LuaType getType(lua_State *ls, int t, const char *n, const char *fname)
	throw(NetMauMau::Lua::Exception::LuaFatalException) {

		if(lua_type(ls, -1) != t || lua_type(ls, -1) == LUA_TNONE) {
			lua_pop(ls, -1);
			throw NetMauMau::Lua::Exception::LuaFatalException(WRONGTYPE + n, fname);
		}

		return CONV(ls, -1);
	}
};

template<typename R, typename T>
R checkRange(const T &t, const char *fname) throw(NetMauMau::Lua::Exception::LuaFatalException) {

	if(!std::numeric_limits<T>::is_specialized) {
		throw NetMauMau::Lua::Exception::LuaFatalException("cannot check return value range",
				fname);
	}

	if((std::numeric_limits<T>::max() - static_cast<T>(t)) > std::numeric_limits<T>::max()) {
		throw NetMauMau::Lua::Exception::LuaFatalException("returned value out of range", fname);
	}

	return static_cast<R>(t);
}

template<>
std::size_t checkRange<std::size_t>(const lua_Integer &t, const char *fname)
throw(NetMauMau::Lua::Exception::LuaFatalException) {

	if(t < 0) {
		throw NetMauMau::Lua::Exception::LuaFatalException("returned value cannot be negative",
				fname);
	} else if((std::numeric_limits<std::size_t>::max() - static_cast<std::size_t>(t)) >
			  std::numeric_limits<std::size_t>::max()) {
		throw NetMauMau::Lua::Exception::LuaFatalException("returned value out of range", fname);
	}

	return static_cast<std::size_t>(t);
}

#ifdef __clang__
#pragma clang diagnostic ignored "-Wunreachable-code"
#pragma clang diagnostic push
#endif
template<>
NetMauMau::Common::ICard::SUIT checkRange<NetMauMau::Common::ICard::SUIT>(const lua_Integer &t,
		const char *fname) throw(NetMauMau::Lua::Exception::LuaFatalException) {

	const NetMauMau::Common::ICard::SUIT s = static_cast<NetMauMau::Common::ICard::SUIT>(t);

	switch(s) {
	case NetMauMau::Common::ICard::DIAMONDS:
	case NetMauMau::Common::ICard::HEARTS:
	case NetMauMau::Common::ICard::SPADES:
	case NetMauMau::Common::ICard::CLUBS:
	case NetMauMau::Common::ICard::SUIT_ILLEGAL:
		return s;

	default:
		throw NetMauMau::Lua::Exception::LuaFatalException("No valid suit", fname);
	}
}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wunreachable-code"
#pragma clang diagnostic push
#endif
template<>
NetMauMau::Common::ICard::RANK checkRange<NetMauMau::Common::ICard::RANK>(const lua_Integer &t,
		const char *fname) throw(NetMauMau::Lua::Exception::LuaFatalException) {

	const NetMauMau::Common::ICard::RANK r = static_cast<NetMauMau::Common::ICard::RANK>(t);

	switch(r) {
	case NetMauMau::Common::ICard::SEVEN:
	case NetMauMau::Common::ICard::EIGHT:
	case NetMauMau::Common::ICard::NINE:
	case NetMauMau::Common::ICard::TEN:
	case NetMauMau::Common::ICard::JACK:
	case NetMauMau::Common::ICard::KING:
	case NetMauMau::Common::ICard::QUEEN:
	case NetMauMau::Common::ICard::ACE:
	case NetMauMau::Common::ICard::RANK_ILLEGAL:
		return r;

	default:
		throw NetMauMau::Lua::Exception::LuaFatalException("No valid rank", fname);
	}
}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

template<typename T>
struct returnTypeCheckerTrait {
	inline T operator()(lua_State *, const char *fname) const
	throw(NetMauMau::Lua::Exception::LuaFatalException) {
		throw NetMauMau::Lua::Exception::LuaFatalException("Illegal return type", fname);
	}
};

template<>
struct returnTypeCheckerTrait<bool> : private LuaTypeCheckerBase<bool, int, lua_toboolean> {
	inline bool operator()(lua_State *ls, const char *fname) const
	throw(NetMauMau::Lua::Exception::LuaFatalException) {
		return checkRange<bool>(getType(ls, LUA_TBOOLEAN, "bool", fname), fname);
	}
};

template<>
struct returnTypeCheckerTrait<std::size_t> : private LuaTypeCheckerBase<std::size_t> {
	inline std::size_t operator()(lua_State *ls, const char *fname) const
	throw(NetMauMau::Lua::Exception::LuaFatalException) {
		return checkRange<std::size_t>(getType(ls, LUA_TNUMBER, "integer", fname), fname);
	}
};

template<>
struct returnTypeCheckerTrait<NetMauMau::Common::ICard::SUIT> :
	private LuaTypeCheckerBase<NetMauMau::Common::ICard::SUIT> {

	inline NetMauMau::Common::ICard::SUIT operator()(lua_State *ls, const char *fname) const
	throw(NetMauMau::Lua::Exception::LuaFatalException) {
		return checkRange<NetMauMau::Common::ICard::SUIT>(getType(ls, LUA_TNUMBER, "SUIT", fname),
				fname);
	}
};

template<>
struct returnTypeCheckerTrait<NetMauMau::Common::ICard::RANK> :
	private LuaTypeCheckerBase<NetMauMau::Common::ICard::RANK> {

	inline NetMauMau::Common::ICard::RANK operator()(lua_State *ls, const char *fname) const
	throw(NetMauMau::Lua::Exception::LuaFatalException) {
		return checkRange<NetMauMau::Common::ICard::RANK>(getType(ls, LUA_TNUMBER, "RANK", fname),
				fname);
	}
};

template<typename T>
T checkReturnType(lua_State *ls,
				  const char *fname) throw(NetMauMau::Lua::Exception::LuaFatalException) {
	return returnTypeCheckerTrait<T>()(ls, fname);
}

}

using namespace NetMauMau::RuleSet;

LuaRuleSet::LuaRuleSet(const std::vector<std::string> &luafiles, bool dirChangePossible,
					   std::size_t icc, const NetMauMau::IAceRoundListener *arl)
throw(NetMauMau::Lua::Exception::LuaException) : IRuleSet() {

	if(luafiles.empty()) throw NetMauMau::Lua::Exception::LuaException("no Lua rule files given");

	for(std::vector<std::string>::const_iterator i(luafiles.begin()); i != luafiles.end(); ++i) {
		logInfo("Loading Lua rules file \"" << *i << " " << ELLIPSIS);
		l.load(*i, dirChangePossible, icc, arl);
	}

	const std::vector<const char *> &missing(checkInterface());

	if(!missing.empty()) {

		std::ostringstream os;
		os << "Your Lua rules is missing following required functions: ";
		std::ostream_iterator<std::string> out_it(os, ", ");
		std::copy(missing.begin(), missing.end(), out_it);

		throw NetMauMau::Lua::Exception::LuaException(os.str().substr(0, os.str().length() - 2));
	}

	reset();
}

LuaRuleSet::~LuaRuleSet() {}

bool LuaRuleSet::isNull() const throw() {
	return false;
}

std::vector<const char *> LuaRuleSet::checkInterface() {
	return std::for_each(FUNCTIONS, &FUNCTIONS[ENDFUNCTIONS], _checkMissing()).missing;
}

void LuaRuleSet::checkInitial(const NetMauMau::Player::IPlayer *player,
							  const NetMauMau::Common::ICardPtr &playedCard)
throw(NetMauMau::Lua::Exception::LuaException) {
	checkCard(player, NetMauMau::Common::ICardPtr(), playedCard, player->isAIPlayer());
}

bool LuaRuleSet::checkCard(const NetMauMau::Player::IPlayer *player,
						   const NetMauMau::Common::ICardPtr &uncoveredCard,
						   const NetMauMau::Common::ICardPtr &playedCard,
						   bool) throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[CHECKCARD];

	lua_getglobal(l, fname);

	l.pushCard(uncoveredCard);
	l.pushCard(playedCard);

	try {

		l.pushPlayer(player);
		l.call(fname, 3);

	} catch(const NetMauMau::Server::Exception::ServerPlayerException &) {
		lua_pop(l, 3);
		throw;
	} catch(const NetMauMau::Common::Exception::SocketException &e) {
		lua_pop(l, 3);
		throw NetMauMau::Lua::Exception::LuaException(std::string("Internal error: ") + e.what(),
				fname);
	}

	return checkReturnType<bool>(l, fname);
}

bool LuaRuleSet::checkCard(const NetMauMau::Common::ICardPtr &uncoveredCard,
						   const NetMauMau::Common::ICardPtr &playedCard) const
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[CHECKCARD];

	lua_getglobal(l, fname);

	l.pushCard(uncoveredCard);
	l.pushCard(playedCard);
	lua_pushnil(l);
	l.call(fname, 3);

	return checkReturnType<bool>(l, fname);
}

std::size_t LuaRuleSet::lostPointFactor(const NetMauMau::Common::ICardPtr &uncoveredCard) const
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[LOSTPOINTFACTOR];
	lua_getglobal(l, fname);
	l.pushCard(uncoveredCard);
	l.call(fname, 1);

	return std::max<std::size_t>(1, checkReturnType<std::size_t>(l, fname));
}

bool LuaRuleSet::hasToSuspend() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[HASTOSUSPEND];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return checkReturnType<bool>(l, fname);
}

void LuaRuleSet::hasSuspended() throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[HASSUSPENDED];
	lua_getglobal(l, fname);
	l.call(fname, 0, 0);
}

std::size_t LuaRuleSet::takeCardCount() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[TAKECARDCOUNT];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return checkReturnType<std::size_t>(l, fname);
}

std::size_t LuaRuleSet::takeCards(const NetMauMau::Common::ICard *playedCard) const
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[TAKECARDS];
	lua_getglobal(l, fname);
	l.pushCard(NetMauMau::Common::ICardPtr(playedCard));
	l.call(fname, 1);

	return checkReturnType<std::size_t>(l, fname);
}

void LuaRuleSet::hasTakenCards() throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[HASTAKENCARDS];
	lua_getglobal(l, fname);
	l.call(fname, 0, 0);
}

std::size_t LuaRuleSet::initialCardCount() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[INITIALCARDCOUNT];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return std::max<std::size_t>(1, checkReturnType<std::size_t>(l, fname));
}

bool LuaRuleSet::takeIfLost() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[TAKEIFLOST];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return checkReturnType<bool>(l, fname);
}

bool LuaRuleSet::takeAfterSevenIfNoMatch() const throw(Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[TAKEAFTERSEVENIFNOMATCH];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return checkReturnType<bool>(l, fname);
}

bool LuaRuleSet::isAceRoundPossible() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[ISACEROUNDPOSSIBLE];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return checkReturnType<bool>(l, fname);
}

NetMauMau::Common::ICard::RANK LuaRuleSet::getAceRoundRank() const
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[GETACEROUNDRANK];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return checkReturnType<NetMauMau::Common::ICard::RANK>(l, fname);
}

bool LuaRuleSet::hasDirChange() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[HASDIRCHANGE];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return checkReturnType<bool>(l, fname);
}

void LuaRuleSet::dirChanged() throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[DIRCHANGED];
	lua_getglobal(l, fname);
	l.call(fname, 0, 0);
}

bool LuaRuleSet::getDirChangeIsSuspend() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[GETDIRCHANGEISSUSPEND];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return checkReturnType<bool>(l, fname);
}

void LuaRuleSet::setDirChangeIsSuspend(bool suspend)
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[SETDIRCHANGEISSUSPEND];
	lua_getglobal(l, fname);
	lua_pushboolean(l, suspend);
	l.call(fname, 1, 0);
}

bool LuaRuleSet::isAceRound() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[ISACEROUND];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return checkReturnType<bool>(l, fname);
}

bool LuaRuleSet::isJackMode() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[ISJACKMODE];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return checkReturnType<bool>(l, fname);
}

NetMauMau::Common::ICard::SUIT LuaRuleSet::getJackSuit() const
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[GETJACKSUIT];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return checkReturnType<NetMauMau::Common::ICard::SUIT>(l, fname);
}

void LuaRuleSet::setJackModeOff() throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[SETJACKMODEOFF];
	lua_getglobal(l, fname);
	l.call(fname, 0, 0);
}

std::size_t LuaRuleSet::getMaxPlayers() const throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[GETMAXPLAYERS];

	lua_getglobal(l, fname);

	if(lua_isnil(l, -1)) {
		lua_pop(l, 1);
		return std::numeric_limits<std::size_t>::max();
	}

	l.call(fname, 0);

	return std::max<std::size_t>(2, checkReturnType<std::size_t>(l, fname));
}

void LuaRuleSet::setCurPlayers(std::size_t players) throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[SETCURPLAYERS];
	lua_getglobal(l, fname);
	lua_pushinteger(l, static_cast<lua_Integer>(players));
	l.call(fname, 1, 0);
}

void LuaRuleSet::reset() throw() {

	const char *fname = FUNCTIONS[INIT];
	lua_getglobal(l, fname);

	try {
		l.call(fname, 0, 0);
	} catch(const NetMauMau::Lua::Exception::LuaException &e) {
		logWarning(e);
	}
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
