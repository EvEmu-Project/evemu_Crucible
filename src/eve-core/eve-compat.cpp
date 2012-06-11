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

#include "eve-core.h"

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

#ifndef HAVE_ASPRINTF
int asprintf( char** strp, const char* fmt, ... )
{
    va_list ap;

    va_start( ap, fmt );
    int res = ::vasprintf( strp, fmt, ap );
    va_end( ap );

    return res;
}
#endif /* !HAVE_ASPRINTF */

#ifndef HAVE_VASPRINTF
int vasprintf( char** strp, const char* fmt, va_list ap )
{
    // Size of the buffer (doubled each cycle)
    size_t size = 0x40 >> 1;
    // Return code
    int code;

    // No buffer yet
    *strp = NULL;

    do
    {
        // Double the size of buffer
        size <<= 1;
        // Reallocate the buffer
        *strp = (char*)::realloc( *strp, size );

        // Try to print into the buffer
        code = ::vsnprintf( *strp, size, fmt, ap );
        // Check for truncation
        if( size <= code )
            // Output truncated
            code = -1;
    } while( 0 > code );

    // Reallocate to save memory
    *strp = (char*)::realloc( *strp, code + 1 );
    // Terminate the string
    (*strp)[ code ] = '\0';

    // Return the code
    return code;
}
#endif /* !HAVE_VASPRINTF */

#ifndef WIN32
void Sleep( uint32 x )
{
    if( 0 < x )
        ::usleep( 1000 * x );
}

uint32 GetTickCount()
{
    timeval tv;
    ::gettimeofday( &tv, NULL );

    return tv.tv_sec  * 1000
         + tv.tv_usec / 1000;
}

int CreateDirectory( const char* name, void* )
{
    // Create the directory with mode 0755
    return 0 == ::mkdir( name,
                          S_IRUSR | S_IWUSR | S_IXUSR
                        | S_IRGRP           | S_IXGRP
                        | S_IROTH           | S_IXOTH );
}
#endif /* !WIN32 */
