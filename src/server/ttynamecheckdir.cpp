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
 *
 * Based upon: http://man7.org/tlpi/code/online/book/tty/ttyname.c.html
 */

#ifndef _WIN32

#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "ttynamecheckdir.h"

char *NetMauMau::Server::ttynameCheckDir(dev_t ttyNr, const char *devDir) {

	struct dirent *dent;
	static char *ttyPath;
	std::size_t ttyLen = 0;
	struct stat devStat;

	if(ttyLen == 0) {

		ttyPath = static_cast<char *>(malloc(50));

		if(!ttyPath) return NULL;

		ttyLen = 50;
	}

	DIR *dirh = opendir(devDir);

	if(!dirh) return NULL;

	bool found = false;

	// cppcheck-suppress nonreentrantFunctionsreaddir
	while((dent = readdir(dirh))) {

		const std::size_t requiredLen = strlen(devDir) + strlen(dent->d_name) + 2;

		if(requiredLen > ttyLen) {

			char *nTtyPath = static_cast<char *>(realloc(ttyPath, requiredLen));

			if(nTtyPath) {
				ttyPath = nTtyPath;
			} else {
				break;
			}

			ttyLen = requiredLen;
		}

		std::snprintf(ttyPath, ttyLen, "%s/%s", devDir, dent->d_name);

		if(stat(ttyPath, &devStat) == -1) continue;

		if(S_ISCHR(devStat.st_mode) && ttyNr == devStat.st_rdev) {
			found = true;
			break;
		}
	}

	closedir(dirh);

	return found ? ttyPath : NULL;
}

#endif

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
