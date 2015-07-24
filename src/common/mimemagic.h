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

#ifndef NETMAUMAU_COMMON_MAGIC_H
#define NETMAUMAU_COMMON_MAGIC_H

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#include <string>
#include <cstddef>                      // for size_t

#if defined(HAVE_MAGIC_H) && defined(HAVE_LIBMAGIC)
#include <magic.h>

#define _MM_CONST

#ifndef MAGIC_MIME_TYPE
#define MAGIC_MIME_TYPE MAGIC_MIME
#endif
#else
#define _MM_CONST _CONST
#endif

#include "linkercontrol.h"
#include "smartptr.h"

#ifdef HAVE_LIBMICROHTTPD
#define _EXPORT_MIMEMAGIC _EXPORT
#else
#define _EXPORT_MIMEMAGIC
#endif

namespace NetMauMau {

namespace Common {

class _EXPORT_MIMEMAGIC MimeMagic {
	DISALLOW_COPY_AND_ASSIGN(MimeMagic)
public:
	typedef SmartPtr<MimeMagic> MimeMagicPtr;

	~MimeMagic() _MM_CONST;

	static MimeMagicPtr getInstance();

	// cppcheck-suppress functionStatic
	std::string getMime(const unsigned char *data, std::size_t dataLen) const _MM_CONST;

	// cppcheck-suppress functionStatic
	bool checkMime(const unsigned char *data, std::size_t dataLen,
				   const char *mime) const _MM_CONST;

private:
	explicit MimeMagic() _MM_CONST;

private:
	static MimeMagicPtr m_instance;

#if defined(HAVE_MAGIC_H) && defined(HAVE_LIBMAGIC)
	magic_t m_magic;
#endif
};

}

}

#endif /* NETMAUMAU_COMMON_MAGIC_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
