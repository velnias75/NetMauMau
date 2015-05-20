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

#ifndef NETMAUMAU_LOGGER_H
#define NETMAUMAU_LOGGER_H

#ifndef LOGGER_CLASS
#define LOG_CHAR char
#define LOGGER_CLASS NetMauMau::Common::Logger
#endif

#include "basiclogger.h"                // for BasicLogger, etc

namespace NetMauMau {

namespace Common {

class _EXPORT Logger : public Commons::IPostLogger<std::ostreambuf_iterator<LOG_CHAR> >,
	public Commons::BasicLogger<std::ostreambuf_iterator<LOG_CHAR> > {
	DISALLOW_COPY_AND_ASSIGN(Logger)
public:
	explicit Logger(const LEVEL &lvl);
	virtual ~Logger();

	virtual void postAction(const logString &ls) const throw();

	static void writeSyslog(bool b);

private:
	static bool m_writeSyslog;
};

}

}

#endif /* NETMAUMAU_LOGGER_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
