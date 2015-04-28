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

#ifndef NETMAUMAU_ENGINE_AI_ABSTRACTACTION_H
#define NETMAUMAU_ENGINE_AI_ABSTRACTACTION_H

#include "iaction.h"
#include "decisionbase.h"

#include "icondition.h"

namespace NetMauMau {

namespace AI {

class AbstractAction : public IAction, protected DecisionBase {
	DISALLOW_COPY_AND_ASSIGN(AbstractAction)
public:
	virtual ~AbstractAction();

	virtual const IConditionPtr &operator()(IAIState &state) const;

	virtual const IConditionPtr &perform(IAIState &state,
										 const Player::IPlayer::CARDS &cards) const = 0;

protected:
	typedef struct _suitCount {

		bool operator<(const _suitCount &sc) const {
			return !(count < sc.count);
		}

		bool operator==(Common::ICard::SUIT s) const {
			return suit == s;
		}

		Common::ICard::SUIT suit;
		Player::IPlayer::CARDS::difference_type count;

	} SUITCOUNT;

	AbstractAction();

	NetMauMau::Player::IPlayer::CARDS::iterator pullSuit(Player::IPlayer::CARDS &cards,
			Common::ICard::SUIT suit) const;

	NetMauMau::Player::IPlayer::CARDS::iterator pullRank(Player::IPlayer::CARDS &cards,
			Common::ICard::RANK rank) const;

	NetMauMau::Player::IPlayer::CARDS::iterator
	pullRank(const NetMauMau::Player::IPlayer::CARDS::iterator &first,
			 const NetMauMau::Player::IPlayer::CARDS::iterator &last,
			 Common::ICard::RANK rank) const;

	NetMauMau::Player::IPlayer::CARDS::iterator
	pushRank(const NetMauMau::Player::IPlayer::CARDS::iterator &first,
			 const NetMauMau::Player::IPlayer::CARDS::iterator &last,
			 Common::ICard::RANK rank) const;

	const NetMauMau::Common::ICard::SUIT *getSuits() const _CONST;

	static void countSuits(SUITCOUNT *suitCount, const Player::IPlayer::CARDS &myCards);

	Common::ICard::SUIT
	getMaxPlayedOffSuit(const IAIState &state,
						Player::IPlayer::CARDS::difference_type *count = 0L) const;

	static const IConditionPtr &getNullCondition() _CONST;
};

}

}

#endif /* NETMAUMAU_ENGINE_AI_ABSTRACTACTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
