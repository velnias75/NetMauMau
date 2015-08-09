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

#include "protocol.h"

namespace NetMauMau {

namespace Common {

namespace Protocol {

namespace V15 {

const std::string ERR_TO_EXC_PLAYER _INIT_PRIO(101) = "Error in communication with player ";
const std::string ERR_TO_EXC_UNKNOWN _INIT_PRIO(101) = "unknown error";
const std::string ERR_TO_EXC_LOSTCONN _INIT_PRIO(101) = "Lost connection to a player.";
const std::string ERR_TO_EXC_LOSTCONNNAMED _INIT_PRIO(101) = "Lost connection to player ";
const std::string ERR_TO_EXC_SHUTDOWNMSG _INIT_PRIO(101) = "The server has been shut down";
const std::string ERR_TO_EXC_MISCONFIGURED _INIT_PRIO(101) =
	"Misconfigured or compromised server. Please report: ";

const std::string PLAYERLIST _INIT_PRIO(101) = "PLAYERLIST";
const std::string PLAYERLISTEND _INIT_PRIO(101) = "PLAYERLISTEND";
const std::string SCORES _INIT_PRIO(101) = "SCORES";
const std::string SCORESEND _INIT_PRIO(101) = "SCORESEND";
const std::string CAP _INIT_PRIO(101) = "CAP";
const std::string CAPEND _INIT_PRIO(101) = "CAPEND";

const std::string ACEROUND _INIT_PRIO(101) = "ACEROUND";
const std::string ACEROUNDENDED _INIT_PRIO(101) = "ACEROUNDENDED";
const std::string ACEROUNDSTARTED _INIT_PRIO(101) = "ACEROUNDSTARTED";
const std::string BYE _INIT_PRIO(101) = "BYE";
const std::string CARDACCEPTED _INIT_PRIO(101) = "CARDACCEPTED";
const std::string CARDCOUNT _INIT_PRIO(101) = "CARDCOUNT";
const std::string CARDREJECTED _INIT_PRIO(101) = "CARDREJECTED";
const std::string CARDSGOT _INIT_PRIO(101) = "CARDSGOT";
const std::string CARDTAKEN _INIT_PRIO(101) = "CARDTAKEN";
const std::string DIRCHANGE _INIT_PRIO(101) = "DIRCHANGE";
const std::string ENDSTATS _INIT_PRIO(101) = "ENDSTATS";
const std::string ERROR _INIT_PRIO(101) = "ERROR";
const std::string FALSE _INIT_PRIO(101) = "FALSE";
const std::string GETCARDS _INIT_PRIO(101) = "GETCARDS";
const std::string HIDDENCARDTAKEN _INIT_PRIO(101) = "HIDDENCARDTAKEN";
const std::string INITIALCARD _INIT_PRIO(101) = "INITIALCARD";
const std::string JACKCHOICE _INIT_PRIO(101) = "JACKCHOICE";
const std::string JACKMODEOFF _INIT_PRIO(101) = "JACKMODEOFF";
const std::string JACKSUIT _INIT_PRIO(101) = "JACKSUIT";
const std::string MESSAGE _INIT_PRIO(101) = "MESSAGE";
const std::string NEXTPLAYER _INIT_PRIO(101) = "NEXTPLAYER";
const std::string OFF _INIT_PRIO(101) = "OFF";
const std::string ON _INIT_PRIO(101) = "ON";
const std::string OPENCARD _INIT_PRIO(101) = "OPENCARD";
const std::string PLAYCARDEND _INIT_PRIO(101) = "PLAYCARDEND";
const std::string PLAYCARD _INIT_PRIO(101) = "PLAYCARD";
const std::string PLAYEDCARD _INIT_PRIO(101) = "PLAYEDCARD";
const std::string PLAYERJOINED _INIT_PRIO(101) = "PLAYERJOINED";
const std::string PLAYERLOST _INIT_PRIO(101) = "PLAYERLOST";
const std::string PLAYERPICKSCARD _INIT_PRIO(101) = "PLAYERPICKSCARD";
const std::string PLAYERPICKSCARDS _INIT_PRIO(101) = "PLAYERPICKSCARDS";
const std::string PLAYERREJECTED _INIT_PRIO(101) = "PLAYERREJECTED";
const std::string PLAYERWINS _INIT_PRIO(101) = "PLAYERWINS";
const std::string STATS _INIT_PRIO(101) = "STATS";
const std::string SUSPENDS _INIT_PRIO(101) = "SUSPENDS";
const std::string SUSPEND _INIT_PRIO(101) = "SUSPEND";
const std::string TAKECOUNT _INIT_PRIO(101) = "TAKECOUNT";
const std::string TALONSHUFFLED _INIT_PRIO(101) = "TALONSHUFFLED";
const std::string TRUE _INIT_PRIO(101) = "TRUE";
const std::string TURN _INIT_PRIO(101) = "TURN";
const std::string VM_ADDPIC _INIT_PRIO(101) = "VM_ADDPIC";

}

}

}

}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
