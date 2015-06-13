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

#include "abstractplayer.h"

#include <numeric>                      // for accumulate
#include <stdbool.h>

#include "cardtools.h"                  // for cardEqual, isRank
#include "icardcountobserver.h"         // for ICardCountObserver
#include "iruleset.h"                   // for IRuleSet
#include "random_gen.h"                 // for genRandom

namespace {

const NetMauMau::IPlayedOutCards::CARDS PLAYEDOUTCARDS;

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct pointSum : std::binary_function<std::size_t, NetMauMau::Common::ICardPtr, std::size_t> {
	inline result_type operator()(first_argument_type i, const second_argument_type &c) const {
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

	inline result_type operator()(const argument_type &card) const {

		if(suit && card != *suit) return;

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
	const NetMauMau::Common::ICard::RANK aceRoundRank;
	const NetMauMau::Common::ICard::RANK ucRank;
	const bool isAceRound;
};
#pragma GCC diagnostic pop

}

using namespace NetMauMau::Player;

AbstractPlayer::AbstractPlayer(const std::string &name, const NetMauMau::IPlayedOutCards *poc)
	: IPlayer(), m_lastPlayedSuit(NetMauMau::Common::ICard::SUIT_ILLEGAL),
	  m_lastPlayedRank(NetMauMau::Common::ICard::RANK_ILLEGAL), m_name(name), m_cards(),
	  m_cardsTaken(false), m_ruleset(0L), m_playerHasFewCards(false), m_nineIsSuspend(false),
	  m_neighbourCount(), m_dirChgEnabled(false), m_playerCount(0), m_engineCtx(0L),
	  m_cardCountObserver(0L), m_poc(poc), m_avoidSuit(NetMauMau::Common::ICard::SUIT_ILLEGAL),
	  m_avoidRank(NetMauMau::Common::ICard::RANK_ILLEGAL), m_neighbourRankSuit() {
	m_cards.reserve(32);
}

AbstractPlayer::~AbstractPlayer() {}

std::string AbstractPlayer::getName() const {
	return m_name;
}

void AbstractPlayer::setRuleSet(const NetMauMau::RuleSet::IRuleSet *ruleset) {
	m_ruleset = ruleset;
}

void AbstractPlayer::setCardCountObserver(const NetMauMau::ICardCountObserver *cco) {
	m_cardCountObserver = cco;
}

void AbstractPlayer::setEngineContext(const NetMauMau::EngineContext *engineCtx) {
	m_engineCtx = engineCtx;
}

void AbstractPlayer::informAIStat(const IPlayer *, std::size_t count, Common::ICard::SUIT lpSuit,
								  NetMauMau::Common::ICard::RANK lpRank) {

	m_playerHasFewCards = count < 3u;
	m_avoidSuit = lpSuit;
	m_avoidRank = lpRank;
}

NetMauMau::Common::ICard::SUIT AbstractPlayer::getLastPlayedSuit() const {
	return m_lastPlayedSuit;
}

NetMauMau::Common::ICard::RANK AbstractPlayer::getLastPlayedRank() const {
	return m_lastPlayedRank;
}

void AbstractPlayer::setNeighbourCardStats(std::size_t playerCount,
		const std::size_t *const neighbourCount, const NEIGHBOURRANKSUIT &nrs) {
	m_neighbourCount[LEFT] = neighbourCount[LEFT];
	m_neighbourCount[RIGHT] = neighbourCount[RIGHT];
	m_neighbourRankSuit = nrs;
	m_playerCount = playerCount;
}

const IPlayer::NEIGHBOURRANKSUIT &AbstractPlayer::getNeighbourRankSuit() const {
	return m_neighbourRankSuit;
}

void AbstractPlayer::setDirChangeEnabled(bool dirChangeEnabled) {
	m_dirChgEnabled = dirChangeEnabled;
}

void AbstractPlayer::setNineIsSuspend(bool b) {
	m_nineIsSuspend = b;
}

std::size_t AbstractPlayer::getPoints() const {
	return static_cast<std::size_t>(std::accumulate(m_cards.begin(), m_cards.end(), 0, pointSum()));
}

const IPlayer::CARDS &AbstractPlayer::getPlayerCards() const {
	return m_cards;
}

IPlayer::CARDS AbstractPlayer::getPossibleCards(const NetMauMau::Common::ICardPtr &uncoveredCard,
		const NetMauMau::Common::ICard::SUIT *suit) const {

	CARDS posCards;

	if(m_cards.size() <= posCards.max_size()) posCards.reserve(m_cards.size());

	return std::for_each(m_cards.begin(), m_cards.end(), _pushIfPossible(posCards, uncoveredCard,
						 m_ruleset, suit)).cards;
}

bool AbstractPlayer::isAceRoundAllowed() const {
	return std::count_if(getPlayerCards().begin(), getPlayerCards().end(),
						 std::bind2nd(NetMauMau::Common::equalTo < CARDS::value_type,
									  NetMauMau::Common::ICard::RANK > (),
									  m_ruleset->getAceRoundRank())) > 1;
}

IPlayer::REASON AbstractPlayer::getNoCardReason(const NetMauMau::Common::ICardPtr &,
		const NetMauMau::Common::ICard::SUIT *) const {
	return m_cards.empty() ? MAUMAU : NOMATCH;
}

void AbstractPlayer::receiveCardSet(const CARDS &cards) {

	m_cards.insert(m_cards.end(), cards.begin(), cards.end());

	if(!cards.empty()) notifyCardCountChange();

	shuffleCards();
}

void AbstractPlayer::notifyCardCountChange() const throw() {
	if(m_cardCountObserver) m_cardCountObserver->cardCountChanged(this);
}

bool AbstractPlayer::cardAccepted(const NetMauMau::Common::ICard *playedCard) {

	const CARDS::iterator &i(std::find_if(m_cards.begin(), m_cards.end(),
										  std::bind2nd(std::equal_to<CARDS::value_type>(),
												  playedCard)));
	m_lastPlayedSuit = playedCard->getSuit();
	m_lastPlayedRank = playedCard->getRank();

	if(i != m_cards.end()) {
		m_cards.erase(i);
		notifyCardCountChange();
	}

	if(!m_cards.empty()) shuffleCards();

	m_cardsTaken = false;

	return m_cards.empty();
}

void AbstractPlayer::talonShuffled() {}

void AbstractPlayer::pushCard(const NetMauMau::Common::ICardPtr &card) {
	m_cards.push_back(card);
}

const NetMauMau::EngineContext *AbstractPlayer::getEngineContext() const {
	return m_engineCtx;
}

const NetMauMau::RuleSet::IRuleSet *AbstractPlayer::getRuleSet() const {
	return m_ruleset;
}

const NetMauMau::IPlayedOutCards::CARDS &AbstractPlayer::getPlayedOutCards() const {
	return m_poc ? m_poc->getCards() : PLAYEDOUTCARDS;
}

std::size_t AbstractPlayer::getLeftCount() const {
	return m_neighbourCount[LEFT];
}

std::size_t AbstractPlayer::getRightCount() const {
	return m_neighbourCount[RIGHT];
}

std::size_t AbstractPlayer::getPlayerCount() const {
	return m_playerCount;
}

bool AbstractPlayer::hasTakenCards() const {
	return m_cardsTaken;
}

void AbstractPlayer::setCardsTaken(bool b) {
	m_cardsTaken = b;
}

bool AbstractPlayer::hasPlayerFewCards() const {
	return m_playerHasFewCards;
}

bool AbstractPlayer::nineIsSuspend() const {
	return m_nineIsSuspend;
}

bool AbstractPlayer::isDirChgEnabled() const {
	return m_dirChgEnabled;
}

void AbstractPlayer::shuffleCards() {
	std::random_shuffle(m_cards.begin(), m_cards.end(),
						NetMauMau::Common::genRandom<CARDS::difference_type>);
}

void AbstractPlayer::reset() throw() {

	m_playerHasFewCards = m_cardsTaken = m_nineIsSuspend = false;
	m_neighbourCount[LEFT] = m_neighbourCount[RIGHT] = m_playerCount = 0u;
	m_avoidSuit = m_lastPlayedSuit = NetMauMau::Common::ICard::SUIT_ILLEGAL;
	m_avoidRank = m_lastPlayedRank = NetMauMau::Common::ICard::RANK_ILLEGAL;
	m_neighbourRankSuit = NEIGHBOURRANKSUIT();
	m_dirChgEnabled = false;
	m_cards.clear();

	notifyCardCountChange();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
