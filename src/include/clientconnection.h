/*
 * Copyright 2014-2015 by Heiko Schäfer <heiko@rangun.de>
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
 * @brief Handles the connection from the client to a server
 * @author Heiko Schäfer <heiko@rangun.de>
 */

#ifndef NETMAUMAU_CLIENTCONNECTION_H
#define NETMAUMAU_CLIENTCONNECTION_H

#include "abstractconnection.h"

namespace NetMauMau {

namespace Client {

class IPlayerPicListener;
class ConnectionImpl;
class IBase64;

/**
 * @brief Handles the connection from the client to a server
 */
class Connection : public Common::AbstractConnection {
	DISALLOW_COPY_AND_ASSIGN(Connection)
	friend class ConnectionImpl;
public:
	using AbstractConnection::connect;

	/**
	 * @brief the score
	 */
	typedef struct {
		std::string name; ///< name of the player
		long long int score; ///< score of the player
	} SCORE;

	/**
	 * @brief a @c vector of scores
	 */
	typedef std::vector<SCORE> SCORES;

	/**
	 * @brief The type of scores
	 */
	struct SCORE_TYPE {
		/**
		 * @brief The type of scores enumeration
		 */
		enum _scoreType {
			NORM, ///< The scores are <em>normal</em>, this means they can contain negative
			/// values as well
			ABS ///< The scores are <em>absolute</em>, this means they contain only values >= 0
		};
	};

	/**
	 * @brief Holds the name as well as the PNG data of the player image
	 */
	typedef struct {
		std::string name; ///< the player name
		const unsigned char *pngData; ///< raw data of the player image, must be freed by the client
		std::size_t pngDataLen; ///< length of the raw data of the player image
	} PLAYERINFO;

	/**
	 * @brief List of currently registered player names
	 */
	typedef std::vector<std::string> PLAYERLIST;

	/**
	 * @brief List of currently registered player infos
	 */
	typedef std::vector<PLAYERINFO> PLAYERINFOS;

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic push
	typedef struct _DEPRECATED _base64RAII {
	private:
		DISALLOW_COPY_AND_ASSIGN(_base64RAII)
	public:
		explicit _base64RAII();
		explicit _base64RAII(const IBase64 *base64);
		~_base64RAII() _CONST;

		// cppcheck-suppress functionConst
		operator const IBase64 *() _CONST _NOUNUSED;

		_base64RAII &operator=(const IBase64 *) _CONST;

	private:
		const IBase64 *m_base64;
	} BASE64RAII;
#pragma GCC diagnostic pop

	explicit Connection(const std::string &pName, const std::string &server, uint16_t port);
	explicit Connection(const std::string &pName, const std::string &server, uint16_t port,
						BASE64RAII &base64);
	virtual ~Connection();

	void setClientVersion(uint32_t clientVersion);

	virtual void connect(const IPlayerPicListener *l, const unsigned char *pngData,
						 std::size_t pngDataLen) throw(Common::Exception::SocketException);
	CAPABILITIES capabilities() throw(NetMauMau::Common::Exception::SocketException);
	PLAYERINFOS playerList(const IPlayerPicListener *hdl,
						   bool playerPNG) throw(Common::Exception::SocketException);
	SCORES getScores(SCORE_TYPE::_scoreType type, std::size_t limit)
	throw(Common::Exception::SocketException);

	void setTimeout(struct timeval *timeout);

	Connection &operator>>(std::string &msg) throw(Common::Exception::SocketException);
	Connection &operator<<(const std::string &msg) throw(Common::Exception::SocketException);

protected:
	virtual bool wire(SOCKET sockfd, const struct sockaddr *addr, socklen_t addrlen) const;
	virtual std::string wireError(const std::string &err) const;

private:
	void init();

private:
	ConnectionImpl *const _pimpl;
};

_EXPORT bool operator<(const std::string &, const Connection::PLAYERINFO &) _PURE;
_EXPORT bool operator<(const Connection::PLAYERINFO &, const std::string &) _PURE;

}

}

#endif /* NETMAUMAU_CLIENTCONNECTION_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
