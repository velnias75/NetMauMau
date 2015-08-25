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

#include "decisionbase.h"               // for DecisionBase
#include "iaction.h"                    // for IAction

#include "icondition.h"                 // for IConditionPtr

namespace NetMauMau {

namespace AI {

class AbstractAction : public IAction, protected DecisionBase {
	DISALLOW_COPY_AND_ASSIGN(AbstractAction)
public:
	virtual ~AbstractAction() throw() _CONST;

	virtual const IConditionPtr &operator()(IAIState &state) const throw();

	virtual const IConditionPtr &perform(IAIState &state,
										 const Player::IPlayer::CARDS &cards) const throw() = 0;
protected:
	typedef struct _suitCount {

		inline _suitCount() throw() : suit(Common::ICard::SUIT_ILLEGAL), count(0) {}
		inline _suitCount(Common::ICard::SUIT s, Player::IPlayer::CARDS::difference_type c) throw()
			: suit(s), count(c) {}

		inline bool operator<(const _suitCount &sc) const throw() {
			return !(count < sc.count);
		}

		inline bool operator==(Common::ICard::SUIT s) const throw() {
			return suit == s;
		}

		Common::ICard::SUIT suit;
		Player::IPlayer::CARDS::difference_type count;

	} SUITCOUNT;

	AbstractAction() throw();

	static Common::ICardPtr hasRankPath(const Common::ICardPtr &uc, Common::ICard::SUIT s,
										Common::ICard::RANK r, const Player::IPlayer::CARDS &mCards,
										bool nineIsSuspend) throw();

	static const Common::ICard::SUIT *getSuits() throw() _CONST;

	static void countSuits(SUITCOUNT *suitCount, const IAIState::PLAYEDOUTCARDS &myCards) throw();

	static Common::ICard::SUIT getMaxPlayedOffSuit(const IAIState &state,
			Player::IPlayer::CARDS::difference_type *count = 0L) throw();

	static Common::ICardPtr findRankTryAvoidSuit(NetMauMau::Common::ICard::RANK rank,
			const NetMauMau::Player::IPlayer::CARDS &cards, Common::ICard::SUIT avoidSuit) throw();

	static const IConditionPtr &getNullCondition() throw() _CONST;

private:
	template<class Iterator, class Tp>
	inline static Iterator pull_internal(Iterator first, Iterator last,
										 typename Commons::RParam<Tp>::Type arg) throw() {
		return std::partition(first, last, std::bind2nd(Common::equalTo
							  <typename Iterator::value_type, Tp>(), arg));
	}

	template<class Iterator, class Tp>
	inline static Iterator stable_pull_internal(Iterator first, Iterator last,
			typename Commons::RParam<Tp>::Type arg) throw() {
		return std::stable_partition(first, last, std::bind2nd(Common::equalTo
									 <typename Iterator::value_type, Tp>(), arg));
	}

	template<class Iterator, class Tp>
	inline static Iterator push_internal(Iterator first, Iterator last,
										 typename Commons::RParam<Tp>::Type arg) throw() {
		return std::stable_partition(first, last, std::not1(std::bind2nd(Common::equalTo
									 <typename Iterator::value_type, Tp>(), arg)));
	}

protected:
	template<class Iterator, class Tp>
	inline static Iterator pull(Iterator first, Iterator last, const Tp &arg) throw() {
		return pull_internal<Iterator, Tp>(first, last, arg);
	}

	template<class Iterator, class Tp>
	inline static Iterator stable_pull(Iterator first, Iterator last, const Tp &arg) throw() {
		return stable_pull_internal<Iterator, Tp>(first, last, arg);
	}

	template<class Iterator, class Tp>
	inline static Iterator push(Iterator first, Iterator last, const Tp &arg) throw() {
		return push_internal<Iterator, Tp>(first, last, arg);
	}

private:
	static Player::IPlayer::CARDS::iterator pullSpecialRank(Player::IPlayer::CARDS &cards,
			Common::ICard::RANK rank, bool nineIsSuspend) throw();
};

}

}

#endif /* NETMAUMAU_ENGINE_AI_ABSTRACTACTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
