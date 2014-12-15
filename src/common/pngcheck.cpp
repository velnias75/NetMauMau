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

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#include <string>

#if defined(HAVE_MAGIC_H) && defined(HAVE_LIBMAGIC)
#include <magic.h>

#ifndef MAGIC_MIME_TYPE
#define MAGIC_MIME_TYPE MAGIC_MIME
#endif
#endif

#include "pngcheck.h"

bool NetMauMau::Common::checkPNG(const unsigned char *pngData, std::size_t pngDataLen) {

	bool magic_png = true;

#if defined(HAVE_MAGIC_H) && defined(HAVE_LIBMAGIC)

	magic_t magic = magic_open(MAGIC_MIME_TYPE | MAGIC_NO_CHECK_ASCII | MAGIC_NO_CHECK_COMPRESS |
							   MAGIC_NO_CHECK_ELF | MAGIC_NO_CHECK_FORTRAN | MAGIC_NO_CHECK_TAR |
							   MAGIC_NO_CHECK_TOKENS | MAGIC_NO_CHECK_TROFF);

	if(magic) {

		if(!magic_load(magic, NULL)) {

			const char *mime = magic_buffer(magic, pngData, pngDataLen);

			if(mime) magic_png = std::string(mime) == "image/png";
		}

		magic_close(magic);
	}

#endif

	return magic_png;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
