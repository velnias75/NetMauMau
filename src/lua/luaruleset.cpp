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

#include "luastate.h"
#include "iplayer.h"
#include "logger.h"

namespace {
const NetMauMau::Lua::LuaState &l = NetMauMau::Lua::LuaState::getInstance();
}

using namespace NetMauMau::RuleSet;

LuaRuleSet::LuaRuleSet(const std::string &luafile, bool dirChangePossible, std::size_t icc,
					   const NetMauMau::IAceRoundListener *arl)
throw(NetMauMau::Lua::Exception::LuaException) : IRuleSet() {
	l.load(luafile, dirChangePossible, icc, arl);
	reset();
}

LuaRuleSet::~LuaRuleSet() {}

void LuaRuleSet::checkInitial(const NetMauMau::Player::IPlayer *player,
							  const NetMauMau::Common::ICard *playedCard)
throw(NetMauMau::Lua::Exception::LuaException) {
	checkCard(player, 0L, playedCard, false);
}

bool LuaRuleSet::checkCard(const NetMauMau::Player::IPlayer *player,
						   const NetMauMau::Common::ICard *uncoveredCard,
						   const NetMauMau::Common::ICard *playedCard,
						   bool) throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = "checkCard";

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

	const char *fname = "checkCard";

	lua_getglobal(l, fname);

	l.pushCard(uncoveredCard);
	l.pushCard(playedCard);
	lua_pushnil(l);
	l.call(fname, 3);

	return static_cast<bool>(lua_toboolean(l, -1));
}

std::size_t LuaRuleSet::lostPointFactor(const NetMauMau::Common::ICard *uncoveredCard) const
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = "lostPointFactor";
	lua_getglobal(l, fname);
	l.pushCard(uncoveredCard);
	l.call(fname, 1);

	return static_cast<std::size_t>(lua_tointeger(l, -1));
}

bool LuaRuleSet::hasToSuspend() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = "hasToSuspend";
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<bool>(lua_toboolean(l, -1));
}

void LuaRuleSet::hasSuspended() throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = "hasSuspended";
	lua_getglobal(l, fname);
	l.call(fname, 0, 0);
}

std::size_t LuaRuleSet::takeCardCount() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = "takeCardCount";
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<std::size_t>(lua_tointeger(l, -1));
}

std::size_t LuaRuleSet::takeCards(const NetMauMau::Common::ICard *playedCard) const
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = "takeCards";
	lua_getglobal(l, fname);
	l.pushCard(playedCard);
	l.call(fname, 1);

	return static_cast<std::size_t>(lua_tointeger(l, -1));
}

void LuaRuleSet::hasTakenCards() throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = "hasTakenCards";
	lua_getglobal(l, fname);
	l.call(fname, 0, 0);
}

std::size_t LuaRuleSet::initialCardCount() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = "initialCardCount";
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<std::size_t>(lua_tointeger(l, -1));
}

bool LuaRuleSet::suspendIfNoMatchingCard() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = "suspendIfNoMatchingCard";
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<bool>(lua_toboolean(l, -1));
}

bool LuaRuleSet::takeIfLost() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = "takeIfLost";
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<bool>(lua_toboolean(l, -1));
}

bool LuaRuleSet::isAceRoundPossible() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = "isAceRoundPossible";
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<bool>(lua_toboolean(l, -1));
}

NetMauMau::Common::ICard::RANK LuaRuleSet::getAceRoundRank() const
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = "getAceRoundRank";
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<NetMauMau::Common::ICard::RANK>(lua_tointeger(l, -1));
}

bool LuaRuleSet::hasDirChange() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = "hasDirChange";
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<bool>(lua_toboolean(l, -1));
}

void LuaRuleSet::dirChanged() throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = "dirChanged";
	lua_getglobal(l, fname);
	l.call(fname, 0, 0);
}

bool LuaRuleSet::getDirChangeIsSuspend() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = "getDirChangeIsSuspend";
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<bool>(lua_toboolean(l, -1));
}

void LuaRuleSet::setDirChangeIsSuspend(bool suspend)
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = "setDirChangeIsSuspend";
	lua_getglobal(l, fname);
	lua_pushboolean(l, suspend);
	l.call(fname, 1, 0);
}

bool LuaRuleSet::isAceRound() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = "isAceRound";
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<bool>(lua_toboolean(l, -1));
}

bool LuaRuleSet::isJackMode() const throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = "isJackMode";
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<bool>(lua_toboolean(l, -1));
}

NetMauMau::Common::ICard::SUIT LuaRuleSet::getJackSuit() const
throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = "getJackSuit";
	lua_getglobal(l, fname);
	l.call(fname, 0);

	return static_cast<NetMauMau::Common::ICard::SUIT>(lua_tointeger(l, -1));
}

void LuaRuleSet::setJackModeOff() throw(NetMauMau::Lua::Exception::LuaException) {
	const char *fname = "setJackModeOff";
	lua_getglobal(l, fname);
	l.call(fname, 0, 0);
}

std::size_t LuaRuleSet::getMaxPlayers() const throw(NetMauMau::Lua::Exception::LuaException) {

	const char *fname = "getMaxPlayers";

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
	const char *fname = "setCurPlayers";
	lua_getglobal(l, fname);
	lua_pushinteger(l, static_cast<lua_Integer>(players));
	l.call(fname, 1, 0);
}

void LuaRuleSet::reset() throw() {

	const char *fname = "init";
	lua_getglobal(l, fname);

	try {
		l.call(fname, 0, 0);
	} catch(const NetMauMau::Lua::Exception::LuaException &e) {
		logWarning(e);
	}
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
