--[[ Copyright 2015 by Heiko Schäfer <heiko@rangun.de>

 This file is part of NetMauMau.

 NetMauMau is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as
 published by the Free Software Foundation, either version 3 of
 the License, or (at your option) any later version.

 NetMauMau is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with NetMauMau.  If not, see <http://www.gnu.org/licenses/> ]]

--- Initializes respectively resets the game rules.
-- This function will be called at every new game state and should init/reset the rule engine's
-- internal state.
function init()
  m_hasToSuspend = false
  m_hasSuspended = false
  m_takeCardCount = 0
  m_jackMode = false
  m_jackSuit = SUIT.SUIT_ILLEGAL
  m_jackSuitOrig = true
  m_aceRoundPlayer = nil
  m_curPlayers = 0
  m_dirChange = false
  m_dirChangeIsSuspend = false
end

--- Factor to multiply score of losing player.
-- If a player has lost the game, the score (i.e. the sum of his card points at hand) can
-- get multiplied by the factor returned by this function.
-- @param uncoveredCard the uncovered card at the moment the player has lost
-- @return the factor (integer)
function lostPointFactor(uncoveredCard)
  return (uncoveredCard.RANK == RANK.JACK and 2 or 1)
end

--- Determnines if a player has to suspend.
-- @return true if the player has to suspend, false otherwise (bool)
function hasToSuspend()
  return (m_hasToSuspend and (not m_hasSuspended))
end

--- The player has suspended.
function hasSuspended()
  m_hasSuspended = true
end

--- The amount of cards the player has to take.
-- @return the amount of cards the player has to take (integer)
function takeCardCount()
  return m_takeCardCount
end

--- The amount of cards the player has to take depending on card.
-- @param playedCard the card to determine the amount of cards to take
-- @return the amount of cards the player has to take (integer)
function takeCards(playedCard)
  return ((playedCard and (playedCard.RANK == RANK.SEVEN)) and 0 or takeCardCount())
end

--- The player has taken the cards.
function hasTakenCards()
  m_takeCardCount = 0
end

--- The amount of cards a player gets at game start.
-- @return the amount of cards a player gets at game start (integer)
function initialCardCount()
  return nmm_initialCardCount
end

--- Decide if a player has to suspend if no matching card can get played out.
-- @return true id the player has to suspend, false otherwise (bool)
function suspendIfNoMatchingCard()
  return false
end

--- Decide if a player has to take cards if losing.
-- Take into account the card count if a player loses (affects talon as well as score)
-- @see takeCardCount
-- @see takeCards
-- @return true, if the player has to take cards, false otherwise (bool)
function takeIfLost()
  return true
end

--- Determines if an ace round is possible.
-- @return true if an ace round is possible, false otherwise (bool)
function isAceRoundPossible()
  return nmm_aceRound.ENABLED
end

--- Gets the rank for ace rounds.
-- Ace rounds mustn't neccessarily played wit ACEs, but with any other rank
-- @return the ace round rank (RANK)
function getAceRoundRank()
  return (nmm_aceRound.ENABLED and nmm_aceRound.RANK or RANK.RANK_ILLEGAL)
end

--- Determines if the game is within an ace round.
-- @return true, if the game is within an ace round, false otherwise (bool)
function isAceRound()
  return (m_aceRoundPlayer ~=nil and isAceRoundPossible())
end

--- Determines if the game is in Jack mode.
-- @return true if the game is in Jack mode, false otherwise (bool)
function isJackMode()
  return m_jackMode
end

--- Set off the jack mode.
function setJackModeOff()
  m_jackMode = false
end

--- Determines if direction changes are possible.
-- @return true if direction changes are possible, false otherwise (bool)
function hasDirChange()
  return m_dirChange
end

--- Set direction changed.
function dirChanged()
  m_dirChange = false
end

--- Determines if a direction change is equal to suspend.
-- @return true if a direction change is equal to suspend, false otherwise (bool)
function getDirChangeIsSuspend()
  return m_dirChangeIsSuspend
end

--- Set if a direction change is equal to suspend.
-- @param isSuspend true if a direction change is equal to suspend, false otherwise (bool)
function setDirChangeIsSuspend(isSuspend)
  m_dirChangeIsSuspend = isSuspend
end

--- Determines the Jack suit.
-- @return the Jack suit (SUIT)
function getJackSuit()
  if m_jackSuitOrig then
    return getRandomSuit()
  else
    return m_jackSuit
  end
end

--- The maximum amount of players able to join the game.
-- There is a hard limit of 5 players. Any higher number will truncated to 5.
-- @return the maximum amount of players able to join the game (integer)
function getMaxPlayers()
  return 5
end

--- Set the current amount of players.
-- @param num the current amount of players
function setCurPlayers(num)
  m_curPlayers = num
end

local function isDirChange(playedCard)
  return nmm_dirChangePossible and playedCard.RANK == RANK.NINE
end

local function isCardAcceptable(uncoveredCard, playedCard)

  if not (uncoveredCard and playedCard) then
    return false
  end

  local inAceRound = (nmm_aceRound.ENABLED and m_aceRoundPlayer ~= nil)

  if inAceRound and playedCard.RANK == nmm_aceRound.RANK then
    return true
  elseif not inAceRound then
    return (playedCard.RANK == RANK.JACK and uncoveredCard.RANK ~= RANK.JACK)
          or (((isJackMode() and getJackSuit() == playedCard.SUIT)
            or (not isJackMode() and (uncoveredCard.SUIT == playedCard.SUIT
                or (uncoveredCard.RANK == playedCard.RANK))))
              and not (playedCard.RANK == RANK.JACK and uncoveredCard.RANK == RANK.JACK))
  else
    return false
  end
end

--- Checks if a card is accepted.
-- @param uncoveredCard the uncovered card (CARD)
-- @param playedCard the card the player played out (CARD)
-- @param player the player who played out the card (PLAYER or nil)
-- @return true if playedCard is accepted, false otherwise (bool)
function checkCard(uncoveredCard, playedCard, player)

  local accepted = (uncoveredCard == nil and true or isCardAcceptable(uncoveredCard, playedCard))

  if player then

    do -- Check if we are in an ace round or can start an ace round
      if accepted and (nmm_aceRound.ENABLED and uncoveredCard
        and (not m_aceRoundPlayer or m_aceRoundPlayer == player.ID)
        and playedCard.RANK == nmm_aceRound.RANK) then

        do local acrCont = (m_aceRoundPlayer ~= nil)

          m_aceRoundPlayer = (getAceRoundChoice(player.INTERFACE) and player.ID or nil)

          if m_aceRoundPlayer then
            aceRoundStarted(player.INTERFACE)
          elseif acrCont then
            aceRoundEnded(player.INTERFACE)
          end
        end
      elseif accepted and isDirChange(playedCard) then -- check if card will change direction
        m_dirChange = true
      end

      m_hasToSuspend = accepted -- does player has to suspend
        and (playedCard.RANK == RANK.EIGHT or (isDirChange(playedCard) and m_dirChangeIsSuspend))

      m_hasSuspended = false

      if accepted and playedCard.RANK == RANK.SEVEN then -- do we need to increase the take counter?
        m_takeCardCount = m_takeCardCount + 2
      elseif accepted and playedCard.RANK == RANK.JACK -- do we need to ask for a jack suit?
        and (m_curPlayers > 2 or player.CARDCOUNT > 1) then
        m_jackSuit = getJackChoice(player.INTERFACE,
          (uncoveredCard and uncoveredCard or playedCard), playedCard)
        m_jackMode = true
        m_jackSuitOrig = false
      end
    end
  end

  return accepted

end

-- kate: indent-mode cstyle; indent-width 2; replace-tabs on; tab-width 2; replace-trailing-space-save on;
