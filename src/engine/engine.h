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

#include <cstddef>                      // for size_t, NULL
#include <vector>                       // for vector

#include "iaceroundlistener.h"          // for IAceRoundListener
#include "icardcountobserver.h"         // for ICardCountObserver
#include "italonchange.h"               // for ITalonChange
#include "socketexception.h"            // for SocketException, SOCKET
#include "sqlite.h"

namespace NetMauMau {

class IPlayedOutCards;
class EngineContext;
class Talon;

namespace Event {
class IEventHandler;
}

namespace RuleSet {
class IRuleSet;
}

class _EXPORT Engine : private ITalonChange, public IAceRoundListener,
	private ICardCountObserver {
	DISALLOW_COPY_AND_ASSIGN(Engine)
	typedef enum { ACCEPT_PLAYERS, NOCARDS, PLAYING, FINISHED } STATE;
public:
	typedef std::vector<Player::IPlayer *> PLAYERS;

	explicit Engine(EngineContext &ctx) throw(Common::Exception::SocketException);

	virtual ~Engine();

	inline const EngineContext &getContext() const {
		return m_ctx;
	}

	const Event::IEventHandler &getEventHandler() const _PURE;

	inline void setAlwaysWait(bool w) {
		m_alwaysWait = w;
	}

	inline void setUltimate(bool u) {
		m_ultimate = u;
	}

	inline void setGameId(DB::GAMEIDX gameIndex) {
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

	const NetMauMau::IPlayedOutCards *getPlayedOutCards() const _PURE;

	void reset() throw();

protected:
	virtual void uncoveredCard(const Common::ICard *top) const
	throw(Common::Exception::SocketException);
	virtual void talonEmpty(bool empty) const throw();
	virtual void shuffled() const;
	virtual void underflow();

	virtual Common::ICard::RANK getAceRoundRank() const _PURE;
	virtual void aceRoundStarted(const Player::IPlayer *player) const
	throw(Common::Exception::SocketException);
	virtual void aceRoundEnded(const Player::IPlayer *player) const
	throw(Common::Exception::SocketException);

private:
	void calcScore(Player::IPlayer *player) const;

	bool checkCard(Player::IPlayer *player, Common::ICardPtr &playedCard,
				   const Common::ICardPtr &uncoveredCard) const
	throw(Common::Exception::SocketException);

	void informAIStat() const;
	std::size_t countAI() const;

	inline std::size_t getAICount() const {
		return m_aiCount;
	}

	void setDirChangeIsSuspend(bool b) throw(Common::Exception::SocketException);
	void jackModeOff() const throw(Common::Exception::SocketException);

	void checkAndPerformDirChange(const Player::IPlayer *player, bool won)
	throw(Common::Exception::SocketException);

	void suspends(Player::IPlayer *player, const Common::ICard *uncoveredCard = NULL) const
	throw(Common::Exception::SocketException);
	bool takeCards(Player::IPlayer *player, const Common::ICard *card) const
	throw(Common::Exception::SocketException);
	void handleWinner(const Player::IPlayer *player) throw(Common::Exception::SocketException);

	RuleSet::IRuleSet *getRuleSet() const;

	PLAYERS::const_iterator find(const std::string &name) const;
	PLAYERS::iterator removePlayer(Player::IPlayer *player)
	throw(Common::Exception::SocketException);

	inline void removePlayers() throw() {
		m_players.clear();
		m_aiCount = 0;
	}

	PLAYERS::iterator erasePlayer(const PLAYERS::iterator &pi);

	void disconnectError(SOCKET fd) const;

	void checkPlayersAlive() const throw(Common::Exception::SocketException);

	virtual void cardCountChanged(const Player::IPlayer *player) const throw();

	long getAIDelay() const;

	bool wait(const Player::IPlayer *player, bool suspend) const;

private:
	EngineContext &m_ctx;
	const DB::SQLite::SQLitePtr m_db;

	STATE m_state;
	Talon *const m_talon;
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
	DB::GAMEIDX m_gameIndex;
	bool m_dirChangeEnabled;
	bool m_talonUnderflow;

	std::size_t m_aiCount;
};

}

#endif /* NETMAUMAU_ENGINE_H  */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
