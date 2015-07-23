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
#include "config.h"                     // for HAVE_SYS_SOCKET_H, etc
#endif

#ifdef _WIN32
#include <ws2tcpip.h>
#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif
#elif defined(HAVE_SYS_SOCKET_H)
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>                 // for shutdown, SHUT_RDWR
#endif

#if defined(HAVE_GSL)
#include <random_gen.h>                 // for GSLRNG
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>                     // for close
#endif

#include <stdbool.h>

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include "nextturn.h"
#include "logger.h"                     // for logDebug, BasicLogger
#include "enginecontext.h"              // for EngineConfig
#include "ieventhandler.h"              // for IEventHandler
#include "iruleset.h"                   // for IRuleSet
#include "talon.h"                      // for Talon
#include "ci_char_traits.h"

namespace {
const std::string TALONUNDERFLOW("TALON-UNDERFLOW: attempt to take more cards from the talon " \
								 "than available!");

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct PlayerNameEqualCI : std::binary_function < NetMauMau::Player::IPlayer *,
		std::string, bool > {

	inline result_type operator()(const first_argument_type x,
								  const second_argument_type y) const {
		return std::equal_to<NetMauMau::Common::ci_string>()(x->getName().c_str(), y.c_str());
	}
};

struct PlayerNameEqual : std::binary_function < NetMauMau::Player::IPlayer *,
		NetMauMau::Player::IPlayer *, bool > {

	inline result_type operator()(const first_argument_type x,
								  const second_argument_type y) const {
		return x->getName() == y->getName();
	}
};
#pragma GCC diagnostic pop
}

using namespace NetMauMau;

Engine::Engine(EngineContext &ctx) throw(Common::Exception::SocketException) : ITalonChange(),
	IAceRoundListener(), ICardCountObserver(), m_ctx(ctx), m_nextTurn(0L), m_state(ACCEPT_PLAYERS),
	m_talon(new Talon(this, ctx.getTalonFactor())), m_players(), m_turn(1), m_curTurn(0),
	m_ultimate(false), m_alwaysWait(false), m_initialNextMessage(ctx.getNextMessage()),
	m_gameIndex(0LL), m_dirChangeEnabled(false), m_talonUnderflow(false), m_aiCount(0) {
	m_players.reserve(5);
	ctx.getEventHandler().acceptingPlayers();
}

Engine::~Engine() {
	delete m_talon;
	delete m_nextTurn;
}

const Event::IEventHandler &Engine::getEventHandler() const {
	return m_ctx.getEventHandler();
}

Engine::PLAYERS::const_iterator Engine::find(const std::string &player) const {
	return std::find_if(m_players.begin(), m_players.end(), std::bind2nd(PlayerNameEqualCI(),
						player));
}

bool Engine::addPlayer(Player::IPlayer *player) throw(Common::Exception::SocketException) {

	if(m_state == ACCEPT_PLAYERS) {

		const PLAYERS::const_iterator &f(find(player->getName()));
		const RuleSet::IRuleSet *ruleSet = getRuleSet();

		if(f == m_players.end() && m_players.size() <= ruleSet->getMaxPlayers()) {

			m_players.push_back(player);

			m_dirChangeEnabled = m_players.size() > 2u;

			if(player->isAIPlayer()) {
				getEventHandler().getConnection().
				addAIPlayers(std::vector<std::string>(1, player->getName()));
			}

			player->setRuleSet(ruleSet);
			player->setEngineContext(&m_ctx);

			getEventHandler().playerAdded(player);
			notify(m_players);

			return true;

		} else if(f != m_players.end()) {

			getEventHandler().playerRejected(player);
			getEventHandler().getConnection().removePlayer(player->getSerial());

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

Engine::PLAYERS::iterator Engine::removePlayer(Player::IPlayer *player)
throw(Common::Exception::SocketException) {

	const PLAYERS::iterator &f(erasePlayer(std::find(m_players.begin(), m_players.end(), player)));

	try {
		getRuleSet()->setCurPlayers(m_players.size());
	} catch(const Common::Exception::SocketException &e) {
		logDebug(e);
	}

	return f;
}

Engine::PLAYERS::iterator Engine::erasePlayer(const PLAYERS::iterator &pi) {

	if(pi != m_players.end()) {

		const PLAYERS::iterator f(m_players.erase(pi));

		m_aiCount = countAI();
		notify(m_players);

		return f;
	}

	return pi;
}

std::size_t Engine::countAI() const {
	return static_cast<std::size_t>(std::count_if(m_players.begin(), m_players.end(),
									std::mem_fun(&Player::IPlayer::isAIPlayer)));
}

bool Engine::distributeCards() throw(Common::Exception::SocketException) {

	m_aiCount = countAI();

	if(m_state == NOCARDS || m_state == ACCEPT_PLAYERS) {

		std::vector<Player::IPlayer::CARDS> cards(m_players.size());

		const std::size_t icc = getRuleSet()->initialCardCount();

		for(std::size_t i = 0u; i < icc; ++i) {

			if(m_talon->empty()) return false;

			for(PLAYERS::size_type j = 0u; j < m_players.size(); ++j) {
				cards[j].push_back(m_talon->top());
				m_talon->pop();
			}
		}

		PLAYERS::const_iterator pi(m_players.begin());

		for(std::size_t k = 0u; pi != m_players.end(); ++pi, ++k) {

			PLAYERS::const_iterator::reference p(*pi);
			const std::vector<Player::IPlayer::CARDS>::reference card(cards[k]);

			p->receiveCardSet(card);
			p->setDirChangeEnabled(m_dirChangeEnabled);
			p->setCardCountObserver(this);

			getEventHandler().cardsDistributed(p, card);
		}

		m_turn = 1u;
		m_curTurn = 0u;
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

void Engine::initialTurn() throw(Common::Exception::SocketException) {

	if(m_nextTurn) delete m_nextTurn;

	m_nextTurn = new NextTurn(this);
}

bool Engine::nextTurn() throw(Common::Exception::SocketException) {
	return m_nextTurn->compute();
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

void Engine::shuffled() const {
	std::for_each(m_players.begin(), m_players.end(),
				  std::mem_fun(&Player::IPlayer::talonShuffled));
}

void Engine::underflow() {
	m_talonUnderflow = true;
#ifndef NDEBUG
	logDebug(TALONUNDERFLOW);
#ifdef HAVE_GSL
	logDebug("GSL: name=" << gsl_rng_default->name << "; seed=" << gsl_rng_default_seed);
#endif

	try {
		message(TALONUNDERFLOW);
	} catch(const Common::Exception::SocketException &e) {
		logDebug(e);
	}

#endif
}

Common::ICard::RANK Engine::getAceRoundRank() const {
	return m_ctx.getAceRoundRank();
}

void Engine::aceRoundStarted(const Player::IPlayer *player) const
throw(Common::Exception::SocketException) {
	m_ctx.getEventHandler().aceRoundStarted(player);
}

void Engine::aceRoundEnded(const Player::IPlayer *player) const
throw(Common::Exception::SocketException) {
	m_ctx.getEventHandler().aceRoundEnded(player);
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

void Engine::cardCountChanged(const Player::IPlayer *p) const throw() {
	try {
		getEventHandler().stats(PLAYERS(1, const_cast<Player::IPlayer *>(p)));
	} catch(const Common::Exception::SocketException &e) {
		logDebug(__PRETTY_FUNCTION__ << ": failed to handle event \'stats()\': " << e.what());
	}
}

RuleSet::IRuleSet *Engine::getRuleSet() {

	try {
		return m_ctx.getRuleSet(this);
	} catch(const Lua::Exception::LuaException &e) {
		logWarning(e);
		return 0L;
	}
}

const IPlayedOutCards *Engine::getPlayedOutCards() const {
	return m_talon;
}

void Engine::reset() throw() {

	m_state = ACCEPT_PLAYERS;
	m_talon->reset();

	m_ctx.getEventHandler().reset();

	try {
		getRuleSet()->reset();
	} catch(const Lua::Exception::LuaException &e) {
		logDebug(e);
	}

	removePlayers();

	m_curTurn = 0u;
	m_turn = 1u;

	m_alwaysWait = m_talonUnderflow = false;

	m_ctx.setNextMessage(m_initialNextMessage);

	delete m_nextTurn;
	m_nextTurn = 0L;
}

const std::string &Engine::getTalonUnderflowString() {
	return TALONUNDERFLOW;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
