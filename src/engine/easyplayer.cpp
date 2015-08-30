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
#include "config.h"                     // IWYU pragma: keep
#endif

#include <ctime>

#if defined(TRACE_AI) && !defined(NDEBUG)
#include <cstdio>
#endif

#include "easyplayer.h"

#include "jackonlycondition.h"
#include "powerjackcondition.h"

#include "random_gen.h"                 // for genRandom

using namespace NetMauMau::Player;

EasyPlayer::EasyPlayer(const std::string &name, const NetMauMau::IPlayedOutCards *poc)
	: AIPlayerBase<AI::JackOnlyCondition, AI::PowerJackCondition>(name, poc) {}

EasyPlayer::~EasyPlayer() throw() {}

NetMauMau::Common::ICardPtr
EasyPlayer::requestCard(const NetMauMau::Common::ICardPtr &uncoveredCard,
						const NetMauMau::Common::ICard::SUIT *jackSuit,
						std::size_t takeCount, bool noSuspend) const {

	NetMauMau::Common::ICardPtr rrc;

	if(!takeCount) {

		// we need to make a copy here, because std::random_shuffle is a mutating algorithm
		IPlayer::CARDS shuffledCards(getPossibleCards(uncoveredCard, jackSuit));

		if(!shuffledCards.empty() && Commons::effModulo<long, 4L>(std::time(0L))) {

			std::random_shuffle(shuffledCards.begin(), shuffledCards.end(),
								NetMauMau::Common::genRandom<CARDS::difference_type>);

			rrc = NetMauMau::Common::find(*shuffledCards.begin(), getPlayerCards().begin(),
										  getPlayerCards().end());
		}

	} else {
		rrc = NetMauMau::Common::ICardPtr
			  (const_cast<const NetMauMau::Common::ICard *>(NetMauMau::Common::getIllegalCard()));
	}

	rrc = noSuspend ? noSuspendCard(rrc, uncoveredCard, jackSuit) : rrc;

#if defined(TRACE_AI) && !defined(NDEBUG)

	const bool ansi = isatty(fileno(stderr));

	if(!std::getenv("NMM_NO_TRACE")) logDebug("-> trace of AI \"" << getName() << "\" -> "
				<< (rrc ? rrc->description(ansi) : "NO CARD") << " <-");

#endif

	return rrc;
}

NetMauMau::Common::ICard::SUIT
EasyPlayer::getJackChoice(const NetMauMau::Common::ICardPtr &,
						  const NetMauMau::Common::ICardPtr &) const {

	const NetMauMau::Player::IPlayer::CARDS::difference_type r =
		NetMauMau::Common::genRandom<NetMauMau::Player::IPlayer::CARDS::difference_type>
		(static_cast<NetMauMau::Player::IPlayer::CARDS::difference_type>(4u));

	const NetMauMau::Common::ICard::SUIT rs =
		NetMauMau::Common::symbolToSuit(NetMauMau::Common::getSuitSymbols()[r]);

#if defined(TRACE_AI) && !defined(NDEBUG)

	const bool ansi = isatty(fileno(stderr));

	if(!getenv("NMM_NO_TRACE")) logDebug("-> -jack- trace of AI \"" << getName() << "\" -> "
											 << Common::suitToSymbol(rs, ansi, ansi) << " <-");

#endif

	return rs;
}

bool EasyPlayer::getAceRoundChoice() const {
	return false;
}

IPlayer::TYPE EasyPlayer::getType() const throw() {
	return EASY;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
