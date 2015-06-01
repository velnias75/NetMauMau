#!/bin/bash
#
#  Copyright 2014 by Heiko Schäfer <heiko@rangun.de>
#
#  This file is part of NetMauMau.
#
#  NetMauMau is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as
#  published by the Free Software Foundation, either version 3 of
#  the License, or (at your option) any later version.
#
#  NetMauMau is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with NetMauMau.  If not, see <http://www.gnu.org/licenses/>.
#

command -v xxd >/dev/null 2>&1 || \
    { echo >&2 "I require xxd but it's not installed.  Aborting."; exit 1; }

if [ -z "$2" -o "$2" == "-" ]; then
    PNGDATA="`xxd -i`"
else
    PNGDATA="`cat $2 | xxd -i`"
fi

HGUARD="CREATE_AI_ICON_`echo -n $1 | tr '[:lower:]' '[:upper:]'`_H"

cat << EOF
/*
 * Copyright `date +"%Y"` by Heiko Schäfer <heiko@rangun.de>
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
 */

#ifndef $HGUARD
#define $HGUARD

namespace {

const unsigned char $1[] = {
$PNGDATA
};

}

#endif /* $HGUARD */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
EOF
