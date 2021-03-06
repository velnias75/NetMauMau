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
 * @brief Common functions to handle cards
 * @author Heiko Schäfer <heiko@rangun.de>
 */

#ifndef NETMAUMAU_CARDTOOLS_H
#define NETMAUMAU_CARDTOOLS_H

#include <algorithm>

#include "tmp.h"
#include "icard.h"

#define ENSURE_PTR_VTYPE(Iterator) \
	typename Commons::check_pointer<typename \
	std::iterator_traits<Iterator>::value_type>::pointer_type

namespace Commons {

template<> struct check_pointer<NetMauMau::Common::ICardPtr> {
	typedef NetMauMau::Common::ICardPtr pointer_type;
};

}

namespace {
template<typename Tp, typename Iterator>
ENSURE_PTR_VTYPE(Iterator) __find(typename Commons::RParam<Tp>::Type arg, Iterator first,
								  Iterator last);
}

/// @brief Main namespace of %NetMauMau
namespace NetMauMau {

/**
 * @brief %Common classes and functions used by clients and server as well
 *
 * @c #include @c "cardtools.h" to use the tool functions.
 */
namespace Common {

/**
 * @name Card SUIT helpers
 * @{
 */

/**
 * @ingroup util
 * @brief Get an array of the four @c SUIT symbols
 *
 * @return the four @c SUIT symbols
 */
_EXPORT const std::string *getSuitSymbols() _CONST;

/**
 * @ingroup util
 * @brief Converts a @c SUIT symbol to a ANSI color representation
 *
 * @param suit the @c SUIT symbol
 * @return a @c SUIT symbol in ANSI color representation
 */
_EXPORT std::string ansiSuit(const std::string &suit);

/**
 * @ingroup util
 * @brief Converts a symbol to a NetMauMau::Common::ICard::SUIT
 *
 * @param symbol the symbol
 * @return the @c SUIT
 */
_EXPORT NetMauMau::Common::ICard::SUIT symbolToSuit(const std::string &symbol) _PURE;

/**
 * @ingroup util
 * @brief Converts a NetMauMau::Common::ICard::SUIT to a symbol
 *
 * @param suit the @c SUIT to convert
 * @param ansi if @c true create a ANSI color representation
 * @param endansi if @c false (default) don't end the ANSI color codes
 * @return the @c SUIT symbol
 */
_EXPORT std::string suitToSymbol(NetMauMau::Common::ICard::SUIT suit, bool ansi,
								 bool endansi = false);

/// @}

/**
 * @name Card description
 * @{
 */

/**
 * @ingroup util
 * @brief Parses a textual description
 *
 * Parses a textual description and stores the suit and the rank into the pointers,
 * which cannot be null.
 *
 * @param[in] desc the textual description of the card
 * @param[out] suit pointer to store the resulting @c SUIT
 * @param[out] rank pointer to store the resulting @c RANK
 * @return @c true if the parsing was successful, @c false otherwise
 */
_EXPORT _NOUNUSED bool parseCardDesc(const std::string &desc, NetMauMau::Common::ICard::SUIT *suit,
									 NetMauMau::Common::ICard::RANK *rank) _NONNULL(2, 3);

/**
 * @ingroup util
 * @brief Creates a card description
 *
 * @param suite the @c SUIT
 * @param rank the @c RANK
 * @param ansi if @c true create a ANSI color representation
 * @return the card description
 */
_EXPORT std::string createCardDesc(NetMauMau::Common::ICard::SUIT suite,
								   NetMauMau::Common::ICard::RANK rank, bool ansi);

/// @}

/**
 * @name Sorting cards
 * @{
 */

/**
 * @ingroup util
 * @brief Gets an ordinal number for a @c SUIT
 *
 * The ordinals follow the Skat and Doppelkopf order\n
 * See here: http://i-p-c-s.org/faq/suit-ranking.php
 *
 * @param suit the @c SUIT to get the ordinal for
 * @return ordinal number for a @c SUIT
 */
_EXPORT unsigned int suitOrderPosition(NetMauMau::Common::ICard::SUIT suit) _CONST;

/**
 * @ingroup util
 * @brief Gets an ordinal number for a @c RANK
 *
 * @param rank the @c RANK to get the ordinal for
 * @return ordinal number for a @c RANK
 */
_EXPORT unsigned int rankOrderPosition(NetMauMau::Common::ICard::RANK rank) _CONST _PURE;

/**
 * @ingroup util
 * @brief Checks if two cards are equal
 *
 * @deprecated use the generic @c NetMauMau::Common::equalTo instead
 *
 * @param lhs a card
 * @param rhs a card
 *
 * @return @c true if the cards are equal, @c false otherwise
 */
_EXPORT bool cardEqual(const NetMauMau::Common::ICard *lhs,
					   const NetMauMau::Common::ICard *rhs) _DEPRECATED;

/**
 * @ingroup util
 * @brief Checks if a card comes before another
 *
 * Useful for sorting with suit first than rank
 *
 * @depop
 *
 * @param lhs a card
 * @param rhs a card
 *
 * @return @c true if @c lhs comes before @c rhs, @c false otherwise
 */
_EXPORT bool cardLess(const NetMauMau::Common::ICard *lhs,
					  const NetMauMau::Common::ICard *rhs) _DEPRECATED;

/**
 * @ingroup util
 * @brief Checks if a card comes before another
 *
 * Useful for sorting with rank first than suit
 *
 * @depop
 *
 * @param lhs a card
 * @param rhs a card
 *
 * @return @c true if @c lhs comes before @c rhs, @c false otherwise
 *
 * @since 0.3
 */
_EXPORT bool cardGreater(const NetMauMau::Common::ICard *lhs,
						 const NetMauMau::Common::ICard *rhs) _DEPRECATED;

/// @}

/**
 * @name Functors (for use in STL-like algorithms)
 * @{
 */

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
/**
 * @ingroup util
 * @brief Generic equality functor
 *
 * @tparam CardType pointer to @c NetMauMau::Common::ICard type
 * @tparam Tp @c NetMauMau::Common::ICard::SUIT, @c NetMauMau::Common::ICard::RANK or
 * pointer to @c NetMauMau::Common::ICard type
 *
 * @since 0.21.0
 */
template<class CardType, class Tp = CardType>
struct equalTo : std::binary_function < typename Commons::check_pointer<CardType>::pointer_type,
		typename Commons::check_pointer<Tp>::pointer_type, bool > {
private:
	typedef std::binary_function < typename Commons::check_pointer<CardType>::pointer_type,
			typename Commons::check_pointer<Tp>::pointer_type, bool > bf;
public:
	inline typename bf::result_type operator()(const typename bf::first_argument_type &lhs,
			const typename bf::second_argument_type &rhs) const {
		return *lhs == *rhs;
	}
};

/**
 * @ingroup util
 *
 * @brief Functor to compare cards
 *
 * @tparam T pointer to card class type
 *
 * @relates NetMauMau::Common::ICard
 *
 * @since 0.20.2
 */
template<class T>
struct cardLessThan : std::binary_function < typename Commons::check_pointer<T>::pointer_type,
		typename Commons::check_pointer<T>::pointer_type, bool > {
private:
	typedef std::binary_function < typename Commons::check_pointer<T>::pointer_type,
			typename Commons::check_pointer<T>::pointer_type, bool > bf;
public:
	inline typename bf::result_type operator()(const typename bf::first_argument_type &lhs,
			const typename bf::second_argument_type &rhs) const {
		return (lhs && rhs) && *lhs < *rhs;
	}
};

/**
 * @ingroup util
 *
 * @brief Functor to compare cards
 *
 * @tparam T pointer to card class type
 *
 * @relates NetMauMau::Common::ICard
 *
 * @since 0.20.2
 */
template<class T>
struct cardGreaterThan : std::binary_function < typename Commons::check_pointer<T>::pointer_type,
		typename Commons::check_pointer<T>::pointer_type, bool > {
private:
	typedef std::binary_function < typename Commons::check_pointer<T>::pointer_type,
			typename Commons::check_pointer<T>::pointer_type, bool > bf;
public:
	inline typename bf::result_type operator()(const typename bf::first_argument_type &lhs,
			const typename bf::second_argument_type &rhs) const {
		return (lhs && rhs) && *lhs > *rhs;
	}
};
#pragma GCC diagnostic pop

/// @}

/**
 * @name Identifying and finding cards
 * @{
 */

/**
 * @ingroup util
 * @brief Checks if the a card is of @c SUIT
 *
 * @param card the card to check
 * @param suit the @c SUIT to check for
 *
 * @deprecated use the generic @c NetMauMau::Common::equalTo instead
 *
 * @return @c true if the card is of @c SUIT
 */
_EXPORT bool isSuit(const NetMauMau::Common::ICard *card,
					NetMauMau::Common::ICard::SUIT suit) _DEPRECATED;

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
template<class CardType>
struct equalTo<CardType, NetMauMau::Common::ICard::SUIT> :
		std::binary_function<CardType, NetMauMau::Common::ICard::SUIT, bool> {
private:
	typedef std::binary_function<CardType, NetMauMau::Common::ICard::SUIT, bool> bf;
public:
	inline typename bf::result_type operator()(const typename bf::first_argument_type &card,
			typename bf::second_argument_type suit) const {
		return card == suit;
	}
};
#pragma GCC diagnostic pop

/**
 * @ingroup util
 * @brief Checks if the a card is of @c RANK
 *
 * @param card the card to check
 * @param rank the @c RANK to check for
 *
 * @deprecated use the generic @c NetMauMau::Common::equalTo instead
 *
 * @return @c true if the card is of @c RANK
 */
_EXPORT bool isRank(const NetMauMau::Common::ICard *card,
					NetMauMau::Common::ICard::RANK rank) _DEPRECATED;

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
template<class CardType>
struct equalTo<CardType, NetMauMau::Common::ICard::RANK> :
		std::binary_function<CardType, NetMauMau::Common::ICard::RANK, bool> {
private:
	typedef std::binary_function<CardType, NetMauMau::Common::ICard::RANK, bool> bf;
public:
	inline typename bf::result_type operator()(const typename bf::first_argument_type &card,
			typename bf::second_argument_type rank) const {
		return card == rank;
	}
};
#pragma GCC diagnostic pop

/**
 * @ingroup util
 * @brief Generic find function
 *
 * Search for the first occurrence of a card
 *
 * @tparam Tp @c NetMauMau::Common::ICard::SUIT, @c NetMauMau::Common::ICard::RANK or
 * pointer to @c NetMauMau::Common::ICard type
 * @tparam Iterator iterator type to a collection of @c NetMauMau::Common::ICard type pointers
 * @param arg @c NetMauMau::Common::ICard::SUIT, @c NetMauMau::Common::ICard::RANK or
 * pointer to @c NetMauMau::Common::ICard type
 * @param first @c iterator to begin of the range to search for
 * @param last @c iterator to end of the range to search for
 *
 * @return a @c NetMauMau::Common::ICard type pointer, or @c NULL if nothing was found
 *
 * @since 0.21.0
 */
template<typename Tp, typename Iterator> inline
ENSURE_PTR_VTYPE(Iterator) find(const Tp &arg, Iterator first, Iterator last) {
	return __find<Tp, Iterator>(arg, first, last);
}

/**
 * @ingroup util
 * @brief Finds the first card of a given @c SUIT
 *
 * @tparam Iterator an iterator
 *
 * @deprecated use the generic @c NetMauMau::Common::find instead
 *
 * @param suit the @c SUIT to find
 * @param first iterator to the first card
 * @param last iterator to the last card
 * @return the card if found @c 0L otherwise
 */
template<typename Iterator> _DEPRECATED
ENSURE_PTR_VTYPE(Iterator) findSuit(NetMauMau::Common::ICard::SUIT suit, Iterator first,
									Iterator last) {
	return NetMauMau::Common::find(suit, first, last);
}

/**
 * @ingroup util
 * @brief Finds the first card of a given @c RANK
 *
 * @tparam Iterator an iterator
 *
 * @deprecated use the generic @c NetMauMau::Common::find instead
 *
 * @param rank the @c RANK to find
 * @param first iterator to the first card
 * @param last iterator to the last card
 * @return the card if found @c 0L otherwise
 */
template<typename Iterator> _DEPRECATED
ENSURE_PTR_VTYPE(Iterator) findRank(NetMauMau::Common::ICard::RANK rank, Iterator first,
									Iterator last) {
	return NetMauMau::Common::find(rank, first, last);
}

/**
 * @ingroup util
 * @brief Finds the first card equal to a given card
 *
 * @tparam Iterator an iterator
 *
 * @deprecated use the generic @c NetMauMau::Common::find instead
 *
 * @param card the card to find
 * @param first iterator to the first card
 * @param last iterator to the last card
 *
 * @return the card if found @c 0L otherwise
 */
template<typename Iterator> _DEPRECATED
ENSURE_PTR_VTYPE(Iterator) findCard(typename Iterator::value_type card, Iterator first,
									Iterator last) {
	return NetMauMau::Common::find(card, first, last);
}

/// @}

/**
 * @name Miscellaneous functions
 * @{
 */

/**
 * @ingroup util
 * @brief Gets an <em>illegal card</em> card to trigger special actions
 *
 * @see NetMauMau::Client::AbstractClient::playCard()
 *
 * @return an <em>illegal card</em> card
 */
_EXPORT NetMauMau::Common::ICard *getIllegalCard() _CONST;

/**
 * @brief Gets the points of a @c RANK
 *
 * Rank    | Points
 * ------: | :-----
 * Seven   | 1
 * Eight   | 2
 * Nine    | 3
 * Ten     | 4
 * Queen   | 5
 * King    | 6
 * Ace     | 11
 * Jack    | 20
 *
 * @see Common::ICard::getPoints
 *
 * @param rank the @c RANK
 * @return the points of a @c RANK
 */
_EXPORT std::size_t getCardPoints(NetMauMau::Common::ICard::RANK rank) _CONST _PURE;

/**
 * @ingroup util
 * @brief Structure containing the configuration of initial cards and decks
 * @since 0.19.1
 */
typedef struct _cardConfig {
	inline _cardConfig(std::size_t icc, std::size_t dks) : initialCards(icc), decks(dks) {}
	inline _cardConfig() : initialCards(5), decks(5) {}
	std::size_t initialCards; ///< the amount of initial cards each player gets at game start
	std::size_t decks; ///< the amount of card decks to play
} CARDCONFIG;

/**
 * @ingroup util
 * @brief Determines reasonable amounts of initial cards and card decks
 *
 * This function calculates reasonable values based on the amount of @c players,
 * @c initialCardCount and @c decks.
 *
 * This function can be used by clients to avoid the server adjusting the given
 * parameters.
 *
 * @param players the amount of players
 * @param initialCardCount the desired amount of initial cards
 * @param decks the desired amount of card decks
 *
 * @see CARDCONFIG
 *
 * @return structure containing the reasonable values
 *
 * @since 0.19.1
 */
_EXPORT CARDCONFIG getCardConfig(std::size_t players, std::size_t initialCardCount = 5,
								 std::size_t decks = 1) _CONST;

/**
 * @ingroup util
 * @brief Gets the executable name of the server
 *
 * @note on non-Windows systems it returns the full path to the executable
 *
 * @return the executable name of the server
 */
_EXPORT const char *getServerExe() _PURE;

/// @}

}

}

namespace {
template<typename Tp, typename Iterator>
ENSURE_PTR_VTYPE(Iterator) __find(typename Commons::RParam<Tp>::Type arg, Iterator first,
								  Iterator last) {

	const Iterator &f(std::find_if(first, last, std::bind2nd(NetMauMau::Common::equalTo < typename
								   std::iterator_traits<Iterator>::value_type, Tp > (), arg)));

	return f != last ? *f : typename std::iterator_traits<Iterator>::value_type();
}
}

/**
 * @defgroup util Utilities
 * @brief Utilities and helper functions and macros
 *
 * This are several utilities and helper functions and macros to ease the use of
 * the @em %NetMauMau @em %client @em API
 *
 */

#endif /* NETMAUMAU_CARDTOOLS_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
