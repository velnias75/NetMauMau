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

#include <cassert>
#include <cstring>
#include <numeric>

#include "stdplayer.h"

#include "iruleset.h"
#include "cardtools.h"
#include "random_gen.h"
#include "nextaction.h"
#include "engineconfig.h"
#include "decisionchain.h"
#include "stdcardfactory.h"
#include "socketexception.h"
#include "powerplayaction.h"
#include "jackonlycondition.h"
#include "havejackcondition.h"
#include "powersuitcondition.h"
#include "icardcountobserver.h"

#include "randomjackcondition.h"

namespace {

NetMauMau::AI::IConditionPtr HAVEJACKCOND(new NetMauMau::AI::HaveJackCondition());

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct pointSum : std::binary_function<std::size_t, NetMauMau::Common::ICardPtr, std::size_t> {
	std::size_t operator()(std::size_t i, const NetMauMau::Common::ICardPtr &c) const {
		return i + c->getPoints();
	}
};

struct _pushIfPossible : std::unary_function<NetMauMau::Common::ICardPtr, void> {

	inline explicit _pushIfPossible(NetMauMau::Player::IPlayer::CARDS &c,
									const NetMauMau::Common::ICardPtr &uc,
									const NetMauMau::RuleSet::IRuleSet *const rs,
									const NetMauMau::Common::ICard::SUIT *const s) : cards(c),
		uncoveredCard(uc), ruleset(rs), suit(s), aceRoundRank(rs->getAceRoundRank()),
		ucRank(uc->getRank()), isAceRound(rs->isAceRound()) {}

	inline void operator()(const NetMauMau::Common::ICardPtr &card) const {

		if(suit && card->getSuit() != *suit) return;

		const NetMauMau::Common::ICard::RANK rank = card->getRank();

		const bool accepted = isAceRound ? rank == aceRoundRank :
							  ruleset->checkCard(NetMauMau::Common::ICardPtr(card), uncoveredCard);

		const bool jack = (rank == NetMauMau::Common::ICard::JACK &&
						   ucRank != NetMauMau::Common::ICard::JACK) && !isAceRound;

		if(accepted || jack) cards.push_back(card);
	}

	NetMauMau::Player::IPlayer::CARDS &cards;

private:
	const NetMauMau::Common::ICardPtr uncoveredCard;
	const NetMauMau::RuleSet::IRuleSet *const ruleset;
	const NetMauMau::Common::ICard::SUIT *const suit;
	NetMauMau::Common::ICard::RANK aceRoundRank;
	NetMauMau::Common::ICard::RANK ucRank;
	bool isAceRound;
};
#pragma GCC diagnostic pop

}

using namespace NetMauMau::Player;

StdPlayer::StdPlayer(const std::string &name) : IPlayer(), IAIState(), m_name(name), m_cards(),
	m_cardsTaken(false), m_ruleset(0L), m_playerHasFewCards(false),
	m_powerSuit(NetMauMau::Common::ICard::SUIT_ILLEGAL), m_powerPlay(false), m_tryAceRound(false),
	m_nineIsEight(false), m_leftCount(0), m_rightCount(0), m_dirChgEnabled(false),
	m_playerCount(0), m_engineCfg(0L), m_cardCountObserver(0L),
	m_decisionTree(NetMauMau::Common::SmartPtr < NetMauMau::AI::DecisionChain
				   <NetMauMau::AI::JackOnlyCondition> > (new NetMauMau::AI::DecisionChain
						   <NetMauMau::AI::JackOnlyCondition>(*this))),
	m_jackDecisionTree(NetMauMau::Common::SmartPtr < NetMauMau::AI::DecisionChain
					   <NetMauMau::AI::PowerSuitCondition> > (new NetMauMau::AI::DecisionChain
							   <NetMauMau::AI::PowerSuitCondition>(*this,
									   NetMauMau::AI::IActionPtr
									   (new NetMauMau::AI::NextAction(HAVEJACKCOND)),
									   NetMauMau::AI::IActionPtr
									   (new NetMauMau::AI::PowerPlayAction(true))))),
	m_card(), m_uncoveredCard(), m_playedCard(), m_noJack(false), m_jackSuit(0L) {
	m_cards.reserve(32);
}

StdPlayer::~StdPlayer() {}

void StdPlayer::reset() throw() {

	m_powerSuit = NetMauMau::Common::ICard::SUIT_ILLEGAL;
	m_cardsTaken = false;
	m_powerPlay = false;
	m_nineIsEight = false;
	m_leftCount = m_rightCount = m_playerCount = 0;
	m_dirChgEnabled = false;
	m_cards.clear();

	m_card = m_uncoveredCard = m_playedCard = NetMauMau::Common::ICardPtr();
	m_noJack = false;
	m_jackSuit = 0L;

	notifyCardCountChange();
}

std::string StdPlayer::getName() const {
	return m_name;
}

int StdPlayer::getSerial() const {
	return INVALID_SOCKET;
}

bool StdPlayer::isAIPlayer() const {
	return true;
}

bool StdPlayer::isAlive() const {
	return true;
}

void StdPlayer::setRuleSet(const NetMauMau::RuleSet::IRuleSet *ruleset) {
	m_ruleset = ruleset;
}

void StdPlayer::setEngineConfig(const NetMauMau::EngineConfig *engineCfg) {
	m_engineCfg = engineCfg;
}

void StdPlayer::setCardCountObserver(const NetMauMau::ICardCountObserver *cco) {
	m_cardCountObserver = cco;
}

void StdPlayer::notifyCardCountChange() {
	if(m_cardCountObserver) m_cardCountObserver->cardCountChanged(this);
}

void StdPlayer::receiveCard(const NetMauMau::Common::ICardPtr &card) {

	if(card) {
		m_cards.push_back(card);
		notifyCardCountChange();
	}
}

void StdPlayer::receiveCardSet(const CARDS &cards) {

	m_cards.insert(m_cards.end(), cards.begin(), cards.end());

	if(!cards.empty()) notifyCardCountChange();

	shuffleCards();
}

void StdPlayer::shuffleCards() {
	std::random_shuffle(m_cards.begin(), m_cards.end(),
						NetMauMau::Common::genRandom<CARDS::difference_type>);
}

NetMauMau::Common::ICardPtr StdPlayer::requestCard(const NetMauMau::Common::ICardPtr &uc,
		const NetMauMau::Common::ICard::SUIT *js, std::size_t) const {

	m_card = NetMauMau::Common::ICardPtr();
	m_uncoveredCard = uc;
	m_jackSuit = js ? const_cast<NetMauMau::Common::ICard::SUIT *>(js) : 0L;

	NetMauMau::Common::ICardPtr bestCard(m_decisionTree->getCard());

	m_playedCard = m_card = NetMauMau::Common::ICardPtr();

	if(bestCard && bestCard->getRank() == NetMauMau::Common::ICard::JACK &&
			uc->getRank() == NetMauMau::Common::ICard::JACK) {
		bestCard = m_decisionTree->getCard(true);
	}

	return m_ruleset ? (m_ruleset->checkCard(uc, NetMauMau::Common::ICardPtr(bestCard)) ?
						NetMauMau::Common::ICardPtr(bestCard) : NetMauMau::Common::ICardPtr()) :
			   NetMauMau::Common::ICardPtr(bestCard);
}

NetMauMau::Common::ICard::SUIT
StdPlayer::getJackChoice(const NetMauMau::Common::ICardPtr &uncoveredCard,
						 const NetMauMau::Common::ICardPtr &playedCard) const {

	const NetMauMau::Common::ICardPtr oc(m_card);
	const NetMauMau::Common::ICardPtr uc(m_uncoveredCard);

	m_card = m_playedCard = playedCard;
	m_uncoveredCard = uncoveredCard;

	const NetMauMau::Common::ICardPtr &rc(m_jackDecisionTree->getCard(true));
	assert(rc);
	const NetMauMau::Common::ICard::SUIT s = rc->getSuit();

	m_card = oc;
	m_uncoveredCard = uc;
	m_playedCard = NetMauMau::Common::ICardPtr();

	return s;
}

bool StdPlayer::getAceRoundChoice() const {

	if(m_tryAceRound && isAceRoundAllowed()) return m_tryAceRound;

	return (m_tryAceRound = false);
}

bool StdPlayer::isAceRoundAllowed() const {
	return std::count_if(getPlayerCards().begin(), getPlayerCards().end(),
						 std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
									  m_ruleset->getAceRoundRank())) > 1;
}

IPlayer::REASON StdPlayer::getNoCardReason(const NetMauMau::Common::ICardPtr &,
		const NetMauMau::Common::ICard::SUIT *) const {
	return m_cards.empty() ? MAUMAU : NOMATCH;
}

bool StdPlayer::cardAccepted(const NetMauMau::Common::ICard *playedCard) {

	const CARDS::iterator &i(std::find_if(m_cards.begin(), m_cards.end(),
										  std::bind2nd(std::ptr_fun(NetMauMau::Common::cardEqual),
												  playedCard)));

	if(i != m_cards.end()) {
		m_cards.erase(i);
		notifyCardCountChange();
	}

	if(!m_cards.empty()) shuffleCards();

	m_cardsTaken = false;

	return m_cards.empty();
}

void StdPlayer::cardPlayed(NetMauMau::Common::ICard *playedCard) {
	m_playedOutCards.push_back(playedCard->description());
}

void StdPlayer::talonShuffled() {
	m_playedOutCards.clear();
}

void StdPlayer::setNineIsEight(bool b) {
	m_nineIsEight = b;
}

std::size_t StdPlayer::getCardCount() const {
	assert(m_cards.size() < (32 * getTalonFactor()));
	return m_cards.size();
}

std::size_t StdPlayer::getPoints() const {
	return static_cast<std::size_t>(std::accumulate(m_cards.begin(), m_cards.end(), 0, pointSum()));
}

IPlayer::CARDS StdPlayer::getPossibleCards(const NetMauMau::Common::ICardPtr &uncoveredCard,
		const NetMauMau::Common::ICard::SUIT *suit) const {

	CARDS posCards;
	posCards.reserve(m_cards.size());

	return std::for_each(m_cards.begin(), m_cards.end(), _pushIfPossible(posCards, uncoveredCard,
						 getRuleSet(), suit)).cards;
}

void StdPlayer::informAIStat(const IPlayer *, std::size_t count) {
	m_playerHasFewCards = count < 3;
}

void StdPlayer::setNeighbourCardCount(std::size_t playerCount, std::size_t leftCount,
									  std::size_t rightCount) {
	m_leftCount = leftCount;
	m_rightCount = rightCount;
	m_playerCount = playerCount;
}

void StdPlayer::setDirChangeEnabled(bool dirChangeEnabled) {
	m_dirChgEnabled = dirChangeEnabled;
}

std::size_t StdPlayer::getTalonFactor() const {
	return m_engineCfg ? m_engineCfg->getTalonFactor() : 1;
}

const NetMauMau::RuleSet::IRuleSet *StdPlayer::getRuleSet() const {
	return m_ruleset;
}

const IPlayer::CARDS &StdPlayer::getPlayerCards() const {
	return m_cards;
}

const std::vector< std::string > &StdPlayer::getPlayedOutCards() const {
	return m_playedOutCards;
}

std::size_t StdPlayer::getPlayerCount() const {
	return m_playerCount;
}

std::size_t StdPlayer::getLeftCount() const {
	return m_leftCount;
}

std::size_t StdPlayer::getRightCount() const {
	return m_rightCount;
}

bool StdPlayer::hasTakenCards() const {
	return m_cardsTaken;
}

void StdPlayer::setCardsTaken(bool b) {
	m_cardsTaken = b;
}

NetMauMau::Common::ICardPtr StdPlayer::getUncoveredCard() const {
	return m_uncoveredCard;
}

NetMauMau::Common::ICardPtr StdPlayer::getPlayedCard() const {
	return m_playedCard;
}

NetMauMau::Common::ICardPtr StdPlayer::getCard() const {
	return m_card;
}

void StdPlayer::setCard(const NetMauMau::Common::ICardPtr &card) {
	m_card = card;
}

bool StdPlayer::isNoJack() const {
	return m_noJack;
}

void StdPlayer::setNoJack(bool b) {
	m_noJack = b;
}

bool StdPlayer::hasPlayerFewCards() const {
	return m_playerHasFewCards;
}

NetMauMau::Common::ICard::SUIT StdPlayer::getPowerSuit() const {
	return m_powerSuit;
}

void StdPlayer::setPowerSuit(NetMauMau::Common::ICard::SUIT suit) {
	m_powerSuit = suit;
}

NetMauMau::Common::ICard::SUIT *StdPlayer::getJackSuit() const {
	return m_jackSuit;
}

void StdPlayer::clearJackSuit() {
	m_jackSuit = 0L;
}

bool StdPlayer::nineIsEight() const {
	return m_nineIsEight;
}

bool StdPlayer::tryAceRound() const {
	return m_tryAceRound;
}

void StdPlayer::setTryAceRound(bool b) {
	m_tryAceRound = b;
}

bool StdPlayer::isDirChgEnabled() const {
	return m_dirChgEnabled;
}

bool StdPlayer::isPowerPlay() const {
	return m_powerPlay;
}

void StdPlayer::setPowerPlay(bool b) {
	m_powerPlay = b;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
