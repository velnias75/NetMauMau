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

/**
 * @file
 * @brief Common functions to handle cards
 * @author Heiko Schäfer <heiko@rangun.de>
 */

#ifndef NETMAUMAU_CARDTOOLS_H
#define NETMAUMAU_CARDTOOLS_H

#include <algorithm>
#include <cstdlib>

#include "icard.h"

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
 * @brief Get an array of the four @c SUIT symbols
 *
 * @return const std::string* the four @c SUIT symbols
 */
_EXPORT const std::string *getSuitSymbols() _CONST;

/**
 * @brief Converts a @c SUIT symbol to a ANSI color representation
 *
 * @param suit the @c SUIT symbol
 * @return std::string a @c SUIT symbol in ANSI color representation
 */
_EXPORT std::string ansiSuit(const std::string &suit);

/**
 * @brief Converts a symbol to a NetMauMau::Common::ICard::SUIT
 *
 * @param symbol the symbol
 * @return NetMauMau::Common::ICard::SUIT the @c SUIT
 */
_EXPORT NetMauMau::Common::ICard::SUIT symbolToSuit(const std::string &symbol);

/**
 * @brief Converts a NetMauMau::Common::ICard::SUIT to a symbol
 *
 * @param suit the @c SUIT to convert
 * @param ansi if @c true create a ANSI color representation
 * @param endansi if @c false (default) don't end the ANSI color codes
 * @return std::string the @c SUIT symbol
 */
_EXPORT std::string suitToSymbol(NetMauMau::Common::ICard::SUIT suit, bool ansi,
								 bool endansi = false);

/// @}

/**
 * @name Card description
 * @{
 */

/**
 * @brief Parses a textual description
 *
 * Parses a textual description and stores the suit and the rank into the pointers,
 * which cannot be null.
 *
 * @param[in] desc the textual description of the card
 * @param[out] suit pointer to store the resulting @c SUIT
 * @param[out] rank pointer to store the resulting @c RANK
 * @return bool @c true if the parsing was successful, @c false otherwise
 */
_EXPORT _NOUNUSED bool parseCardDesc(const std::string &desc, NetMauMau::Common::ICard::SUIT *suit,
									 NetMauMau::Common::ICard::RANK *rank) _NONNULL(2, 3);

/**
 * @brief Creates a card description
 *
 * @param suite the @c SUIT
 * @param rank the @c RANK
 * @param ansi if @c true create a ANSI color representation
 * @return std::string the card description
 */
_EXPORT std::string createCardDesc(NetMauMau::Common::ICard::SUIT suite,
								   NetMauMau::Common::ICard::RANK rank, bool ansi);

/// @}

/**
 * @name Sorting cards
 * @{
 */

/**
 * @brief Gets an ordinal number for a @c SUIT
 *
 * The ordinals follow the Skat and Doppelkopf order\n
 * See here: http://i-p-c-s.org/faq/suit-ranking.php
 *
 * @param suit the @c SUIT to get the ordinal for
 * @return unsigned int ordinal number for a @c SUIT
 */
_EXPORT unsigned int suitOrderPosition(NetMauMau::Common::ICard::SUIT suit) _CONST;

/**
 * @brief Gets an ordinal number for a @c RANK
 *
 * @param rank the @c RANK to get the ordinal for
 * @return unsigned int ordinal number for a @c RANK
 */
_EXPORT unsigned int rankOrderPosition(NetMauMau::Common::ICard::RANK rank) _CONST;

/**
 * @brief Checks if two cards are equal
 *
 * @param x a card
 * @param y a card
 *
 * @return bool @c true if the cards are equal, @c false otherwise
 */
_EXPORT bool cardEqual(const NetMauMau::Common::ICard *x, const NetMauMau::Common::ICard *y);

/**
 * @brief Checks if a card comes before another
 *
 * @param x a card
 * @param y a card
 *
 * @return bool @c true if @c x comes before @c y, @c false otherwise
 */
_EXPORT bool cardLess(const NetMauMau::Common::ICard *x, const NetMauMau::Common::ICard *y);

/// @}

/**
 * @name Identifying and finding cards
 * @{
 */

/**
 * @brief Checks if the a card is of @c SUIT
 *
 * @param card the card to check
 * @param suit the @c SUIT to check for
 *
 * @return bool @c true if the card is of @c SUIT
 */
_EXPORT bool isSuit(const NetMauMau::Common::ICard *card, NetMauMau::Common::ICard::SUIT suit);

/**
 * @brief Checks if the a card is of @c RANK
 *
 * @param card the card to check
 * @param rank the @c RANK to check for
 *
 * @return bool @c true if the card is of @c RANK
 */
_EXPORT bool isRank(const NetMauMau::Common::ICard *card, NetMauMau::Common::ICard::RANK rank);

/**
 * @brief Finds the first card of a given @c SUIT
 *
 * @tparam Iterator an iterator
 *
 * @param suit the @c SUIT to find
 * @param first iterator to the first card
 * @param last iterator to the last card
 * @return NetMauMau::Common::ICard* the card if found @c 0L otherwise
 */
template<typename Iterator>
typename Iterator::value_type findSuit(NetMauMau::Common::ICard::SUIT suit, Iterator first,
									   Iterator last) {

	const Iterator &f(std::find_if(first, last, std::bind2nd(std::ptr_fun(isSuit), suit)));

	return f != last ? *f : 0L;
}

/**
 * @brief Finds the first card of a given @c RANK
 *
 * @tparam Iterator an iterator
 *
 * @param rank the @c RANK to find
 * @param first iterator to the first card
 * @param last iterator to the last card
 * @return the card if found @c 0L otherwise
 */
template<typename Iterator>
typename Iterator::value_type findRank(NetMauMau::Common::ICard::RANK rank, Iterator first,
									   Iterator last) {

	const Iterator &f(std::find_if(first, last, std::bind2nd(std::ptr_fun(isRank), rank)));

	return f != last ? *f : 0L;
}

/**
 * @brief Finds the first card equal to a given card
 *
 * @tparam Iterator an iterator
 *
 * @param card the card to find
 * @param first iterator to the first card
 * @param last iterator to the last card
 * @return the card if found @c 0L otherwise
 */
template<typename Iterator>
typename Iterator::value_type findCard(typename Iterator::value_type card, Iterator first,
									   Iterator last) {

	const Iterator &f(std::find_if(first, last, std::bind2nd(std::ptr_fun(cardEqual), card)));

	return f != last ? *f : 0L;
}

/// @}

/**
 * @name Miscellaneous functions
 * @{
 */

/**
 * @brief Gets an <em>illegal card</em> card to trigger special actions
 *
 * @see NetMauMau::Client::AbstractClient::playCard()
 *
 * @return NetMauMau::Common::ICard* an <em>illegal card</em> card
 */
_EXPORT NetMauMau::Common::ICard *getIllegalCard() _CONST;

/**
 * @brief Gets the points of a @c RANK
 *
 * @param rank the @c RANK
 * @return std::size_t the points of a @c RANK
 */
_EXPORT std::size_t getCardPoints(NetMauMau::Common::ICard::RANK rank) _CONST;

/// @}

}

}

#endif /* NETMAUMAU_CARDTOOLS_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
