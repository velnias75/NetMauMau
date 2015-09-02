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

#include "mutexlocker.h"

namespace NetMauMau {

namespace Common {

#if GCC_VERSION >= 47000
// on g++ before v4.7 this causes a compiler bug with LTO enabled
template class MutexLockerBase<pthread_mutex_t,  pthread_mutex_lock,    pthread_mutex_unlock>;
template class MutexLockerBase<pthread_rwlock_t, pthread_rwlock_rdlock, pthread_rwlock_unlock>;
template class MutexLockerBase<pthread_rwlock_t, pthread_rwlock_wrlock, pthread_rwlock_unlock>;
#endif

}

}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
