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

#ifndef NETMAUMAU_ENGINE_AI_BASEAIPLAYER_H
#define NETMAUMAU_ENGINE_AI_BASEAIPLAYER_H

#include "decisionchain.h"

namespace NetMauMau {

namespace AI {

template<class RootCond, class RootCondJack>
class BaseAIPlayer : protected IAIState {
	DISALLOW_COPY_AND_ASSIGN(BaseAIPlayer)
public:
	typedef RootCond root_condition_type;
	typedef RootCondJack jack_root_condition_type;

	virtual ~BaseAIPlayer() throw();

protected:
	typedef Common::SmartPtr<DecisionChain<root_condition_type> > DecisionChainPtr;
	typedef Common::SmartPtr<DecisionChain<jack_root_condition_type, true> > JackDecisionChainPtr;

	explicit BaseAIPlayer(const IActionPtr &trueAct, const IActionPtr &falseAct) throw()
		: IAIState(), m_decisionChain(Common::SmartPtr<DecisionChain<root_condition_type> >
									  (new DecisionChain<root_condition_type>(*this))),
		m_jackDecisionChain(Common::SmartPtr<DecisionChain<jack_root_condition_type, true> >
							(new DecisionChain<jack_root_condition_type, true>(*this,
									IActionPtr(trueAct), IActionPtr(falseAct)))), m_card(),
		m_uncoveredCard(), m_jackSuit(0L), m_playedCard(), m_noJack(false) {}

	explicit BaseAIPlayer() throw() : IAIState(),
		m_decisionChain(Common::SmartPtr<DecisionChain<root_condition_type> >
						(new DecisionChain<root_condition_type>(*this))),
		m_jackDecisionChain(Common::SmartPtr<DecisionChain<jack_root_condition_type, true> >
							(new DecisionChain<jack_root_condition_type, true>(*this))), m_card(),
		m_uncoveredCard(), m_jackSuit(0L), m_playedCard(), m_noJack(false) {}

	virtual Common::ICardPtr getUncoveredCard() const throw() {
		return m_uncoveredCard;
	}

	virtual Common::ICardPtr getPlayedCard() const throw() {
		return m_playedCard;
	}

	virtual Common::ICardPtr getCard() const throw() {
		return m_card;
	}

	virtual void setCard(const Common::ICardPtr &card) throw() {
		m_card = card;
	}

	virtual bool isNoJack() const throw() {
		return m_noJack;
	}

	virtual void setNoJack(bool b) throw() {
		m_noJack = b;
	}

	virtual Common::ICard::SUIT *getJackSuit() const throw() {
		return m_jackSuit;
	}

	const DecisionChainPtr &getDecisionChain() const throw() {
		return m_decisionChain;
	}

	const JackDecisionChainPtr &getJackDecisionChain() const throw() {
		return m_jackDecisionChain;
	}

	void reset() const throw() {
		m_card = m_uncoveredCard = m_playedCard = Common::ICardPtr();
		m_noJack = false;
		m_jackSuit = 0L;
	}

private:
	DecisionChainPtr m_decisionChain;
	JackDecisionChainPtr m_jackDecisionChain;

protected:
	mutable Common::ICardPtr m_card;
	mutable Common::ICardPtr m_uncoveredCard;
	mutable Common::ICard::SUIT *m_jackSuit;
	mutable Common::ICardPtr m_playedCard;
	mutable bool m_noJack;
};

template<class RootCond, class RootCondJack>
BaseAIPlayer<RootCond, RootCondJack>::~BaseAIPlayer() throw() {}

}

}

#endif /* NETMAUMAU_ENGINE_AI_BASEAIPLAYER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
