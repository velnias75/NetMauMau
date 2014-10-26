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
 * @file socketexception.h
 * @author Heiko Schäfer <heiko@rangun.de>
 */

#ifndef NETMAUMAU_SOCKETEXCEPTION_H
#define NETMAUMAU_SOCKETEXCEPTION_H

#include <exception>
#include <string>

#include "linkercontrol.h"

namespace NetMauMau {

namespace Common {

/**
 * @brief tbw
 */
namespace Exception {

/**
 * @brief tbw
 */
class _EXPORT SocketException : public std::exception {
	SocketException &operator=(const SocketException &);
public:
	SocketException(const SocketException &o) throw();
	SocketException(const std::string &msg, int sockfd = -1) throw();
	virtual ~SocketException() throw();

	virtual const char *what() const throw() _PURE;
	int sockfd() const throw() _PURE;

private:
	std::string m_msg;
	int m_sockfd;
};

}

}

}

#endif /* NETMAUMAU_SOCKETEXCEPTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
