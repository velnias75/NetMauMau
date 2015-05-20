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

#ifndef NETMAUMAU_PLAYER_EASYPLAYER_H
#define NETMAUMAU_PLAYER_EASYPLAYER_H

#include "hardplayer.h"                 // for HardPlayer

namespace NetMauMau {

namespace Player {

class EasyPlayer : public HardPlayer {
	DISALLOW_COPY_AND_ASSIGN(EasyPlayer)
public:
	explicit EasyPlayer(const std::string &name, const IPlayedOutCards *poc);
	virtual ~EasyPlayer();

	virtual TYPE getType() const _CONST;

	virtual Common::ICardPtr requestCard(const Common::ICardPtr &uncoveredCard,
										 const Common::ICard::SUIT *jackSuit,
										 std::size_t takeCount, bool noSuspend) const;

	virtual Common::ICard::SUIT getJackChoice(const Common::ICardPtr &uncoveredCard,
			const Common::ICardPtr &playedCard) const;
	virtual bool getAceRoundChoice() const _CONST;
};

}

}

#endif /* NETMAUMAU_PLAYER_EASYPLAYER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
