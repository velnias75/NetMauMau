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

/*
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

   Modifications by Heiko Schäfer <heiko@rangun.de> and from here:
   http://stackoverflow.com/questions/180947

*/

/**
 * @file
 * @brief BASE64 de- and encoding routines
 *
 * @author Heiko Schäfer <heiko@rangun.de>
 */

#ifndef _BASE64_H_
#define _BASE64_H_

#include <vector>
#include <string>

#include "linkercontrol.h"

namespace NetMauMau {

namespace Common {

/**
 * @name Miscellaneous functions
 * @{
 */

/// @brief byte type for use in the base64 functions
typedef unsigned char BYTE;

/**
 * @brief Encodes a buffer to base64
 *
 * @code #include <base64.h> @endcode
 *
 * @param buf the buffer
 * @param bufLen length of the buffer
 * @return the base64 encoded data
 *
 * @since 0.4
 */
_EXPORT std::string base64_encode(BYTE const *buf, unsigned int bufLen);

/**
 * @brief Decodes a base64 encoded string
 *
 * @code #include <base64.h> @endcode
 *
 * @param base64 the base64 encoded data
 * @return the decoded data
 *
 * @since 0.4
 */
_EXPORT std::vector<BYTE> base64_decode(std::string const &base64);

/// @}

}

}

#endif

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
