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

#ifndef __UTILS_TIME_H__INCL__
#define __UTILS_TIME_H__INCL__

/*
SEC = 10000000L
MIN = (SEC * 60L)
HOUR = (MIN * 60L)
DAY = (HOUR * 24L)
MONTH = (30 * DAY)
YEAR = (12 * MONTH)
*/
extern const uint64 Win32Time_Second;
extern const uint64 Win32Time_Minute;
extern const uint64 Win32Time_Hour;
extern const uint64 Win32Time_Day;
extern const uint64 Win32Time_Month;
extern const uint64 Win32Time_Year;

extern uint64 UnixTimeToWin32Time( time_t sec, uint32 nsec );
extern uint64 Win32TimeNow();
extern void Win32TimeToUnixTime( uint64 win32t, time_t &unix_time, uint32 &nsec );
extern std::string Win32TimeToString(uint64 win32t);

#endif /* !__UTILS_TIME_H__INCL__ */
