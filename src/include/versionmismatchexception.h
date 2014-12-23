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

#ifndef NETMAUMAU_VERSIONMISMATCHEXCEPTION_H
#define NETMAUMAU_VERSIONMISMATCHEXCEPTION_H

#include <stdint.h>

#include "socketexception.h"

namespace NetMauMau {

namespace Client {

namespace Exception {

/**
 * @brief The server version is not supported by the client
 * @since 0.4
 */
class _EXPORT VersionMismatchException : public Common::Exception::SocketException {
	VersionMismatchException &operator=(const VersionMismatchException &);
public:
	VersionMismatchException(const VersionMismatchException &o) throw();
	VersionMismatchException(uint32_t serverVersion, uint32_t clientVersion,
							 SOCKET sockfd = INVALID_SOCKET) throw();
	virtual ~VersionMismatchException() throw();

	virtual const char *what() const throw() _PURE;

	/**
	 * @brief Gets the server version
	 *
	 * @return the server version
	 */
	uint32_t getServerVersion() const throw() _PURE;

	/**
	 * @brief Gets the client version
	 *
	 * @return the client version
	 */
	uint32_t getClientVersion() const throw() _PURE;

private:
	std::string m_vMsg;
	const uint32_t m_serverVersion;
	const uint32_t m_clientVersion;
};

}

}

}

#endif /* NETMAUMAU_VERSIONMISMATCHEXCEPTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
