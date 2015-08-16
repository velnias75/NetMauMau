/*
 * Copyright 2014-2015 by Heiko Schäfer <heiko@rangun.de>
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

#ifndef NETMAUMAU_PLAYER_ABSTRACTPLAYER_H
#define NETMAUMAU_PLAYER_ABSTRACTPLAYER_H

#include <cstddef>                      // for size_t

#include "iplayedoutcards.h"            // for IPlayedOutCards, etc
#include "iplayer.h"                    // for IPlayer::CARDS, IPlayer, etc

namespace NetMauMau {

namespace Player {

class AbstractPlayer : public IPlayer {
	DISALLOW_COPY_AND_ASSIGN(AbstractPlayer)
public:
	virtual ~AbstractPlayer();

	virtual std::string getName() const;
	virtual int getSerial() const = 0;
	virtual bool isAIPlayer() const throw() = 0;
	virtual bool isAlive() const = 0;

	virtual void setRuleSet(const RuleSet::IRuleSet *ruleset) _NONNULL_ALL;
	virtual void setCardCountObserver(const ICardCountObserver *cco) _NONNULL_ALL;
	virtual void setEngineContext(const EngineContext *engineCtx);

	virtual void receiveCard(const Common::ICardPtr &card) = 0;
	virtual void receiveCardSet(const CARDS &cards);

	virtual Common::ICardPtr requestCard(const Common::ICardPtr &uncoveredCard,
										 const Common::ICard::SUIT *jackSuit,
										 std::size_t takeCount, bool noSuspend) const = 0;
	virtual Common::ICard::SUIT getJackChoice(const Common::ICardPtr &uncoveredCard,
			const Common::ICardPtr &playedCard) const = 0;

	virtual bool getAceRoundChoice() const = 0;

	virtual void informAIStat(const IPlayer *player, std::size_t count, Common::ICard::SUIT lpSuit,
							  Common::ICard::RANK lpRank);

	virtual Common::ICard::SUIT getLastPlayedSuit() const _PURE;
	virtual Common::ICard::RANK getLastPlayedRank() const _PURE;

	virtual bool cardAccepted(const Common::ICard *playedCard);
	virtual void setNeighbourCardStats(std::size_t playerCount,
									   const std::size_t *const neighbourCount,
									   const NEIGHBOURRANKSUIT &nrs);
	virtual const NEIGHBOURRANKSUIT &getNeighbourRankSuit() const _CONST;
	virtual void setDirChangeEnabled(bool dirChangeEnabled);
	virtual void talonShuffled() _CONST;
	virtual void setNineIsSuspend(bool b);

	virtual REASON getNoCardReason(const Common::ICardPtr &uncoveredCard,
								   const Common::ICard::SUIT *suit) const _PURE;

	virtual std::size_t getCardCount() const = 0;
	virtual std::size_t getPoints() const;

	virtual void reset() throw();

	virtual const CARDS &getPlayerCards() const _CONST;

protected:
	explicit AbstractPlayer(const std::string &name, const IPlayedOutCards *poc);

	virtual void shuffleCards();

	const RuleSet::IRuleSet *getRuleSet() const _PURE;
	const IPlayedOutCards::CARDS &getPlayedOutCards() const;
	std::size_t getPlayerCount() const _PURE;
	std::size_t getLeftCount() const _PURE;
	std::size_t getRightCount() const _PURE;
	bool hasTakenCards() const _PURE;

	// cppcheck-suppress functionConst
	void setCardsTaken(bool b);

	bool hasPlayerFewCards() const _PURE;
	bool nineIsSuspend() const _PURE;
	bool isDirChgEnabled() const _PURE;

	const EngineContext *getEngineContext() const _PURE;

	// cppcheck-suppress functionConst
	CARDS getPossibleCards(const Common::ICardPtr &uncoveredCard,
						   const Common::ICard::SUIT *suit) const;

	void notifyCardCountChange() const throw();
	bool isAceRoundAllowed() const;

	inline Common::ICard::SUIT getAvoidSuit() const {
		return m_avoidSuit;
	}

	inline Common::ICard::RANK getAvoidRank() const {
		return m_avoidRank;
	}

	void pushCard(const Common::ICardPtr &card);

protected:
	mutable Common::ICard::SUIT m_lastPlayedSuit;
	mutable Common::ICard::RANK m_lastPlayedRank;

private:
	const std::string m_name;
	CARDS m_cards;
	mutable bool m_cardsTaken;
	const RuleSet::IRuleSet *m_ruleset;
	bool m_playerHasFewCards;
	bool m_nineIsSuspend;
	std::size_t m_neighbourCount[2];
	bool m_dirChgEnabled;
	std::size_t m_playerCount;
	const EngineContext *m_engineCtx;
	const ICardCountObserver *m_cardCountObserver;
	const IPlayedOutCards *const m_poc;
	Common::ICard::SUIT m_avoidSuit;
	Common::ICard::RANK m_avoidRank;
	NEIGHBOURRANKSUIT m_neighbourRankSuit;
};

}

}

#endif /* NETMAUMAU_PLAYER_ABSTRACTPLAYER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
