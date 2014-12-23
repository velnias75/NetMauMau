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

/**
 * @file
 * @author Heiko Schäfer <heiko@rangun.de>
 */

#ifndef NETMAUMAU_RANDOM_GEN_H
#define NETMAUMAU_RANDOM_GEN_H

#if defined(HAVE_CONFIG_H) || defined(IN_IDE_PARSER)
#include "config.h"
#endif

#if defined(HAVE_GSL)
#include <ctime>
#include <gsl/gsl_rng.h>
#include "linkercontrol.h"
#endif

#include <cstdlib>

namespace NetMauMau {

namespace Common {

#if defined(HAVE_GSL)
template<typename T>
class GSLRNG {
	DISALLOW_COPY_AND_ASSIGN(GSLRNG)
public:
	GSLRNG(long unsigned int seed = std::time(0L)) : m_rng(gsl_rng_alloc(gsl_rng_ranlxs2)) {
		if(m_rng) gsl_rng_set(m_rng, seed);
	}

	~GSLRNG() {
		if(m_rng) gsl_rng_free(m_rng);
	}

	T rand(T ubound) const {

		if(m_rng) {
			return ubound > 0 ? static_cast<T>(gsl_rng_uniform_int(m_rng,
											   static_cast<unsigned long int>(ubound))) : 0;
		}

#if HAVE_ARC4RANDOM_UNIFORM
		return ubound > 0 ? ::arc4random_uniform(ubound) : 0;
#else
		return ubound > 0 ? (std::rand() % ubound) : 0;
#endif
	}

private:
	gsl_rng *m_rng;
};

extern const GSLRNG<std::ptrdiff_t> RNG;

#endif

/**
 * @brief Generates a random number
 *
 * @tparam T type of the number
 * @param ubound upper bound
 * @return the random number
 */
template<typename T>
inline T genRandom(T ubound) {
#if defined(HAVE_GSL)
	return RNG.rand(ubound);
#elif HAVE_ARC4RANDOM_UNIFORM
	return ubound > 0 ? ::arc4random_uniform(ubound) : 0;
#else
	return ubound > 0 ? (std::rand() % ubound) : 0;
#endif
}

}

}

#endif /* NETMAUMAU_RANDOM_GEN_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
