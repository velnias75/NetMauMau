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

#include "iplayer.h"

namespace NetMauMau {

namespace Engine {

namespace AIDT {

class AIState {
	DISALLOW_COPY_AND_ASSIGN(AIState)
public:
	AIState(const Player::IPlayer::CARDS &cards, const Common::ICardPtr &uc,
			const RuleSet::IRuleSet *ruleSet);
	~AIState();

	const Player::IPlayer::CARDS &getCards() const {
		return m_cards;
	}

	const Common::ICardPtr &getUncoveredCard() const {
		return m_uncoveredCard;
	}

	const RuleSet::IRuleSet *getRuleSet() const {
		return m_ruleSet;
	}

private:
	const Player::IPlayer::CARDS &m_cards;
	const Common::ICardPtr &m_uncoveredCard;
	const RuleSet::IRuleSet *const m_ruleSet;
};

}

}

}

#endif /* ETMAUMAU_ENGINE_AIDT_AISTATE_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
