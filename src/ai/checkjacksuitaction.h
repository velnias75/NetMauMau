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

#ifndef NETMAUMAU_ENGINE_AI_CHECKSUITJACKACTION_H
#define NETMAUMAU_ENGINE_AI_CHECKSUITJACKACTION_H

#include "abstractaction.h"

namespace NetMauMau {

namespace AI {

class CheckJackSuitAction : public AbstractAction {
	DISALLOW_COPY_AND_ASSIGN(CheckJackSuitAction)
public:
	CheckJackSuitAction();
	virtual ~CheckJackSuitAction();

	virtual const IConditionPtr &perform(IAIState &state,
										 const Player::IPlayer::CARDS &cards) const;

private:
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
	struct _hasRankPath : std::unary_function<Common::ICardPtr, bool> {

		explicit _hasRankPath(const Player::IPlayer::CARDS &c, Common::ICard::RANK r, bool nie) :
			mCards(c), rank(r), nineIsEight(nie) {}

		bool operator()(const Common::ICardPtr &c) const;

	private:
		const Player::IPlayer::CARDS &mCards;
		const Common::ICard::RANK rank;
		bool nineIsEight;
	};
#pragma GCC diagnostic pop

	Common::ICard::SUIT findJackChoice(const IAIState &state) const;

	static Common::ICardPtr hasRankPath(const Common::ICardPtr &uc,  Common::ICard::SUIT s,
										Common::ICard::RANK r, const Player::IPlayer::CARDS &cards,
										bool nineIsEight);
};

}

}

#endif /* NETMAUMAU_ENGINE_AI_CHECKSUITJACKACTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
