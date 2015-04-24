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

#ifndef NETMAUMAU_ENGINE_AIDT_AISTATE_H
#define NETMAUMAU_ENGINE_AIDT_AISTATE_H

#include "smartptr.h"
#include "iplayer.h"
#include "icard.h"

namespace NetMauMau {

namespace Engine {

namespace AIDT {

class AIState {
	DISALLOW_COPY_AND_ASSIGN(AIState)
public:
	typedef Common::SmartPtr<RuleSet::IRuleSet> IRuleSetPtr;
	typedef std::vector<std::string> PLAYEDOUTCARDS;

	AIState(const Player::IPlayer::CARDS &cards, const Common::ICardPtr &uc,
			const IRuleSetPtr &ruleSet, const PLAYEDOUTCARDS &playedOutCards);

	~AIState();

	// cppcheck-suppress functionStatic
	inline const Player::IPlayer::CARDS &getCards() const {
		return m_cards;
	}

	inline Common::ICardPtr getUncoveredCard() const {
		return m_uncoveredCard;
	}

	IRuleSetPtr getRuleSet() const;

	inline const std::vector<std::string> &getPlayedOutCards() const {
		return m_playedOutCards;
	}

	inline bool hasTakenCards() const {
		return m_cardsTaken;
	}

	inline void setCardsTaken(bool b) {
		m_cardsTaken = b;
	}

	// cppcheck-suppress functionStatic
	std::size_t getTalonFactor() const;

	void setCard(const Common::ICardPtr &card);

	inline Common::ICardPtr getCard() const {
		return m_card;
	}

private:
	Common::ICardPtr m_card;
	const Player::IPlayer::CARDS m_cards;
	const Common::ICardPtr m_uncoveredCard;
	const IRuleSetPtr m_ruleSet;
	const PLAYEDOUTCARDS m_playedOutCards;
	bool m_cardsTaken;
};

}

}

}

#endif /* ETMAUMAU_ENGINE_AIDT_AISTATE_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
