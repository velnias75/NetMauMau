/*
 * Copyright 2015 by Heiko Schäfer <heiko@rangun.de>
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

#ifndef NETMAUMAU_PLAYER_AIPLAYERBASE_H
#define NETMAUMAU_PLAYER_AIPLAYERBASE_H

#include <cassert>

#include "baseaiplayer.h"
#include "abstractplayer.h"

#include "iruleset.h"
#include "enginecontext.h"
#include "socketexception.h"

namespace NetMauMau {

namespace Player {

template<class RootCond, class RootCondJack>
class AIPlayerBase : public AbstractPlayer, public AI::BaseAIPlayer<RootCond, RootCondJack> {
	DISALLOW_COPY_AND_ASSIGN(AIPlayerBase)
public:
	virtual ~AIPlayerBase() {}

	virtual std::string getName() const;
	virtual int getSerial() const;
	virtual bool isAIPlayer() const;
	virtual bool isAlive() const;

	virtual void setRuleSet(const RuleSet::IRuleSet *ruleset);
	virtual void setCardCountObserver(const ICardCountObserver *cco);
	virtual void setEngineContext(const EngineContext *engineCtx);

	virtual void receiveCard(const Common::ICardPtr &card);
	virtual void receiveCardSet(const CARDS &cards);

	virtual Common::ICardPtr requestCard(const Common::ICardPtr &uncoveredCard,
										 const Common::ICard::SUIT *jackSuit,
										 std::size_t takeCount, bool noSuspend) const;
	virtual Common::ICard::SUIT getJackChoice(const Common::ICardPtr &uncoveredCard,
			const Common::ICardPtr &playedCard) const;

	virtual bool getAceRoundChoice() const;

	virtual void informAIStat(const IPlayer *player, std::size_t count, Common::ICard::SUIT lpSuit,
							  Common::ICard::RANK lpRank);

	virtual Common::ICard::SUIT getAvoidSuit() const;
	virtual Common::ICard::RANK getAvoidRank() const;

	virtual bool cardAccepted(const Common::ICard *playedCard);
	virtual void setNeighbourCardStats(std::size_t playerCount,
									   const std::size_t *const neighbourCount,
									   const NEIGHBOURRANKSUIT &nrs);
	virtual const NEIGHBOURRANKSUIT &getNeighbourRankSuit() const;
	virtual void setDirChangeEnabled(bool dirChangeEnabled);
	virtual void setNineIsSuspend(bool b);
	virtual void talonShuffled();

	virtual REASON getNoCardReason(const Common::ICardPtr &uncoveredCard,
								   const Common::ICard::SUIT *suit) const;

	virtual std::size_t getCardCount() const;
	virtual std::size_t getPoints() const;

	virtual void reset() throw();

	virtual const RuleSet::IRuleSet *getRuleSet() const;

	virtual const CARDS &getPlayerCards() const;
	virtual const IPlayedOutCards::CARDS &getPlayedOutCards() const;

	virtual std::size_t getPlayerCount() const;

	virtual std::size_t getLeftCount() const;
	virtual std::size_t getRightCount() const;

	virtual Common::ICard::SUIT getPowerSuit() const;
	virtual void setPowerSuit(Common::ICard::SUIT suit);
	virtual bool isPowerPlay() const;
	virtual void setPowerPlay(bool b);

	virtual bool hasPlayerFewCards() const;

	virtual bool hasTakenCards() const;
	virtual void setCardsTaken(bool b);

protected:
	explicit AIPlayerBase(const std::string &name, const AI::IActionPtr &trueAct,
						  const AI::IActionPtr &falseAct, const IPlayedOutCards *poc)
		: AbstractPlayer(name, poc), AI::BaseAIPlayer<RootCond, RootCondJack>(trueAct, falseAct),
		  m_powerSuit(Common::ICard::SUIT_ILLEGAL), m_powerPlay(false),
		  m_tryAceRound(false) {}

	virtual void shuffleCards();
	virtual std::size_t getTalonFactor() const;
	virtual bool nineIsSuspend() const;
	virtual bool isDirChgEnabled() const;
	virtual bool tryAceRound() const;
	virtual void setTryAceRound(bool b);

	Common::ICardPtr noSuspendCard(const Common::ICardPtr &c, const Common::ICardPtr &uc,
								   const Common::ICard::SUIT *js) const;

private:
	mutable Common::ICard::SUIT m_powerSuit;
	mutable bool m_powerPlay;
	mutable bool m_tryAceRound;
};

template<class RootCond, class RootCondJack>
inline void AIPlayerBase<RootCond, RootCondJack>::reset() throw() {

	m_powerSuit = Common::ICard::SUIT_ILLEGAL;
	m_tryAceRound = m_powerPlay = false;

	AI::BaseAIPlayer<RootCond, RootCondJack>::reset();
	AbstractPlayer::reset();
}

template<class RootCond, class RootCondJack>
inline std::string AIPlayerBase<RootCond, RootCondJack>::getName() const {
	return AbstractPlayer::getName();
}

template<class RootCond, class RootCondJack>
inline int AIPlayerBase<RootCond, RootCondJack>::getSerial() const {
	return INVALID_SOCKET;
}

template<class RootCond, class RootCondJack>
inline bool AIPlayerBase<RootCond, RootCondJack>::isAIPlayer() const {
	return true;
}

template<class RootCond, class RootCondJack>
inline bool AIPlayerBase<RootCond, RootCondJack>::isAlive() const {
	return true;
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase < RootCond, RootCondJack >::setRuleSet(const RuleSet::IRuleSet *ruleset) {
	AbstractPlayer::setRuleSet(ruleset);
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase < RootCond,
RootCondJack >::setEngineContext(const EngineContext *engineCtx) {
	AbstractPlayer::setEngineContext(engineCtx);
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase < RootCond,
RootCondJack >::setCardCountObserver(const ICardCountObserver *cco) {
	AbstractPlayer::setCardCountObserver(cco);
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase < RootCond, RootCondJack >::receiveCard(const Common::ICardPtr &card) {

	if(card) {
		pushCard(card);
		notifyCardCountChange();
	}
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase<RootCond, RootCondJack>::receiveCardSet(const CARDS &cards) {
	AbstractPlayer::receiveCardSet(cards);
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase<RootCond, RootCondJack>::shuffleCards() {
	AbstractPlayer::shuffleCards();
}

template<class RootCond, class RootCondJack>
inline Common::ICardPtr AIPlayerBase < RootCond,
	   RootCondJack >::requestCard(const Common::ICardPtr &uc, const Common::ICard::SUIT *js,
std::size_t, bool noSuspend) const {

	AI::BaseAIPlayer<RootCond, RootCondJack>::m_card = Common::ICardPtr();
	AI::BaseAIPlayer<RootCond, RootCondJack>::m_uncoveredCard = uc;
	AI::BaseAIPlayer<RootCond, RootCondJack>::m_jackSuit =
		js ? const_cast<Common::ICard::SUIT *>(js) : 0L;

	Common::ICardPtr
	bestCard(AI::BaseAIPlayer<RootCond, RootCondJack>::getDecisionChain()->getCard());

	AI::BaseAIPlayer<RootCond, RootCondJack>::m_playedCard =
		AI::BaseAIPlayer<RootCond, RootCondJack>::m_card = Common::ICardPtr();

	if(bestCard && bestCard == Common::ICard::JACK && uc == Common::ICard::JACK) {
		bestCard = AI::BaseAIPlayer<RootCond, RootCondJack>::getDecisionChain()->getCard(true);
	}

	const Common::ICardPtr &rc(getRuleSet() ? (getRuleSet()->checkCard(uc,
							   Common::ICardPtr(bestCard)) ? Common::ICardPtr(bestCard) :
							   Common::ICardPtr()) : Common::ICardPtr(bestCard));

	return noSuspend ? noSuspendCard(rc, uc, js) : rc;
}

template<class RootCond, class RootCondJack>
Common::ICardPtr AIPlayerBase<RootCond, RootCondJack>::noSuspendCard(const Common::ICardPtr &c,
		const Common::ICardPtr &uc, const Common::ICard::SUIT *js) const {

	if(!(c && c != Common::ICard::SEVEN)) {

		const CARDS &pc(getPossibleCards(uc, js));
		CARDS::const_iterator i(pc.begin());

		while(i != pc.end() && ((*i) == Common::ICard::SEVEN)) ++i;

		if(i != pc.end()) {
#if defined(TRACE_AI) && !defined(NDEBUG)

			const bool ansi = isatty(fileno(stderr));

			if(!getenv("NMM_NO_TRACE")) logDebug("-> trace of AI \"" << getName()
													 << "\" -> TALON-UNDERFLOW-CHOICE: "
													 << (*i)->description(ansi) << " <-");

#endif
			return *i;

		} else if(!pc.empty()) {
			return pc.back();
		}
	}

	return c;
}

template<class RootCond, class RootCondJack>
inline Common::ICard::SUIT AIPlayerBase < RootCond,
	   RootCondJack >::getJackChoice(const Common::ICardPtr &uncoveredCard,
const Common::ICardPtr &playedCard) const {

	const Common::ICardPtr oc(AI::BaseAIPlayer<RootCond, RootCondJack>::m_card);
	const Common::ICardPtr uc(AI::BaseAIPlayer<RootCond, RootCondJack>::m_uncoveredCard);

	AI::BaseAIPlayer<RootCond, RootCondJack>::m_card =
		AI::BaseAIPlayer<RootCond, RootCondJack>::m_playedCard = playedCard;
	AI::BaseAIPlayer<RootCond, RootCondJack>::m_uncoveredCard = uncoveredCard;

	const Common::ICardPtr &rc(AI::BaseAIPlayer < RootCond,
							   RootCondJack >::getJackDecisionChain()->getCard(true));
	assert(rc);
	const Common::ICard::SUIT s = rc->getSuit();

	AI::BaseAIPlayer<RootCond, RootCondJack>::m_card = oc;
	AI::BaseAIPlayer<RootCond, RootCondJack>::m_uncoveredCard = uc;
	AI::BaseAIPlayer<RootCond, RootCondJack>::m_playedCard = Common::ICardPtr();

	return s;
}

template<class RootCond, class RootCondJack>
inline bool AIPlayerBase<RootCond, RootCondJack>::getAceRoundChoice() const {

	if(m_tryAceRound && isAceRoundAllowed()) return m_tryAceRound;

	return (m_tryAceRound = false);
}

template<class RootCond, class RootCondJack>
inline IPlayer::REASON AIPlayerBase < RootCond,
RootCondJack >::getNoCardReason(const Common::ICardPtr &c, const Common::ICard::SUIT *s) const {
	return AbstractPlayer::getNoCardReason(c, s);
}

template<class RootCond, class RootCondJack>
inline bool AIPlayerBase < RootCond, RootCondJack >::cardAccepted(const Common::ICard *playedCard) {
	return AbstractPlayer::cardAccepted(playedCard);
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase<RootCond, RootCondJack>::talonShuffled() {
	AbstractPlayer::talonShuffled();
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase<RootCond, RootCondJack>::setNineIsSuspend(bool b) {
	AbstractPlayer::setNineIsSuspend(b);
}

template<class RootCond, class RootCondJack>
inline std::size_t AIPlayerBase<RootCond, RootCondJack>::getCardCount() const {
	assert(getPlayerCards().size() < (32 * getTalonFactor()));
	return getPlayerCards().size();
}

template<class RootCond, class RootCondJack>
inline std::size_t AIPlayerBase<RootCond, RootCondJack>::getPoints() const {
	return AbstractPlayer::getPoints();
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase < RootCond,
	   RootCondJack >::informAIStat(const IPlayer *p, std::size_t count, Common::ICard::SUIT lpSuit,
Common::ICard::RANK lpRank) {
	AbstractPlayer::informAIStat(p, count, lpSuit, lpRank);
}

template<class RootCond, class RootCondJack>
inline Common::ICard::SUIT AIPlayerBase<RootCond, RootCondJack>::getAvoidSuit() const {
	return AbstractPlayer::getAvoidSuit();
}

template<class RootCond, class RootCondJack>
inline Common::ICard::RANK AIPlayerBase<RootCond, RootCondJack>::getAvoidRank() const {
	return AbstractPlayer::getAvoidRank();
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase<RootCond, RootCondJack>::setNeighbourCardStats(std::size_t playerCount,
		const std::size_t *const neighbourCount, const NEIGHBOURRANKSUIT &nrs) {
	AbstractPlayer::setNeighbourCardStats(playerCount, neighbourCount, nrs);
}

template<class RootCond, class RootCondJack>
inline const Player::IPlayer::NEIGHBOURRANKSUIT &AIPlayerBase < RootCond,
RootCondJack >::getNeighbourRankSuit() const {
	return AbstractPlayer::getNeighbourRankSuit();
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase<RootCond, RootCondJack>::setDirChangeEnabled(bool dirChangeEnabled) {
	AbstractPlayer::setDirChangeEnabled(dirChangeEnabled);
}

template<class RootCond, class RootCondJack>
inline std::size_t AIPlayerBase<RootCond, RootCondJack>::getTalonFactor() const {
	return getEngineContext() ? getEngineContext()->getTalonFactor() : 1;
}

template<class RootCond, class RootCondJack>
inline const RuleSet::IRuleSet *AIPlayerBase<RootCond, RootCondJack>::getRuleSet() const {
	return AbstractPlayer::getRuleSet();
}

template<class RootCond, class RootCondJack>
inline const IPlayer::CARDS &AIPlayerBase<RootCond, RootCondJack>::getPlayerCards() const {
	return AbstractPlayer::getPlayerCards();
}

template<class RootCond, class RootCondJack>
inline const IPlayedOutCards::CARDS &AIPlayerBase < RootCond,
RootCondJack >::getPlayedOutCards() const {
	return AbstractPlayer::getPlayedOutCards();
}

template<class RootCond, class RootCondJack>
inline std::size_t AIPlayerBase<RootCond, RootCondJack>::getPlayerCount() const {
	return AbstractPlayer::getPlayerCount();
}

template<class RootCond, class RootCondJack>
inline std::size_t AIPlayerBase<RootCond, RootCondJack>::getLeftCount() const {
	return AbstractPlayer::getLeftCount();
}

template<class RootCond, class RootCondJack>
inline std::size_t AIPlayerBase<RootCond, RootCondJack>::getRightCount() const {
	return AbstractPlayer::getRightCount();
}

template<class RootCond, class RootCondJack>
inline bool AIPlayerBase<RootCond, RootCondJack>::hasTakenCards() const {
	return AbstractPlayer::hasTakenCards();
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase<RootCond, RootCondJack>::setCardsTaken(bool b) {
	AbstractPlayer::setCardsTaken(b);
}

template<class RootCond, class RootCondJack>
inline bool AIPlayerBase<RootCond, RootCondJack>::hasPlayerFewCards() const {
	return AbstractPlayer::hasPlayerFewCards();
}

template<class RootCond, class RootCondJack>
inline Common::ICard::SUIT AIPlayerBase<RootCond, RootCondJack>::getPowerSuit() const {
	return m_powerSuit;
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase<RootCond, RootCondJack>::setPowerSuit(Common::ICard::SUIT suit) {
	m_powerSuit = suit;
}

template<class RootCond, class RootCondJack>
inline bool AIPlayerBase<RootCond, RootCondJack>::nineIsSuspend() const {
	return AbstractPlayer::nineIsSuspend();
}

template<class RootCond, class RootCondJack>
inline bool AIPlayerBase<RootCond, RootCondJack>::tryAceRound() const {
	return m_tryAceRound;
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase<RootCond, RootCondJack>::setTryAceRound(bool b) {
	m_tryAceRound = b;
}

template<class RootCond, class RootCondJack>
inline bool AIPlayerBase<RootCond, RootCondJack>::isDirChgEnabled() const {
	return AbstractPlayer::isDirChgEnabled();
}

template<class RootCond, class RootCondJack>
inline bool AIPlayerBase<RootCond, RootCondJack>::isPowerPlay() const {
	return m_powerPlay;
}

template<class RootCond, class RootCondJack>
inline void AIPlayerBase<RootCond, RootCondJack>::setPowerPlay(bool b) {
	m_powerPlay = b;
}

}

}

#endif /* NETMAUMAU_PLAYER_AIPLAYERBASE_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
