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
#include "stdcardfactory.h"

namespace {

const NetMauMau::Common::ICard::SUIT SUIT[4] = {
	NetMauMau::Common::ICard::HEARTS,
	NetMauMau::Common::ICard::DIAMONDS,
	NetMauMau::Common::ICard::SPADES,
	NetMauMau::Common::ICard::CLUBS
};

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct cardGreater : std::binary_function < NetMauMau::Common::ICard *, NetMauMau::Common::ICard *,
		bool > {
	bool operator()(const NetMauMau::Common::ICard *x, NetMauMau::Common::ICard *y) const {
		return !(x->getPoints() < y->getPoints());
	}
};

struct playedOutSuit : std::binary_function<std::string, NetMauMau::Common::ICard::SUIT, bool> {
	bool operator()(const std::string &desc, NetMauMau::Common::ICard::SUIT suit) const {

		NetMauMau::Common::ICard::RANK r = NetMauMau::Common::ICard::RANK_ILLEGAL;
		NetMauMau::Common::ICard::SUIT s = NetMauMau::Common::ICard::SUIT_ILLEGAL;

		if(NetMauMau::Common::parseCardDesc(desc, &s, &r)) {
			return s == suit;
		}

		return false;
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

struct pointSum : std::binary_function<std::size_t, NetMauMau::Common::ICard *, std::size_t> {
	std::size_t operator()(std::size_t i, const NetMauMau::Common::ICard *c) const {
		return i + c->getPoints();
	}
};

struct _isSpecialRank : std::binary_function < NetMauMau::Common::ICard *,
		NetMauMau::Common::ICard::RANK, bool > {

	_isSpecialRank(bool nineIsEight) : m_nineIsEight(nineIsEight) {}

	bool operator()(const NetMauMau::Common::ICard *c, NetMauMau::Common::ICard::RANK r) const {
		return m_nineIsEight && r == NetMauMau::Common::ICard::EIGHT ?
			   (c->getRank() == NetMauMau::Common::ICard::EIGHT ||
				c->getRank() == NetMauMau::Common::ICard::NINE) : c->getRank() == r;
	}

private:
	bool m_nineIsEight;
};
#pragma GCC diagnostic pop

}

using namespace NetMauMau::Player;

bool StdPlayer::m_jackPlayed = false;

bool StdPlayer::_hasRankPath::operator()(const NetMauMau::Common::ICard *c) const {

	bool hrp = false;

	if(c->getRank() != rank) {
		for(CARDS::const_iterator i(mCards.begin()); i != mCards.end(); ++i) {
			if((hrp = hasRankPath(c, (*i)->getSuit(), rank, mCards, nineIsEight))) break;
		}
	}

	return hrp;
}

StdPlayer::StdPlayer(const std::string &name) : m_name(name), m_cards(), m_cardsTaken(false),
	m_ruleset(0), m_playerHasFewCards(false), m_powerSuit(NetMauMau::Common::ICard::SUIT_ILLEGAL),
	m_powerPlay(false), m_tryAceRound(false), m_nineIsEight(false), m_leftCount(0), m_rightCount(0),
	m_dirChgEnabled(false), m_playerCount(0) {
	m_cards.reserve(32);
}

StdPlayer::~StdPlayer() {
	for(CARDS::const_iterator i(m_cards.begin()); i != m_cards.end(); ++i) {
		delete *i;
	}
}

void StdPlayer::reset() throw() {
	m_powerSuit = NetMauMau::Common::ICard::SUIT_ILLEGAL;
	m_cardsTaken = false;
	m_powerPlay = false;
	m_nineIsEight = false;
	m_leftCount = m_rightCount = m_playerCount = 0;
	m_dirChgEnabled = false;
	m_cards.clear();
}

void StdPlayer::resetJackState() throw() {
	m_jackPlayed = false;
}

std::string StdPlayer::getName() const {
	return m_name;
}

int StdPlayer::getSerial() const {
	return -1;
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

const NetMauMau::RuleSet::IRuleSet *StdPlayer::getRuleSet() const {
	return m_ruleset;
}

void StdPlayer::receiveCard(NetMauMau::Common::ICard *card) {
	if(card) m_cards.push_back(card);
}

void StdPlayer::receiveCardSet(const CARDS &cards) {
	m_cards.insert(m_cards.end(), cards.begin(), cards.end());
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

NetMauMau::Common::ICard *
StdPlayer::hasRankPath(const NetMauMau::Common::ICard *uc, NetMauMau::Common::ICard::SUIT s,
					   NetMauMau::Common::ICard::RANK r, const CARDS &cards, bool nineIsEight) {

	CARDS mCards(cards);

	if(mCards.size() > 1) {

		const CARDS::iterator
		&e(std::partition(mCards.begin(), mCards.end(), std::bind2nd(_isSpecialRank(nineIsEight),
						  r)));

		if(std::distance(mCards.begin(), e)) {

			CARDS::value_type f_src(NetMauMau::Common::findSuit(uc->getSuit(), mCards.begin(), e));

			if(f_src) {
				CARDS::value_type f_dest(NetMauMau::Common::findSuit(s, mCards.begin(), e));

				if(f_dest) return f_src;
			}
		}
	}

	return 0L;
}

NetMauMau::Common::ICard *StdPlayer::findBestCard(const NetMauMau::Common::ICard *uc,
		const NetMauMau::Common::ICard::SUIT *js, bool noJack) const {

	CARDS myCards(m_cards);

	if(noJack) {
		myCards.erase(std::remove_if(myCards.begin(), myCards.end(),
									 std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
											 NetMauMau::Common::ICard::JACK)), myCards.end());
	}

	NetMauMau::Common::ICard *bestCard = 0L;

	if(uc->getRank() == NetMauMau::Common::ICard::SEVEN) {

		const CARDS::iterator &e(js ? std::partition(myCards.begin(), myCards.end(),
								 std::bind2nd(std::ptr_fun(NetMauMau::Common::isSuit), *js)) :
								 myCards.end());

		const CARDS::value_type f = NetMauMau::Common::findRank(NetMauMau::Common::ICard::SEVEN,
									myCards.begin(), e);

		if(f) {
			bestCard = f;
		} else if(!m_cardsTaken) {
			bestCard = NetMauMau::Common::getIllegalCard();
			m_cardsTaken = true;
		}
	}

	if(!bestCard && m_playerCount > 2 && m_rightCount < getCardCount() &&
			std::count_if(m_playedOutCards.begin(), m_playedOutCards.end(),
						  std::bind2nd(playedOutRank(), NetMauMau::Common::ICard::SEVEN))) {

		const CARDS::value_type nine = m_dirChgEnabled ?
									   NetMauMau::Common::findRank(NetMauMau::Common::ICard::NINE,
											   myCards.begin(), myCards.end()) : 0L;

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
					NetMauMau::Common::findRank(NetMauMau::Common::ICard::SEVEN, myCards.begin(),
												myCards.end());

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

			const CARDS::iterator
			&e(std::partition(myCards.begin(), myCards.end(),
							  std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
										   NetMauMau::Common::ICard::SEVEN)));

			const CARDS::value_type f = NetMauMau::Common::findSuit(js ? *js : uc->getSuit(),
										myCards.begin(), e);

			const CARDS::difference_type mySevens = std::distance(myCards.begin(), e);
			const CARDS::difference_type poSevens = std::count_if(m_playedOutCards.begin(),
													m_playedOutCards.end(),
													std::bind2nd(playedOutRank(),
															NetMauMau::Common::ICard::SEVEN));

			if(f && (m_powerPlay || mySevens + poSevens > 2)) bestCard = f;

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

					if(f) {
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

				if(!noJack) std::partition(e, myCards.end(), std::not1
											   (std::bind2nd(std::ptr_fun(NetMauMau::Common::isRank),
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
											  m_ruleset->getAceRoundRank())) >
						(m_tryAceRound ? 0 : 1);

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
}

NetMauMau::Common::ICard *StdPlayer::requestCard(const NetMauMau::Common::ICard *uc,
		const NetMauMau::Common::ICard::SUIT *js, std::size_t) const {

	if(m_ruleset) {
		if(m_cards.size() == 1 && !(uc->getRank() == NetMauMau::Common::ICard::JACK &&
									(*m_cards.begin())->getRank() ==
									NetMauMau::Common::ICard::JACK)) {
			return m_ruleset->checkCard(uc, *m_cards.begin()) ? *m_cards.begin() : 0L;
		} else if(m_cards.size() == 1) {
			return 0L;
		}
	}

	NetMauMau::Common::ICard *bestCard = findBestCard(uc, js, false);

	if(bestCard && bestCard->getRank() == NetMauMau::Common::ICard::JACK &&
			uc->getRank() == NetMauMau::Common::ICard::JACK) {
		bestCard = findBestCard(uc, js, true);
	}

	return m_ruleset ? (m_ruleset->checkCard(uc, bestCard) ? bestCard : 0L) : bestCard;
}

NetMauMau::Common::ICard::SUIT
StdPlayer::getJackChoice(const NetMauMau::Common::ICard *uncoveredCard,
						 const NetMauMau::Common::ICard *playedCard) const {

	if(m_powerSuit != NetMauMau::Common::ICard::SUIT_ILLEGAL) {
		const NetMauMau::Common::ICard::SUIT s = m_powerSuit;
		m_powerSuit = NetMauMau::Common::ICard::SUIT_ILLEGAL;
		m_powerPlay = true;

		assert(s != NetMauMau::Common::ICard::SUIT_ILLEGAL);

		return s;
	}

	if(m_cards.size() == 2 && NetMauMau::Common::findRank(NetMauMau::Common::ICard::JACK,
			m_cards.begin(), m_cards.end())) {

		const CARDS::const_iterator &f(std::find_if(m_cards.begin(), m_cards.end(),
									   std::not1(std::bind2nd(
											   std::ptr_fun(NetMauMau::Common::isRank),
											   NetMauMau::Common::ICard::JACK))));

		if(f != m_cards.end()) {
			assert((*f)->getSuit() != NetMauMau::Common::ICard::SUIT_ILLEGAL);
			return (*f)->getSuit();
		}
	}

	if(m_cards.size() < 8) {

		const NetMauMau::Common::ICard *bc = 0L;

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

	for(std::set<std::string>::const_iterator i(m_playedOutCards.begin());
			i != m_playedOutCards.end(); ++i) {

		NetMauMau::Common::ICard::SUIT s;
		NetMauMau::Common::ICard::RANK r;

		if(NetMauMau::Common::parseCardDesc(*i, &s, &r)) {
			pcVec.push_back(NetMauMau::StdCardFactory().create(s, r));
		}
	}

	countSuits(poSuitCount, pcVec);

	for(CARDS::const_iterator i(pcVec.begin()); i != pcVec.end(); ++i) delete *i;

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

IPlayer::REASON StdPlayer::getNoCardReason() const {
	return m_cards.empty() ? MAUMAU : NOMATCH;
}

bool StdPlayer::cardAccepted(const NetMauMau::Common::ICard *playedCard) {

	const CARDS::iterator &i(std::find(m_cards.begin(), m_cards.end(), playedCard));

	if(i != m_cards.end()) m_cards.erase(i);

	if(!m_cards.empty()) shuffleCards();

	m_cardsTaken = false;

	return m_cards.empty();
}

void StdPlayer::cardPlayed(NetMauMau::Common::ICard *playedCard) {
	m_playedOutCards.insert(playedCard->description());
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

const StdPlayer::CARDS &StdPlayer::getPlayerCards() const {
	return m_cards;
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

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
