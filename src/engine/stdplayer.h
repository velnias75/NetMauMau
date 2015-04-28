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

#ifndef NETMAUMAU_PLAYER_STDPLAYER_H
#define NETMAUMAU_PLAYER_STDPLAYER_H

#include "aiplayer.h"

namespace NetMauMau {

namespace AI {
class JackOnlyCondition;
class PowerSuitCondition;
}

namespace Player {

class StdPlayer : public AI::AIPlayer<AI::JackOnlyCondition, AI::PowerSuitCondition> {
	DISALLOW_COPY_AND_ASSIGN(StdPlayer)
public:
	explicit StdPlayer(const std::string &name);
	virtual ~StdPlayer();

	virtual std::string getName() const;
	virtual int getSerial() const _CONST;
	virtual bool isAIPlayer() const _CONST;
	virtual bool isAlive() const _CONST;

	virtual void setRuleSet(const RuleSet::IRuleSet *ruleset);
	virtual void setCardCountObserver(const ICardCountObserver *cco);
	virtual void setEngineConfig(const EngineConfig *engineCfg);

	virtual void receiveCard(const Common::ICardPtr &card);
	virtual void receiveCardSet(const CARDS &cards);

	virtual Common::ICardPtr requestCard(const Common::ICardPtr &uncoveredCard,
										 const Common::ICard::SUIT *jackSuit,
										 std::size_t takeCount) const;
	virtual Common::ICard::SUIT getJackChoice(const Common::ICardPtr &uncoveredCard,
			const Common::ICardPtr &playedCard) const;

	virtual bool getAceRoundChoice() const;

	virtual bool cardAccepted(const Common::ICard *playedCard);
	virtual void cardPlayed(Common::ICard *playedCard);
	virtual void informAIStat(const IPlayer *player, std::size_t count);
	virtual void setNeighbourCardCount(std::size_t playerCount,
									   std::size_t leftCount, std::size_t rightCount);
	virtual void setDirChangeEnabled(bool dirChangeEnabled);
	virtual void talonShuffled();
	virtual void setNineIsEight(bool b);

	virtual REASON getNoCardReason(const Common::ICardPtr &uncoveredCard,
								   const Common::ICard::SUIT *suit) const _PURE;

	virtual std::size_t getCardCount() const _PURE;
	virtual std::size_t getPoints() const;

	virtual void reset() throw();

	virtual const CARDS &getPlayerCards() const _CONST;

	virtual const RuleSet::IRuleSet *getRuleSet() const _PURE;

	virtual const std::vector<std::string> &getPlayedOutCards() const _CONST;

	virtual std::size_t getPlayerCount() const _PURE;

	virtual std::size_t getLeftCount() const _PURE;

	virtual std::size_t getRightCount() const _PURE;

	virtual Common::ICard::SUIT getPowerSuit() const _PURE;

	virtual void setPowerSuit(Common::ICard::SUIT suit);

	virtual bool isPowerPlay() const _PURE;

	virtual void setPowerPlay(bool b);

	virtual bool hasPlayerFewCards() const _PURE;

	virtual bool hasTakenCards() const _PURE;

	virtual void setCardsTaken(bool b);

protected:
	CARDS getPossibleCards(const Common::ICardPtr &uncoveredCard,
						   const Common::ICard::SUIT *suit) const;

	// cppcheck-suppress functionConst
	void notifyCardCountChange();

	virtual void shuffleCards();

	bool isAceRoundAllowed() const;

	virtual std::size_t getTalonFactor() const _PURE;

	virtual bool nineIsEight() const _PURE;

	virtual bool isDirChgEnabled() const _PURE;

	virtual bool tryAceRound() const _PURE;

	virtual void setTryAceRound(bool b);

private:
	const std::string m_name;
	CARDS m_cards;
	mutable bool m_cardsTaken;
	const RuleSet::IRuleSet *m_ruleset;
	std::vector<std::string> m_playedOutCards;
	bool m_playerHasFewCards;
	mutable Common::ICard::SUIT m_powerSuit;
	mutable bool m_powerPlay;
	mutable bool m_tryAceRound;
	bool m_nineIsEight;
	std::size_t m_leftCount;
	std::size_t m_rightCount;
	bool m_dirChgEnabled;
	std::size_t m_playerCount;
	const EngineConfig *m_engineCfg;
	const ICardCountObserver *m_cardCountObserver;
};

}

}

#endif /* NETMAUMAU_PLAYER_STDPLAYER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
