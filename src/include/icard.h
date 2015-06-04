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

/**
 * @file
 * @brief Describes a playing card
 * @author Heiko Schäfer <heiko@rangun.de>
 */

#ifndef NETMAUMAU_ICARD_H
#define NETMAUMAU_ICARD_H

#include <string>

#include "linkercontrol.h"

namespace NetMauMau {

namespace Common {

/**
 * @interface ICard
 * @brief Describes a playing card
 */
class _EXPORT ICard {
	DISALLOW_COPY_AND_ASSIGN(ICard)
public:
	/**
	 * @brief The suit of the card
	 */
	typedef enum { DIAMONDS, ///< &diams;
				   HEARTS, ///< &hearts;
				   SPADES, ///< &spades;
				   CLUBS, ///< &clubs;
				   SUIT_ILLEGAL ///< used to send a surely not accepted card
				 } SUIT;

	/**
	 * @brief The rank of the card
	 */
	typedef enum { SEVEN = 7, ///< 7
				   EIGHT = 8, ///< 8
				   NINE = 9, ///< 9
				   TEN = 10, ///< 10
				   JACK, ///< Jack
				   QUEEN, ///< Queen
				   KING, ///< King
				   ACE, ///< Ace
				   RANK_ILLEGAL ///< used to send a surely not accepted card
				 } RANK;

	virtual ~ICard() {}

	/**
	 * @brief Gets the suit of the card
	 *
	 * @return the suit of the card
	 */
	virtual SUIT getSuit() const = 0;

	/**
	 * @brief Gets the rank of the card
	 *
	 * @return the rank of the card
	 */
	virtual RANK getRank() const = 0;

	/**
	 * @brief Gets the points of the card
	 * @see Common::getCardPoints
	 * @return the points of the card
	 */
	virtual std::size_t getPoints() const = 0;

	/**
	 * @brief Returns the textual description of the card
	 * @see NetMauMau::Common::parseCardDesc
	 * @param ansi @c true if ANSI color code should be used, @c false otherwise
	 * @return the textual description of the card
	 */
	virtual const std::string &description(bool ansi = false) const = 0;

protected:
	explicit ICard() {}
};

template<class> class SmartPtr;

typedef SmartPtr<ICard> ICardPtr;

}

}

/**
 * @ingroup util
 *
 * @brief Compares a card against a rank
 *
 * @relates NetMauMau::Common::ICard
 *
 * @param card card to compare to
 * @param rank rank to compare to
 *
 * @return @c true if equal, @c false otherwise
 *
 * @since 0.20.2
 */
inline bool operator==(const NetMauMau::Common::ICard &card, NetMauMau::Common::ICard::RANK rank) {
	return card.getRank() == rank;
}

/**
 * @ingroup util
 *
 * @brief Compares a card against a rank
 *
 * @relates NetMauMau::Common::ICard
 *
 * @param card card to compare to
 * @param rank rank to compare to
 *
 * @return @c false if equal, @c true otherwise
 *
 * @since 0.20.2
 */
inline bool operator!=(const NetMauMau::Common::ICard &card, NetMauMau::Common::ICard::RANK rank) {
	return card.getRank() != rank;
}

/**
 * @ingroup util
 *
 * @brief Compares a card against a suit
 *
 * @relates NetMauMau::Common::ICard
 *
 * @param card card to compare to
 * @param suit suit to compare to
 *
 * @return @c true if equal, @c false otherwise
 *
 * @since 0.20.2
 */
inline bool operator==(const NetMauMau::Common::ICard &card, NetMauMau::Common::ICard::SUIT suit) {
	return card.getSuit() == suit;
}

/**
 * @ingroup util
 *
 * @brief Compares a card against a suit
 *
 * @relates NetMauMau::Common::ICard
 *
 * @param card card to compare to
 * @param suit suit to compare to
 *
 * @return @c false if equal, @c true otherwise
 *
 * @since 0.20.2
 */
inline bool operator!=(const NetMauMau::Common::ICard &card, NetMauMau::Common::ICard::SUIT suit) {
	return card.getSuit() != suit;
}

/**
 * @ingroup util
 *
 * @brief Compares cards
 *
 * @relates NetMauMau::Common::ICard
 *
 * @param lhs left card
 * @param rhs right card
 *
 * @return @c true if equal, @c false otherwise
 *
 * @since 0.20.2
 */
inline bool operator==(const NetMauMau::Common::ICard &lhs, const NetMauMau::Common::ICard &rhs) {
	return !(lhs != rhs.getRank() || lhs != rhs.getSuit());
}

/**
 * @ingroup util
 *
 * @brief Compares cards
 *
 * @relates NetMauMau::Common::ICard
 *
 * @param lhs left card
 * @param rhs right card
 *
 * @return @c false if equal, @c true otherwise
 *
 * @since 0.20.2
 */
inline bool operator!=(const NetMauMau::Common::ICard &lhs, const NetMauMau::Common::ICard &rhs) {
	return !(lhs == rhs);
}

/**
 * @ingroup util
 *
 * @brief Compares cards
 *
 * @relates NetMauMau::Common::ICard
 *
 * @param lhs left card
 * @param rhs right card
 *
 * @return @c true if lesser, @c false otherwise
 *
 * @since 0.20.2
 */
_EXPORT bool operator<(const NetMauMau::Common::ICard &lhs, const NetMauMau::Common::ICard &rhs);

/**
 * @ingroup util
 *
 * @brief Compares cards
 *
 * @relates NetMauMau::Common::ICard
 *
 * @param lhs left card
 * @param rhs right card
 *
 * @return @c true if greater, @c false otherwise
 *
 * @since 0.20.2
 */
_EXPORT bool operator>(const NetMauMau::Common::ICard &lhs, const NetMauMau::Common::ICard &rhs);

/**
 * @ingroup util
 *
 * @brief Compares a card against a rank
 *
 * @relates NetMauMau::Common::ICard
 *
 * @param card card to compare to
 * @param rank rank to compare to
 *
 * @return @c true if equal, @c false otherwise
 *
 * @since 0.20.2
 */
inline bool operator==(const NetMauMau::Common::ICard *card, NetMauMau::Common::ICard::RANK rank) {
	return *card == rank;
}

/**
 * @ingroup util
 *
 * @brief Compares a card against a rank
 *
 * @relates NetMauMau::Common::ICard
 *
 * @param card card to compare to
 * @param rank rank to compare to
 *
 * @return @c false if equal, @c true otherwise
 *
 * @since 0.20.2
 */
inline bool operator!=(const NetMauMau::Common::ICard *card, NetMauMau::Common::ICard::RANK rank) {
	return *card != rank;
}

/**
 * @ingroup util
 *
 * @brief Compares a card against a suit
 *
 * @relates NetMauMau::Common::ICard
 *
 * @param card card to compare to
 * @param suit suit to compare to
 *
 * @return @c true if equal, @c false otherwise
 *
 * @since 0.20.2
 */
inline bool operator==(const NetMauMau::Common::ICard *card, NetMauMau::Common::ICard::SUIT suit) {
	return *card == suit;
}

/**
 * @ingroup util
 *
 * @brief Compares a card against a suit
 *
 * @relates NetMauMau::Common::ICard
 *
 * @param card card to compare to
 * @param suit suit to compare to
 *
 * @return @c false if equal, @c true otherwise
 *
 * @since 0.20.2
 */
inline bool operator!=(const NetMauMau::Common::ICard *card, NetMauMau::Common::ICard::SUIT suit) {
	return *card != suit;
}

_EXPORT bool operator==(const NetMauMau::Common::ICardPtr &x, const NetMauMau::Common::ICardPtr &y);
_EXPORT bool operator!=(const NetMauMau::Common::ICardPtr &x, const NetMauMau::Common::ICardPtr &y);
_EXPORT bool operator==(const NetMauMau::Common::ICardPtr &x, NetMauMau::Common::ICard::RANK y);
_EXPORT bool operator!=(const NetMauMau::Common::ICardPtr &x, NetMauMau::Common::ICard::RANK y);
_EXPORT bool operator==(const NetMauMau::Common::ICardPtr &x, NetMauMau::Common::ICard::SUIT y);
_EXPORT bool operator!=(const NetMauMau::Common::ICardPtr &x, NetMauMau::Common::ICard::SUIT y);

#endif /* NETMAUMAU_ICARD_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
