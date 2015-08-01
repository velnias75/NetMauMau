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

#ifndef NETMAUMAU_COMMON_ZSTREAMBUF_H
#define NETMAUMAU_COMMON_ZSTREAMBUF_H

#include <streambuf>
#include <vector>

#include <zlib.h>

#include "zlibexception.h"

namespace NetMauMau {

namespace Common {

class _EXPORT Zstreambuf : public std::streambuf {
	DISALLOW_COPY_AND_ASSIGN(Zstreambuf)
public:
	Zstreambuf(const std::ostream &os, int compressionLevel = Z_DEFAULT_COMPRESSION,
			   bool flush = false) throw(Exception::ZLibException);
	virtual ~Zstreambuf();

protected:
	virtual int sync();
	virtual int_type overflow(int_type);

private:
	int_type deflateBuffer(int flush);

private:
	z_stream m_zstream;
	std::streambuf *m_sbuf;
	char_type *m_buf;
	uLong m_deflateBound;
	bool m_flush;
};

}

}

#endif /* NETMAUMAU_COMMON_ZSTREAMBUF_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
