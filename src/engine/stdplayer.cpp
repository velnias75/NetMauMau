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
#include "engineconfig.h"
#include "stdcardfactory.h"
#include "socketexception.h"
#include "icardcountobserver.h"

#include "decisiontree.h"

namespace {

const NetMauMau::Common::ICard::SUIT SUIT[4] = {
	NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::CLUBS
};

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct cardGreater : std::binary_function < NetMauMau::Common::ICardPtr,
		NetMauMau::Common::ICardPtr, bool > {
	bool operator()(const NetMauMau::Common::ICardPtr &x,
					const NetMauMau::Common::ICardPtr &y) const {
		return !(x->getPoints() < y->getPoints());
	}
};

struct playedOutRank : std::binary_function<std::string, NetMauMau::Common::ICard::RANK, bool> {
	bool operator()(const std::string &desc, NetMauMau::Common::ICard::RANK rank) const {

		NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::RANK_ILLEGAL;
		NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::SUIT_ILLEGAL;

		if(NetMauMau::Common::parseCardDesc(desc, &s, &r)) {
			return r == rank;
		}

		return false;
	}
};

struct pointSum : std::binary_function<std::size_t, NetMauMau::Common::ICardPtr, std::size_t> {
	std::size_t operator()(std::size_t i, const NetMauMau::Common::ICardPtr &c) const {
		return i + c->getPoints();
	}
};

struct _isSpecialRank : std::binary_function < NetMauMau::Common::ICardPtr,
		NetMauMau::Common::ICard::RANK, bool > {

	explicit _isSpecialRank(bool nineIsEight) : m_nineIsEight(nineIsEight) {}

	bool operator()(const NetMauMau::Common::ICardPtr &c, NetMauMau::Common::ICard::RANK r) const {
		return m_nineIsEight && r == NetMauMau::Common::ICard::EIGHT ?
			   (c->getRank() == NetMauMau::Common::ICard::EIGHT ||
				c->getRank() == NetMauMau::Common::ICard::NINE) : c->getRank() == r;
	}

private:
	bool m_nineIsEight;
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

bool StdPlayer::m_jackPlayed = false;

bool StdPlayer::_hasRankPath::operator()(const NetMauMau::Common::ICardPtr &c) const {

	bool hrp = false;

	if(c->getRank() != rank) {
		for(CARDS::const_iterator i(mCards.begin()); i != mCards.end(); ++i) {
			if((hrp = hasRankPath(c, (*i)->getSuit(), rank, mCards, nineIsEight))) break;
		}
	}

	return hrp;
}

StdPlayer::StdPlayer(const std::string &name) : IPlayer(), IAIState(), m_name(name), m_cards(),
	m_cardsTaken(false), m_ruleset(0L), m_playerHasFewCards(false),
	m_powerSuit(NetMauMau::Common::ICard::SUIT_ILLEGAL), m_powerPlay(false), m_tryAceRound(false),
	m_nineIsEight(false), m_leftCount(0), m_rightCount(0), m_dirChgEnabled(false),
	m_playerCount(0), m_engineCfg(0L), m_cardCountObserver(0L),
	m_decisisionTree(NetMauMau::Common::SmartPtr<NetMauMau::AIDT::DecisionTree>
					 (new NetMauMau::AIDT::DecisionTree(*this))), m_card(), m_uncoveredCard(),
	m_noJack(false), m_jackSuit(0L) {
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

void StdPlayer::countSuits(SUITCOUNT *suitCount, const CARDS &myCards) const {

	std::memset(suitCount, 0, sizeof(SUITCOUNT) * 4);

	const bool noCards = myCards.empty();

	for(std::size_t i = 0; i < 4; ++i) {
		const SUITCOUNT sc = { SUIT[i], noCards ? 0 : std::count_if(myCards.begin(), myCards.end(),
							   std::bind2nd(std::ptr_fun(NetMauMau::Common::isSuit), SUIT[i]))
							 };
		suitCount[i] = sc;
	}

	if(!noCards) std::sort(suitCount, suitCount + 4);
}

NetMauMau::Common::ICardPtr
StdPlayer::hasRankPath(const NetMauMau::Common::ICardPtr &uc, NetMauMau::Common::ICard::SUIT s,
					   NetMauMau::Common::ICard::RANK r, const CARDS &cards, bool nineIsEight) {

	CARDS mCards(cards);

	if(mCards.size() > 1) {

		const CARDS::iterator &e(std::partition(mCards.begin(), mCards.end(),
												std::bind2nd(_isSpecialRank(nineIsEight), r)));

		if(std::distance(mCards.begin(), e)) {

			CARDS::value_type f_src(NetMauMau::Common::findSuit(uc->getSuit(), mCards.begin(), e));

			if(f_src) {

				CARDS::value_type f_dest(NetMauMau::Common::findSuit(s, mCards.begin(), e));

				if(f_dest) return f_src;
			}
		}
	}

	return NetMauMau::Common::ICardPtr();
}

NetMauMau::Common::ICardPtr StdPlayer::findBestCard(const NetMauMau::Common::ICardPtr &uc,
		const NetMauMau::Common::ICard::SUIT *js, bool noJack) const {

	CARDS myCards(m_cards);

	if(noJack) {
		myCards.erase(std::remove_if(myCards.begin(), myCards.end(),
									 std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
											 NetMauMau::Common::ICard::JACK)), myCards.end());
	}

	m_uncoveredCard = uc;
	m_jackSuit = const_cast<NetMauMau::Common::ICard::SUIT *>(js);

	return m_decisisionTree->getCard();

#if 0

	if(m_playedOutCards.size() > (4 * getTalonFactor()) &&
			uc->getRank() == NetMauMau::Common::ICard::SEVEN) {

		const CARDS::iterator &e(js ? std::partition(myCards.begin(), myCards.end(),
								 std::bind2nd(std::ptr_fun(NetMauMau::Common::isSuit), *js)) :
								 myCards.end());

		const CARDS::value_type f =
			NetMauMau::Common::findRank(NetMauMau::Common::ICard::SEVEN, myCards.begin(), e);

		if(f) {
			bestCard = f;
		} else if(!m_cardsTaken) {
			bestCard = NetMauMau::Common::getIllegalCard();
			m_cardsTaken = true;
		}
	}

	if(!bestCard && m_playerCount > 2 && (m_rightCount < getCardCount() ||
										  m_rightCount < m_leftCount) &&
			std::count_if(m_playedOutCards.begin(), m_playedOutCards.end(),
						  std::bind2nd(playedOutRank(), NetMauMau::Common::ICard::SEVEN))) {

		const CARDS::value_type nine = m_dirChgEnabled ?
									   NetMauMau::Common::findRank(NetMauMau::Common::ICard::NINE,
											   myCards.begin(), myCards.end()) :
									   NetMauMau::Common::ICardPtr();

		const CARDS::value_type seven = NetMauMau::Common::findRank(NetMauMau::Common::ICard::SEVEN,
										myCards.begin(), myCards.end());

		bestCard = nine ? nine : seven ? seven :
				   NetMauMau::Common::findRank(NetMauMau::Common::ICard::EIGHT, myCards.begin(),
											   myCards.end());
	}

	if(!bestCard && !noJack && m_playerHasFewCards && std::count_if(myCards.begin(), myCards.end(),
			std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
						 NetMauMau::Common::ICard::JACK))) {

		SUITCOUNT suitCount[4];
		countSuits(suitCount, myCards);

		if(std::count_if(myCards.begin(), myCards.end(),
						 std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
									  NetMauMau::Common::ICard::SEVEN))) {

			for(int p = 0; p < 4; ++p) {

				std::partition(myCards.begin(), myCards.end(),
							   std::bind2nd(std::ptr_fun(NetMauMau::Common::isSuit),
											suitCount[p].suit));

				const CARDS::value_type f =
					NetMauMau::Common::findRank(NetMauMau::Common::ICard::SEVEN,
												myCards.begin(), myCards.end());

				if(f) {
					m_powerSuit = f->getSuit();
					break;
				}
			}

		} else {
			m_powerSuit = getMaxPlayedOffSuit();
		}

	} else {
		m_powerSuit = NetMauMau::Common::ICard::SUIT_ILLEGAL;
	}

	if(m_powerSuit == NetMauMau::Common::ICard::SUIT_ILLEGAL) {

		if(!bestCard) {

			const CARDS::iterator &e(std::partition(
										 myCards.begin(), myCards.end(),
										 std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
												 NetMauMau::Common::ICard::SEVEN)));

			const CARDS::value_type f = NetMauMau::Common::findSuit(js ? *js : uc->getSuit(),
										myCards.begin(), e);

			const CARDS::difference_type mySevens = std::distance(myCards.begin(), e);
			const CARDS::difference_type poSevens = std::count_if(m_playedOutCards.begin(),
													m_playedOutCards.end(),
													std::bind2nd(playedOutRank(),
															NetMauMau::Common::ICard::SEVEN));

			if(f && (m_powerPlay || mySevens + poSevens > static_cast<CARDS::difference_type>(2 *
					 getTalonFactor()))) bestCard = f;

			m_powerPlay = false;
		}

		if(!bestCard) {

			SUITCOUNT suitCount[4];
			countSuits(suitCount, myCards);

			for(std::size_t i = 0; i < 4; ++i) {

				const CARDS::iterator
				&e(std::partition(myCards.begin(), myCards.end(),
								  std::bind2nd(std::ptr_fun(NetMauMau::Common::isSuit),
											   suitCount[i].suit)));

				if(js) {

					if(suitCount[i].count && (suitCount[i].suit == *js)) {
						std::partition(myCards.begin(), e,
									   std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
													NetMauMau::Common::ICard::SEVEN));
						bestCard = myCards.front();
						break;
					}

				} else if(!((m_ruleset->isAceRoundPossible() && (bestCard = hasRankPath(uc,
							 suitCount[i].suit, m_ruleset->getAceRoundRank(), myCards,
							 m_nineIsEight))) || (bestCard = hasRankPath(uc, suitCount[i].suit,
												  NetMauMau::Common::ICard::EIGHT, myCards,
												  m_nineIsEight)))) {

					std::sort(myCards.begin(), e, cardGreater());

					std::stable_partition(myCards.begin(), e,
										  std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
													   NetMauMau::Common::ICard::SEVEN));

					const CARDS::value_type f = NetMauMau::Common::findRank(uc->getRank(),
												myCards.begin(), e);

					if(f && (f->getRank() != NetMauMau::Common::ICard::SEVEN ||
							 m_playedOutCards.size() > (4 * getTalonFactor()))) {
						bestCard = f;
						break;
					}
				}
			}

			if(!bestCard) {

				const CARDS::iterator
				&e(std::partition(myCards.begin(), myCards.end(),
								  std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
											   NetMauMau::Common::ICard::SEVEN)));

				if(!noJack) std::partition(e, myCards.end(),
											   std::not1(std::bind2nd
														 (std::ptr_fun(NetMauMau::Common::isRank),
														  NetMauMau::Common::ICard::JACK)));

				const CARDS::value_type f = NetMauMau::Common::findSuit(js ? *js : uc->getSuit(),
											myCards.begin(), myCards.end());

				if(f && f->getRank() != NetMauMau::Common::ICard::JACK) bestCard = f;
			}
		}
	}

	if(m_tryAceRound || (!m_ruleset->isAceRound() && m_ruleset->isAceRoundPossible())) {

		m_tryAceRound = std::count_if(myCards.begin(), myCards.end(),
									  std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
											  m_ruleset->getAceRoundRank()))
						> (m_tryAceRound ? 0 : 1);

		if(m_tryAceRound) {
			std::partition(myCards.begin(), myCards.end(),
						   std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
										m_ruleset->getAceRoundRank()));
			return *myCards.begin();
		}
	}

	if(!noJack && (!bestCard || m_powerSuit != NetMauMau::Common::ICard::SUIT_ILLEGAL)) {

		const CARDS::size_type jackCnt =
			static_cast<CARDS::size_type>(std::count_if(myCards.begin(), myCards.end(),
										  std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
												  NetMauMau::Common::ICard::JACK)));

		if(jackCnt) {
			std::partition(myCards.begin(), myCards.end(),
						   std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
										NetMauMau::Common::ICard::JACK));

			bestCard = myCards[static_cast<std::size_t>
							   (NetMauMau::Common::genRandom<CARDS::difference_type>
								(static_cast<CARDS::difference_type>(jackCnt)))];
		}
	}

	return bestCard;
#endif
}

NetMauMau::Common::ICardPtr StdPlayer::requestCard(const NetMauMau::Common::ICardPtr &uc,
		const NetMauMau::Common::ICard::SUIT *js, std::size_t) const {

	m_uncoveredCard = uc;
	m_jackSuit = const_cast<NetMauMau::Common::ICard::SUIT *>(js);

// 	if(m_ruleset) {
// 		if(m_cards.size() == 1 &&
// 				!(uc->getRank() == NetMauMau::Common::ICard::JACK &&
// 				  (*m_cards.begin())->getRank() == NetMauMau::Common::ICard::JACK)) {
// 			return m_ruleset->checkCard(uc, NetMauMau::Common::ICardPtr(*m_cards.begin())) ?
// 				   NetMauMau::Common::ICardPtr(*m_cards.begin()) : NetMauMau::Common::ICardPtr();
// 		} else if(m_cards.size() == 1) {
// 			return NetMauMau::Common::ICardPtr();
// 		}
// 	}

	NetMauMau::Common::ICardPtr bestCard(m_decisisionTree->getCard());

// 	NetMauMau::Common::ICardPtr bestCard(findBestCard(uc, js, false));

	if(bestCard && bestCard->getRank() == NetMauMau::Common::ICard::JACK &&
			uc->getRank() == NetMauMau::Common::ICard::JACK) {
		bestCard = findBestCard(uc, js, true);
	}

	return m_ruleset ? (m_ruleset->checkCard(uc, NetMauMau::Common::ICardPtr(bestCard)) ?
						NetMauMau::Common::ICardPtr(bestCard) : NetMauMau::Common::ICardPtr()) :
			   NetMauMau::Common::ICardPtr(bestCard);
}

NetMauMau::Common::ICard::SUIT
StdPlayer::getJackChoice(const NetMauMau::Common::ICardPtr &uncoveredCard,
						 const NetMauMau::Common::ICardPtr &playedCard) const {

	if(m_powerSuit != NetMauMau::Common::ICard::SUIT_ILLEGAL) {

		const NetMauMau::Common::ICard::SUIT s = m_powerSuit;
		m_powerSuit = NetMauMau::Common::ICard::SUIT_ILLEGAL;
		m_powerPlay = true;

		assert(s != NetMauMau::Common::ICard::SUIT_ILLEGAL);

		return s;
	}

	if(m_cards.size() == 2 && NetMauMau::Common::findRank(NetMauMau::Common::ICard::JACK,
			m_cards.begin(), m_cards.end())) {

		const CARDS::const_iterator &f(std::find_if(m_cards.begin(), m_cards.end(), std::not1
									   (std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
											   NetMauMau::Common::ICard::JACK))));

		if(f != m_cards.end()) {
			assert((*f)->getSuit() != NetMauMau::Common::ICard::SUIT_ILLEGAL);
			return (*f)->getSuit();
		}
	}

	if(m_cards.size() < 8) {

		NetMauMau::Common::ICardPtr bc;

		if((bc = findBestCard(uncoveredCard, 0L, true)) &&
				bc->getSuit() != NetMauMau::Common::ICard::SUIT_ILLEGAL) {
			return bc->getSuit();
		}
	}

	NetMauMau::Common::ICard::SUIT s = findJackChoice();

	if(s == NetMauMau::Common::ICard::SUIT_ILLEGAL) {
		while(((s = SUIT[NetMauMau::Common::genRandom<std::ptrdiff_t>(4)]) ==
				uncoveredCard->getSuit() || s == playedCard->getSuit()));
	}

	assert(s != NetMauMau::Common::ICard::SUIT_ILLEGAL);

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

NetMauMau::Common::ICard::SUIT StdPlayer::getMaxPlayedOffSuit(CARDS::difference_type *count) const {

	SUITCOUNT poSuitCount[4];
	CARDS pcVec;

	pcVec.reserve(m_playedOutCards.size());

	for(std::vector<std::string>::const_iterator i(m_playedOutCards.begin());
			i != m_playedOutCards.end(); ++i) {

		NetMauMau::Common::ICard::SUIT s;
		NetMauMau::Common::ICard::RANK r;

		if(NetMauMau::Common::parseCardDesc(*i, &s, &r)) {
			pcVec.push_back(NetMauMau::Common::ICardPtr(NetMauMau::StdCardFactory().create(s, r)));
		}
	}

	countSuits(poSuitCount, pcVec);

	if(count) *count = poSuitCount[0].count;

	return poSuitCount[0].suit;
}

NetMauMau::Common::ICard::SUIT StdPlayer::findJackChoice() const {

	const CARDS::const_iterator &f(std::find_if(m_cards.begin(), m_cards.end(),
								   _hasRankPath(m_cards, NetMauMau::Common::ICard::EIGHT,
										   m_nineIsEight)));

	if(f != m_cards.end()) {
		return (*f)->getSuit();
	} else {
		CARDS::difference_type count = 0;
		const NetMauMau::Common::ICard::SUIT s = getMaxPlayedOffSuit(&count);
		return count ? s : NetMauMau::Common::ICard::SUIT_ILLEGAL;
	}
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

bool StdPlayer::nineIsEight() const {
	return m_nineIsEight;
}

bool StdPlayer::tryAceRound() const {
	return m_tryAceRound;
}

void StdPlayer::setTryAceRound(bool b) {
	m_tryAceRound = b;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
