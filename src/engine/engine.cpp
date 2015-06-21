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

#include <cassert>                      // for assert
#include <stdbool.h>

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include "abstractsocket.h"
#include "logger.h"                     // for logDebug, BasicLogger
#include "enginecontext.h"              // for EngineConfig
#include "ieventhandler.h"              // for IEventHandler
#include "iplayer.h"                    // for IPlayer, IPlayer::CARDS, etc
#include "iruleset.h"                   // for IRuleSet
#include "luafatalexception.h"          // for LuaException
#include "talon.h"                      // for Talon
#include "ci_char_traits.h"
#include "protocol.h"

namespace {
const std::string TALONUNDERFLOW("TALON-UNDERFLOW: attempt to take more cards from the talon " \
								 "than available!");

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
	IAceRoundListener(), ICardCountObserver(), m_ctx(ctx), m_db(DB::SQLite::getInstance()),
	m_state(ACCEPT_PLAYERS), m_talon(new Talon(this, ctx.getTalonFactor())), m_players(),
	m_nxtPlayer(0), m_turn(1), m_curTurn(0), m_jackMode(false), m_initialChecked(false),
	m_ultimate(false), m_initialJack(false), m_alwaysWait(false), m_alreadyWaited(false),
	m_initialNextMessage(ctx.getNextMessage()), m_gameIndex(0LL), m_dirChangeEnabled(false),
	m_talonUnderflow(false) {
	m_players.reserve(5);
	ctx.getEventHandler().acceptingPlayers();
}

Engine::~Engine() {
	delete m_talon;
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

	const PLAYERS::iterator &f(std::find(m_players.begin(), m_players.end(), player));

	if(f != m_players.end()) return m_players.erase(f);

	try {
		getRuleSet()->setCurPlayers(m_players.size());
	} catch(const Common::Exception::SocketException &e) {
		logDebug(e);
	}

	return f;
}

bool Engine::distributeCards() throw(Common::Exception::SocketException) {

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

			getEventHandler().cardsDistributed(p, card);
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

void Engine::suspends(Player::IPlayer *p,
					  const Common::ICard *uc) const throw(Common::Exception::SocketException) {
	getEventHandler().playerSuspends(p, uc);
}

bool Engine::nextTurn() {

	m_alreadyWaited = false;

	if(getEventHandler().shutdown() || m_state != PLAYING) return false;

	if(getAICount() == 1) {
		m_alwaysWait = false;
		m_ctx.setNextMessage(false);
	}

	try {

		checkPlayersAlive();

		PLAYERS::value_type player(m_players[m_nxtPlayer]);

		player->setCardCountObserver(this);

		if(m_curTurn != m_turn) {

			m_db->turn(m_gameIndex, m_turn);

			getEventHandler().turn(m_turn);

			if(m_turn == 1u) {

				m_db->gamePlayStarted(m_gameIndex);
				getEventHandler().initialCard(m_talon->uncoverCard());

				if(getAICount() && m_talon->getUncoveredCard() == Common::ICard::EIGHT) {
					getEventHandler().getConnection().
					wait(static_cast<long int>(std::floor(static_cast
														  <float>(getAIDelay()) * 1.5f)));
				}
			}

			m_curTurn = m_turn;
		}

		if(m_ctx.getNextMessage()) getEventHandler().nextPlayer(player);

		const Common::ICardPtr uc(m_talon->getUncoveredCard());

		if(!m_initialChecked) {
			getRuleSet()->checkInitial(player, uc);
			checkAndPerformDirChange(player, false);
			m_initialJack = getRuleSet()->isJackMode();
			m_initialChecked = true;
		}

		const bool suspend = getRuleSet()->hasToSuspend() && !m_talonUnderflow;
		const Common::ICard::SUIT js = getRuleSet()->getJackSuit();

		assert(uc != Common::ICard::JACK || (uc == Common::ICard::JACK &&
											 ((m_jackMode || m_initialJack) &&
											  js != Common::ICard::SUIT_ILLEGAL)));

		Common::ICardPtr pc(!suspend ? player->requestCard(uc, (m_jackMode || m_initialJack)
							? &js : 0L, getRuleSet()->takeCardCount(), m_talonUnderflow)
								: Common::ICardPtr());

		if(m_initialJack && !pc) m_jackMode = true;

		bool won = false;

		if(!suspend) {

			const bool noCardOk = takeCards(player, pc);

sevenRule:

			if(!pc) {

				const Player::IPlayer::REASON reason = noCardOk ? Player::IPlayer::SUSPEND :
													   player->getNoCardReason(uc, m_jackMode ?
															   &js : 0L);

				if(!noCardOk && (pc = m_talon->takeCard())) {
					player->receiveCard(pc);
					getEventHandler().playerPicksCard(player);
				}

				if(reason == Player::IPlayer::SUSPEND) {
					suspends(player);
					pc = Common::ICardPtr();
				} else if(reason == Player::IPlayer::NOMATCH) {
					pc = Common::ICardPtr(const_cast<const Common::ICard *>
										  (Common::getIllegalCard()));
				}

			} else if(pc == Common::ICard::SUIT_ILLEGAL) {
				pc = player->requestCard(uc, m_jackMode ? &js : 0L, getRuleSet()->takeCardCount());
				goto sevenRule;
			}

			const bool cardAccepted = checkCard(player, pc, uc);

			if(pc && cardAccepted) {

				if(m_jackMode || m_initialJack) jackModeOff();

				won = player->cardAccepted(pc);

				m_talon->playCard(pc);
				m_talonUnderflow = m_talon->thresholdReached(1u + (8u * m_ctx.getTalonFactor()));

				getEventHandler().playerPlaysCard(player, pc, uc);

				if((m_jackMode = getRuleSet()->isJackMode())) {
					getEventHandler().playerChooseJackSuit(player, getRuleSet()->getJackSuit());
				}

				if(won) {
					handleWinner(player);
				} else if(wait(player, true) && (pc == Common::ICard::EIGHT ||
												 (pc == Common::ICard::NINE &&
												  getRuleSet()->getDirChangeIsSuspend()))) {
					getEventHandler().getConnection().wait(getAIDelay());
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

		checkAndPerformDirChange(player, won);

		const PLAYERS::value_type curPlayer(player);

		if(!won) {

			const std::size_t leftCount = player->getCardCount();
			const Common::ICard::SUIT lps = player->getLastPlayedSuit();
			const Common::ICard::RANK lpr = player->getLastPlayedRank();

			m_nxtPlayer = (m_nxtPlayer + 1u) >= m_players.size() ? 0u : m_nxtPlayer + 1u;

			const Player::IPlayer *rightPlayer = m_players[(m_nxtPlayer + 1u) >= m_players.size()
												 ? 0u : m_nxtPlayer + 1u];

			const std::size_t  rightCount = rightPlayer->getCardCount();
			const Common::ICard::SUIT rps = rightPlayer->getLastPlayedSuit();
			const Common::ICard::RANK rpr = rightPlayer->getLastPlayedRank();

			const std::size_t neighbourCount[] = { leftCount, rightCount };

			Player::IPlayer::NEIGHBOURRANKSUIT nrs;
			nrs.rank[Player::IPlayer::LEFT]  = lpr;
			nrs.rank[Player::IPlayer::RIGHT] = rpr;
			nrs.suit[Player::IPlayer::LEFT]  = lps;
			nrs.suit[Player::IPlayer::RIGHT] = rps;

			m_players[m_nxtPlayer]->setNeighbourCardStats(m_players.size(), neighbourCount, nrs);
		}

		if(!m_nxtPlayer) ++m_turn;

		m_initialJack = false;

		if(!m_alreadyWaited && wait(curPlayer, false)) {
			getEventHandler().getConnection().wait(getAIDelay());
		}

		informAIStat();

	} catch(const Lua::Exception::LuaFatalException &) {
		throw;
	} catch(const Common::Exception::SocketException &e) {

		logDebug("SocketException: " << e);

		if(TALONUNDERFLOW == e.what()) {
			try {
				getEventHandler().error(TALONUNDERFLOW);
			} catch(const Common::Exception::SocketException &) {}
		}

		Common::IConnection &con(getEventHandler().getConnection());
		const std::string &pName(con.getPlayerName(e.sockfd()));

		const std::vector<std::string> ex(1, pName);
		const PLAYERS::const_iterator &f(find(pName));

		const bool lostWatchingPlayer = !pName.empty() && f == m_players.end();

		if(!lostWatchingPlayer) {

			if(!pName.empty()) {

				std::ostringstream os;

				os << NetMauMau::Common::Protocol::V15::ERR_TO_EXC_LOSTCONNNAMED
				   << "\"" << pName << "\"";

				try {
					getEventHandler().error(os.str(), ex);
				} catch(const Common::Exception::SocketException &) {}

			} else {
				try {
					getEventHandler().error(NetMauMau::Common::Protocol::V15::ERR_TO_EXC_LOSTCONN,
											ex);
				} catch(const Common::Exception::SocketException &) {}
			}

		} else {

			try {

				std::ostringstream watcher;
				watcher << pName << " is no more watching us";
				getEventHandler().message(watcher.str());

			} catch(const Common::Exception::SocketException &) {}
		}

		con.removePlayer(e.sockfd());
		disconnectError(e.sockfd());

		if(f != m_players.end()) removePlayer(*f);

		if(!lostWatchingPlayer) m_state = FINISHED;

	}

	return true;
}

void Engine::handleWinner(const Player::IPlayer *player) throw(Common::Exception::SocketException) {

	PLAYERS::iterator f(m_players.begin());
	std::advance(f, std::min(m_players.size() - 1u, m_nxtPlayer));

	if(f != m_players.end()) m_players.erase(f);

	const PLAYERS::size_type plsCnt = m_players.size();
	RuleSet::IRuleSet *const rulSet = getRuleSet();

	m_nxtPlayer = m_nxtPlayer < plsCnt ? m_nxtPlayer + 1 : 0;

	rulSet->setCurPlayers(plsCnt);

	if(!hasPlayers()) {

		if(rulSet->takeIfLost() && m_talon->getUncoveredCard() == Common::ICard::SEVEN) {
			takeCards(m_players[m_nxtPlayer], Common::getIllegalCard());
		}

		const Common::IConnection::NAMESOCKFD nsf = (m_players[m_nxtPlayer]->isAIPlayer()) ?
				Common::IConnection::NAMESOCKFD(m_players[m_nxtPlayer]->getName(), "",
												m_players[m_nxtPlayer]->getSerial(), 0) :
				getEventHandler().getConnection().
				getPlayerInfo(m_players[m_nxtPlayer]->getSerial());

		m_db->playerLost(m_gameIndex, nsf, std::time(0L),
						 getEventHandler().playerLost(m_players[m_nxtPlayer], m_turn,
								 rulSet->lostPointFactor(m_talon->getUncoveredCard())));
		m_state = FINISHED;

	} else if(m_dirChangeEnabled && plsCnt <= 2u) {
		setDirChangeIsSuspend(true);
	}

	getEventHandler().playerWins(player, m_turn, m_ultimate);

	const Common::IConnection::NAMESOCKFD nsf = (player->isAIPlayer()) ?
			Common::IConnection::NAMESOCKFD(player->getName(), "", player->getSerial(), 0) :
			getEventHandler().getConnection().getPlayerInfo(player->getSerial());

	m_db->playerWins(m_gameIndex, nsf);
}

bool Engine::checkCard(Player::IPlayer *player, Common::ICardPtr &playedCard,
					   const Common::ICardPtr &uc) const throw(Common::Exception::SocketException) {

	bool noMatch, cardAccepted = false;
	RuleSet::IRuleSet *ruleSet = getRuleSet();

	while(playedCard && ((noMatch = (playedCard == Common::ICard::SUIT_ILLEGAL)) ||
						 !(cardAccepted = ruleSet->checkCard(player, uc, playedCard,
										  !m_ctx.getNextMessage())))) {

		if(!noMatch) getEventHandler().cardRejected(player, uc, playedCard);

		const Common::ICard::SUIT js = ruleSet->getJackSuit();

		if(!(playedCard = player->requestCard(uc, m_jackMode ? &js : 0L,
											  ruleSet->takeCardCount()))) {

			if(!noMatch) {
				Common::ICardPtr rc(m_talon->takeCard());

				if(rc) player->receiveCard(rc);
			}

			suspends(player);
			break;
		}
	}

	return cardAccepted;
}

void Engine::disconnectError(SOCKET fd) const {
	if(fd != INVALID_SOCKET) Common::AbstractSocket::shutdown(fd);
}

void Engine::checkAndPerformDirChange(const Player::IPlayer *player, bool won)
throw(Common::Exception::SocketException) {

	RuleSet::IRuleSet *ruleSet = getRuleSet();

	if(ruleSet->hasDirChange()) {

		if(m_dirChangeEnabled && m_players.size() > 2u) {

			std::reverse(m_players.begin(), m_players.end());

			m_nxtPlayer = static_cast<std::size_t>(std::distance(m_players.begin(),
												   std::find(m_players.begin(), m_players.end(),
														   won ? m_players[m_nxtPlayer] : player)));
			assert(m_nxtPlayer <= m_players.size());

			getEventHandler().directionChange();

		} else if(m_players.size() <= 2u) {
			setDirChangeIsSuspend(true);
		}

		ruleSet->dirChanged();
	}
}

bool Engine::takeCards(Player::IPlayer *player, const Common::ICard *card) const
throw(Common::Exception::SocketException) {

	const std::size_t cardCount = getRuleSet()->takeCards(card);

	if(cardCount) {

		Player::IPlayer::CARDS tmp;

		if(cardCount <= tmp.max_size()) tmp.reserve(cardCount);

		std::size_t j = 0;

		for(std::size_t i = 0; i < cardCount; ++i) {

			const Common::ICardPtr &c(m_talon->takeCard());

			if(c) {
				tmp.push_back(c);
				++j;
			}
		}

		if(j == cardCount) {

			for(Player::IPlayer::CARDS::const_iterator ri(tmp.begin()); ri != tmp.end(); ++ri)
				player->receiveCard(*ri);

			getEventHandler().playerPicksCards(player, j);

		} else if(j) {
			for(Player::IPlayer::CARDS::const_reverse_iterator ri(tmp.rbegin());
					ri != tmp.rend(); ++ri) m_talon->pushBackCard(*ri);
		}

		getRuleSet()->hasTakenCards();

		return !getRuleSet()->takeAfterSevenIfNoMatch();
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
	message(TALONUNDERFLOW);
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

void Engine::informAIStat() const {
	std::for_each(m_players.begin(), m_players.end(), _informAIStat(m_players));
}

void Engine::setDirChangeIsSuspend(bool b) throw(Common::Exception::SocketException) {

	getRuleSet()->setDirChangeIsSuspend(b);

	std::for_each(m_players.begin(), m_players.end(),
				  std::bind2nd(std::mem_fun(&Player::IPlayer::setNineIsSuspend), b));
}

std::size_t Engine::getAICount() const {
	return static_cast<std::size_t>(std::count_if(m_players.begin(), m_players.end(),
									std::mem_fun(&Player::IPlayer::isAIPlayer)));
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

void Engine::jackModeOff() const throw(Common::Exception::SocketException) {

	getRuleSet()->setJackModeOff();

	try {
		getEventHandler().setJackModeOff();
	} catch(const Common::Exception::SocketException &e) {
		logDebug(__PRETTY_FUNCTION__ << ": failed to handle event \'jackModeOff\': " << e.what());
	}
}

void Engine::checkPlayersAlive() const throw(Common::Exception::SocketException) {
	for(PLAYERS::const_iterator i(m_players.begin()); i != m_players.end(); ++i) {

		PLAYERS::const_iterator::reference r(*i);

		if(!r->isAlive()) {
			r->setCardCountObserver(0L);
			throw Common::Exception::SocketException((*i)->getName() + " is dead");
		}
	}
}

long Engine::getAIDelay() const {
	return (!getEventHandler().getConnection().hasHumanPlayers()) ? 0L : m_ctx.getAIDelay();
}

bool Engine::wait(const Player::IPlayer *p, bool suspend) const {

	const bool isAI = p->isAIPlayer();
	const PLAYERS::size_type pcnt(m_players.size());

	if(isAI && getAICount() == pcnt) return true;

	if(pcnt == 2) return suspend && isAI;

	return isAI ? m_alwaysWait : false;
}

void Engine::cardCountChanged(const Player::IPlayer *p) const throw() {
	try {
		getEventHandler().stats(PLAYERS(1, const_cast<Player::IPlayer *>(p)));
	} catch(const Common::Exception::SocketException &e) {
		logDebug(__PRETTY_FUNCTION__ << ": failed to handle event \'stats()\': " << e.what());
	}
}

RuleSet::IRuleSet *Engine::getRuleSet() const {
	return m_ctx.getRuleSet(this);
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

	m_nxtPlayer = m_curTurn = 0u;
	m_turn = 1u;

	m_jackMode = m_initialChecked =
					 m_initialJack = m_alwaysWait = m_alreadyWaited = m_talonUnderflow = false;

	m_ctx.setNextMessage(m_initialNextMessage);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
