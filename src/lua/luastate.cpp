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

#include "luastate.h"

extern "C" {
#include <lauxlib.h>                    // for luaL_loadfile, etc
#include <lualib.h>                     // for luaL_openlibs
}

#include <cstring>                      // for strncmp

#include "logger.h"                     // for logInfo
#include "nullaceroundlistener.h"
#include "iplayer.h"                    // for IPlayer
#include "random_gen.h"                 // for genRandom
#include "serverplayerexception.h"
#include "protocol.h"                   // for CARDCOUNT

namespace {
const char *INTERFACE = "INTERFACE";
}

using namespace NetMauMau::Lua;

const NetMauMau::IAceRoundListener *NetMauMau::Lua::LuaState::m_arl =
	NullAceRoundListener::getInstance();

LuaState::LuaState() throw(Exception::LuaException) : m_state(luaL_newstate()) {

	if(m_state) {

		luaL_openlibs(m_state);

		lua_newtable(m_state);
		lua_pushinteger(m_state, NetMauMau::Common::ICard::DIAMONDS);
		lua_setfield(m_state, -2, "DIAMONDS");
		lua_pushinteger(m_state, NetMauMau::Common::ICard::HEARTS);
		lua_setfield(m_state, -2, "HEARTS");
		lua_pushinteger(m_state, NetMauMau::Common::ICard::SPADES);
		lua_setfield(m_state, -2, "SPADES");
		lua_pushinteger(m_state, NetMauMau::Common::ICard::CLUBS);
		lua_setfield(m_state, -2, "CLUBS");
		lua_pushinteger(m_state, NetMauMau::Common::ICard::SUIT_ILLEGAL);
		lua_setfield(m_state, -2, "SUIT_ILLEGAL");
		lua_setfield(m_state, LUA_GLOBALSINDEX, "SUIT");

		lua_newtable(m_state);
		lua_pushinteger(m_state, NetMauMau::Common::ICard::SEVEN);
		lua_setfield(m_state, -2, "SEVEN");
		lua_pushinteger(m_state, NetMauMau::Common::ICard::EIGHT);
		lua_setfield(m_state, -2, "EIGHT");
		lua_pushinteger(m_state, NetMauMau::Common::ICard::NINE);
		lua_setfield(m_state, -2, "NINE");
		lua_pushinteger(m_state, NetMauMau::Common::ICard::TEN);
		lua_setfield(m_state, -2, "TEN");
		lua_pushinteger(m_state, NetMauMau::Common::ICard::JACK);
		lua_setfield(m_state, -2, "JACK");
		lua_pushinteger(m_state, NetMauMau::Common::ICard::QUEEN);
		lua_setfield(m_state, -2, "QUEEN");
		lua_pushinteger(m_state, NetMauMau::Common::ICard::KING);
		lua_setfield(m_state, -2, "KING");
		lua_pushinteger(m_state, NetMauMau::Common::ICard::ACE);
		lua_setfield(m_state, -2, "ACE");
		lua_pushinteger(m_state, NetMauMau::Common::ICard::RANK_ILLEGAL);
		lua_setfield(m_state, -2, "RANK_ILLEGAL");
		lua_setfield(m_state, LUA_GLOBALSINDEX, "RANK");

		lua_register(m_state, "print", print);
		lua_register(m_state, "write", print);
		lua_register(m_state, "getRandomSuit", getRandomSuit);
		lua_register(m_state, "getJackChoice", playerGetJackChoice);
		lua_register(m_state, "getAceRoundChoice", playerGetAceRoundChoice);
		lua_register(m_state, "aceRoundStarted", playerAceRoundStarted);
		lua_register(m_state, "aceRoundEnded", playerAceRoundEnded);

	} else {
		// cppcheck-suppress exceptThrowInDestructor
		throw Exception::LuaException("couldn't initialize Lua");
	}
}

LuaState::~LuaState() {
	if(m_state) lua_close(m_state);
}

LuaState &LuaState::getInstance() throw(Exception::LuaException) {
	static LuaState instance;
	return instance;
}

void LuaState::load(const std::string &luafile, bool dirChangePossible,
					std::size_t initialCardCount,
					const NetMauMau::IAceRoundListener *arl) const throw(Exception::LuaException) {
	m_arl = arl;

	switch(luaL_loadfile(m_state, luafile.c_str())) {
	case LUA_ERRSYNTAX:
		throw Exception::LuaException(lua_tostring(m_state, -1));

	case LUA_ERRMEM:
		throw Exception::LuaException("memory allocation error", luafile.c_str());

	case LUA_ERRFILE:
		throw Exception::LuaException("cannot open/read the file", luafile.c_str());
	}

	call("init", 0, 0);

	lua_pushboolean(m_state, dirChangePossible);
	lua_setglobal(m_state, "nmm_dirChangePossible");
	lua_pushinteger(m_state, static_cast<lua_Integer>(initialCardCount));
	lua_setglobal(m_state, "nmm_initialCardCount");

	lua_newtable(m_state);
	lua_pushboolean(m_state, !m_arl->isNull());
	lua_setfield(m_state, -2, "ENABLED");

	if(arl) {
		lua_pushinteger(m_state, static_cast<lua_Integer>(arl->getAceRoundRank()));
	} else {
		lua_pushnil(m_state);
	}

	lua_setfield(m_state, -2, "RANK");
	lua_setglobal(m_state, "nmm_aceRound");
}

void LuaState::call(const char *fname, int nargs,
					int nresults) const throw(Exception::LuaException) {

	switch(lua_pcall(m_state, nargs, nresults, 0)) {
	case LUA_ERRRUN:
		throw Exception::LuaException(lua_tostring(m_state, -1), fname);

	case LUA_ERRMEM:
		throw Exception::LuaException("memory allocation error", fname);

	case LUA_ERRERR:
		throw Exception::LuaException("error while running the error handler function", fname);
	}
}

void LuaState::pushCard(const NetMauMau::Common::ICard *card) const throw() {

	if(card) {

		lua_newtable(m_state);
		lua_pushinteger(m_state, card->getSuit());
		lua_setfield(m_state, -2, "SUIT");
		lua_pushinteger(m_state, card->getRank());
		lua_setfield(m_state, -2, "RANK");
		lua_pushinteger(m_state, static_cast<lua_Integer>(card->getPoints()));
		lua_setfield(m_state, -2, "POINTS");
		new(lua_newuserdata(m_state, sizeof(NetMauMau::Common::ICardPtr)))
		NetMauMau::Common::ICardPtr(card);
		lua_setfield(m_state, -2, INTERFACE);

	} else {
		lua_pushnil(m_state);
	}
}

void LuaState::pushPlayer(const NetMauMau::Player::IPlayer *player) const
throw(NetMauMau::Common::Exception::SocketException) {

	if(player) {

		lua_newtable(m_state);
		lua_pushinteger(m_state, reinterpret_cast<lua_Integer>(player));
		lua_setfield(m_state, -2, "ID");

		try {
			lua_pushinteger(m_state, static_cast<lua_Integer>(player->getCardCount()));
			lua_setfield(m_state, -2, NetMauMau::Common::Protocol::V15::CARDCOUNT.c_str());
		} catch(const NetMauMau::Server::Exception::ServerPlayerException &) {
			lua_pop(m_state, 1);
			throw;
		} catch(const NetMauMau::Common::Exception::SocketException &) {
			lua_pop(m_state, 1);
			throw;
		}

		const NetMauMau::Player::IPlayer **bp =
			reinterpret_cast<const NetMauMau::Player::IPlayer **>(lua_newuserdata(m_state,
					sizeof(const NetMauMau::Player::IPlayer **)));
		*bp = player;
		lua_setfield(m_state, -2, INTERFACE);

	} else {
		lua_pushnil(m_state);
	}
}

NetMauMau::Common::ICardPtr LuaState::getCard(lua_State *l, int idx) {

	if(lua_isnil(l, idx)) return NetMauMau::Common::ICardPtr();

	NetMauMau::Common::ICardPtr c, *d;

	lua_pushnil(l);

	while(lua_next(l, idx) != 0) {

		if(!std::strncmp(INTERFACE, lua_tostring(l, -2), 9)) {
			d = reinterpret_cast<NetMauMau::Common::ICardPtr *>(lua_touserdata(l, -1));
			c = *d;
			d->~SmartPtr<NetMauMau::Common::ICard>();
		}

		lua_pop(l, 1);
	}

	return c;
}

int LuaState::print(lua_State *l) {

	if(lua_gettop(l)) logInfo("[Lua] " << lua_tostring(l, 1));

	return 0;
}

int LuaState::getRandomSuit(lua_State *l) {

	if(lua_gettop(l) != 0) {
		lua_pushstring(l, "getRandomSuit takes no arguments");
		return lua_error(l);
	}

	lua_pushinteger(l, static_cast<lua_Integer>
					(NetMauMau::Common::symbolToSuit(NetMauMau::Common::getSuitSymbols()
							[NetMauMau::Common::genRandom(4)])));
	return 1;
}

int LuaState::playerGetJackChoice(lua_State *l) {

	if(!(lua_gettop(l) == 3 && lua_type(l, 1) == LUA_TUSERDATA &&
			(lua_istable(l, 2) || lua_isnil(l, 2)) &&
			(lua_istable(l, 3) || lua_isnil(l, 3)))) {
		lua_pushstring(l, "incorrect arguments to getJackChoice");
		return lua_error(l);
	}

	lua_pushinteger(l, (*reinterpret_cast<const NetMauMau::Player::IPlayer **>
						(lua_touserdata(l, 1)))->getJackChoice(getCard(l, 2), getCard(l, 3)));
	return 1;
}

int LuaState::playerGetAceRoundChoice(lua_State *l) {

	if(!(lua_gettop(l) == 1 && lua_type(l, 1) == LUA_TUSERDATA)) {
		lua_pushstring(l, "incorrect argument to getAceRoundChoice");
		return lua_error(l);
	}

	lua_pushboolean(l, (*reinterpret_cast<const NetMauMau::Player::IPlayer **>
						(lua_touserdata(l, 1)))->getAceRoundChoice());
	return 1;
}

int LuaState::playerAceRoundStarted(lua_State *l) {

	if(!(lua_gettop(l) == 1 && lua_type(l, 1) == LUA_TUSERDATA)) {
		lua_pushstring(l, "incorrect argument to aceRoundStarted");
		return lua_error(l);
	}

	m_arl->aceRoundStarted(*reinterpret_cast<const NetMauMau::Player::IPlayer **>
						   (lua_touserdata(l, 1)));

	return 0;
}

int LuaState::playerAceRoundEnded(lua_State *l) {

	if(!(lua_gettop(l) == 1 && lua_type(l, 1) == LUA_TUSERDATA)) {
		lua_pushstring(l, "incorrect argument to aceRoundEnded");
		return lua_error(l);
	}

	m_arl->aceRoundEnded(*reinterpret_cast<const NetMauMau::Player::IPlayer **>
						 (lua_touserdata(l, 1)));

	return 0;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
