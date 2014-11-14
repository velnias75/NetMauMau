/*
 * Copyright 2014 by Heiko Schäfer <heiko@rangun.de>
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

#ifndef NETMAUMAU_STDPLAYER_H
#define NETMAUMAU_STDPLAYER_H

#include <algorithm>

#include "iplayer.h"

namespace NetMauMau {

namespace Player {

class _EXPORT StdPlayer : public IPlayer {
	DISALLOW_COPY_AND_ASSIGN(StdPlayer)
public:
	StdPlayer(const std::string &name);
	virtual ~StdPlayer();

	virtual std::string getName() const;
	virtual int getSerial() const _CONST;
	virtual bool isAIPlayer() const _CONST;

	virtual void setRuleSet(const RuleSet::IRuleSet *ruleset);

	virtual void receiveCard(Common::ICard *card);
	virtual void receiveCardSet(const std::vector<Common::ICard *> &cards);

	virtual Common::ICard *requestCard(const Common::ICard *uncoveredCard,
									   const Common::ICard::SUIT *jackSuit) const;
	virtual Common::ICard::SUIT getJackChoice(const Common::ICard *uncoveredCard,
			const Common::ICard *playedCard) const;

	virtual bool cardAccepted(const Common::ICard *playedCard);
	virtual void cardPlayed(Common::ICard *playedCard);
	virtual void talonShuffled();

	virtual REASON getNoCardReason() const _PURE;

	virtual std::size_t getCardCount() const _PURE;
	virtual std::size_t getPoints() const;

	virtual void reset() throw();

	static void resetJackState() throw();

protected:
	const std::vector<Common::ICard *> &getPlayerCards() const _CONST;
	const RuleSet::IRuleSet *getRuleSet() const _PURE;

	virtual void shuffleCards();

private:
	typedef struct _suitCount {
		bool operator<(const _suitCount &sc) const {
			return !(count < sc.count);
		}

		bool operator==(NetMauMau::Common::ICard::SUIT s) const {
			return suit == s;
		}

		NetMauMau::Common::ICard::SUIT suit;
		std::vector<NetMauMau::Common::ICard *>::difference_type count;
	} SUITCOUNT;

	void countSuits(SUITCOUNT *suitCount,
					const std::vector<NetMauMau::Common::ICard *> &myCards) const;

	Common::ICard *findBestCard(const Common::ICard *uc, const Common::ICard::SUIT *js,
								bool noJack) const;

private:
	std::string m_name;
	mutable std::vector<Common::ICard *> m_cards;
	mutable bool m_cardsTaken;
	const RuleSet::IRuleSet *m_ruleset;
	std::vector<Common::ICard *> m_playedOutCards;

	static bool m_jackPlayed;
};

}

}

#endif /* NETMAUMAU_STDPLAYER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
