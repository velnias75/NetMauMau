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

#include <cstdio>
#include <cassert>

#include "nextturn.h"

#include "luafatalexception.h"
#include "abstractsocket.h"
#include "ieventhandler.h"
#include "enginecontext.h"
#include "iruleset.h"
#include "talon.h"

#include "protocol.h"

namespace {
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct _informCardCount : public std::unary_function<NetMauMau::Player::IPlayer *, void> {
	explicit inline _informCardCount(NetMauMau::Player::IPlayer *player) : m_player(player) {}
	inline result_type operator()(const argument_type player) const {
		if(m_player != player) m_player->informAIStat(player, player->getCardCount(),
					player->getLastPlayedSuit(), player->getLastPlayedRank());
	}

private:
	NetMauMau::Player::IPlayer *m_player;
};

struct _informAIStat : public std::unary_function<NetMauMau::Player::IPlayer *, void> {
	explicit inline _informAIStat(const NetMauMau::Engine::PLAYERS &players) : m_players(players) {}
	inline result_type operator()(const argument_type p) const {
		if(p->isAIPlayer()) std::for_each(m_players.begin(), m_players.end(), _informCardCount(p));
	}

private:
	const NetMauMau::Engine::PLAYERS &m_players;
};
#pragma GCC diagnostic pop
}

using namespace NetMauMau;

NextTurn::NextTurn(Engine *const engine) throw(Common::Exception::SocketException)
	: m_engine(engine), m_db(DB::SQLite::getInstance()),
	  m_player(m_engine->m_players[engine->m_nxtPlayer]), m_curPlayer(0L), m_rightPlayer(0L),
	  m_uncoveredCard(), m_playedCard(), m_neighbourCount(), m_leftCount(0u), m_rightCount(0u),
	  m_reason(Player::IPlayer::MAUMAU), m_jackSuit(Common::ICard::SUIT_ILLEGAL),
	  m_lps(Common::ICard::SUIT_ILLEGAL), m_lpr(Common::ICard::RANK_ILLEGAL),
	  m_rps(Common::ICard::SUIT_ILLEGAL), m_rpr(Common::ICard::RANK_ILLEGAL), m_nrs(),
	  m_initialJack(false), m_suspend(false), m_won(false), m_noCardOk(false),
	  m_cardAccepted(false), m_lostWatchingPlayer(false) {

	checkPlayersAlive();

	m_db->gamePlayStarted(m_engine->m_gameIndex);

	m_engine->getEventHandler().initialCard(m_engine->m_talon->uncoverCard());

	if(getAICount() && m_engine->m_talon->getUncoveredCard() == Common::ICard::EIGHT) {
		m_engine->getEventHandler().getConnection().
		wait(static_cast<long int>(std::floor(static_cast<float>(getAIDelay()) * 1.5f)));
	}

	m_uncoveredCard = m_engine->m_talon->getUncoveredCard();
	m_engine->getRuleSet()->checkInitial(m_player, m_uncoveredCard);

	checkAndPerformDirChange(m_player, false);

	m_initialJack = m_engine->getRuleSet()->isJackMode();
}

NextTurn::~NextTurn() {}

bool NextTurn::compute() throw(Common::Exception::SocketException) {

	m_engine->m_alreadyWaited = false;

	if(m_engine->getEventHandler().shutdown() || m_engine->m_state != Engine::PLAYING) return false;

	if(getAICount() == 1) {
		m_engine->m_alwaysWait = false;
		m_engine->m_ctx.setNextMessage(false);
	}

	try {

		checkPlayersAlive();

		m_player = m_engine->m_players[m_engine->m_nxtPlayer];

		if(m_engine->m_curTurn != m_engine->m_turn) {
			m_db->turn(m_engine->m_gameIndex, m_engine->m_turn);
			m_engine->getEventHandler().turn(m_engine->m_turn);
			m_engine->m_curTurn = m_engine->m_turn;
		}

		if(m_engine->m_ctx.getNextMessage()) m_engine->getEventHandler().nextPlayer(m_player);

		m_suspend = m_engine->getRuleSet()->hasToSuspend() && !m_engine->m_talonUnderflow;
		m_jackSuit = m_engine->getRuleSet()->getJackSuit();
		m_uncoveredCard = m_engine->m_talon->getUncoveredCard();

		assert(m_uncoveredCard != Common::ICard::JACK || (m_uncoveredCard == Common::ICard::JACK &&
				((m_engine->m_jackMode || m_initialJack) &&
				 m_jackSuit != Common::ICard::SUIT_ILLEGAL)));

		m_playedCard = !m_suspend ? m_player->requestCard(m_uncoveredCard, (m_engine->m_jackMode ||
					   m_initialJack) ? &m_jackSuit : 0L, m_engine->getRuleSet()->takeCardCount(),
					   m_engine->m_talonUnderflow) : Common::ICardPtr();

		if(m_initialJack && !m_playedCard) m_engine->m_jackMode = true;

		m_won = false;

		if(!m_suspend) {

			m_noCardOk = takeCards(m_player, m_playedCard);

sevenRule:

			if(!m_playedCard) {

				m_reason = m_noCardOk ? Player::IPlayer::SUSPEND :
						   m_player->getNoCardReason(m_uncoveredCard, m_engine->m_jackMode ?
													 &m_jackSuit : 0L);

				if(!m_noCardOk && (m_playedCard = m_engine->m_talon->takeCard())) {
					m_player->receiveCard(m_playedCard);
					m_engine->getEventHandler().playerPicksCard(m_player);
				}

				if(m_reason == Player::IPlayer::SUSPEND) {
					suspends(m_player);
					m_playedCard = Common::ICardPtr();
				} else if(m_reason == Player::IPlayer::NOMATCH) {
					m_playedCard = Common::ICardPtr(const_cast<const Common::ICard *>
													(Common::getIllegalCard()));
				}

			} else if(m_playedCard == Common::ICard::SUIT_ILLEGAL) {
				m_playedCard = m_player->requestCard(m_uncoveredCard, m_engine->m_jackMode ?
													 &m_jackSuit : 0L,
													 m_engine->getRuleSet()->takeCardCount());
				goto sevenRule;
			}

			m_cardAccepted = checkCard(m_player, m_playedCard, m_uncoveredCard);

			if(m_playedCard && m_cardAccepted) {

				if(m_engine->m_jackMode || m_initialJack) jackModeOff();

				m_won = m_player->cardAccepted(m_playedCard);

				m_engine->m_talon->playCard(m_playedCard);
				m_engine->m_talonUnderflow = m_engine->m_talon->thresholdReached(1u +
											 (8u * m_engine->m_ctx.getTalonFactor()));

				m_engine->getEventHandler().playerPlaysCard(m_player, m_playedCard,
						m_uncoveredCard);

				if((m_engine->m_jackMode = m_engine->getRuleSet()->isJackMode())) {
					m_engine->getEventHandler().playerChooseJackSuit(m_player,
							m_engine->getRuleSet()->getJackSuit());
				}

				if(m_won) {
					handleWinner(m_player);
				} else if(wait(m_player, true) && (m_playedCard == Common::ICard::EIGHT ||
												   (m_playedCard == Common::ICard::NINE &&
													m_engine->getRuleSet()->
													getDirChangeIsSuspend()))) {
					m_engine->getEventHandler().getConnection().wait(getAIDelay());
					m_engine->m_alreadyWaited = true;
				}
			}

		} else {

			suspends(m_player, m_uncoveredCard);
			m_engine->getRuleSet()->hasSuspended();

			if(m_engine->m_jackMode) {
				jackModeOff();
				m_engine->m_jackMode = false;
			}
		}

		checkAndPerformDirChange(m_player, m_won);

		m_curPlayer = m_player;

		if(!m_won) {

			m_leftCount = m_player->getCardCount();
			m_lps = m_player->getLastPlayedSuit();
			m_lpr = m_player->getLastPlayedRank();

			m_engine->m_nxtPlayer = (m_engine->m_nxtPlayer + 1u) >= m_engine->m_players.size() ?
									0u : m_engine->m_nxtPlayer + 1u;

			m_rightPlayer = m_engine->m_players[(m_engine->m_nxtPlayer + 1u) >=
												m_engine->m_players.size() ? 0u :
												m_engine->m_nxtPlayer + 1u];

			m_rightCount = m_rightPlayer->getCardCount();
			m_rps = m_rightPlayer->getLastPlayedSuit();
			m_rpr = m_rightPlayer->getLastPlayedRank();

			m_neighbourCount[0] = m_leftCount;
			m_neighbourCount[1] = m_rightCount;

			m_nrs.rank[Player::IPlayer::LEFT]  = m_lpr;
			m_nrs.rank[Player::IPlayer::RIGHT] = m_rpr;
			m_nrs.suit[Player::IPlayer::LEFT]  = m_lps;
			m_nrs.suit[Player::IPlayer::RIGHT] = m_rps;

			m_engine->m_players[m_engine->m_nxtPlayer]->
			setNeighbourCardStats(m_engine->m_players.size(), m_neighbourCount, m_nrs);
		}

		if(!m_engine->m_nxtPlayer) ++m_engine->m_turn;

		m_initialJack = false;

		if(!m_engine->m_alreadyWaited && wait(m_curPlayer, false)) {
			m_engine->getEventHandler().getConnection().wait(getAIDelay());
		}

		informAIStat();

	} catch(const Lua::Exception::LuaFatalException &) {
		throw;
	} catch(const Common::Exception::SocketException &e) {

		logDebug("SocketException: " << e);

		if(Engine::getTalonUnderflowString() == e.what()) {
			try {
				m_engine->getEventHandler().error(Engine::getTalonUnderflowString());
			} catch(const Common::Exception::SocketException &) {}
		}

		Common::IConnection &con(m_engine->getEventHandler().getConnection());
		const std::string &pName(con.getPlayerName(e.sockfd()));

		const std::vector<std::string> ex(1, pName);
		const Engine::PLAYERS::const_iterator &f(m_engine->find(pName));

		m_lostWatchingPlayer = !pName.empty() && f == m_engine->m_players.end();

		if(!m_lostWatchingPlayer) {

			if(!pName.empty()) {

				const unsigned int lcl =
					NetMauMau::Common::Protocol::V15::ERR_TO_EXC_LOSTCONNNAMED.length() +
					pName.length();

				char lc[1 + lcl];
				std::snprintf(lc, lcl, "%s\"%s\"", NetMauMau::Common::Protocol::V15::
							  ERR_TO_EXC_LOSTCONNNAMED.c_str(), pName.c_str());

				try {
					m_engine->getEventHandler().error(lc, ex);
				} catch(const Common::Exception::SocketException &) {}

			} else {
				try {
					m_engine->getEventHandler().
					error(NetMauMau::Common::Protocol::V15::ERR_TO_EXC_LOSTCONN, ex);
				} catch(const Common::Exception::SocketException &) {}
			}

		} else {

			try {

				char watcher[25 + pName.length()];
				std::snprintf(watcher, 24 + pName.length(), "%s is no more watching us",
							  pName.c_str());

				m_engine->getEventHandler().message(watcher);

			} catch(const Common::Exception::SocketException &) {}
		}

		con.removePlayer(e.sockfd());
		disconnectError(e.sockfd());

		if(f != m_engine->m_players.end()) m_engine->removePlayer(*f);

		if(!m_lostWatchingPlayer) m_engine->m_state = Engine::FINISHED;

	}

	return true;
}

bool NextTurn::checkCard(Player::IPlayer *player, Common::ICardPtr &playedCard,
						 const Common::ICardPtr &uc) const
throw(Common::Exception::SocketException) {

	bool noMatch, cardAccepted = false;
	RuleSet::IRuleSet *ruleSet = m_engine->getRuleSet();

	while(playedCard && ((noMatch = (playedCard == Common::ICard::SUIT_ILLEGAL)) ||
						 !(cardAccepted = ruleSet->checkCard(player, uc, playedCard,
										  !m_engine->m_ctx.getNextMessage())))) {

		if(!noMatch) m_engine->getEventHandler().cardRejected(player, uc, playedCard);

		const Common::ICard::SUIT js = ruleSet->getJackSuit();

		if(!(playedCard = player->requestCard(uc, m_engine->m_jackMode ? &js : 0L,
											  ruleSet->takeCardCount()))) {

			if(!noMatch) {
				Common::ICardPtr rc(m_engine->m_talon->takeCard());

				if(rc) player->receiveCard(rc);
			}

			suspends(player);
			break;
		}
	}

	return cardAccepted;
}

void NextTurn::jackModeOff() const throw(Common::Exception::SocketException) {

	m_engine->getRuleSet()->setJackModeOff();

	try {
		m_engine->getEventHandler().setJackModeOff();
	} catch(const Common::Exception::SocketException &e) {
		logDebug(__PRETTY_FUNCTION__ << ": failed to handle event \'jackModeOff\': " << e.what());
	}
}

void NextTurn::informAIStat() const {
	std::for_each(m_engine->m_players.begin(), m_engine->m_players.end(),
				  _informAIStat(m_engine->m_players));
}

void NextTurn::handleWinner(const Player::IPlayer *player)
throw(Common::Exception::SocketException) {

	Engine::PLAYERS::iterator f(m_engine->m_players.begin());
	std::advance(f, std::min(m_engine->m_players.size() - 1u, m_engine->m_nxtPlayer));
	m_engine->erasePlayer(f);

	const Engine::PLAYERS::size_type plsCnt = m_engine->m_players.size();
	RuleSet::IRuleSet *const ruleSet = m_engine->getRuleSet();

	m_engine->m_nxtPlayer = m_engine->m_nxtPlayer < plsCnt ? m_engine->m_nxtPlayer + 1 : 0;

	ruleSet->setCurPlayers(plsCnt);

	if(!m_engine->hasPlayers()) {

		if(ruleSet->takeIfLost() && m_engine->m_talon->getUncoveredCard() == Common::ICard::SEVEN) {
			takeCards(m_engine->m_players[m_engine->m_nxtPlayer], Common::getIllegalCard());
		}

		const Common::IConnection::NAMESOCKFD nsf =
			(m_engine->m_players[m_engine->m_nxtPlayer]->isAIPlayer()) ?
			Common::IConnection::NAMESOCKFD(m_engine->m_players[m_engine->m_nxtPlayer]->getName(),
											"",
											m_engine->m_players[m_engine->m_nxtPlayer]->getSerial(),
											0) : m_engine->getEventHandler().getConnection().
			getPlayerInfo(m_engine->m_players[m_engine->m_nxtPlayer]->getSerial());

		m_db->playerLost(m_engine->m_gameIndex, nsf, std::time(0L), m_engine->getEventHandler().
						 playerLost(m_engine->m_players[m_engine->m_nxtPlayer], m_engine->m_turn,
									ruleSet->lostPointFactor(m_engine->m_talon->
											getUncoveredCard())));
		m_engine->m_state = Engine::FINISHED;

	} else if(m_engine->m_dirChangeEnabled && plsCnt <= 2u) {
		setDirChangeIsSuspend(true);
	}

	m_engine->getEventHandler().playerWins(player, m_engine->m_turn, m_engine->m_ultimate);

	const Common::IConnection::NAMESOCKFD nsf = (player->isAIPlayer()) ?
			Common::IConnection::NAMESOCKFD(player->getName(), "", player->getSerial(), 0) :
			m_engine->getEventHandler().getConnection().getPlayerInfo(player->getSerial());

	m_db->playerWins(m_engine->m_gameIndex, nsf);
}

void NextTurn::disconnectError(SOCKET fd) const {
	if(fd != INVALID_SOCKET) Common::AbstractSocket::shutdown(fd);
}

void NextTurn::checkPlayersAlive() const throw(Common::Exception::SocketException) {

	for(Engine::PLAYERS::const_iterator i(m_engine->m_players.begin());
			i != m_engine->m_players.end(); ++i) {

		Engine::PLAYERS::const_iterator::reference r(*i);

		if(!r->isAlive()) {
			r->setCardCountObserver(0L);
			throw Common::Exception::SocketException((*i)->getName() + " is dead");
		}
	}
}

long NextTurn::getAIDelay() const {
	return (!m_engine->getEventHandler().getConnection().hasHumanPlayers()) ? 0L :
		   m_engine->m_ctx.getAIDelay();
}

bool NextTurn::wait(const Player::IPlayer *p, bool suspend) const {

	const bool isAI = p->isAIPlayer();
	const Engine::PLAYERS::size_type pcnt(m_engine->m_players.size());

	if(isAI && getAICount() == pcnt) return true;

	if(pcnt == 2) return suspend && isAI;

	return isAI ? m_engine->m_alwaysWait : false;
}

void NextTurn::checkAndPerformDirChange(const Player::IPlayer *player, bool won)
throw(Common::Exception::SocketException) {

	RuleSet::IRuleSet *ruleSet = m_engine->getRuleSet();

	if(ruleSet->hasDirChange()) {

		if(m_engine->m_dirChangeEnabled && m_engine->m_players.size() > 2u) {

			std::reverse(m_engine->m_players.begin(), m_engine->m_players.end());

			m_engine->m_nxtPlayer =
				static_cast<std::size_t>(std::distance(m_engine->m_players.begin(),
										 std::find(m_engine->m_players.begin(),
												   m_engine->m_players.end(),
												   won ? m_engine->m_players[m_engine->m_nxtPlayer]
												   : player)));

			assert(m_engine->m_nxtPlayer <= m_engine->m_players.size());

			m_engine->getEventHandler().directionChange();

		} else if(m_engine->m_players.size() <= 2u) {
			setDirChangeIsSuspend(true);
		}

		ruleSet->dirChanged();
	}
}

void NextTurn::setDirChangeIsSuspend(bool b) throw(Common::Exception::SocketException) {

	m_engine->getRuleSet()->setDirChangeIsSuspend(b);

	std::for_each(m_engine->m_players.begin(), m_engine->m_players.end(),
				  std::bind2nd(std::mem_fun(&Player::IPlayer::setNineIsSuspend), b));
}

void NextTurn::suspends(Player::IPlayer *p,
						const Common::ICard *uc) const throw(Common::Exception::SocketException) {
	m_engine->getEventHandler().playerSuspends(p, uc);
}

bool NextTurn::takeCards(Player::IPlayer *player, const Common::ICard *card) const
throw(Common::Exception::SocketException) {

	const std::size_t cardCount = m_engine->getRuleSet()->takeCards(card);

	if(cardCount) {

		Player::IPlayer::CARDS tmp;

		if(cardCount <= tmp.max_size()) tmp.reserve(cardCount);

		std::size_t j = 0;

		for(std::size_t i = 0; i < cardCount; ++i) {

			const Common::ICardPtr &c(m_engine->m_talon->takeCard());

			if(c) {
				tmp.push_back(c);
				++j;
			}
		}

		if(j == cardCount) {

			for(Player::IPlayer::CARDS::const_iterator ri(tmp.begin()); ri != tmp.end(); ++ri)
				player->receiveCard(*ri);

			m_engine->getEventHandler().playerPicksCards(player, j);

		} else if(j) {
			for(Player::IPlayer::CARDS::const_reverse_iterator ri(tmp.rbegin());
					ri != tmp.rend(); ++ri) m_engine->m_talon->pushBackCard(*ri);
		}

		m_engine->getRuleSet()->hasTakenCards();

		return !m_engine->getRuleSet()->takeAfterSevenIfNoMatch();
	}

	return false;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
