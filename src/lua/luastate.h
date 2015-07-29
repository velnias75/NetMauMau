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

#ifndef NETMAUMAU_LUA_LUASTATE
#define NETMAUMAU_LUA_LUASTATE

extern "C" {
#include <lua.h>                        // for lua_State
}

#include <cstddef>                      // for size_t

#include "icard.h"                      // for ICard (ptr only), ICardPtr
#include "luaexception.h"               // for LuaException

namespace NetMauMau {

class IAceRoundListener;

namespace Player {
class IPlayer;
}

namespace Lua {

class LuaState {
	DISALLOW_COPY_AND_ASSIGN(LuaState)
public:
	~LuaState();

	static LuaState &getInstance() throw(Exception::LuaException);

	void load(const std::string &luafile, bool dirChangePossible,
			  std::size_t initialCardCount, const NetMauMau::IAceRoundListener *arl) const
	throw(Exception::LuaException) _NONNULL_ALL;
	void call(const char *fname, int nargs, int nresults = 1) const throw(Exception::LuaException);

	void pushCard(const Common::ICard *card) const throw();
	void pushPlayer(const Player::IPlayer *player) const throw(Common::Exception::SocketException);

	inline operator lua_State *() const {
		return m_state;
	}

private:
	explicit LuaState() throw(Exception::LuaException);

	static Common::ICardPtr getCard(lua_State *l, int idx) _NOUNUSED;

	static int print(lua_State *l);
	static int getRandomSuit(lua_State *l);
	static int playerGetJackChoice(lua_State *l);
	static int playerGetAceRoundChoice(lua_State *l);
	static int playerAceRoundStarted(lua_State *l);
	static int playerAceRoundEnded(lua_State *l);

private:
	static const NetMauMau::IAceRoundListener *m_arl;
	lua_State *m_state;
};

}

}

#endif /* NETMAUMAU_LUA_LUASTATE */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
