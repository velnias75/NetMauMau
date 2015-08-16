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

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"                     // for MAXPICBYTES, etc
#endif

#include "errorstring.h"

#include <cerrno>                       // for errno
#include <cstring>                      // for strerror

#ifdef HAVE_NETDB_H
#include <netdb.h>                      // for NI_MAXHOST, NI_MAXSERV, etc
#endif

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#endif

#ifdef _WIN32
namespace {
TCHAR buffer[1024];
}
#endif

const char *NetMauMau::Common::errorString() throw() {
#ifndef _WIN32
	return errorString(errno);
#else
	return errorString(WSAGetLastError());
#endif
}

const char *NetMauMau::Common::errorString(int errnum, bool gai) throw() {
#ifndef _WIN32
#ifdef HAVE_NETDB_H
	return !gai ? std::strerror(errnum) : gai_strerror(errnum);
#else
	return std::strerror(errnum);
#endif
#else

	if(!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
					  errnum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, sizeof(buffer),
					  NULL)) {
		return std::strerror(errnum);
	}

	return buffer;

#endif
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
