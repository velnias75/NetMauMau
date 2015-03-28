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

#ifndef NETMAUMAU_COMMON_ICONNECTION_H
#define NETMAUMAU_COMMON_ICONNECTION_H

#include <vector>

#include <stdint.h>

#include "socketexception.h"

/**
 * @defgroup util Utilities
 *
 * @def MAKE_VERSION(maj, min)
 * @hideinitializer
 * @ingroup util
 *
 * Computes the corresponding version number integer to an usigned 32 bit integer
 *
 * @param maj the major of the version number
 * @param min the minor of the version number
 *
 * @since 0.8
 */
#define MAKE_VERSION(maj, min) static_cast<uint32_t>((static_cast<uint16_t>(maj) << 16u) | \
		static_cast<uint16_t>(min))

/**
 * @def MAKE_VERSION_REL(maj, min, rel)
 * @hideinitializer
 * @ingroup util
 *
 * Computes the corresponding version number integer to an usigned 32 bit integer
 *
 * @note Release version differ only for security and bugfix releases which are **NEITHER**
 * adding new features **NOR** changing the protocol.
 *
 * @param maj the major of the version number
 * @param min the minor of the version number
 * @param rel the release of the version number
 *
 * @since 0.17.0
 */
#define MAKE_VERSION_REL(maj, min, rel) static_cast<uint32_t>((((maj) & 0x3FF) << 11) | \
		(((min) & 0x3FF) << 21) | ((rel) & 0x3FF) | (1UL << 31UL))

/**
 * @def VERSION_REL(v)
 * @hideinitializer
 * @ingroup util
 *
 * Gets the release number of the version as unsigned 16 bit integer
 *
 * @param v the version number
 *
 * @since 0.17.0
 */
#define VERSION_REL(v) static_cast<uint16_t>((((v) >> 31) & 1) ? ((v) & (((1 << 21) - 1) & 0x3FF)) \
		: 0)

/**
 * @def VERSION_MIN(v)
 * @hideinitializer
 * @ingroup util
 *
 * Gets the minor number of the version as unsigned 16 bit integer
 *
 * @param v the version number
 *
 * @since 0.17.0
 */
#define VERSION_MIN(v) static_cast<uint16_t>((((v) >> 31) & 1) ? ((((v) & ((1UL << 31UL) - 1)) \
		>> 21) & 0x3FF) : ((v) & ((1 << 16) - 1)))

/**
 * @def VERSION_MAJ(v)
 * @hideinitializer
 * @ingroup util
 *
 * Gets the major number of the version as unsigned 16 bit integer
 *
 * @param v the version number
 *
 * @since 0.17.0
 */
#define VERSION_MAJ(v) static_cast<uint16_t>((((v) >> 31) & 1) ? ((((v) & ((1UL << 31UL) - 1)) \
		>> 11) & 0x3FF) : ((v) >> 16))

namespace NetMauMau {

namespace Common {

/**
 * @interface IConnection
 * @brief Interface to a connection
 * @author Heiko Schäfer <heiko@rangun.de>
 * @since 0.14
 */
class IConnection {
	DISALLOW_COPY_AND_ASSIGN(IConnection)
public:
	typedef struct _EXPORT _info {
		explicit _info();
		~_info();
		SOCKET sockfd;
		std::string  name;
		std::string  host;
		uint16_t     port;
		uint16_t maj, min;
	} INFO;

	typedef struct _EXPORT _nameSockFD {
		explicit _nameSockFD();
		explicit _nameSockFD(const std::string &name, const std::string &playerPic, SOCKET sockfd,
							 uint32_t clientVersion);
		~_nameSockFD();
		std::string name;
		mutable std::string playerPic;
		SOCKET sockfd;
		uint32_t clientVersion;
	} NAMESOCKFD;

	typedef std::vector<NAMESOCKFD> PLAYERINFOS;

	virtual ~IConnection() {}

	virtual NAMESOCKFD getPlayerInfo(SOCKET sockfd) const = 0;
	virtual std::string getPlayerName(SOCKET sockfd) const = 0;
	virtual void removePlayer(SOCKET sockfd) = 0;
	virtual void addAIPlayers(const std::vector<std::string> &aiPlayers) = 0;

	virtual bool hasHumanPlayers() const = 0;

	virtual void wait(long ms) throw(Exception::SocketException) = 0;

protected:
	explicit IConnection() {}
};

}

}

#endif /* NETMAUMAU_COMMON_ICONNECTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
