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
 * @brief Exceptions thrown in both clients and the server
 */
namespace Exception {

/**
 * @brief There have been an error in the communication between server and client
 */
class _EXPORT SocketException : public std::exception {
	SocketException &operator=(const SocketException &);
public:
	SocketException(const SocketException &o) throw();
	SocketException(const std::string &msg, int sockfd = -1, int err = 0) throw();
	virtual ~SocketException() throw();

	/**
	 * @brief A description of the error
	 *
	 * @return const char* the description of the error
	 */
	virtual const char *what() const throw() _PURE;

	/**
	 * @brief The socket on which the error occurred
	 *
	 * @return int the socket
	 */
	int sockfd() const throw() _PURE;

	int error() const throw() _PURE;

private:
	const std::string m_msg;
	const int m_sockfd;
	const int m_errno;
};

}

}

}

#endif /* NETMAUMAU_SOCKETEXCEPTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
