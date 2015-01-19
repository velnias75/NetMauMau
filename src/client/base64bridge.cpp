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

#include "base64bridge.h"

#include "base64.h"

using namespace NetMauMau::Client;

Base64Bridge::Base64Bridge() : IBase64() {}

Base64Bridge::~Base64Bridge() {}

std::string Base64Bridge::encode(const unsigned char *buf, unsigned int bufLen) const {
	return NetMauMau::Common::base64_encode(buf, bufLen);
}

std::vector<unsigned char> Base64Bridge::decode(const std::string &base64) const {
	return NetMauMau::Common::base64_decode(base64);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
