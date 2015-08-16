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

#ifndef NETMAUMAU_SERVER_GAMECONTEXT_H
#define NETMAUMAU_SERVER_GAMECONTEXT_H

#include <vector>

#include "enginecontext.h"

namespace NetMauMau {

namespace Event {
class IEventHandler;
}

namespace Server {

class GameContext {
	GameContext &operator=(const GameContext &);
public:
	typedef std::vector<std::string> AINAMES;

	explicit GameContext(const GameContext &) throw();
	explicit GameContext(Event::IEventHandler &evtHdlr, long aiDelay, bool dirChange,
						 Common::CARDCONFIG &cc, bool aiPlayer = false,
						 const AINAMES &aiNames = AINAMES(), char aceRound = 0) throw();
	~GameContext() throw();

	inline bool hasAIPlayer() const throw() {
		return m_aiPlayer;
	}

	inline const AINAMES &getAINames() const throw() {
		return m_aiNames;
	}

	inline EngineContext &getEngineContext() throw() {
		return m_engineCtx;
	}

	inline Common::CARDCONFIG &getCardConfig() const throw() {
		return m_cardConfig;
	}

private:
	const bool m_aiPlayer;
	const AINAMES m_aiNames;
	EngineContext m_engineCtx;
	Common::CARDCONFIG &m_cardConfig;
};

}

}

#endif /* NETMAUMAU_SERVER_GAMECONTEXT_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
