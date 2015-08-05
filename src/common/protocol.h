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

#ifndef NETMAUMAU_COMMON_PROTOCOL_H
#define NETMAUMAU_COMMON_PROTOCOL_H

#include <string>

#include "linkercontrol.h"

#if defined(_WIN32)
#undef TRUE
#undef FALSE
#undef ERROR
#endif

namespace NetMauMau {

namespace Common {

namespace Protocol {

namespace V15 _EXPORT {

extern const std::string ERR_TO_EXC_UNKNOWN;
extern const std::string ERR_TO_EXC_LOSTCONN;
extern const std::string ERR_TO_EXC_SHUTDOWNMSG;
extern const std::string ERR_TO_EXC_LOSTCONNNAMED;
extern const std::string ERR_TO_EXC_MISCONFIGURED;

extern const std::string PLAYERLIST;
extern const std::string PLAYERLISTEND;
extern const std::string SCORES;
extern const std::string SCORESEND;
extern const std::string CAP;
extern const std::string CAPEND;

extern const std::string ACEROUND;
extern const std::string ACEROUNDENDED;
extern const std::string ACEROUNDSTARTED;
extern const std::string BYE;
extern const std::string CARDACCEPTED;
extern const std::string CARDCOUNT;
extern const std::string CARDREJECTED;
extern const std::string CARDSGOT;
extern const std::string CARDTAKEN;
extern const std::string DIRCHANGE;
extern const std::string ENDSTATS;
extern const std::string ERROR;
extern const std::string FALSE;
extern const std::string GETCARDS;
extern const std::string HIDDENCARDTAKEN;
extern const std::string INITIALCARD;
extern const std::string JACKCHOICE;
extern const std::string JACKMODEOFF;
extern const std::string JACKSUIT;
extern const std::string MESSAGE;
extern const std::string NEXTPLAYER;
extern const std::string OFF;
extern const std::string ON;
extern const std::string OPENCARD;
extern const std::string PLAYCARD;
extern const std::string PLAYCARDEND;
extern const std::string PLAYEDCARD;
extern const std::string PLAYERJOINED;
extern const std::string PLAYERLOST;
extern const std::string PLAYERPICKSCARD;
extern const std::string PLAYERPICKSCARDS;
extern const std::string PLAYERREJECTED;
extern const std::string PLAYERWINS;
extern const std::string STATS;
extern const std::string SUSPEND;
extern const std::string SUSPENDS;
extern const std::string TAKECOUNT;
extern const std::string TALONSHUFFLED;
extern const std::string TRUE;
extern const std::string TURN;
extern const std::string VM_ADDPIC;

}

}

}

}

#endif /* NETMAUMAU_COMMON_PROTOCOL_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
