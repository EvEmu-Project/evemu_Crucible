/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:     Zhur
*/

#include "CommonPCH.h"

#include "utils/utils_time.h"

const uint64 Win32Time_Second = 10000000L;
const uint64 Win32Time_Minute = Win32Time_Second*60;
const uint64 Win32Time_Hour = Win32Time_Minute*60;
const uint64 Win32Time_Day = Win32Time_Hour*24;
const uint64 Win32Time_Month = Win32Time_Day*30;
const uint64 Win32Time_Year = Win32Time_Month*12;

static const uint64 SECS_BETWEEN_EPOCHS = 11644473600LL;
static const uint64 SECS_TO_100NS = 10000000; // 10^7

uint64 UnixTimeToWin32Time( time_t sec, uint32 nsec ) {
    return(
        (((uint64) sec) + SECS_BETWEEN_EPOCHS) * SECS_TO_100NS
        + (nsec / 100)
    );
}

void Win32TimeToUnixTime( uint64 win32t, time_t &unix_time, uint32 &nsec ) {
    win32t -= (SECS_BETWEEN_EPOCHS * SECS_TO_100NS);
    nsec = (win32t % SECS_TO_100NS) * 100;
    win32t /= SECS_TO_100NS;
    unix_time = win32t;
}

std::string Win32TimeToString(uint64 win32t) {
    time_t unix_time;
    uint32 nsec;
    Win32TimeToUnixTime(win32t, unix_time, nsec);

    char buf[256];
    strftime(buf, 256,
#ifdef WIN32
        "%Y-%m-%d %X",
#else
        "%F %T",
#endif /* !WIN32 */
    localtime(&unix_time));

    return(buf);
}

uint64 Win32TimeNow() {
#ifdef WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    return((uint64(ft.dwHighDateTime) << 32) | uint64(ft.dwLowDateTime));
#else
    return(UnixTimeToWin32Time(time(NULL), 0));
#endif
}
