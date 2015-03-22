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

LuaRuleSet::LuaRuleSet(const std::string &luafile, const NetMauMau::IAceRoundListener *arl)
	: IRuleSet(), m_validLua(l.load(luafile)) {}

LuaRuleSet::~LuaRuleSet() {}

void LuaRuleSet::checkInitial(const NetMauMau::Player::IPlayer *player,
							  const NetMauMau::Common::ICard *playedCard) {
	checkCard(player, 0L, playedCard, false);
}

bool LuaRuleSet::checkCard(const NetMauMau::Player::IPlayer *player,
						   const NetMauMau::Common::ICard *uncoveredCard,
						   const NetMauMau::Common::ICard *playedCard, bool) {
	if(m_validLua) {

		const char *fname = "checkCard";

		lua_getglobal(l, fname);

		l.pushCard(uncoveredCard);
		l.pushCard(playedCard);
		l.pushPlayer(player);

		if(l.call(fname, 3, 1)) return static_cast<bool>(lua_toboolean(l, -1));
	}

	return false;
}

bool LuaRuleSet::checkCard(const NetMauMau::Common::ICard *uncoveredCard,
						   const NetMauMau::Common::ICard *playedCard) const {
	if(m_validLua) {

		const char *fname = "checkCard";

		lua_getglobal(l, fname);

		l.pushCard(uncoveredCard);
		l.pushCard(playedCard);

		if(l.call(fname, 2, 1)) return static_cast<bool>(lua_toboolean(l, -1));
	}

	return false;
}

std::size_t LuaRuleSet::lostPointFactor(const NetMauMau::Common::ICard *uncoveredCard) const {

	if(m_validLua) {
		const char *fname = "lostPointFactor";
		lua_getglobal(l, fname);
		l.pushCard(uncoveredCard);

		if(l.call(fname, 1, 1)) return static_cast<std::size_t>(lua_tointeger(l, -1));
	}

	return 1;
}

bool LuaRuleSet::hasToSuspend() const {

	if(m_validLua) {
		const char *fname = "hasToSuspend";
		lua_getglobal(l, fname);

		if(l.call(fname, 0, 1)) return static_cast<bool>(lua_toboolean(l, -1));
	}

	return false;
}

void LuaRuleSet::hasSuspended() {
	if(m_validLua) {
		const char *fname = "hasSuspended";
		lua_getglobal(l, fname);
		l.call(fname, 0, 0);
	}
}

std::size_t LuaRuleSet::takeCardCount() const {

	if(m_validLua) {
		const char *fname = "takeCardCount";
		lua_getglobal(l, fname);

		if(l.call(fname, 0, 1)) return static_cast<std::size_t>(lua_tointeger(l, -1));
	}

	return 0;
}

std::size_t LuaRuleSet::takeCards(const NetMauMau::Common::ICard *playedCard) const {

	if(m_validLua) {
		const char *fname = "takeCards";
		lua_getglobal(l, fname);
		l.pushCard(playedCard);

		if(l.call(fname, 1, 1)) return static_cast<std::size_t>(lua_tointeger(l, -1));
	}

	return 0;
}

void LuaRuleSet::hasTakenCards() {
	if(m_validLua) {
		const char *fname = "hasTakenCards";
		lua_getglobal(l, fname);
		l.call(fname, 0, 0);
	}
}

std::size_t LuaRuleSet::initialCardCount() const {

	if(m_validLua) {
		const char *fname = "initialCardCount";
		lua_getglobal(l, fname);

		if(l.call(fname, 0, 1)) return static_cast<std::size_t>(lua_tointeger(l, -1));
	}

	return 5;
}

bool LuaRuleSet::suspendIfNoMatchingCard() const {

	if(m_validLua) {
		const char *fname = "suspendIfNoMatchingCard";
		lua_getglobal(l, fname);

		if(l.call(fname, 0, 1)) return static_cast<bool>(lua_toboolean(l, -1));
	}

	return false;
}

bool LuaRuleSet::takeIfLost() const {

	if(m_validLua) {
		const char *fname = "takeIfLost";
		lua_getglobal(l, fname);

		if(l.call(fname, 0, 1)) return static_cast<bool>(lua_toboolean(l, -1));
	}

	return false;
}

bool LuaRuleSet::isAceRoundPossible() const {

	if(m_validLua) {
		const char *fname = "isAceRoundPossible";
		lua_getglobal(l, fname);

		if(l.call(fname, 0, 1)) return static_cast<bool>(lua_toboolean(l, -1));
	}

	return false;
}

NetMauMau::Common::ICard::RANK LuaRuleSet::getAceRoundRank() const {

	if(m_validLua) {
		const char *fname = "getAceRoundRank";
		lua_getglobal(l, fname);

		if(l.call(fname, 0, 1)) {
			return static_cast<NetMauMau::Common::ICard::RANK>(lua_tointeger(l, -1));
		}
	}

	return NetMauMau::Common::ICard::RANK_ILLEGAL;
}

bool LuaRuleSet::hasDirChange() const {

	if(m_validLua) {
		const char *fname = "hasDirChange";
		lua_getglobal(l, fname);

		if(l.call(fname, 0, 1)) return static_cast<bool>(lua_toboolean(l, -1));
	}

	return false;
}

void LuaRuleSet::dirChanged() {
	if(m_validLua) {
		const char *fname = "dirChanged";
		lua_getglobal(l, fname);
		l.call(fname, 0, 0);
	}
}

bool LuaRuleSet::getDirChangeIsSuspend() const {

	if(m_validLua) {
		const char *fname = "getDirChangeIsSuspend";
		lua_getglobal(l, fname);

		if(l.call(fname, 0, 1)) return static_cast<bool>(lua_toboolean(l, -1));
	}

	return false;
}

void LuaRuleSet::setDirChangeIsSuspend(bool suspend) {
	if(m_validLua) {
		const char *fname = "takeIfLost";
		lua_getglobal(l, fname);
		lua_pushboolean(l, suspend);
		l.call(fname, 1, 0);
	}
}

bool LuaRuleSet::isAceRound() const {

	if(m_validLua) {
		const char *fname = "isAceRound";
		lua_getglobal(l, fname);

		if(l.call(fname, 0, 1)) return static_cast<bool>(lua_toboolean(l, -1));
	}

	return false;
}

bool LuaRuleSet::isJackMode() const {

	if(m_validLua) {
		const char *fname = "isJackMode";
		lua_getglobal(l, fname);

		if(l.call(fname, 0, 1)) return static_cast<bool>(lua_toboolean(l, -1));
	}

	return false;
}

NetMauMau::Common::ICard::SUIT LuaRuleSet::getJackSuit() const {

	if(m_validLua) {
		const char *fname = "getJackSuit";
		lua_getglobal(l, fname);

		if(l.call(fname, 0, 1)) {
			return static_cast<NetMauMau::Common::ICard::SUIT>(lua_tointeger(l, -1));
		}
	}

	return NetMauMau::Common::ICard::SUIT_ILLEGAL;
}

void LuaRuleSet::setJackModeOff() {
	if(m_validLua) {
		const char *fname = "setJackModeOff";
		lua_getglobal(l, fname);
		l.call(fname, 0, 0);
	}
}

std::size_t LuaRuleSet::getMaxPlayers() const {

	if(m_validLua) {

		const char *fname = "getMaxPlayers";

		lua_getglobal(l, fname);

		if(l.call(fname, 0, 1)) {

			const std::size_t r = static_cast<std::size_t>(lua_tointeger(l, -1));

			if(r <= 5) {
				return r;
			} else {
				logWarning("[Lua " << fname << "] " << r << ">5; fixing to 5...");
			}
		}
	}

	return 5;
}

void LuaRuleSet::setCurPlayers(std::size_t players) {
	if(m_validLua) {
		const char *fname = "setCurPlayers";
		lua_getglobal(l, fname);
		lua_pushinteger(l, static_cast<lua_Integer>(players));
		l.call(fname, 1, 0);
	}
}

void LuaRuleSet::reset() throw() {
	if(m_validLua) {
		const char *fname = "reset";
		lua_getglobal(l, fname);
		l.call(fname, 0, 0);
	}
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
