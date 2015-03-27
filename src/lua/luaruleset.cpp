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

#include <iterator>
#include <algorithm>

#include "luaruleset.h"

#include "luastate.h"
#include "iplayer.h"
#include "logger.h"

namespace {

const NetMauMau::Lua::LuaState &l = NetMauMau::Lua::LuaState::getInstance();

const char *FUNCTIONS[] = {
	"checkCard",
	"dirChanged",
	"getAceRoundRank",
	"getDirChangeIsSuspend",
	"getJackSuit",
	"getMaxPlayers",
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
	"suspendIfNoMatchingCard",
	"takeCardCount",
	"takeCards",
	"takeIfLost"
};

enum FUNCTIONNAMES {
	CHECKCARD = 0,
	DIRCHANGED,
	GETACEROUNDRANK,
	GETDIRCHANGEISSUSPEND,
	GETJACKSUIT,
	GETMAXPLAYERS,
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
	SUSPENDIFNOMATCHINGCARD,
	TAKECARDCOUNT,
	TAKECARDS,
	TAKEIFLOST,
	ENDFUNCTIONS
};

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct _checkMissing : public std::unary_function<const char *, void> {

	inline _checkMissing() : missing() {
		missing.reserve(sizeof(FUNCTIONS));
	}

	inline void operator()(const char *fn) {
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

}

using namespace NetMauMau::RuleSet;

LuaRuleSet::LuaRuleSet(const std::string &luafile, bool dirChangePossible, std::size_t icc,
					   const NetMauMau::IAceRoundListener *arl)
throw(NetMauMau::Lua::Exception::LuaException) : IRuleSet() {

	l.load(luafile, dirChangePossible, icc, arl);

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

std::vector<const char *> LuaRuleSet::checkInterface() {
	return std::for_each(FUNCTIONS, &FUNCTIONS[ENDFUNCTIONS], _checkMissing()).missing;
}

void LuaRuleSet::checkInitial(const NetMauMau::Player::IPlayer *player,
							  const NetMauMau::Common::ICard *playedCard)
throw(NetMauMau::Lua::Exception::LuaException) {
	checkCard(player, 0L, playedCard, player->isAIPlayer());
}

bool LuaRuleSet::checkCard(const NetMauMau::Player::IPlayer *player,
						   const NetMauMau::Common::ICard *uncoveredCard,
						   const NetMauMau::Common::ICard *playedCard,
						   bool) throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[CHECKCARD];

	lua_getglobal(l, fname);

	l.pushCard(uncoveredCard);
	l.pushCard(playedCard);

	try {

		l.pushPlayer(player);
		l.call(fname, 3);

	} catch(const NetMauMau::Common::Exception::SocketException &e) {
		lua_pop(l, 3);
		throw NetMauMau::Lua::Exception::LuaException(std::string("Internal error: ") + e.what(),
				fname);
	}

	return static_cast<bool>(lua_toboolean(l, -1));
}

bool LuaRuleSet::checkCard(const NetMauMau::Common::ICard *uncoveredCard,
						   const NetMauMau::Common::ICard *playedCard) const
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[CHECKCARD];

	lua_getglobal(l, fname);

	l.pushCard(uncoveredCard);
	l.pushCard(playedCard);
	lua_pushnil(l);
	l.call(fname, 3);

	return static_cast<bool>(lua_toboolean(l, -1));
}

std::size_t LuaRuleSet::lostPointFactor(const NetMauMau::Common::ICard *uncoveredCard) const
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[LOSTPOINTFACTOR];
	lua_getglobal(l, fname);
	l.pushCard(uncoveredCard);
	l.call(fname, 1);

	return std::max<std::size_t>(1, static_cast<std::size_t>(lua_tointeger(l, -1)));
}

bool LuaRuleSet::hasToSuspend() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[HASTOSUSPEND];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<bool>(lua_toboolean(l, -1));
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

	return static_cast<std::size_t>(lua_tointeger(l, -1));
}

std::size_t LuaRuleSet::takeCards(const NetMauMau::Common::ICard *playedCard) const
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[TAKECARDS];
	lua_getglobal(l, fname);
	l.pushCard(playedCard);
	l.call(fname, 1);

	return static_cast<std::size_t>(lua_tointeger(l, -1));
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

	return std::max<std::size_t>(1, static_cast<std::size_t>(lua_tointeger(l, -1)));
}

bool LuaRuleSet::suspendIfNoMatchingCard() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[SUSPENDIFNOMATCHINGCARD];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<bool>(lua_toboolean(l, -1));
}

bool LuaRuleSet::takeIfLost() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[TAKEIFLOST];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<bool>(lua_toboolean(l, -1));
}

bool LuaRuleSet::isAceRoundPossible() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[ISACEROUNDPOSSIBLE];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<bool>(lua_toboolean(l, -1));
}

NetMauMau::Common::ICard::RANK LuaRuleSet::getAceRoundRank() const
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[GETACEROUNDRANK];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<NetMauMau::Common::ICard::RANK>(lua_tointeger(l, -1));
}

bool LuaRuleSet::hasDirChange() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[HASDIRCHANGE];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<bool>(lua_toboolean(l, -1));
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

	return static_cast<bool>(lua_toboolean(l, -1));
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

	return static_cast<bool>(lua_toboolean(l, -1));
}

bool LuaRuleSet::isJackMode() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[ISJACKMODE];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<bool>(lua_toboolean(l, -1));
}

NetMauMau::Common::ICard::SUIT LuaRuleSet::getJackSuit() const
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[GETJACKSUIT];
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<NetMauMau::Common::ICard::SUIT>(lua_tointeger(l, -1));
}

void LuaRuleSet::setJackModeOff() throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = FUNCTIONS[SETJACKMODEOFF];
	lua_getglobal(l, fname);
	l.call(fname, 0, 0);
}

std::size_t LuaRuleSet::getMaxPlayers() const throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = FUNCTIONS[GETMAXPLAYERS];

	lua_getglobal(l, fname);
	l.call(fname, 0);

	const std::size_t r = static_cast<std::size_t>(lua_tointeger(l, -1));

	if(r <= 5) {
		return r;
	} else {
		logWarning("[Lua " << fname << "] " << r << ">5; fixing to 5...");
	}

	return 5;
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
