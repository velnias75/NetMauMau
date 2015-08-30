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

#ifndef NETMAUMAU_NEXTTURN_H
#define NETMAUMAU_NEXTTURN_H

#include "engine.h"
#include "iplayer.h"

namespace NetMauMau {

class NextTurn {
	DISALLOW_COPY_AND_ASSIGN(NextTurn)
public:
	explicit NextTurn(Engine *const engine) throw(Common::Exception::SocketException);
	~NextTurn();

	bool compute() throw(Common::Exception::SocketException);

private:
	void informAIStat() const;

	inline std::size_t getAICount() const {
		return m_engine->m_aiCount;
	}

	bool checkCard(Player::IPlayer *player, Common::ICardPtr &playedCard,
				   const Common::ICardPtr &uncoveredCard) const
	throw(Common::Exception::SocketException);

	bool takeCards(Player::IPlayer *player, const Common::ICard *card) const
	throw(Common::Exception::SocketException);

	void suspends(Player::IPlayer *player, const Common::ICard *uncoveredCard = NULL) const
	throw(Common::Exception::SocketException);

	void setDirChangeIsSuspend(bool b) throw(Common::Exception::SocketException);
	void checkAndPerformDirChange(const Player::IPlayer *player, bool won)
	throw(Common::Exception::SocketException);

	void jackModeOff() const throw(Common::Exception::SocketException);

	void handleWinner(const Player::IPlayer *player) throw(Common::Exception::SocketException);

	void disconnectError(SOCKET fd) const;

	void checkPlayersAlive() const throw(Common::Exception::SocketException);

	long getAIDelay() const;
	bool wait(const Player::IPlayer *player, bool suspend) const;

private:
	Engine *const m_engine;
	const DB::SQLite::SQLitePtr m_db;

	Engine::PLAYERS::value_type m_player;
	Engine::PLAYERS::value_type m_curPlayer;
	Engine::PLAYERS::value_type m_rightPlayer;

	Common::ICardPtr m_uncoveredCard;
	Common::ICardPtr m_playedCard;

	std::size_t m_neighbourCount[2];
	std::size_t m_leftCount;
	std::size_t m_rightCount;
	std::size_t m_nxtPlayer;

	Player::IPlayer::REASON m_reason;

	Common::ICard::SUIT m_jackSuit;
	Common::ICard::SUIT m_lps;
	Common::ICard::RANK m_lpr;
	Common::ICard::SUIT m_rps;
	Common::ICard::RANK m_rpr;

	Player::IPlayer::NEIGHBOURRANKSUIT m_nrs;

	bool m_jackMode;
	bool m_initialJack;
	bool m_suspend;
	bool m_won;
	bool m_noCardOk;
	bool m_cardAccepted;
	bool m_lostWatchingPlayer;
	bool m_alreadyWaited;
};

}

#endif /* NETMAUMAU_NEXTTURN_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
