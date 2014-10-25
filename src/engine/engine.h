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

#ifndef NETMAUMAU_ENGINE_H
#define NETMAUMAU_ENGINE_H

#include <vector>
#include <string>

#include "linkercontrol.h"

namespace NetMauMau {

class ICardFactory;
class ICard;
class Talon;

namespace Event {
class IEventHandler;
}

namespace Player {
class IPlayer;
}

namespace RuleSet {
class IRuleSet;
}

class _EXPORT Engine {
	DISALLOW_COPY_AND_ASSIGN(Engine)

	typedef enum { ACCEPT_PLAYERS, NOCARDS, PLAYING, FINISHED } STATE;

public:
	typedef std::vector<Player::IPlayer *> PLAYERS;

	Engine(Event::IEventHandler &eventHandler, bool nextMessage = true);
	Engine(Event::IEventHandler &eventHandler, RuleSet::IRuleSet *ruleset, bool nextMessage = true);
	~Engine();

	bool addPlayer(Player::IPlayer *player);

	bool hasPlayers() const _PURE;
	void reversePlayers();
	std::size_t getPlayerCount() const _PURE;

	bool distributeCards();
	bool nextTurn();

	void message(const std::string &msg);

	void reset();

private:
	void calcScore(Player::IPlayer *p);
	PLAYERS::iterator find(const std::string &name);
	PLAYERS::iterator removePlayer(Player::IPlayer *player);
	void removePlayers();

private:
	Event::IEventHandler &m_eventHandler;

	STATE m_state;

	Talon *m_talon;
	RuleSet::IRuleSet *m_ruleset;
	PLAYERS m_players;
	mutable std::size_t m_nxtPlayer;
	mutable std::size_t m_turn;
	std::size_t m_curTurn;

	bool m_delRuleSet;
	bool m_jackMode;
	bool m_initialChecked;
	bool m_nextMessage;
};

}

#endif /* NETMAUMAU_ENGINE_H  */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
