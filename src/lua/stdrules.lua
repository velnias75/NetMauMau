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
 
local function ternary(expr, tVal, fVal)
  if not not expr then return tVal else return fVal end
end

local function isDirChange(playedCard)
  return nmm_dirChangePossible and playedCard.RANK == RANK.NINE
end

local function isCardAcceptable(uncoveredCard, playedCard)
  
  if not (uncoveredCard and playedCard) then
    return false
  end

  return not not ternary(nmm_aceRound.ENABLED and m_aceRoundPlayer ~= nil,
    playedCard.RANK == nmm_aceRound.RANK, -- in ace rounds only nmm_aceRound.RANK is allowed
    (playedCard.RANK == RANK.JACK and uncoveredCard.RANK ~= RANK.JACK) 
      or ((((isJackMode() and getJackSuit() == playedCard.SUIT) 
        or (not isJackMode()
          and (uncoveredCard.SUIT == playedCard.SUIT or (uncoveredCard.RANK == playedCard.RANK)))))
        and not (playedCard.RANK == RANK.JACK and uncoveredCard.RANK == RANK.JACK)))
end

function checkCard(uncoveredCard, playedCard, player)
  
  local accepted = not not ternary(uncoveredCard, isCardAcceptable(uncoveredCard, playedCard), true)
  
  if player then

    do -- Check if we are in an ace round or can start an ace round
      if accepted and (nmm_aceRound.ENABLED and uncoveredCard 
        and (not m_aceRoundPlayer or m_aceRoundPlayer == player.ID) 
        and playedCard.RANK == nmm_aceRound.RANK) then

        do
          
          local acrCont = not not (m_aceRoundPlayer ~= nil)
        
          m_aceRoundPlayer = ternary(getAceRoundChoice(player.INTERFACE), player.ID, nil)

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
          ternary(uncoveredCard, uncoveredCard, playedCard), playedCard)
        m_jackMode = true
      end
    end
  end
  
  return accepted

end

function lostPointFactor(uncoveredCard)
  return ternary(uncoveredCard.RANK == RANK.JACK, 2, 1)
end

function hasToSuspend()
  return not not (m_hasToSuspend and (not m_hasSuspended))
end

function hasSuspended()
  m_hasSuspended = true
end

function takeCardCount()
  return m_takeCardCount
end

function takeCards(playedCard)
  return ternary(playedCard and (playedCard.RANK == RANK.SEVEN), 0, takeCardCount())
end

function hasTakenCards()
  m_takeCardCount = 0
end

function initialCardCount()
  return nmm_initialCardCount
end

function suspendIfNoMatchingCard()
  return false
end

function takeIfLost()
  return true
end

function isAceRoundPossible()
  return nmm_aceRound.ENABLED
end

function getAceRoundRank()
  return ternary(nmm_aceRound.ENABLED, nmm_aceRound.RANK, RANK.RANK_ILLEGAL)
end

function isAceRound()
  return not not (m_aceRoundPlayer and isAceRoundPossible())
end

function isJackMode()
  return not not m_jackMode
end

function setJackModeOff()
  m_jackMode = false
end

function hasDirChange()
  return not not m_dirChange
end

function dirChanged()
  m_dirChange = false
end

function getDirChangeIsSuspend()
  return not not m_dirChangeIsSuspend
end

function setDirChangeIsSuspend(isSuspend)
  m_dirChangeIsSuspend = isSuspend
end

function getJackSuit()
  return ternary(m_jackSuit == SUIT.SUIT_ILLEGAL, getRandomSuit(), m_jackSuit)
end

function getMaxPlayers()
  return 5
end

function setCurPlayers(num)
  m_curPlayers = num
end

function reset()
  m_hasToSuspend = false
  m_hasSuspended = false
  m_takeCardCount = 0
  m_jackMode = false
  m_jackSuit = SUIT.SUIT_ILLEGAL
  m_aceRoundPlayer = nil
  m_curPlayers = 0
  m_dirChange = false
  m_dirChangeIsSuspend = false
end

reset()

-- kate: indent-mode cstyle; indent-width 2; replace-tabs on; tab-width 2; 
