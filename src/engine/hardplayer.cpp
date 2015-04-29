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

#include "hardplayer.h"

#include "nextaction.h"
#include "powerplayaction.h"
#include "jackonlycondition.h"
#include "havejackcondition.h"
#include "powersuitcondition.h"

namespace {
const NetMauMau::AI::IConditionPtr HAVEJACKCOND(new NetMauMau::AI::HaveJackCondition());
}

using namespace NetMauMau::Player;

HardPlayer::HardPlayer(const std::string &name) : AIPlayerBase < NetMauMau::AI::JackOnlyCondition,
	NetMauMau::AI::PowerSuitCondition > (name, NetMauMau::AI::IActionPtr
										 (new NetMauMau::AI::NextAction(HAVEJACKCOND)),
										 NetMauMau::AI::IActionPtr
										 (new NetMauMau::AI::PowerPlayAction(true))) {}

HardPlayer::~HardPlayer() {}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
