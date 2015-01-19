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

#ifndef NETMAUMAU_CLIENT_BASE64BRIDGE_H
#define NETMAUMAU_CLIENT_BASE64BRIDGE_H

#include "ibase64.h"

namespace NetMauMau {

namespace Client {

class Base64Bridge : public IBase64 {
	DISALLOW_COPY_AND_ASSIGN(Base64Bridge)
public:
	Base64Bridge();
	virtual ~Base64Bridge();

	virtual std::string encode(const unsigned char *buf, unsigned int bufLen) const;
	virtual std::vector<unsigned char> decode(const std::string &base64) const;
};

}

}

#endif /* NETMAUMAU_CLIENT_BASE64BRIDGE_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
