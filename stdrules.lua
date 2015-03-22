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

function getMaxPlayers()
  return 5
end

function setCurPlayers(num)
  m_curPlayers = num
end

local function isDirChange(playedCard)
  return nmm_dirChangePossible and playedCard.RANK == RANK.NINE
end

local function isCardAcceptable(uncoveredCard, playedCard)
  
  if(uncoveredCard == nil or playedCard == nil) then
	return false
  end

  return (nmm_aceRound.ENABLED and m_aceRoundPlayer) and (playedCard.RANK == getAceRoundRank()) or
		 ((playedCard.RANK == RANK.JACK and uncoveredCard.RANK ~= RANK.JACK) or
		  ((((isJackMode() and getJackSuit() == playedCard.SUIT) or
			 ((not isJackMode()) and (uncoveredCard.SUIT == playedCard.SUIT or
								(uncoveredCard.RANK == playedCard.RANK))))) and
		   not (playedCard.RANK == RANK.JACK and uncoveredCard.RANK == RANK.JACK)))
end

function checkCard(uncoveredCard, playedCard, player)

  if player == nil then
	return isCardAcceptable(uncoveredCard, playedCard)
  end
  
  local accepted = uncoveredCard ~= nil and isCardAcceptable(uncoveredCard, playedCard) or true
  
  if(accepted and (nmm_aceRound.ENABLED and uncoveredCard ~= nil and (m_aceRoundPlayer == nil 
	or m_aceRoundPlayer == player) and playedCard.RANK == getAceRoundRank())) then

	local acrCont = m_aceRoundPlayer ~= nil

	m_aceRoundPlayer = getAceRoundChoice(player.INTERFACE) and player or nil

	if(m_aceRoundPlayer ~= nil) then
	  aceRoundStarted(player.INTERFACE);
	elseif(acrCont) then
	  aceRoundEnded(player.INTERFACE);
	end
  elseif(accepted and isDirChange(playedCard)) then
	m_dirChange = true
  end

  m_hasToSuspend = accepted and (playedCard.RANK == RANK.EIGHT or (isDirChange(playedCard) 
	and m_dirChangeIsSuspend))

  m_hasSuspended = false

  if(accepted and playedCard.RANK == RANK.SEVEN) then
	m_takeCardCount = m_takeCardCount + 2
  elseif(accepted and playedCard.RANK == RANK.JACK and (m_curPlayers > 2 
	or player.CARDCOUNT > 1)) then
	m_jackSuit = getJackChoice(player.INTERFACE, 
	  uncoveredCard ~= nil and uncoveredCard or playedCard, playedCard)
	m_jackMode = true
  end

  return accepted
end

function lostPointFactor(uncoveredCard)
  return uncoveredCard.RANK == RANK.JACK and 2 or 1
end

function hasToSuspend()
  return m_hasToSuspend and not m_hasSuspended
end

function hasSuspended()
  m_hasSuspended = true
end

function takeCardCount()
  return m_takeCardCount
end

function takeCards(playedCard)
  return (playedCard ~= nil and playedCard.RANK == RANK.SEVEN) and 0 or takeCardCount()
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
  return nmm_aceRound.ENABLED and nmm_aceRound.RANK or RANK.RANK_ILLEGAL
end

function isAceRound()
  return m_aceRoundPlayer ~= nil and isAceRoundPossible()
end

function isJackMode()
  return m_jackMode
end

function setJackModeOff()
  m_jackMode = false
end

function hasDirChange()
  return m_dirChange
end

function dirChanged()
  m_dirChange = false
end

function getDirChangeIsSuspend()
  return m_dirChangeIsSuspend
end

function setDirChangeIsSuspend(isSuspend)
  m_dirChangeIsSuspend = isSuspend
end

function getJackSuit()
  return m_jackSuit == SUIT.SUIT_ILLEGAL and getRandomSuit() and m_jackSuit
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
