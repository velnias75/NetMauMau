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

#include <vector>

#include "engineconfig.h"               // for EngineConfig

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
						Common::CARDCONFIG &cc, bool aiPlayer = false,
						const std::vector<std::string> &aiName = std::vector<std::string>(),
						char aceRound = 0);
	~GameConfig();

	bool hasAIPlayer() const _PURE;
	const std::vector<std::string> &getAINames() const _CONST;
	EngineConfig &getEngineConfig() _CONST;
	Common::CARDCONFIG &getCardConfig() const _PURE;

private:
	const bool m_aiPlayer;
	const std::vector<std::string> m_aiName;
	EngineConfig m_engineCfg;
	Common::CARDCONFIG &m_cardConfig;
};

}

}

#endif /* NETMAUMAU_SERVER_GAMECONFIG_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
