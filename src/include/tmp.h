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

#ifndef COMMONS_TMP_H
#define COMMONS_TMP_H

namespace Commons {

template<class T>
struct check_pointer;

template<class T>
struct check_pointer<T *> {
	typedef T *pointer_type;
};

#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic push
template<typename T>
class IsClassT {
private:
	typedef char One;
	typedef struct {
		char a[2];
	} Two;
	template<typename C> static One test(int C:: *);
	template<typename C> static Two test(...);
public:
	enum { Yes = sizeof(IsClassT<T>::test<T>(0L)) == 1 };
	enum { No = !Yes };
};
#pragma GCC diagnostic pop

template<bool C, typename Ta, typename Tb>
class IfThenElse;

template<typename Ta, typename Tb>
class IfThenElse<true, Ta, Tb> {
public:
	typedef Ta ResultT;
};

template<typename Ta, typename Tb>
class IfThenElse<false, Ta, Tb> {
public:
	typedef Tb ResultT;
};

template<typename T>
class RParam {
public:
	typedef typename IfThenElse<IsClassT<T>::No, T, T const &>::ResultT Type;
};

}

#endif /* COMMONS_TMP_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
