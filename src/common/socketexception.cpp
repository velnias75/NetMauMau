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

#include "socketexception.h"

using namespace NetMauMau::Common::Exception;

SocketException::SocketException(const std::string &msg, SOCKET sfd, int err) throw() :
	std::exception(), m_msg(msg), m_sockfd(sfd), m_errno(err) {}

SocketException::SocketException(const SocketException &o) throw() : m_msg(o.m_msg),
	m_sockfd(o.m_sockfd), m_errno(o.m_errno) {}

SocketException::~SocketException() throw() {}

const char *SocketException::what() const throw() {
	return m_msg.c_str();
}

SOCKET SocketException::sockfd() const throw() {
	return m_sockfd;
}

int SocketException::error() const throw() {
	return m_errno;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
