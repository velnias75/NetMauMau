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

#include <ostream>

#include "zstreambuf.h"

#define Z_DEFLATE_CHUNK 32768

using namespace NetMauMau::Common;

Zstreambuf::Zstreambuf(const std::ostream &os, int compressionLevel,
					   bool flush) throw(NetMauMau::Common::Exception::ZLibException) : m_zstream(),
	m_sbuf(os.rdbuf()), m_buf(new char_type[Z_DEFLATE_CHUNK]), m_deflateBound(0), m_flush(flush) {

	m_zstream.zalloc = Z_NULL;
	m_zstream.zfree  = Z_NULL;
	m_zstream.opaque = Z_NULL;

	int err;

	if(unlikely((err = deflateInit2(&m_zstream, compressionLevel, Z_DEFLATED, -15, 9,
									Z_DEFAULT_STRATEGY)) != Z_OK)) {
		throw NetMauMau::Common::Exception::ZLibException(zError(err));
	}

	m_deflateBound = deflateBound(&m_zstream, Z_DEFLATE_CHUNK);

	setp(m_buf, m_buf + Z_DEFLATE_CHUNK - 1);
}

Zstreambuf::~Zstreambuf() {
	deflateEnd(&m_zstream);
	delete [] m_buf;
}

Zstreambuf::int_type Zstreambuf::deflateBuffer(int flush) {

	int_type num = pptr() - pbase();

	Byte *outBuf = new Byte[m_deflateBound];

	m_zstream.avail_in = static_cast<uLong>(num);
	m_zstream.next_in = reinterpret_cast<Byte *>(m_buf);

	do {

		m_zstream.avail_out = m_deflateBound;
		m_zstream.next_out = outBuf;

		if(unlikely(deflate(&m_zstream, flush) == Z_STREAM_ERROR)) {
			deflateEnd(&m_zstream);
			return std::char_traits<char_type>::eof();
		}

		std::streamsize have = static_cast<std::streamsize>(m_deflateBound - m_zstream.avail_out);

		if(unlikely(m_sbuf->sputn(reinterpret_cast<char_type *>(outBuf), have) != have)) {
			deflateEnd(&m_zstream);
			return std::char_traits<char_type>::eof();
		}

	} while(m_zstream.avail_out == 0);

	delete [] outBuf;
	outBuf = NULL;

	pbump(-num);
	return num;
}

int Zstreambuf::sync() {
	return (deflateBuffer(Z_FINISH) == std::char_traits<char_type>::eof()) ? -1 : 0;
}

Zstreambuf::int_type Zstreambuf::overflow(int_type c) {

	if(c != std::char_traits<char_type>::eof()) {
		*pptr() = static_cast<char_type>(c);
		pbump(1);
	}

	if(deflateBuffer(Z_NO_FLUSH) == std::char_traits<char_type>::eof()) {
		return std::char_traits<char_type>::eof();
	}

	if(m_flush) m_sbuf->pubsync();

	return c;
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
