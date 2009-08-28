/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2008 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
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

#include <float.h>
#include <assert.h>

#include "common.h"
#include "EVEUtils.h"
#include "Exceptions.h"
#include "MiscFunctions.h"

#include "../packets/General.h"

const uint64 Win32Time_Second = 10000000L;
const uint64 Win32Time_Minute = Win32Time_Second*60;
const uint64 Win32Time_Hour = Win32Time_Minute*60;
const uint64 Win32Time_Day = Win32Time_Hour*24;
const uint64 Win32Time_Month = Win32Time_Day*30; //according to the eve client
const uint64 Win32Time_Year = Win32Time_Month*12;    //according to the eve client

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

UserError *MakeUserError(const char *exceptionType, const std::map<std::string, PyRep *> &args)
{
	UserError *err = new UserError( exceptionType );

	std::map<std::string, PyRep *>::const_iterator cur, end;
	cur = args.begin();
	end = args.end();
	for(; cur != end; cur++)
		err->AddKeyword( cur->first.c_str(), cur->second );

	return err;
}

UserError *MakeCustomError(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    char *str = NULL;
    assert(vasprintf(&str, fmt, va) > 0);
    va_end(va);

	UserError *err = new UserError( "CustomError" );
	err->AddKeyword( "error", new PyString( str ) );
    SafeFree(str);

    return err;
}

uint8 DBTYPE_SizeOf(DBTYPE type)
{
    switch( type )
    {
        case DBTYPE_I8:
        case DBTYPE_UI8:
        case DBTYPE_R8:
        case DBTYPE_CY:
        case DBTYPE_FILETIME:
            return 64;
        case DBTYPE_I4:
        case DBTYPE_UI4:
        case DBTYPE_R4:
            return 32;
        case DBTYPE_I2:
        case DBTYPE_UI2:
            return 16;
        case DBTYPE_I1:
        case DBTYPE_UI1:
            return 8;
        case DBTYPE_BOOL:
            return 1;
        case DBTYPE_BYTES:
        case DBTYPE_STR:
        case DBTYPE_WSTR:
            return 0;
    }
    return 0;
}

uint8 DBTYPE_GetSize( DBTYPE type )
{
    switch( type )
    {
    case DBTYPE_I8:
    case DBTYPE_UI8:
    case DBTYPE_R8:
    case DBTYPE_CY:
    case DBTYPE_FILETIME:
        return 8;
    case DBTYPE_I4:
    case DBTYPE_UI4:
    case DBTYPE_R4:
        return 4;
    case DBTYPE_I2:
    case DBTYPE_UI2:
        return 2;
    case DBTYPE_I1:
    case DBTYPE_UI1:
        return 1;
    case DBTYPE_BOOL:
        return 0; // compensated outside of this function
    case DBTYPE_BYTES:
    case DBTYPE_STR:
    case DBTYPE_WSTR:
        return 0;
    }
    return 0;
}

bool DBTYPE_IsCompatible(DBTYPE type, const PyRep &rep)
{
// Helper macro, checks type and range
#define CheckTypeRangeUnsigned( type, lower_bound, upper_bound ) \
    ( rep.Is##type() && (uint64)rep.As##type().value >= lower_bound && (uint64)rep.As##type().value <= upper_bound )
#define CheckTypeRange( type, lower_bound, upper_bound ) \
    ( rep.Is##type() && rep.As##type().value >= lower_bound && rep.As##type().value <= upper_bound )

    switch( type )
    {
        case DBTYPE_UI8:
        case DBTYPE_CY:
        case DBTYPE_FILETIME:
            return CheckTypeRangeUnsigned( Int, 0LL, 0xFFFFFFFFFFFFFFFFLL )
                   || CheckTypeRangeUnsigned( Long, 0LL, 0xFFFFFFFFFFFFFFFFLL )
                   || CheckTypeRangeUnsigned( Float, 0LL, 0xFFFFFFFFFFFFFFFFLL );
        case DBTYPE_UI4:
            return CheckTypeRangeUnsigned( Int, 0L, 0xFFFFFFFFL )
                   || CheckTypeRangeUnsigned( Long, 0L, 0xFFFFFFFFL )
                   || CheckTypeRangeUnsigned( Float, 0L, 0xFFFFFFFFL );
        case DBTYPE_UI2:
            return CheckTypeRangeUnsigned( Int, 0, 0xFFFF )
                   || CheckTypeRangeUnsigned( Long, 0, 0xFFFF )
                   || CheckTypeRangeUnsigned( Float, 0, 0xFFFF );
        case DBTYPE_UI1:
            return CheckTypeRangeUnsigned( Int, 0, 0xFF )
                   || CheckTypeRangeUnsigned( Long, 0, 0xFF )
                   || CheckTypeRangeUnsigned( Float, 0, 0xFF );

        case DBTYPE_I8:
            return CheckTypeRange( Int, -0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFFLL )
                   || CheckTypeRange( Long, -0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFFLL )
                   || CheckTypeRange( Float, -0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFFLL );
        case DBTYPE_I4:
            return CheckTypeRange( Int, -0x7FFFFFFFL, 0x7FFFFFFFL )
                   || CheckTypeRange( Long, -0x7FFFFFFFL, 0x7FFFFFFFL )
                   || CheckTypeRange( Float, -0x7FFFFFFFL, 0x7FFFFFFFL );
        case DBTYPE_I2:
            return CheckTypeRange( Int, -0x7FFF, 0x7FFF )
                   || CheckTypeRange( Long, -0x7FFF, 0x7FFF )
                   || CheckTypeRange( Float, -0x7FFF, 0x7FFF );
        case DBTYPE_I1:
            return CheckTypeRange( Int, -0x7F, 0x7F )
                   || CheckTypeRange( Long, -0x7F, 0x7F )
                   || CheckTypeRange( Float, -0x7F, 0x7F );

        case DBTYPE_R8:
            return CheckTypeRange( Int, -DBL_MAX, DBL_MAX )
                   || CheckTypeRange( Long, -DBL_MAX, DBL_MAX )
                   || CheckTypeRange( Float, -DBL_MAX, DBL_MAX );
        case DBTYPE_R4:
            return CheckTypeRange( Int, -FLT_MAX, FLT_MAX )
                   || CheckTypeRange( Long, -FLT_MAX, FLT_MAX )
                   || CheckTypeRange( Float, -FLT_MAX, FLT_MAX );

        case DBTYPE_BOOL:
            return rep.IsBool();

        case DBTYPE_BYTES:
            return rep.IsBuffer();

        case DBTYPE_STR:
        case DBTYPE_WSTR:
            return rep.IsString();
    }

    return rep.IsNone();

#undef CheckTypeRange
}
