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

#include <numeric>

#include "abstractplayer.h"

#include "iruleset.h"
#include "cardtools.h"
#include "random_gen.h"
#include "icardcountobserver.h"

namespace {

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

AbstractPlayer::AbstractPlayer(const std::string &name) : IPlayer(), m_name(name), m_cards(),
	m_cardsTaken(false), m_ruleset(0L), m_playedOutCards(), m_playerHasFewCards(false),
	m_nineIsEight(false), m_leftCount(0), m_rightCount(0), m_dirChgEnabled(false), m_playerCount(0),
	m_engineCfg(0L), m_cardCountObserver(0L) {
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

void AbstractPlayer::setEngineConfig(const NetMauMau::EngineConfig *engineCfg) {
	m_engineCfg = engineCfg;
}

void AbstractPlayer::cardPlayed(NetMauMau::Common::ICard *playedCard) {
	m_playedOutCards.push_back(playedCard->description());
}

void AbstractPlayer::informAIStat(const IPlayer *, std::size_t count) {
	m_playerHasFewCards = count < 3;
}

void AbstractPlayer::setNeighbourCardCount(std::size_t playerCount, std::size_t leftCount,
		std::size_t rightCount) {
	m_leftCount = leftCount;
	m_rightCount = rightCount;
	m_playerCount = playerCount;
}

void AbstractPlayer::setDirChangeEnabled(bool dirChangeEnabled) {
	m_dirChgEnabled = dirChangeEnabled;
}

void AbstractPlayer::setNineIsEight(bool b) {
	m_nineIsEight = b;
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
	posCards.reserve(m_cards.size());

	return std::for_each(m_cards.begin(), m_cards.end(), _pushIfPossible(posCards, uncoveredCard,
						 m_ruleset, suit)).cards;
}

bool AbstractPlayer::isAceRoundAllowed() const {
	return std::count_if(getPlayerCards().begin(), getPlayerCards().end(),
						 std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
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

void AbstractPlayer::talonShuffled() {
	m_playedOutCards.clear();
}

void AbstractPlayer::pushCard(const NetMauMau::Common::ICardPtr &card) {
	m_cards.push_back(card);
}

const NetMauMau::EngineConfig *AbstractPlayer::getEngineConfig() const {
	return m_engineCfg;
}

const NetMauMau::RuleSet::IRuleSet *AbstractPlayer::getRuleSet() const {
	return m_ruleset;
}

const std::vector< std::string > &AbstractPlayer::getPlayedOutCards() const {
	return m_playedOutCards;
}

std::size_t AbstractPlayer::getLeftCount() const {
	return m_leftCount;
}

std::size_t AbstractPlayer::getRightCount() const {
	return m_rightCount;
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

bool AbstractPlayer::nineIsEight() const {
	return m_nineIsEight;
}

bool AbstractPlayer::isDirChgEnabled() const {
	return m_dirChgEnabled;
}

void AbstractPlayer::shuffleCards() {
	std::random_shuffle(m_cards.begin(), m_cards.end(),
						NetMauMau::Common::genRandom<CARDS::difference_type>);
}

void AbstractPlayer::reset() throw() {

	/*m_playerHasFewCards =*/ m_cardsTaken = m_nineIsEight = false;
	m_leftCount = m_rightCount = m_playerCount = 0;
	m_dirChgEnabled = false;
	m_cards.clear();
// 	m_playedOutCards.clear();

	notifyCardCountChange();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
