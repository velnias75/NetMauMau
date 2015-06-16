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

#include <sstream>

#include "versionmismatchexception.h"

using namespace NetMauMau::Client::Exception;

VersionMismatchException::VersionMismatchException(uint32_t serverVersion, uint32_t clientVersion,
		SOCKET sfd) throw() : SocketException("", sfd), m_vMsg(), m_serverVersion(serverVersion),
	m_clientVersion(clientVersion) {

	std::ostringstream os;

	os << "Client (version "
	   << static_cast<uint16_t>(m_clientVersion >> 16) << "."
	   << static_cast<uint16_t>(m_clientVersion)
	   << ") not supported. Server wants at least version "
	   << static_cast<uint16_t>(m_serverVersion >> 16) << "."
	   << static_cast<uint16_t>(m_serverVersion);

	os.str().swap(m_vMsg);
}

VersionMismatchException::~VersionMismatchException() throw() {}

const char *VersionMismatchException::what() const throw() {
	return m_vMsg.c_str();
}

uint32_t VersionMismatchException::getClientVersion() const throw() {
	return m_clientVersion;
}

uint32_t VersionMismatchException::getServerVersion() const throw() {
	return m_serverVersion;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
