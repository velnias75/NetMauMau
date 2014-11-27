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

#include "socketexception.h"
#include "italonchange.h"

namespace NetMauMau {

class ICardFactory;
class Talon;

namespace Common {
class ICard;
}

namespace Event {
class IEventHandler;
}

namespace Player {
class IPlayer;
}

namespace RuleSet {
class IRuleSet;
}

class _EXPORT Engine : protected ITalonChange {
	DISALLOW_COPY_AND_ASSIGN(Engine)

	typedef enum { ACCEPT_PLAYERS, NOCARDS, PLAYING, FINISHED } STATE;

public:
	typedef std::vector<Player::IPlayer *> PLAYERS;

	Engine(Event::IEventHandler &eventHandler, long aiDelay = 1000L, bool nextMessage = true);
	Engine(Event::IEventHandler &eventHandler, long aiDelay, RuleSet::IRuleSet *ruleset,
		   bool nextMessage = true);

	virtual ~Engine();

	inline void setAlwaysWait(bool wait) {
		m_alwaysWait = wait;
	}

	inline void setUltimate(bool ultimate) {
		m_ultimate = ultimate;
	}

	bool addPlayer(Player::IPlayer *player) throw(Common::Exception::SocketException);

	inline bool hasPlayers() const {
		return m_players.size() > 1;
	}

	void setFirstPlayer(Player::IPlayer *p);

	inline std::size_t getPlayerCount() const throw() {
		return m_players.size();
	}

	bool distributeCards() throw(Common::Exception::SocketException);
	bool nextTurn();

	void message(const std::string &msg) const throw(Common::Exception::SocketException);

	void gameOver() const throw();
	void reset() throw();

protected:
	virtual void uncoveredCard(const Common::ICard *top) const
	throw(Common::Exception::SocketException);
	virtual void talonEmpty(bool empty) const throw();
	virtual void cardPlayed(Common::ICard *card) const;
	virtual void shuffled() const;

private:
	void calcScore(Player::IPlayer *p) const;

	void informAIStat() const;
	std::size_t getAICount() const;

	void suspends(Player::IPlayer *p, const Common::ICard *uc = NULL) const;

	PLAYERS::const_iterator find(const std::string &name) const;
	PLAYERS::iterator removePlayer(Player::IPlayer *player);

	inline void removePlayers() throw() {
		m_players.clear();
	}

private:
	Event::IEventHandler &m_eventHandler;

	STATE m_state;

	Talon *m_talon;
	RuleSet::IRuleSet *m_ruleset;
	PLAYERS m_players;
	std::size_t m_nxtPlayer;
	std::size_t m_turn;
	std::size_t m_curTurn;

	const bool m_delRuleSet;

	bool m_jackMode;
	bool m_initialChecked;
	bool m_nextMessage;
	bool m_ultimate;
	bool m_initialJack;
	bool m_alwaysWait;

	const bool m_initialNextMessage;
	const long m_aiDelay;
};

}

#endif /* NETMAUMAU_ENGINE_H  */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
