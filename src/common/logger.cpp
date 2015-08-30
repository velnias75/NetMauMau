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
#include "config.h"
#endif

#include "logger.h"
#include <iostream>                     // for ostreambuf_iterator, cerr
#include <stdbool.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif

#ifdef HAVE_SYSLOG_H
#undef LOG_DEBUG
#include <syslog.h>                     // for closelog, openlog, syslog, etc
#endif

#ifndef _WIN32
#define TIDTYPE long int
#else
#include <windows.h>
#endif

namespace {
const std::ios_base::Init _INIT_PRIO(101) __avoid_seg_fault;
const std::ostreambuf_iterator<LOG_CHAR> out _INIT_PRIO(102) =
	std::ostreambuf_iterator<LOG_CHAR>(std::cerr);

#if !((defined(HAVE_UNISTD_H) && defined(HAVE_SYS_SYSCALL_H) && defined(SYS_gettid)) \
	|| defined(_WIN32))
volatile std::size_t rotatingLogBufSelect = 1u;
#else
volatile TIDTYPE tidMap[NetMauMau::Common::Logger::BUFCNT];
volatile std::size_t tidPtr = 0u;
#endif
}

using namespace NetMauMau::Common;

std::size_t NetMauMau::Common::nextLogBuf() {
#if (defined(HAVE_UNISTD_H) && defined(HAVE_SYS_SYSCALL_H) && defined(SYS_gettid)) \
	|| defined(_WIN32)

#ifndef _WIN32
	const TIDTYPE tid = syscall(SYS_gettid);
#else
	const TIDTYPE tid = GetCurrentThreadId();
#endif

	for(std::size_t tti = 0u; tti < NetMauMau::Common::Logger::BUFCNT;
			++tti) if(tidMap[tti] == tid) return std::max<std::size_t>(1u, tti);

	tidPtr = tidPtr < NetMauMau::Common::Logger::BUFCNT ? tidPtr : 0u;

	tidMap[tidPtr++] = tid;

	const std::size_t aux(tidPtr);

	return std::min<std::size_t>(aux, NetMauMau::Common::Logger::BUFCNT - 1u);

#elif GCC_VERSION >= 41000
	return __sync_bool_compare_and_swap(&rotatingLogBufSelect,
										(NetMauMau::Common::Logger::BUFCNT - 1u), 1u) ?
		   rotatingLogBufSelect : __sync_add_and_fetch(&rotatingLogBufSelect, 1u);
#else
	return rotatingLogBufSelect == (NetMauMau::Common::Logger::BUFCNT - 1u) ? 1u :
		   ++rotatingLogBufSelect;
#endif
}

#ifndef _WIN32
bool Logger::m_writeSyslog = false;
#else
// on Win32 we don't write to syslog, but we flush std::cerr after every log
bool Logger::m_writeSyslog = true;
#endif

Logger::Logger(const LEVEL &lvl) : Commons::IPostLogger < std::ostreambuf_iterator<LOG_CHAR>,
	LOGBUFS > (), Commons::BasicLogger<std::ostreambuf_iterator<LOG_CHAR>, LOGBUFS>(out, lvl,
			m_writeSyslog ? this : 0L) {

#ifdef HAVE_SYSLOG_H

	if(m_writeSyslog) openlog(PACKAGE_NAME, LOG_PID, LOG_DAEMON);

#endif
}

Logger::~Logger() {
#ifdef HAVE_SYSLOG_H

	if(m_writeSyslog) closelog();

#endif
}

void Logger::postAction(const logString &ls) const throw() {
#ifdef _WIN32
	std::cerr.flush();
#endif
#ifdef HAVE_SYSLOG_H

	if(m_writeSyslog) syslog(LOG_NOTICE, "%s", ls.c_str());

#endif
}

void Logger::writeSyslog(bool b) {
	m_writeSyslog = b;
}

template class Commons::IPostLogger<std::ostreambuf_iterator<LOG_CHAR>, LOGBUFS>;
template class Commons::BasicLogger<std::ostreambuf_iterator<LOG_CHAR>, LOGBUFS>;
template class Commons::BasicLoggerBufferSwitcher<std::ostreambuf_iterator<LOG_CHAR>, LOGBUFS>;

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
