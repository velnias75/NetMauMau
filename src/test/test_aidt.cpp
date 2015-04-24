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

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#include <iostream>
#include <cstdlib>
#include <cstdio>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "aistate.h"
#include "luaruleset.h"
#include "decisiontree.h"
#include "luaexception.h"
#include "stdcardfactory.h"

using namespace NetMauMau;

int main(int, const char **) {

	const char *luaRules = getenv("NETMAUMAU_RULES");

	if(!luaRules) {
		std::cerr << "No environment variable NETMAUMAU_RULES set" << std::endl;
		return EXIT_FAILURE;
	}

	try {

		Common::ICardPtr jc[2] = {
			Common::ICardPtr(StdCardFactory().create(Common::ICard::SPADES, Common::ICard::TEN)),
			Common::ICardPtr(StdCardFactory().create(Common::ICard::SPADES, Common::ICard::NINE))
		};

		Common::ICardPtr uc(StdCardFactory().create(Common::ICard::SPADES, Common::ICard::SEVEN));

		Engine::AIDT::AIState::PLAYEDOUTCARDS::value_type poc[5] = {
			"", "", "", "", ""
		};

		Engine::AIDT::AIState state(Player::IPlayer::CARDS(jc, jc + 2), uc,
									Engine::AIDT::AIState::IRuleSetPtr
									(new RuleSet::LuaRuleSet(luaRules, true)),
									Engine::AIDT::AIState::PLAYEDOUTCARDS(poc, poc + 5));

		Engine::AIDT::DecisionTree dt(state);

		std::cout << (dt.getCard() ? dt.getCard()->description(isatty(fileno(stdout))) : "SUSPEND")
				  << std::endl;

	} catch(const Lua::Exception::LuaException &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
