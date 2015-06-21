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

#include <algorithm>

#include "abstractconnectionimpl.h"

#include "ci_char_traits.h"

namespace {
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic push
struct _isInfo : std::binary_function < NetMauMau::Common::AbstractConnection::NAMESOCKFD,
		NetMauMau::Common::AbstractConnection::INFO, bool > {
	result_type operator()(const first_argument_type &nsd, const second_argument_type &info) const {
		return nsd.name == info.name;
	}
};

struct _isSocketFD :
		std::binary_function<NetMauMau::Common::IConnection::NAMESOCKFD, SOCKET, bool> {
	result_type operator()(const first_argument_type &nsd, second_argument_type sockfd) const {
		return nsd.sockfd == sockfd;
	}
};

struct _makeCIVector :
		std::unary_function<NetMauMau::Common::IConnection::PLAYERNAMES::value_type, void> {

	typedef std::vector<NetMauMau::Common::ci_string> CI_VECTOR;

	inline explicit _makeCIVector(NetMauMau::Common::IConnection::PLAYERNAMES::size_type s)
		: m_ciVector() {
		m_ciVector.reserve(s);
	}

	inline result_type operator()(const argument_type &str) {
		m_ciVector.push_back(str.c_str());
	}

	inline CI_VECTOR getCIVector() const {
		return m_ciVector;
	}

private:
	CI_VECTOR m_ciVector;
};
#pragma GCC diagnostic pop
}

using namespace NetMauMau::Common;

AbstractConnectionImpl::AbstractConnectionImpl() : m_registeredPlayers(), m_aiPlayers() {}

AbstractConnectionImpl::~AbstractConnectionImpl() {}

IConnection::PLAYERINFOS::const_iterator AbstractConnectionImpl::findBySocket(SOCKET sockfd) const {
	return std::find_if(m_registeredPlayers.begin(), m_registeredPlayers.end(),
						std::bind2nd(_isSocketFD(), sockfd));
}

IConnection::PLAYERINFOS::iterator AbstractConnectionImpl::findBySocket(SOCKET sockfd) {
	return std::find_if(m_registeredPlayers.begin(), m_registeredPlayers.end(),
						std::bind2nd(_isSocketFD(), sockfd));
}

IConnection::NAMESOCKFD AbstractConnectionImpl::getPlayerInfo(SOCKET sockfd) const {

	const IConnection::PLAYERINFOS::const_iterator &f(findBySocket(sockfd));

	return f != m_registeredPlayers.end() ? *f : IConnection::NAMESOCKFD();
}

bool AbstractConnectionImpl::registerPlayer(const IConnection::NAMESOCKFD &nfd,
		const IConnection::PLAYERNAMES &ai) {

	if(getPlayerName(nfd.sockfd).empty()) {

		const bool aiEmpty = ai.empty();

		const _makeCIVector::CI_VECTOR &ciai(aiEmpty ? _makeCIVector::CI_VECTOR() :
											 std::for_each(ai.begin(), ai.end(),
													 _makeCIVector(ai.size())).getCIVector());

		if(aiEmpty || std::find(ciai.begin(), ciai.end(), nfd.name.c_str()) == ciai.end()) {
			m_registeredPlayers.push_back(nfd);
			return true;
		}
	}

	return false;
}

void AbstractConnectionImpl::removePlayer(const IConnection::INFO &info) {
	removePlayer(m_registeredPlayers, std::find_if(m_registeredPlayers.begin(),
				 m_registeredPlayers.end(), std::bind2nd(_isInfo(), info)));
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
