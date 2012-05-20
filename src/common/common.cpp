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

#ifndef HAVE_LOCALTIME_R
tm* localtime_r( const time_t* timep, tm* result )
{
#   ifdef HAVE_LOCALTIME_S
    const errno_t err = ::localtime_s( result, timep );
    if( 0 != err )
    {
        errno = err;
        return NULL;
    }
#   else /* !HAVE_LOCALTIME_S */
    /* This is quite dangerous stuff (not necessarily
       thread-safe), but what can we do? Also,
       there is a chance that localtime will use
       thread-specific memory (MS's localtime does that). */
    ::memcpy( result, ::localtime( timep ), sizeof( tm ) );
#   endif /* !HAVE_LOCALTIME_S */

    return result;
}
#endif /* !HAVE_LOCALTIME_R */

#ifdef WIN32

int gettimeofday( timeval* tp, void* reserved )
{
    timeb tb;
    ftime( &tb );

    tp->tv_sec  = (long)tb.time;
    tp->tv_usec = tb.millitm * 1000;

    return 0;
}

int asprintf( char** strp, const char* fmt, ... )
{
    va_list ap;

    va_start( ap, fmt );
    int res = vasprintf( strp, fmt, ap );
    va_end( ap );

    return res;
}

int vasprintf( char** strp, const char* fmt, va_list ap )
{
    //va_list ap_temp;
    //va_copy(ap_temp, ap);
    //int size = vsnprintf(NULL, 0, fmt, ap);
    int size = 0x8000;

    char* buff = (char*)malloc( size + 1 );
    if( buff == NULL )
        return -1;

    size = vsnprintf( buff, size, fmt, ap );
    if( size < 0 )
    {
        SafeFree( buff );
    }
    else
    {
        // do not waste memory
        buff = (char*)realloc( buff, size + 1 );
        buff[size] = '\0';

        (*strp) = buff;
    }

    return size;
}

int mkdir( const char* pathname, int mode )
{
    int result = CreateDirectory( pathname, NULL );

    /* mkdir returns 0 for success, opposite of CreateDirectory() */
    return ( result ? 0 : -1 );
}

#else /* !WIN32 */

void Sleep( uint32 x )
{
    if( 0 < x )
        usleep( 1000 * x );
}

uint32 GetTickCount()
{
    timeval tv;
    gettimeofday( &tv, NULL );

    return ( tv.tv_sec * 1000 ) + ( tv.tv_usec / 1000 );
}

#endif /* !WIN32 */
