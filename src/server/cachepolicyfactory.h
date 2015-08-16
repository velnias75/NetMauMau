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

#ifndef NETMAUMAU_SERVER_CACHEPOLICYFACTORY_H
#define NETMAUMAU_SERVER_CACHEPOLICYFACTORY_H

#include "smartsingleton.h"

namespace NetMauMau {

namespace Server {

class CachePolicyFactory : public Common::SmartSingleton<CachePolicyFactory> {
	DISALLOW_COPY_AND_ASSIGN(CachePolicyFactory)
	friend class Common::SmartSingleton<CachePolicyFactory>;
public:

	class ICachePolicy {
		DISALLOW_COPY_AND_ASSIGN(ICachePolicy)
	public:
		virtual ~ICachePolicy() {}

		virtual bool expires() const = 0;
		virtual const char *getExpiryDate() const = 0;
		virtual const char *getCacheControl() const = 0;

	protected:
		ICachePolicy() {}
	};

	typedef Common::SmartPtr<ICachePolicy> ICachePolicyPtr;

	virtual ~CachePolicyFactory() throw();

	// cppcheck-suppress functionStatic
	const ICachePolicyPtr createNoCachePolicy() const;

	// cppcheck-suppress functionStatic
	const ICachePolicyPtr createPublicCachePolicy(long maxage = -1L) const;

	// cppcheck-suppress functionStatic
	const ICachePolicyPtr createPrivateCachePolicy(long maxage = -1L) const;

protected:
	class NoCachePolicy : public virtual ICachePolicy {
		DISALLOW_COPY_AND_ASSIGN(NoCachePolicy)
		friend class CachePolicyFactory;
	public:
		virtual ~NoCachePolicy();

		virtual bool expires() const _CONST;
		virtual const char *getExpiryDate() const _PURE _CONST;
		virtual const char *getCacheControl() const _CONST;

	protected:
		NoCachePolicy();
	};

	class PublicCachePolicy : public virtual ICachePolicy {
		DISALLOW_COPY_AND_ASSIGN(PublicCachePolicy)
		friend class CachePolicyFactory;
	public:
		virtual ~PublicCachePolicy();

		virtual bool expires() const _CONST;
		virtual const char *getExpiryDate() const _PURE _CONST;
		virtual const char *getCacheControl() const _CONST;

	protected:
		explicit PublicCachePolicy(long maxage);

		void createCacheControl(const char *s, long maxage);

	private:
		char m_cc[200];
	};

	class PrivateCachePolicy : public PublicCachePolicy {
		DISALLOW_COPY_AND_ASSIGN(PrivateCachePolicy)
		friend class CachePolicyFactory;
	public:
		virtual ~PrivateCachePolicy();

	protected:
		explicit PrivateCachePolicy(long maxage);
	};

private:
	CachePolicyFactory();
};

}

}

#endif /* NETMAUMAU_SERVER_CACHEPOLICYFACTORY_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
