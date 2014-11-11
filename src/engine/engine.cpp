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

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#include <sstream>
#include <algorithm>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "engine.h"

#include "talon.h"
#include "logger.h"
#include "iplayer.h"
#include "stdruleset.h"
#include "ieventhandler.h"
#include "abstractconnection.h"

using namespace NetMauMau;

Engine::Engine(Event::IEventHandler &eventHandler, bool nextMessage) : m_eventHandler(eventHandler),
	m_state(ACCEPT_PLAYERS), m_talon(new Talon(this)), m_ruleset(new RuleSet::StdRuleSet()),
	m_players(), m_nxtPlayer(0), m_turn(1), m_curTurn(0), m_delRuleSet(true), m_jackMode(false),
	m_initialChecked(false), m_nextMessage(nextMessage), m_ultimate(false) {
	m_players.reserve(5);
	m_eventHandler.acceptingPlayers();
}

Engine::Engine(Event::IEventHandler &eventHandler, RuleSet::IRuleSet *ruleset, bool nextMessage) :
	m_eventHandler(eventHandler), m_state(ACCEPT_PLAYERS), m_talon(new Talon(this)),
	m_ruleset(ruleset), m_players(), m_nxtPlayer(0), m_turn(1), m_curTurn(0), m_delRuleSet(false),
	m_jackMode(false), m_initialChecked(false), m_nextMessage(nextMessage), m_ultimate(false) {
	m_players.reserve(5);
	m_eventHandler.acceptingPlayers();
}

Engine::~Engine() {
	delete m_talon;

	if(m_delRuleSet) delete m_ruleset;
}

Engine::PLAYERS::iterator Engine::find(const std::string &name) {

	std::string nameB(name);

	for(PLAYERS::iterator i(m_players.begin()); i != m_players.end(); ++i) {

		std::string nameA((*i)->getName());

		std::transform(nameA.begin(), nameA.end(), nameA.begin(), ::tolower);
		std::transform(nameB.begin(), nameB.end(), nameB.begin(), ::tolower);

		if(nameA == nameB) return i;
	}

	return m_players.end();
}

bool Engine::addPlayer(Player::IPlayer *player) throw(Common::Exception::SocketException) {

	if(m_state == ACCEPT_PLAYERS) {

		const PLAYERS::iterator &f(find(player->getName()));

		if(f == m_players.end() && m_players.size() <= m_ruleset->getMaxPlayers()) {

			m_players.push_back(player);

			if(player->isAIPlayer()) {

				Common::AbstractConnection *con = m_eventHandler.getConnection();

				if(con) con->addAIPlayers(std::vector<std::string>(1, player->getName()));
			}

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

	return f;
}

bool Engine::distributeCards() throw(Common::Exception::SocketException) {

	if(m_state == NOCARDS || m_state == ACCEPT_PLAYERS) {

		PLAYERS::const_iterator pi(m_players.begin());
		const PLAYERS::const_iterator &pe(m_players.end());

		for(; pi != pe; ++pi) {

			Player::IPlayer *p = *pi;

			std::vector<Common::ICard *> cards;
			cards.reserve(5);

			for(std::size_t i = 0; i < 5; ++i) {
				if(m_talon->empty()) return false;

				cards.push_back(m_talon->top());
				m_talon->pop();
			}

			p->receiveCardSet(cards);
			m_eventHandler.cardsDistributed(p, cards);

		}

		m_turn = 1;
		m_curTurn = 0;
		m_state = PLAYING;

		return true;

	} else {
		m_eventHandler.cardsAlreadyDistributed();
	}

	return false;
}

void Engine::reversePlayers() {
	std::reverse(m_players.begin(), m_players.end());
}

void Engine::message(const std::string &msg) throw(Common::Exception::SocketException) {
	m_eventHandler.message(msg);
}

bool Engine::nextTurn() {

	if(m_eventHandler.shutdown() || m_state != PLAYING) return false;

	try {

		Player::IPlayer *player = m_players[m_nxtPlayer];

		if(m_curTurn != m_turn) {
			m_eventHandler.turn(m_turn);

			if(m_turn == 1) m_eventHandler.initialCard(m_talon->uncoverCard());

			m_curTurn = m_turn;
		}

		if(m_nextMessage) m_eventHandler.nextPlayer(player);

		const Common::ICard *uc = m_talon->getUncoveredCard();

		if(!m_initialChecked) {
			m_ruleset->checkInitial(player, uc);
			m_initialChecked = true;
		}

		m_eventHandler.stats(m_players);

		const bool csuspend = m_ruleset->hasToSuspend();
		const Common::ICard::SUIT js = m_ruleset->getJackSuit();

		Common::ICard *pc = !csuspend ? player->requestCard(uc, m_jackMode ? &js : 0L) : 0L;

		bool won = false;

		if(!csuspend) {

			bool suspend = false;

			if(m_ruleset->takeCards(pc)) {

				const std::size_t cardCount = m_ruleset->takeCards(pc);

				for(std::size_t i = 0; i < cardCount; ++i) {
					player->receiveCard(m_talon->takeCard());
				}

				m_eventHandler.playerPicksCards(player, cardCount);
				m_ruleset->hasTakenCards();
			}

			if(!pc) {
				player->receiveCard(pc = m_talon->takeCard());

				if(player->getNoCardReason() == Player::IPlayer::SUSPEND) {
					m_eventHandler.playerSuspends(player);
					pc = 0L;
				}
			}

			bool cc = false;

			while(pc && !(cc = m_ruleset->checkCard(player, uc, pc, !m_nextMessage))) {

				const bool aiSusp = (!cc && player->isAIPlayer());

				if(suspend || aiSusp) {
					m_eventHandler.playerSuspends(player);
					break;
				}

				m_eventHandler.cardRejected(player, uc, pc);

				const Common::ICard::SUIT js2 = m_ruleset->getJackSuit();

				if((!(pc = player->requestCard(uc, m_jackMode ? &js2 : 0L)))) {

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

				if(m_jackMode) m_ruleset->setJackModeOff();

				won = player->cardAccepted(pc);
				m_talon->playCard(pc);

				m_eventHandler.playerPlaysCard(player, pc, uc);

				if((m_jackMode = m_ruleset->isJackMode())) {
					m_eventHandler.playerChooseJackSuit(player, m_ruleset->getJackSuit());
				}

				if(won) {

					PLAYERS::iterator f(m_players.begin());
					std::advance(f, m_nxtPlayer);
					PLAYERS::iterator nxt = m_players.erase(f);

					m_nxtPlayer = nxt != m_players.end() ? std::distance(m_players.begin(), nxt)
								  : 0;

					if(!hasPlayers()) {
						m_eventHandler.playerLost(m_players[m_nxtPlayer], m_turn);
						m_state = FINISHED;
					}

					m_eventHandler.playerWins(player, m_turn, m_ultimate);
				}
			}

		} else {
			m_eventHandler.playerSuspends(player, uc);
			m_ruleset->hasSuspended();
		}

		if(!won) m_nxtPlayer = (m_nxtPlayer + 1) >= m_players.size() ? 0 : m_nxtPlayer + 1;

		if(!m_nxtPlayer) ++m_turn;

	} catch(const Common::Exception::SocketException &e) {

		Common::AbstractConnection *con = m_eventHandler.getConnection();

		if(con) {

			const std::string &pName(con->getPlayerName(e.sockfd()));

			std::vector<std::string> ex(1, pName);
			const PLAYERS::iterator &f(find(pName));
			std::ostringstream os;

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

			con->removePlayer(e.sockfd());

			shutdown(e.sockfd(), SHUT_RDWR);
			close(e.sockfd());

			if(f != m_players.end()) removePlayer(*f);
		}

		m_state = FINISHED;
	}

	return true;
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

void Engine::gameOver() throw() {
	try {
		m_eventHandler.gameOver();
	} catch(const Common::Exception::SocketException &e) {
		logDebug(__PRETTY_FUNCTION__ << ": failed to handle event \'gameOver\': " << e.what());
	}
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
	m_initialChecked = false;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
