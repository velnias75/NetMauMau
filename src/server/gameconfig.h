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

#ifndef NETMAUMAU_SERVER_GAMECONFIG_H
#define NETMAUMAU_SERVER_GAMECONFIG_H

#include <string>

#include "engineconfig.h"

namespace NetMauMau {

namespace Event {
class IEventHandler;
}

namespace Server {

class GameConfig {
	GameConfig &operator=(const GameConfig &);
public:
	explicit GameConfig(const GameConfig &);
	explicit GameConfig(Event::IEventHandler &evtHdlr, long aiDelay, bool dirChange,
						bool aiPlayer = false, const std::string *aiName = 0L, char aceRound = 0,
						std::size_t factor = 1, std::size_t initialCardCount = 5);
	~GameConfig();

	bool getAIPlayer() const _PURE;
	const std::string *getAIName() const _PURE;
	EngineConfig &getEngineConfig() _CONST;

private:
	const bool m_aiPlayer;
	const std::string *m_aiName;
	EngineConfig m_engineCfg;
};

}

}

#endif /* NETMAUMAU_SERVER_GAMECONFIG_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
