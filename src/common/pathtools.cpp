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

#ifdef _WIN32
#include "windows.h"
#include <cstring>
#endif

#include <climits>
#include <cstdlib>
#include <cstdio>

#include "linkercontrol.h"
#include "pathtools.h"

namespace NetMauMau _EXPORT {

namespace Common _EXPORT {

std::string getModulePath(MPATH mpath, const char *name, const char *ext) {

#ifdef _WIN32

	TCHAR retPath[MAX_PATH];

	std::memset(retPath, 0, MAX_PATH);

	if(mpath != USER) {
		GetModuleFileName(NULL, retPath, MAX_PATH);
	} else {
		std::strncpy(retPath, std::getenv("APPDATA"), MAX_PATH);
	}

	if(name && ext) {

		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char fext[_MAX_EXT];

		_splitpath(retPath, drive, dir, fname, fext);
		_makepath(retPath, drive, dir, name, ext);
	}

#else
	char retPath[PATH_MAX];
	std::snprintf(retPath, PATH_MAX, "%s/%s%s%s", mpath == BINDIR ? NMM_EXE_PATH : (mpath == USER ?
				  std::getenv("HOME") : PKGDATADIR), (name ? name : NMM_EXE_NAME),
				  (ext && *ext != '.' ? "." : ""), (ext ? ext : (*NMM_EXE_EXT ? NMM_EXE_EXT : "")));

#endif

	return std::string(retPath);
}

}

}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
