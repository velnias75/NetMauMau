function getAceRoundRank()
  return RANK.ACE
end

function isAceRound()
  return m_aceRound
end

function isAceRoundPossible()
  return m_aceRound
end

function isJackMode()
  return m_jackMode
end

function setJackModeOff()
  m_jackMode = false
end

function getJackSuit()
  return m_jackSuit
end

function dirChanged()
  m_dirChange = false
end

function hasDirChange()
  return m_dirChange
end

function getDirChangeIsSuspend()
  return m_dirChangeIsSuspend
end

function initialCardCount()
  return m_initialCardCount
end

function hasSuspended()
  m_hasSuspended = true
end

function hasToSuspend()
  return m_hasToSuspend and not m_hasSuspended
end

function takeCardCount()
  return m_takeCardCount
end

function takeCards(playedCard)
  return (playedCard ~= nil and playedCard.RANK == RANK.SEVEN) and 0 or takeCardCount()
end

function takeIfLost() {
  return true
}

function lostPointFactor(uncoveredCard)
  return uncoveredCard.RANK == RANK.JACK and 2 or 1
end

function hasTakenCards()
  m_takeCardCount = 0
end

function checkCard(uncoveredCard, playedCard, player)
  
  if(uncoveredCard == nil or playedCard == nil) then
	return false
  end

  local accepted = (m_aceRound and m_aceRoundPlayer) and (playedCard.RANK == getAceRoundRank()) or
		 ((playedCard.RANK == RANK.JACK and uncoveredCard.RANK ~= RANK.JACK) or
		  ((((isJackMode() and getJackSuit() == playedCard.SUIT) or
			 ((not isJackMode()) and (uncoveredCard.SUIT == playedCard.SUIT or
								(uncoveredCard.RANK == playedCard.RANK))))) and
		   not (playedCard.RANK == RANK.JACK and uncoveredCard.RANK == RANK.JACK)))
  
  if player == nil then 
	return accepted
  elseif(accepted and (m_aceRound and (m_aceRoundPlayer == nil or m_aceRoundPlayer == player) 
	and playedCard.RANK == getAceRoundRank())) then

	local acrCont = m_aceRoundPlayer ~= nil
	m_aceRoundPlayer = getAceRoundChoice(player.INTERFACE) and player or nil

	if(m_aceRoundPlayer ~= nil) then
	  --m_arl->aceRoundStarted(player);
	elseif(acrCont) then
	  --m_arl->aceRoundEnded(player);
	end
  elseif(accepted and isDirChange(playedCard)) then
	m_dirChange = true
  end

  m_hasToSuspend = accepted and (playedCard.RANK == RANK.EIGHT or (isDirChange(playedCard) 
	and m_dirChangeIsSuspend))

  m_hasSuspended = false

  if(accepted and playedCard.RANK == RANK.SEVEN) then
	m_takeCardCount = m_takeCardCount + 2
  elseif(accepted and playedCard.RANK == RANK.JACK 
	and (m_curPlayers > 2 or player.CARDCOUNT > 1)) then
	m_jackSuit = getJackChoice(player.INTERFACE, uncoveredCard, playedCard)
	m_jackMode = true
  end

  return accepted
end

function setCurPlayers(num)
  m_curPlayers = num
end

function getMaxPlayers()
  return 5
end

function reset()
  m_curPlayers = 0
  m_hasToSuspend = false
  m_hasSuspended = false
  m_takeCardCount = 0
  m_jackMode = false
  m_jackSuit = SUIT.SUIT_ILLEGAL
  m_aceRound = false
  m_aceRoundPlayer = false
  m_dirChange = false
  m_dirChangeIsSuspend = false
  m_dirChangePossible = true
  m_initialCardCount = 5
end

function isDirChange(playedCard)
  return m_dirChangePossible and playedCard.RANK == RANK.NINE
end

reset()
