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

#include <cstdio>

#include "cachepolicyfactory.h"

namespace {
const char *EXPDATE = "Thu, 01 Dec 1994 16:00:00 GMT";
}

using namespace NetMauMau::Server;

CachePolicyFactory::NoCachePolicy::NoCachePolicy() : ICachePolicy() {}

CachePolicyFactory::NoCachePolicy::~NoCachePolicy() {}

bool CachePolicyFactory::NoCachePolicy::expires() const {
	return true;
}

const char *CachePolicyFactory::NoCachePolicy::getExpiryDate() const {
	return EXPDATE;
}

const char *CachePolicyFactory::NoCachePolicy::getCacheControl() const {
	return "no-store";
}

CachePolicyFactory::PublicCachePolicy::PublicCachePolicy(long maxage) : ICachePolicy(), m_cc() {
	createCacheControl("public", maxage);
}

CachePolicyFactory::PublicCachePolicy::~PublicCachePolicy() {}

bool CachePolicyFactory::PublicCachePolicy::expires() const {
	return false;
}

const char *CachePolicyFactory::PublicCachePolicy::getExpiryDate() const {
	return EXPDATE;
}

const char *CachePolicyFactory::PublicCachePolicy::getCacheControl() const {
	return m_cc;
}

void CachePolicyFactory::PublicCachePolicy::createCacheControl(const char *s, long maxage) {
	std::snprintf(m_cc, 199, "%s, max-age=%ld", s, maxage != -1L ? maxage : 31536000L);
}

CachePolicyFactory::PrivateCachePolicy::PrivateCachePolicy(long maxage) :
	PublicCachePolicy(maxage) {
	createCacheControl("private", maxage);
}

CachePolicyFactory::PrivateCachePolicy::~PrivateCachePolicy() {}

CachePolicyFactory::CachePolicyFactory() : Common::SmartSingleton<CachePolicyFactory>() {}

CachePolicyFactory::~CachePolicyFactory() throw() {}

const CachePolicyFactory::ICachePolicyPtr CachePolicyFactory::createNoCachePolicy() const {
	return ICachePolicyPtr(new NoCachePolicy());
}

const CachePolicyFactory::ICachePolicyPtr
CachePolicyFactory::createPublicCachePolicy(long maxage) const {
	return ICachePolicyPtr(new PublicCachePolicy(maxage));
}

const CachePolicyFactory::ICachePolicyPtr
CachePolicyFactory::createPrivateCachePolicy(long maxage) const {
	return ICachePolicyPtr(new PrivateCachePolicy(maxage));
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
