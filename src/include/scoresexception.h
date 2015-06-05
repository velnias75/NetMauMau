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

/**
 * @file
 * @author Heiko Schäfer <heiko@rangun.de>
 */

#ifndef NETMAUMAU_SCORESEXCEPTION_H
#define NETMAUMAU_SCORESEXCEPTION_H

#include "socketexception.h"

namespace NetMauMau {

namespace Client {

namespace Exception {

/**
 * @ingroup exceptions
 * @brief Failure while retrieving scores
 * @since 0.9
 */
class _EXPORT ScoresException : public Common::Exception::SocketException {
	ScoresException &operator=(const ScoresException &);
public:
	ScoresException(const ScoresException &o) throw();
	explicit ScoresException(const std::string &msg, SOCKET sockfd = INVALID_SOCKET) throw();
	virtual ~ScoresException() throw();
};

}

}

}

#endif /* NETMAUMAU_SCORESEXCEPTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
