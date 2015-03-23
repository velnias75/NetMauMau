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

#ifdef _WIN32
#include <ws2tcpip.h>
#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif
#elif defined(HAVE_SYS_SOCKET_H)
#include <sys/types.h>
#include <sys/socket.h>
#endif

#include "engine.h"

#include "talon.h"
#include "sqlite.h"
#include "logger.h"
#include "iplayer.h"
#include "iruleset.h"
#include "cardtools.h"
#include "engineconfig.h"
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

Engine::Engine(EngineConfig &cfg) throw(Common::Exception::SocketException) : ITalonChange(),
	IAceRoundListener(), ICardCountObserver(), m_cfg(cfg), m_state(ACCEPT_PLAYERS),
	m_talon(new Talon(this, cfg.getTalonFactor())), m_players(), m_nxtPlayer(0), m_turn(1),
	m_curTurn(0), m_jackMode(false), m_initialChecked(false), m_ultimate(false),
	m_initialJack(false), m_alwaysWait(false), m_alreadyWaited(false),
	m_initialNextMessage(cfg.getNextMessage()), m_gameIndex(0LL), m_dirChangeEnabled(false) {
	m_players.reserve(5);
	cfg.getEventHandler().acceptingPlayers();
}

Engine::~Engine() {
	delete m_talon;
}

const Event::IEventHandler &Engine::getEventHandler() const {
	return m_cfg.getEventHandler();
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

		if(f == m_players.end() && m_players.size() <= getRuleSet()->getMaxPlayers()) {

			m_players.push_back(player);

			m_dirChangeEnabled = m_players.size() > 2;

			if(player->isAIPlayer()) {
				getEventHandler().getConnection()->addAIPlayers(std::vector<std::string>(1,
						player->getName()));
			}

			player->setRuleSet(getRuleSet());
			player->setEngineConfig(&m_cfg);

			getEventHandler().playerAdded(player);

			return true;

		} else if(f != m_players.end()) {

			getEventHandler().playerRejected(player);
			getEventHandler().getConnection()->removePlayer(player->getSerial());

			return false;
		}

	} else {
		getEventHandler().playerRejected(player);
	}

	m_state = m_state != PLAYING ? NOCARDS : m_state;

	return false;
}

void Engine::removePlayer(const std::string &player) {

	const PLAYERS::const_iterator &f(find(player));

	if(f != m_players.end() && !(*f)->isAIPlayer()) removePlayer(*f);
}

Engine::PLAYERS::iterator Engine::removePlayer(Player::IPlayer *player) {

	const PLAYERS::iterator &f(std::find(m_players.begin(), m_players.end(), player));

	if(f != m_players.end()) return m_players.erase(f);

	getRuleSet()->setCurPlayers(m_players.size());

	return f;
}

bool Engine::distributeCards() throw(Common::Exception::SocketException) {

	if(m_state == NOCARDS || m_state == ACCEPT_PLAYERS) {

		std::vector<std::vector<Common::ICard *> > cards(m_players.size());

		const std::size_t icc = getRuleSet()->initialCardCount();

		for(std::size_t i = 0; i < icc; ++i) {

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

			getEventHandler().cardsDistributed(p, cards[k]);
		}

		m_turn = 1;
		m_curTurn = 0;
		m_state = PLAYING;

		getRuleSet()->setCurPlayers(m_players.size());

		return true;

	} else {
		getEventHandler().cardsAlreadyDistributed();
	}

	return false;
}

void Engine::setFirstPlayer(Player::IPlayer *p) {
	std::stable_partition(m_players.begin(), m_players.end(), std::bind2nd(PlayerNameEqual(), p));
}

void Engine::message(const std::string &msg) const throw(Common::Exception::SocketException) {
	getEventHandler().message(msg);
}

void Engine::error(const std::string &msg) const throw() {
	try {
		getEventHandler().error(msg);
	} catch(const Common::Exception::SocketException &) {}
}

void Engine::suspends(Player::IPlayer *p, const Common::ICard *uc) const
throw(Common::Exception::SocketException) {
	getEventHandler().playerSuspends(p, uc);
}

bool Engine::nextTurn() {

	m_alreadyWaited = false;

	if(getEventHandler().shutdown() || m_state != PLAYING) return false;

	if(getAICount() == 1) {
		m_alwaysWait = false;
		m_cfg.setNextMessage(false);
	}

	try {

		checkPlayersAlive();

		Player::IPlayer *player = m_players[m_nxtPlayer];

		player->setCardCountObserver(this);

		if(m_curTurn != m_turn) {

			DB::SQLite::getInstance().turn(m_gameIndex, m_turn);

			getEventHandler().turn(m_turn);

			if(m_turn == 1) {

				DB::SQLite::getInstance().gamePlayStarted(m_gameIndex);

				Common::ICard *ic = m_talon->uncoverCard();
				getEventHandler().initialCard(ic);
				cardPlayed(ic);
			}

			m_curTurn = m_turn;
		}

		if(m_cfg.getNextMessage()) getEventHandler().nextPlayer(player);

		const Common::ICard *uc = m_talon->getUncoveredCard();

		if(!m_initialChecked) {
			getRuleSet()->checkInitial(player, uc);
			m_initialJack = getRuleSet()->isJackMode();
			m_initialChecked = true;
		}

		const bool csuspend = getRuleSet()->hasToSuspend();
		const Common::ICard::SUIT js = getRuleSet()->getJackSuit();

		assert(uc->getRank() != Common::ICard::JACK || (uc->getRank() == Common::ICard::JACK &&
				((m_jackMode || m_initialJack) && js != Common::ICard::SUIT_ILLEGAL)));

		Common::ICard *pc = !csuspend ? player->requestCard(uc, (m_jackMode || m_initialJack)
							? &js : 0L, getRuleSet()->takeCardCount()) : 0L;

		if(m_initialJack && !pc) m_jackMode = true;

		bool won = false;

		if(!csuspend) {

			bool suspend = false;

			const bool noCardOk = takeCards(player, pc) || getRuleSet()->isAceRound();

sevenRule:

			if(!pc) {

				if(!noCardOk) {
					player->receiveCard(pc = m_talon->takeCard());
					getEventHandler().playerPicksCard(player);
				}

				const Player::IPlayer::REASON reason = noCardOk ? Player::IPlayer::SUSPEND :
													   player->getNoCardReason(uc, m_jackMode ?
															   &js : 0L);

				if(reason == Player::IPlayer::SUSPEND) {
					suspends(player);
					pc = 0L;
				} else if(!player->isAIPlayer() && reason == Player::IPlayer::NOMATCH) {

					pc = player->requestCard(uc, m_jackMode ? &js : 0L,
											 getRuleSet()->takeCardCount());
				}

			} else if(pc->getSuit() == Common::ICard::SUIT_ILLEGAL) {

				pc = player->requestCard(uc, m_jackMode ? &js : 0L, getRuleSet()->takeCardCount());

				goto sevenRule;
			}

			bool cc = false;

			while(pc && !(cc = getRuleSet()->checkCard(player, uc, pc, !m_cfg.getNextMessage()))) {

				const bool aiSusp = !cc && player->isAIPlayer();

				if(suspend || aiSusp) {
					suspends(player);
					break;
				}

				getEventHandler().cardRejected(player, uc, pc);

				const Common::ICard::SUIT js2 = getRuleSet()->getJackSuit();

				if((!(pc = player->requestCard(uc, m_jackMode ? &js2 : 0L,
											   getRuleSet()->takeCardCount())))) {

					bool decidedSuspend = false;

					switch(player->getNoCardReason(uc, m_jackMode ? &js2 : 0L)) {
					case Player::IPlayer::SUSPEND:
						decidedSuspend = true;

					case Player::IPlayer::NOMATCH:

						player->receiveCard(pc = m_talon->takeCard());
						getEventHandler().playerPicksCard(player, pc);

						suspend = true;
						pc = getRuleSet()->suspendIfNoMatchingCard() || decidedSuspend ? 0L : pc;

						break;

					default:
						pc = 0L;
						break;
					}
				}
			}

			if(pc && cc) {

				if(m_jackMode || m_initialJack) jackModeOff();

				won = player->cardAccepted(pc);
				m_talon->playCard(pc);

				getEventHandler().playerPlaysCard(player, pc, uc);

				if((m_jackMode = getRuleSet()->isJackMode())) {
					getEventHandler().playerChooseJackSuit(player, getRuleSet()->getJackSuit());
				}

				if(won) {

					PLAYERS::iterator f(m_players.begin());
					std::advance(f, m_nxtPlayer);
					const PLAYERS::iterator nxt = m_players.erase(f);

					getRuleSet()->setCurPlayers(m_players.size());

					m_nxtPlayer = nxt != m_players.end() ?
								  static_cast<std::size_t>(std::distance(m_players.begin(), nxt))
								  : 0;

					if(!hasPlayers()) {

						if(getRuleSet()->takeIfLost() && m_talon->getUncoveredCard()->getRank() ==
								Common::ICard::SEVEN) {
							takeCards(m_players[m_nxtPlayer], Common::getIllegalCard());
						}

						const Common::IConnection::NAMESOCKFD nsf =
							(m_players[m_nxtPlayer]->isAIPlayer()) ?
							Common::IConnection::NAMESOCKFD(m_players[m_nxtPlayer]->getName(), "",
															m_players[m_nxtPlayer]->getSerial(),
															0) :
							getEventHandler().getConnection()->
							getPlayerInfo(m_players[m_nxtPlayer]->getSerial());

						DB::SQLite::getInstance().
						playerLost(m_gameIndex, nsf, std::time(0L),
								   getEventHandler().playerLost(m_players[m_nxtPlayer], m_turn,
																getRuleSet()->
																lostPointFactor(m_talon->
																		getUncoveredCard())));
						m_state = FINISHED;
					}

					getEventHandler().playerWins(player, m_turn, m_ultimate);

					const Common::IConnection::NAMESOCKFD nsf = (player->isAIPlayer()) ?
							Common::IConnection::NAMESOCKFD(player->getName(), "",
															player->getSerial(), 0) :
							getEventHandler().getConnection()->
							getPlayerInfo(player->getSerial());

					DB::SQLite::getInstance().playerWins(m_gameIndex, nsf);

				} else if(wait(player, true) && (pc->getRank() == Common::ICard::EIGHT ||
												 (pc->getRank() == Common::ICard::NINE &&
												  getRuleSet()->getDirChangeIsSuspend()))) {
					getEventHandler().getConnection()->wait(getAIDelay());
					m_alreadyWaited = true;
				}
			}

		} else {

			suspends(player, uc);
			getRuleSet()->hasSuspended();

			if(m_jackMode) {
				jackModeOff();
				m_jackMode = false;
			}
		}

		if(getRuleSet()->hasDirChange()) {

			if(m_dirChangeEnabled && m_players.size() > 2) {

				std::reverse(m_players.begin(), m_players.end());

				m_nxtPlayer = static_cast<std::size_t>(std::distance(m_players.begin(),
													   std::find(m_players.begin(), m_players.end(),
															   player)));
				assert(m_nxtPlayer <= m_players.size());

				getEventHandler().directionChange();

			} else if(m_dirChangeEnabled) {
				setDirChangeIsSuspend(true);
			}

			getRuleSet()->dirChanged();
		}

		const Player::IPlayer *curPlayer = player;

		if(!won) {
			const std::size_t leftCount = player->getCardCount();
			m_nxtPlayer = (m_nxtPlayer + 1) >= m_players.size() ? 0 : m_nxtPlayer + 1;
			const std::size_t rightCount = m_players[(m_nxtPlayer + 1) >= m_players.size() ? 0 :
										   m_nxtPlayer + 1]->getCardCount();
			m_players[m_nxtPlayer]->setNeighbourCardCount(m_players.size(), leftCount, rightCount);
		}

		if(!m_nxtPlayer) ++m_turn;

		m_initialJack = false;

		if(!m_alreadyWaited && wait(curPlayer, false)) {
			getEventHandler().getConnection()->wait(getAIDelay());
		}

		informAIStat();

	} catch(const Common::Exception::SocketException &e) {

		logDebug("SocketException: " << e);

		Common::IConnection *con = getEventHandler().getConnection();
		const std::string &pName(con->getPlayerName(e.sockfd()));

		std::vector<std::string> ex(1, pName);
		const PLAYERS::const_iterator &f(find(pName));
		std::ostringstream os;

		const bool lostWatchingPlayer = !pName.empty() && f == m_players.end();

		if(!lostWatchingPlayer) {

			if(!pName.empty()) {

				os << "Lost connection to player \"" << pName << "\"";

				try {
					getEventHandler().error(os.str(), ex);
				} catch(const Common::Exception::SocketException &) {}

			} else {
				try {
					getEventHandler().error("Lost connection to a player", ex);
				} catch(const Common::Exception::SocketException &) {}
			}

		} else {

			try {

				std::ostringstream watcher;
				watcher << pName << " is no more watching us";
				getEventHandler().message(watcher.str());

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

		if(!lostWatchingPlayer) m_state = FINISHED;

	}

	return true;
}

bool Engine::takeCards(Player::IPlayer *player, const Common::ICard *card) const
throw(Common::Exception::SocketException) {

	const std::size_t cardCount = getRuleSet()->takeCards(card);

	if(cardCount) {

		for(std::size_t i = 0; i < cardCount; ++i) player->receiveCard(m_talon->takeCard());

		getEventHandler().playerPicksCards(player, cardCount);
		getRuleSet()->hasTakenCards();

		return true;
	}

	return false;
}

void Engine::uncoveredCard(const Common::ICard *top) const
throw(Common::Exception::SocketException) {
	getEventHandler().uncoveredCard(top);
}

void Engine::talonEmpty(bool empty) const throw() {
	try {
		getEventHandler().talonEmpty(empty);
	} catch(const Common::Exception::SocketException &e) {
		logDebug(__PRETTY_FUNCTION__ << ": failed to handle event \'talonEmpty\': " << e.what());
	}
}

void Engine::cardPlayed(Common::ICard *card) const {
	for(PLAYERS ::const_iterator i(m_players.begin()); i != m_players.end(); ++i) {
		(*i)->cardPlayed(card);
	}
}

void Engine::shuffled() const {
	for(PLAYERS ::const_iterator i(m_players.begin()); i != m_players.end(); ++i) {
		(*i)->talonShuffled();
	}
}

Common::ICard::RANK Engine::getAceRoundRank() const {
	return m_cfg.getAceRoundRank();
}

void Engine::aceRoundStarted(const Player::IPlayer *player) const
throw(Common::Exception::SocketException) {
	m_cfg.getEventHandler().aceRoundStarted(player);
}

void Engine::aceRoundEnded(const Player::IPlayer *player) const
throw(Common::Exception::SocketException) {
	m_cfg.getEventHandler().aceRoundEnded(player);
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
	getRuleSet()->setDirChangeIsSuspend(b);

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
	getEventHandler().gameAboutToStart();
}

void Engine::gameOver() const throw() {
	try {
		getEventHandler().gameOver();
	} catch(const Common::Exception::SocketException &e) {
		logDebug(__PRETTY_FUNCTION__ << ": failed to handle event \'gameOver\': " << e.what());
	}
}

void Engine::jackModeOff() const {

	getRuleSet()->setJackModeOff();

	try {
		getEventHandler().setJackModeOff();
	} catch(const Common::Exception::SocketException &e) {
		logDebug(__PRETTY_FUNCTION__ << ": failed to handle event \'jackModeOff\': " << e.what());
	}
}

void Engine::checkPlayersAlive() const throw(Common::Exception::SocketException) {
	for(PLAYERS::const_iterator i(m_players.begin()); i != m_players.end(); ++i) {
		if(!(*i)->isAlive()) {
			(*i)->setCardCountObserver(0L);
			throw Common::Exception::SocketException((*i)->getName() + " is dead");
		}
	}
}

long Engine::getAIDelay() const {
	return (!getEventHandler().getConnection()->hasHumanPlayers()) ? 0L : m_cfg.getAIDelay();
}

bool Engine::wait(const Player::IPlayer *p, bool suspend) const {

	const bool isAI = p->isAIPlayer();

	if(isAI && getAICount() == m_players.size()) return true;

	if(m_players.size() == 2) return suspend && isAI;

	return isAI ? m_alwaysWait : false;
}

void Engine::cardCountChanged(Player::IPlayer *p) const throw() {

	try {
		getEventHandler().stats(PLAYERS(1, p));
	} catch(const Common::Exception::SocketException &e) {
		logDebug(__PRETTY_FUNCTION__ << ": failed to handle event \'stats()\': " << e.what());
	}
}

RuleSet::IRuleSet *Engine::getRuleSet() const {
	return m_cfg.getRuleSet(this);
}

void Engine::reset() throw() {

	m_state = ACCEPT_PLAYERS;

	delete m_talon;
	m_talon = new Talon(this, m_cfg.getTalonFactor());

	m_cfg.getEventHandler().reset();
	getRuleSet()->reset();
	removePlayers();

	m_nxtPlayer = 0,
	m_turn = 1;
	m_curTurn = 0;

	m_jackMode = false;
	m_cfg.setNextMessage(m_initialNextMessage);
	m_initialChecked = false;
	m_initialJack = false;
	m_alwaysWait = false;
	m_alreadyWaited = false;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
