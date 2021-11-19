/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Updates:    Allan
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
namespace EvE {
    namespace Time {
        enum:int64_t {
            mSecond     = 1000L,        //1000
            Second      = 10000000L,    //10000000
            Minute      = (Second * 60L),
            Hour        = (Minute * 60L),
            Day         = (Hour * 24L),
            Week        = (Day * 7L),
            Month       = (Day * 30L),
            Year        = (Day * 365L)
        };
    }

    struct TimeParts {
        uint16 year;
        uint8 month;
        uint8 wd;       // day of week
        uint8 wn;       // week of year
        uint8 day;
        uint8 hour;
        uint8 min;
        uint8 sec;
        uint16 dy;      // day of year
        uint16 ms;
    };

}

extern const int64 Win32Time_Second;
extern const int64 Win32Time_Minute;
extern const int64 Win32Time_Hour;
extern const int64 Win32Time_Day;
extern const int64 Win32Time_Month;
extern const int64 Win32Time_Year;
extern int64 UnixTimeToWin32Time( time_t sec, uint32 nsec );
extern int64 Win32TimeNow();
extern void Win32TimeToUnixTime( int64 win32t, time_t &unix_time, uint32 &nsec );
extern std::string Win32TimeToString(int64 win32t);

// returns delta between time and now, in hours
int32 GetElapsedHours(int64 time);
// this returns 100 nanosecond resolution in filetime format
double GetFileTimeNow();        // replacement for Win32TimeNow()
double GetRelativeFileTime(int days=0, int hours=0, int minutes=0, int seconds=0);
//  this returns milliseconds
int64 GetSteadyTime();
//  this returns milliseconds in microsecond resolution
double GetTimeMSeconds();
//  this returns microseconds in nanosecond resolution
double GetTimeUSeconds();

// return elapsed time formatted in units
std::string GetUTimeTillNow(double fromTime);
// return elapsed time formatted in units
std::string GetMTimeTillNow(double fromTime);

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime();

// break down given filetime into year, month, day, hour, min, sec, ms
EvE::TimeParts GetTimeParts(int64 filetime=0);  // also gives day of week, day of year, and week of year

// Get linux filetime from dataTime format YYYY-MM-DDTHH:mm:ssZ (where "T" is a separator and "Z" denotes 'zulo')
//std::time_t getEpochTime(const std::wstring& dateTime);

#endif /* !__UTILS_TIME_H__INCL__ */
