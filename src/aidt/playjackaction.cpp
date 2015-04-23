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

#include "playjackaction.h"

#include "smartptr.h"
#include "iruleset.h"
#include "aistate.h"
#include "icard.h"

using namespace NetMauMau::Engine::AIDT;

PlayJackAction::PlayJackAction() : AbstractAction() {}

PlayJackAction::~PlayJackAction() {}

const IConditionPtr &PlayJackAction::operator()(AIState &state) const {

	const NetMauMau::Common::ICardPtr firstCard(*state.getCards().begin());

	state.setCard(state.getRuleSet()->checkCard(state.getUncoveredCard(), firstCard) ?
				  firstCard : NetMauMau::Common::ICardPtr());

	return AbstractAction::getNullCondition();
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
