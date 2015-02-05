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

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#include <cassert>
#include <sstream>
#include <algorithm>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "engine.h"

#include "talon.h"
#include "sqlite.h"
#include "logger.h"
#include "iplayer.h"
#include "cardtools.h"
#include "stdruleset.h"
#include "ieventhandler.h"

#if defined(HAVE_GSL)
#include <random_gen.h>
#endif

namespace {
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct PlayerNameEqual : public std::binary_function < NetMauMau::Player::IPlayer *,
		NetMauMau::Player::IPlayer *, bool > {
	bool operator()(const NetMauMau::Player::IPlayer *x,
					const NetMauMau::Player::IPlayer *y) const {
		return x->getName() == y->getName();
	}
};
#pragma GCC diagnostic pop
}

#if defined(HAVE_GSL)
namespace NetMauMau _EXPORT {

namespace Common _EXPORT {
const GSLRNG<std::ptrdiff_t> RNG;
}

}
#endif

using namespace NetMauMau;

Engine::Engine(Event::IEventHandler &eventHandler, bool dirChange, long aiDelay, bool nextMessage,
			   char aceRound) : m_eventHandler(eventHandler), m_state(ACCEPT_PLAYERS),
	m_talon(new Talon(this)), m_ruleset(new RuleSet::StdRuleSet(dirChange, aceRound ? this : 0L)),
	m_players(), m_nxtPlayer(0), m_turn(1), m_curTurn(0), m_delRuleSet(true), m_jackMode(false),
	m_initialChecked(false), m_nextMessage(nextMessage), m_ultimate(false), m_initialJack(false),
	m_alwaysWait(false), m_initialNextMessage(nextMessage), m_aiDelay(aiDelay),
	m_aceRoundRank(aceRound == 'A' ? Common::ICard::ACE : (aceRound == 'Q' ? Common::ICard::QUEEN :
				   (aceRound == 'K' ? Common::ICard::KING : Common::ICard::RANK_ILLEGAL))),
	m_gameIndex(0LL), m_dirChangeEnabled(false) {
	m_players.reserve(5);
	m_eventHandler.acceptingPlayers();
}

Engine::Engine(Event::IEventHandler &eventHandler, long aiDelay, RuleSet::IRuleSet *ruleset,
			   bool nextMessage) : m_eventHandler(eventHandler), m_state(ACCEPT_PLAYERS),
	m_talon(new Talon(this)), m_ruleset(ruleset), m_players(), m_nxtPlayer(0), m_turn(1),
	m_curTurn(0), m_delRuleSet(false), m_jackMode(false), m_initialChecked(false),
	m_nextMessage(nextMessage), m_ultimate(false), m_initialJack(false), m_alwaysWait(false),
	m_initialNextMessage(nextMessage), m_aiDelay(aiDelay),
	m_aceRoundRank(Common::ICard::RANK_ILLEGAL), m_gameIndex(0LL), m_dirChangeEnabled(false) {
	m_players.reserve(5);
	m_eventHandler.acceptingPlayers();
}

Engine::~Engine() {
	delete m_talon;

	if(m_delRuleSet) delete m_ruleset;
}

Engine::PLAYERS::const_iterator Engine::find(const std::string &name) const {

	std::string nameB(name);

	for(PLAYERS::const_iterator i(m_players.begin()); i != m_players.end(); ++i) {

		std::string nameA((*i)->getName());

		std::transform(nameA.begin(), nameA.end(), nameA.begin(), ::tolower);
		std::transform(nameB.begin(), nameB.end(), nameB.begin(), ::tolower);

		if(nameA == nameB) return i;
	}

	return m_players.end();
}

bool Engine::addPlayer(Player::IPlayer *player) throw(Common::Exception::SocketException) {

	if(m_state == ACCEPT_PLAYERS) {

		const PLAYERS::const_iterator &f(find(player->getName()));

		if(f == m_players.end() && m_players.size() <= m_ruleset->getMaxPlayers()) {

			m_players.push_back(player);

			m_dirChangeEnabled = m_players.size() > 2;

			if(player->isAIPlayer()) {

				Common::AbstractConnection *con = m_eventHandler.getConnection();

				if(con) con->addAIPlayers(std::vector<std::string>(1, player->getName()));
			}

			player->setRuleSet(m_ruleset);
			m_eventHandler.playerAdded(player);

			return true;

		} else if(f != m_players.end()) {

			Common::AbstractConnection *con = m_eventHandler.getConnection();

			m_eventHandler.playerRejected(player);

			if(con) con->removePlayer(player->getSerial());

			return false;
		}

	} else {
		m_eventHandler.playerRejected(player);
	}

	m_state = m_state != PLAYING ? NOCARDS : m_state;

	return false;
}

Engine::PLAYERS::iterator Engine::removePlayer(Player::IPlayer *player) {

	const PLAYERS::iterator &f(std::find(m_players.begin(), m_players.end(), player));

	if(f != m_players.end()) return m_players.erase(f);

	m_ruleset->setCurPlayers(m_players.size());

	return f;
}

bool Engine::distributeCards() throw(Common::Exception::SocketException) {

	if(m_state == NOCARDS || m_state == ACCEPT_PLAYERS) {

		std::vector<std::vector<Common::ICard *> > cards(m_players.size());

		for(std::size_t i = 0; i < m_ruleset->initialCardCount(); ++i) {

			if(m_talon->empty()) return false;

			for(PLAYERS::size_type j = 0; j < m_players.size(); ++j) {
				cards[j].push_back(m_talon->top());
				m_talon->pop();
			}
		}

		PLAYERS::const_iterator pi(m_players.begin());
		const PLAYERS::const_iterator &pe(m_players.end());

		for(std::size_t k = 0; pi != pe; ++pi, ++k) {

			Player::IPlayer *p = *pi;

			p->receiveCardSet(cards[k]);
			p->setDirChangeEnabled(m_dirChangeEnabled);

			m_eventHandler.cardsDistributed(p, cards[k]);
		}

		m_turn = 1;
		m_curTurn = 0;
		m_state = PLAYING;

		m_ruleset->setCurPlayers(m_players.size());

		return true;

	} else {
		m_eventHandler.cardsAlreadyDistributed();
	}

	return false;
}

void Engine::setFirstPlayer(Player::IPlayer *p) {
	std::stable_partition(m_players.begin(), m_players.end(), std::bind2nd(PlayerNameEqual(), p));
}

void Engine::message(const std::string &msg) const throw(Common::Exception::SocketException) {
	m_eventHandler.message(msg);
}

void Engine::error(const std::string &msg) const throw() {
	try {
		m_eventHandler.error(msg);
	} catch(const Common::Exception::SocketException &) {}
}

void Engine::suspends(Player::IPlayer *p, const Common::ICard *uc) const {

	m_eventHandler.playerSuspends(p, uc);

	Common::AbstractConnection *con = m_eventHandler.getConnection();

	if(p->isAIPlayer() && m_alwaysWait && con) con->wait(getAIDelay());
}

bool Engine::nextTurn() {

	if(m_eventHandler.shutdown() || m_state != PLAYING) return false;

	if(getAICount() == 1) {
		m_alwaysWait  = false;
		m_nextMessage = false;
	}

	try {

		checkPlayersAlive();

		Player::IPlayer *player = m_players[m_nxtPlayer];

		if(m_curTurn != m_turn) {

			DB::SQLite::getInstance().turn(m_gameIndex, m_turn);

			m_eventHandler.turn(m_turn);

			if(m_turn == 1) {

				DB::SQLite::getInstance().gamePlayStarted(m_gameIndex);

				Common::ICard *ic = m_talon->uncoverCard();
				m_eventHandler.initialCard(ic);
				cardPlayed(ic);
			}

			m_curTurn = m_turn;
		}

		if(m_nextMessage) m_eventHandler.nextPlayer(player);

		const Common::ICard *uc = m_talon->getUncoveredCard();

		if(!m_initialChecked) {
			m_ruleset->checkInitial(player, uc);
			m_initialJack = m_ruleset->isJackMode();
			m_initialChecked = true;

			if((uc->getRank() == Common::ICard::EIGHT || (uc->getRank() == Common::ICard::NINE &&
					m_ruleset->getDirChangeIsSuspend())) && getAICount()) {
				Common::AbstractConnection *con = m_eventHandler.getConnection();

				if(con) con->wait(getAIDelay());
			}
		}

		m_eventHandler.stats(m_players);
		informAIStat();

		const bool csuspend = m_ruleset->hasToSuspend();
		const Common::ICard::SUIT js = m_ruleset->getJackSuit();

		assert(uc->getRank() != Common::ICard::JACK || (uc->getRank() == Common::ICard::JACK &&
				((m_jackMode || m_initialJack) && js != Common::ICard::SUIT_ILLEGAL)));

		Common::ICard *pc = !csuspend ? player->requestCard(uc, (m_jackMode || m_initialJack)
							? &js : 0L, m_ruleset->takeCardCount()) : 0L;

		if(m_initialJack && !pc) m_jackMode = true;

		bool won = false;

		if(!csuspend) {

			bool suspend = false;

			takeCards(player, pc);

sevenRule:

			if(!pc) {

				player->receiveCard(pc = m_talon->takeCard());

				if(player->getNoCardReason() == Player::IPlayer::SUSPEND) {
					suspends(player);
					pc = 0L;
				}

			} else if(pc->getSuit() == Common::ICard::SUIT_ILLEGAL) {
				pc = player->requestCard(uc, m_jackMode ? &js : 0L, m_ruleset->takeCardCount());
				goto sevenRule;
			}

			bool cc = false;

			while(pc && !(cc = m_ruleset->checkCard(player, uc, pc, !m_nextMessage))) {

				const bool aiSusp = !cc && player->isAIPlayer();

				if(suspend || aiSusp) {
					suspends(player);
					break;
				}

				m_eventHandler.cardRejected(player, uc, pc);

				const Common::ICard::SUIT js2 = m_ruleset->getJackSuit();

				if((!(pc = player->requestCard(uc, m_jackMode ? &js2 : 0L,
											   m_ruleset->takeCardCount())))) {

					bool decidedSuspend = false;

					switch(player->getNoCardReason()) {
					case Player::IPlayer::SUSPEND:
						decidedSuspend = true;

					case Player::IPlayer::NOMATCH:
						player->receiveCard(pc = m_talon->takeCard());

						m_eventHandler.playerPicksCard(player, pc);

						suspend = true;
						pc = m_ruleset->suspendIfNoMatchingCard() || decidedSuspend ? 0L : pc;

						break;

					default:
						pc = 0L;
						break;
					}
				}
			}

			if(pc && cc) {

				if(m_jackMode || m_initialJack) m_ruleset->setJackModeOff();

				won = player->cardAccepted(pc);
				m_talon->playCard(pc);

				m_eventHandler.playerPlaysCard(player, pc, uc);

				if((m_jackMode = m_ruleset->isJackMode())) {
					m_eventHandler.playerChooseJackSuit(player, m_ruleset->getJackSuit());
				}

				if(won) {

					PLAYERS::iterator f(m_players.begin());
					std::advance(f, m_nxtPlayer);
					const PLAYERS::iterator nxt = m_players.erase(f);

					m_ruleset->setCurPlayers(m_players.size());

					m_nxtPlayer = nxt != m_players.end() ?
								  static_cast<std::size_t>(std::distance(m_players.begin(), nxt))
								  : 0;

					if(!hasPlayers()) {

						if(m_ruleset->takeIfLost() &&
								m_talon->getUncoveredCard()->getRank() == Common::ICard::SEVEN) {
							takeCards(m_players[m_nxtPlayer], Common::getIllegalCard());
						}

						Common::AbstractConnection *con = m_eventHandler.getConnection();

						const Common::AbstractConnection::NAMESOCKFD nsf =
							(!con || m_players[m_nxtPlayer]->isAIPlayer()) ?
							Common::AbstractConnection::NAMESOCKFD(m_players[m_nxtPlayer]->
									getName(), "", m_players[m_nxtPlayer]->getSerial(), 0)
							: con->getPlayerInfo(m_players[m_nxtPlayer]->getSerial());

						DB::SQLite::getInstance().
						playerLost(m_gameIndex, nsf, std::time(0L),
								   m_eventHandler.playerLost(m_players[m_nxtPlayer], m_turn,
															 m_ruleset->lostPointFactor(m_talon->
																	 getUncoveredCard())));

						m_state = FINISHED;
					}

					m_eventHandler.playerWins(player, m_turn, m_ultimate);

					Common::AbstractConnection *con = m_eventHandler.getConnection();

					const Common::AbstractConnection::NAMESOCKFD nsf =
						(!con || player->isAIPlayer()) ?
						Common::AbstractConnection::NAMESOCKFD(player->getName(), "",
								player->getSerial(), 0) : con->getPlayerInfo(player->getSerial());

					DB::SQLite::getInstance().
					playerWins(m_gameIndex, nsf);

				} else if(player->isAIPlayer() && ((pc->getRank() == Common::ICard::EIGHT ||
													(pc->getRank() == Common::ICard::NINE &&
													 m_ruleset->getDirChangeIsSuspend())) ||
												   m_alwaysWait)) {

					Common::AbstractConnection *con = m_eventHandler.getConnection();

					if(con) con->wait(getAIDelay());
				}
			}

		} else {
			suspends(player, uc);
			m_ruleset->hasSuspended();

			if(m_jackMode) {
				m_ruleset->setJackModeOff();
				m_jackMode = false;
			}
		}

		if(m_ruleset->hasDirChange()) {

			if(m_dirChangeEnabled && m_players.size() > 2) {

				std::reverse(m_players.begin(), m_players.end());

				m_nxtPlayer = static_cast<std::size_t>(std::distance(m_players.begin(),
													   std::find(m_players.begin(), m_players.end(),
															   player)));
				assert(m_nxtPlayer <= m_players.size());

				m_eventHandler.directionChange();

			} else if(m_dirChangeEnabled) {
				setDirChangeIsSuspend(true);
			}

			m_ruleset->dirChanged();
		}

		if(!won) {
			const std::size_t leftCount = player->getCardCount();
			m_nxtPlayer = (m_nxtPlayer + 1) >= m_players.size() ? 0 : m_nxtPlayer + 1;
			const std::size_t rightCount = m_players[(m_nxtPlayer + 1) >= m_players.size() ? 0 :
										   m_nxtPlayer + 1]->getCardCount();
			m_players[m_nxtPlayer]->setNeighbourCardCount(m_players.size(), leftCount, rightCount);
		}

		if(!m_nxtPlayer) ++m_turn;

		m_initialJack = false;

	} catch(const Common::Exception::SocketException &e) {

		logDebug("SocketException: " << e);

		Common::AbstractConnection *con = m_eventHandler.getConnection();
		bool lostWatchingPlayer = false;

		if(con) {

			const std::string &pName(con->getPlayerName(e.sockfd()));

			std::vector<std::string> ex(1, pName);
			const PLAYERS::const_iterator &f(find(pName));
			std::ostringstream os;

			lostWatchingPlayer = !pName.empty() && f == m_players.end();

			if(!lostWatchingPlayer) {

				if(!pName.empty()) {

					os << "Lost connection to player \"" << pName << "\"";

					try {
						m_eventHandler.error(os.str(), ex);
					} catch(const Common::Exception::SocketException &) {}

				} else {
					try {
						m_eventHandler.error("Lost connection to a player", ex);
					} catch(const Common::Exception::SocketException &) {}
				}

			} else {

				try {

					std::ostringstream watcher;
					watcher << pName << " is no more watching us";
					m_eventHandler.message(watcher.str());

				} catch(const Common::Exception::SocketException &) {}
			}

			con->removePlayer(e.sockfd());

			shutdown(e.sockfd(), SHUT_RDWR);
#ifndef _WIN32
			close(e.sockfd());
#else
			closesocket(e.sockfd());
#endif

			if(f != m_players.end()) removePlayer(*f);
		}

		if(!lostWatchingPlayer) m_state = FINISHED;

	}

	return true;
}

void Engine::takeCards(Player::IPlayer *player, const Common::ICard *card) const
throw(Common::Exception::SocketException) {

	const std::size_t cardCount = m_ruleset->takeCards(card);

	if(cardCount) {

		for(std::size_t i = 0; i < cardCount; ++i) {
			player->receiveCard(m_talon->takeCard(false));
		}

		m_eventHandler.playerPicksCards(player, cardCount);
		m_ruleset->hasTakenCards();

		cardTaken();
	}
}

void Engine::uncoveredCard(const Common::ICard *top) const
throw(Common::Exception::SocketException) {
	m_eventHandler.uncoveredCard(top);
}

void Engine::talonEmpty(bool empty) const throw() {
	try {
		m_eventHandler.talonEmpty(empty);
	} catch(const Common::Exception::SocketException &e) {
		logDebug(__PRETTY_FUNCTION__ << ": failed to handle event \'talonEmpty\': " << e.what());
	}
}

void Engine::cardPlayed(Common::ICard *card) const {
	for(PLAYERS ::const_iterator i(m_players.begin()); i != m_players.end(); ++i) {
		(*i)->cardPlayed(card);
	}
}

void Engine::cardTaken(const Common::ICard *) const throw(Common::Exception::SocketException) {
	m_eventHandler.stats(m_players);
	informAIStat();
}

void Engine::shuffled() const {
	for(PLAYERS ::const_iterator i(m_players.begin()); i != m_players.end(); ++i) {
		(*i)->talonShuffled();
	}
}

Common::ICard::RANK Engine::getAceRoundRank() const {
	return m_aceRoundRank;
}

void Engine::aceRoundStarted(const Player::IPlayer *player) const
throw(Common::Exception::SocketException) {
	m_eventHandler.aceRoundStarted(player);
}

void Engine::aceRoundEnded(const Player::IPlayer *player) const
throw(Common::Exception::SocketException) {
	m_eventHandler.aceRoundEnded(player);
}

void Engine::informAIStat() const {
	for(PLAYERS ::const_iterator i(m_players.begin()); i != m_players.end(); ++i) {
		if((*i)->isAIPlayer()) {
			for(PLAYERS ::const_iterator j(m_players.begin()); j != m_players.end(); ++j) {
				if(*i != *j)(*i)->informAIStat(*j, (*j)->getCardCount());
			}
		}
	}
}

void Engine::setDirChangeIsSuspend(bool b) {
	m_ruleset->setDirChangeIsSuspend(b);

	for(PLAYERS ::const_iterator i(m_players.begin()); i != m_players.end(); ++i) {
		(*i)->setNineIsEight(b);
	}
}

std::size_t Engine::getAICount() const {

	std::size_t cnt = 0;

	for(PLAYERS ::const_iterator i(m_players.begin()); i != m_players.end(); ++i) {
		if((*i)->isAIPlayer()) ++cnt;
	}

	return cnt;
}

void Engine::gameAboutToStart() const {
	m_eventHandler.gameAboutToStart();
}

void Engine::gameOver() const throw() {
	try {
		m_eventHandler.gameOver();
	} catch(const Common::Exception::SocketException &e) {
		logDebug(__PRETTY_FUNCTION__ << ": failed to handle event \'gameOver\': " << e.what());
	}
}

void Engine::checkPlayersAlive() const throw(Common::Exception::SocketException) {
	for(PLAYERS::const_iterator i(m_players.begin()); i != m_players.end(); ++i) {
		if(!(*i)->isAlive()) throw Common::Exception::SocketException((*i)->getName() + " is dead");
	}
}

long int Engine::getAIDelay() const {
	const Common::AbstractConnection *con = m_eventHandler.getConnection();
	return (con && !con->hasHumanPlayers()) ? 0L :
		   DB::SQLite::getInstance().getDBFilename().empty() ? 0L : m_aiDelay;
}

void Engine::reset() throw() {

	m_state = ACCEPT_PLAYERS;

	delete m_talon;
	m_talon = new Talon(this);

	m_eventHandler.reset();
	m_ruleset->reset();
	removePlayers();

	m_nxtPlayer = 0,
	m_turn = 1;
	m_curTurn = 0;

	m_jackMode = false;
	m_nextMessage = m_initialNextMessage;
	m_initialChecked = false;
	m_initialJack = false;
	m_alwaysWait = false;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
