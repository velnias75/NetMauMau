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

#include "base64.h"

#include "logger.h"                     // for logDebug

static const std::string base64_chars =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

static inline bool is_base64(NetMauMau::Common::BYTE c) {
	return (std::isalnum(c) || (c == '+') || (c == '/'));
}

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic push
std::string NetMauMau::Common::base64_encode(BYTE const *buf, unsigned int bufLen) {

	std::string ret;

	int i = 0;
	BYTE char_array_3[3];
	BYTE char_array_4[4];

	const std::string::size_type resBuf = (3 * bufLen + 23) / 2;

	if(resBuf <= ret.max_size()) {

		try {
			ret.reserve(resBuf);
		} catch(const std::bad_alloc &) {
			logWarning("BASE64: out of memory while encoding");
			return ret;
		}
	}

	while(bufLen--) {

		char_array_3[i++] = *(buf++);

		if(i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];

			i = 0;
		}
	}

	if(i) {

		int j = 0;

		for(j = i; j < 3; ++j) char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for(j = 0; (j < i + 1); ++j) ret += base64_chars[char_array_4[j]];

		while((i++ < 3)) ret += '=';
	}

	return ret;
}

std::vector<NetMauMau::Common::BYTE>
NetMauMau::Common::base64_decode(std::string const &encoded_string) {

	int in_len = encoded_string.size();
	int i = 0;
	int in_ = 0;
	BYTE char_array_4[4], char_array_3[3];

	std::vector<BYTE> ret;

	const std::string::size_type resBuf =
		static_cast<std::string::size_type>(std::max(12, (2 * in_len - 23) / 3));

	if(resBuf <= ret.max_size()) {

		try {
			ret.reserve(resBuf);
		} catch(const std::bad_alloc &) {
			logWarning("BASE64: out of memory while decoding");
			return ret;
		}
	}

	while(in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {

		char_array_4[i++] = encoded_string[in_];
		++in_;

		if(i == 4) {

			for(i = 0; i < 4; ++i) char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for(i = 0; (i < 3); ++i) ret.push_back(char_array_3[i]);

			i = 0;
		}
	}

	if(i) {

		int j = 0;

		for(j = i; j < 4; ++j) char_array_4[j] = 0;

		for(j = 0; j < 4; ++j) char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for(j = 0; (j < i - 1); ++j) ret.push_back(char_array_3[j]);
	}

	return ret;
}
#pragma GCC diagnostic pop

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
