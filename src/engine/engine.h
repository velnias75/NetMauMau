/*
 * Copyright 2014-2015 by Heiko Schäfer <heiko@rangun.de>
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

#include "icardcountobserver.h"
#include "iaceroundlistener.h"
#include "socketexception.h"
#include "italonchange.h"

namespace NetMauMau {

class EngineConfig;
class Talon;

namespace Event {
class IEventHandler;
}

namespace RuleSet {
class IRuleSet;
}

class _EXPORT Engine : protected ITalonChange, protected IAceRoundListener,
	protected ICardCountObserver {

	DISALLOW_COPY_AND_ASSIGN(Engine)

	typedef enum { ACCEPT_PLAYERS, NOCARDS, PLAYING, FINISHED } STATE;

public:
	typedef std::vector<Player::IPlayer *> PLAYERS;

	Engine(EngineConfig &config) throw(Common::Exception::SocketException);

	virtual ~Engine();

	inline const EngineConfig &getConfig() const {
		return m_cfg;
	}

	const Event::IEventHandler &getEventHandler() const _PURE;

	inline void setAlwaysWait(bool w) {
		m_alwaysWait = w;
	}

	inline void setUltimate(bool ultimate) {
		m_ultimate = ultimate;
	}

	inline void setGameId(long long int gameIndex) {
		m_gameIndex = gameIndex;
	}

	bool addPlayer(Player::IPlayer *player) throw(Common::Exception::SocketException);
	void removePlayer(const std::string &player);

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
	void error(const std::string &msg) const throw();

	void gameAboutToStart() const;
	void gameOver() const throw();
	void reset() throw();

protected:


	virtual void uncoveredCard(const Common::ICard *top) const
	throw(Common::Exception::SocketException);
	virtual void talonEmpty(bool empty) const throw();
	virtual void cardPlayed(Common::ICard *card) const;
	virtual void cardTaken(const NetMauMau::Common::ICard* = 0L) const
	throw(Common::Exception::SocketException) _CONST;
	virtual void shuffled() const;

	virtual Common::ICard::RANK getAceRoundRank() const _PURE;
	virtual void aceRoundStarted(const Player::IPlayer *player) const
	throw(Common::Exception::SocketException);
	virtual void aceRoundEnded(const Player::IPlayer *player) const
	throw(Common::Exception::SocketException);

private:
	void calcScore(Player::IPlayer *p) const;

	void informAIStat() const;
	std::size_t getAICount() const;
	void setDirChangeIsSuspend(bool b);
	void jackModeOff() const;

	void suspends(Player::IPlayer *p, const Common::ICard *uc = NULL) const
	throw(Common::Exception::SocketException);
	bool takeCards(Player::IPlayer *player, const Common::ICard *card) const
	throw(Common::Exception::SocketException);

	PLAYERS::const_iterator find(const std::string &name) const;
	PLAYERS::iterator removePlayer(Player::IPlayer *player);

	inline void removePlayers() throw() {
		m_players.clear();
	}

	void checkPlayersAlive() const throw(Common::Exception::SocketException);

	virtual void cardCountChanged(Player::IPlayer *player) const throw();

	long getAIDelay() const;
	bool wait(const Player::IPlayer *p, bool suspend) const;

private:
	EngineConfig &m_cfg;

	STATE m_state;
	Talon *m_talon;
	PLAYERS m_players;
	std::size_t m_nxtPlayer;
	std::size_t m_turn;
	std::size_t m_curTurn;

	bool m_jackMode;
	bool m_initialChecked;
	bool m_ultimate;
	bool m_initialJack;
	bool m_alwaysWait;
	bool m_alreadyWaited;

	const bool m_initialNextMessage;
	long long int m_gameIndex;
	bool m_dirChangeEnabled;
};

}

#endif /* NETMAUMAU_ENGINE_H  */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
