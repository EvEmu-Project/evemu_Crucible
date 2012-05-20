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
    Author:        Aim, Captnoord, Zhur, Bloody.Rabbit
*/

#ifndef __COMMON_H__INCL__
#define __COMMON_H__INCL__

/*
 * Custom config include.
 */
#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif /* HAVE_CONFIG_H */

/*************************************************************************/
/* Pre-include                                                           */
/*************************************************************************/

// Define basic Windows configuration
#ifdef WIN32
#   define _WIN32_WINNT 0x0500 // building for Win2k
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#endif /* !WIN32 */

// Visual Studio configuration
#ifdef MSVC
#   define _CRT_SECURE_NO_WARNINGS                  1
#   define _CRT_SECURE_NO_DEPRECATE                 1
#   define _CRT_SECURE_COPP_OVERLOAD_STANDARD_NAMES 1
#   define _SCL_SECURE_NO_WARNINGS                  1
#endif /* _MSC_VER */

#ifdef HAVE_CRTDBG_H
#   define _CRTDBG_MAP_ALLOC 1
#endif /* HAVE_CRTDBG_H */

// We must "explicitly request" the format strings ...
#ifdef HAVE_INTTYPES_H
#   define __STDC_FORMAT_MACROS 1
#endif /* HAVE_INTTYPES_H */

/*************************************************************************/
/* Include                                                               */
/*************************************************************************/

// Standard library includes
#include <cassert>
#include <cerrno>
#include <cfloat>
#include <climits>
#include <cmath>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

// Standard Template Library includes
#include <algorithm>
#include <list>
#include <map>
#include <memory>
#include <new>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

/*
 * Technical Report 1 Standard Template Library includes
 * Note: my fellow developers please read 'http://en.wikipedia.org/wiki/Technical_Report_1'. I know its a wiki page
 *       but it gives you the general idea.
 */

#ifdef MSVC
#   include <functional>
#   include <tuple>
#   include <unordered_map>
#   include <unordered_set>
#else /* !MSVC */
#   include <tr1/functional>
#   include <tr1/tuple>
#   include <tr1/unordered_map>
#   include <tr1/unordered_set>
#endif /* !MSVC */

// Platform-dependent includes
#ifdef WIN32
#   include <process.h>
#   include <windows.h>
#   include <winsock2.h>
#else /* !WIN32 */
#   include <arpa/inet.h>
#   include <dirent.h>
#   include <execinfo.h>
#   include <fcntl.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   include <pthread.h>
#   include <sys/socket.h>
#   include <unistd.h>
#endif /* !WIN32 */

// Feature includes
#ifdef HAVE_CRTDBG_H
#   include <crtdbg.h>
    // This is necessary to track leaks introduced by operator new()
#   define new new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#endif /* HAVE_CRTDBG_H */

#ifdef HAVE_INTTYPES_H
#   include <inttypes.h>
#endif /* HAVE_INTTYPES_H */

#ifdef HAVE_VLD_H
#   include <vld.h>
#endif /* HAVE_VLD_H */

// Compiler-dependent includes.
#ifdef MSVC
#   include <io.h>
#   include <sys/stat.h>
#   include <sys/timeb.h>
#else /* !MSVC */
#   include <sys/stat.h>
#   include <sys/time.h>
#   include <sys/types.h>
#endif /* !MSVC */

/*************************************************************************/
/* Post-include                                                          */
/*************************************************************************/

/* If not defined, define va_copy.
   Using memcpy() because of possible array implementation. */
#ifndef HAVE_VA_COPY
#   define va_copy( a, b ) ::memcpy( &( a ), &( b ), sizeof( va_list ) )
#endif /* !HAVE_VA_COPY */

// Define finite() and isnan()
#if defined( HAVE__FINITE )
#   define finite _finite
#elif defined( HAVE___FINITE )
#   define finite __finite
#elif defined( HAVE_STD_FINITE )
#   define finite std::finite
#elif defined( HAVE_STD_ISFINITE )
#   define finite std::isfinite
#endif

#if defined( HAVE__ISNAN )
#   define isnan _isnan
#elif defined( HAVE___ISNAN )
#   define isnan __isnan
#elif defined( HAVE_STD_ISNAN )
#   define isnan std::isnan
#endif

/*
 * u?int(8|16|32|64)
 */
#ifdef HAVE_INTTYPES_H
typedef  int8_t   int8;
typedef uint8_t  uint8;
typedef  int16_t  int16;
typedef uint16_t uint16;
typedef  int32_t  int32;
typedef uint32_t uint32;
typedef  int64_t  int64;
typedef uint64_t uint64;
#else /* !HAVE_INTTYPES_H */
typedef   signed __int8   int8;
typedef unsigned __int8  uint8;
typedef   signed __int16  int16;
typedef unsigned __int16 uint16;
typedef   signed __int32  int32;
typedef unsigned __int32 uint32;
typedef   signed __int64  int64;
typedef unsigned __int64 uint64;
#endif /* !HAVE_INTTYPES_H */

/*
 * PRI[diouxX](8|16|32|64)
 * SCN[diouxX](8|16|32|64)
 */
#ifndef HAVE_INTTYPES_H
#   define PRId8 "hhd"
#   define PRIi8 "hhi"
#   define PRIo8 "hho"
#   define PRIu8 "hhu"
#   define PRIx8 "hhx"
#   define PRIX8 "hhX"

#   define PRId16 "hd"
#   define PRIi16 "hi"
#   define PRIo16 "ho"
#   define PRIu16 "hu"
#   define PRIx16 "hx"
#   define PRIX16 "hX"

#   define PRId32 "I32d"
#   define PRIi32 "I32i"
#   define PRIo32 "I32o"
#   define PRIu32 "I32u"
#   define PRIx32 "I32x"
#   define PRIX32 "I32X"

#   define PRId64 "I64d"
#   define PRIi64 "I64i"
#   define PRIo64 "I64o"
#   define PRIu64 "I64u"
#   define PRIx64 "I64x"
#   define PRIX64 "I64X"

#   define SCNd8 "hhd"
#   define SCNi8 "hhi"
#   define SCNo8 "hho"
#   define SCNu8 "hhu"
#   define SCNx8 "hhx"

#   define SCNd16 "hd"
#   define SCNi16 "hi"
#   define SCNo16 "ho"
#   define SCNu16 "hu"
#   define SCNx16 "hx"

#   define SCNd32 "I32d"
#   define SCNi32 "I32i"
#   define SCNo32 "I32o"
#   define SCNu32 "I32u"
#   define SCNx32 "I32x"

#   define SCNd64 "I64d"
#   define SCNi64 "I64i"
#   define SCNo64 "I64o"
#   define SCNu64 "I64u"
#   define SCNx64 "I64x"
#endif /* !HAVE_INTTYPES_H */

// Return thread macro's
// URL: http://msdn.microsoft.com/en-us/library/hw264s73(VS.80).aspx
// Important Quote: "_endthread and _endthreadex cause C++ destructors pending in the thread not to be called."
// Result: mem leaks under windows
#ifdef WIN32
#   define THREAD_RETURN( x ) ( x )
typedef void thread_return_t;
#else /* !WIN32 */
#   define THREAD_RETURN( x ) return ( x )
typedef void* thread_return_t;
#endif /* !WIN32 */

// Basic programming tips
// URL: http://nedprod.com/programs/index.html
// Note: always nullify pointers after deletion, why? because its safer on a MT application
#define SafeDelete( p )      { if( p != NULL ) { delete p; p = NULL; } }
#define SafeDeleteArray( p ) { if( p != NULL ) { delete[] p; p = NULL; } }
#define SafeFree( p )        { if( p != NULL ) { free( p ); p = NULL; } }

/*
 * Post-include platform-dependent
 */
#ifdef WIN32
#   define MSG_NOSIGNAL 0

int gettimeofday( timeval* tv, void* reserved );
int asprintf( char** strp, const char* fmt, ... );
int vasprintf( char** strp, const char* fmt, va_list ap );
int mkdir( const char* pathname, int mode );

#else /* !WIN32 */
#   define INVALID_SOCKET ( -1 )
#   define SOCKET_ERROR   ( -1 )

#   if defined( FREE_BSD )
#       define MSG_NOSIGNAL 0
#   elif defined( APPLE )
#       define MSG_NOSIGNAL SO_NOSIGPIPE
#   endif

#   ifndef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#       define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP \
            { 0, 0, 0, PTHREAD_MUTEX_RECURSIVE_NP, __LOCK_INITIALIZER }
#   endif /* !PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP */

typedef int SOCKET;

void Sleep( uint32 x );
uint32 GetTickCount();

#endif /* !WIN32 */

/*
 * Post-include compiler-specific
 */

#ifdef MSVC
#   define S_IRWXU      0
#   define S_IRWXG      0
#   define S_IRWXO      0

#   define snprintf            _snprintf
#   define strncasecmp         _strnicmp
#   define strcasecmp          _stricmp
#   define localtime_r( x, y ) localtime_s( y, x )

//#   define strtoq   _strtoi64
//#   define strtouq  _strtoui64
#   define strtoll  _strtoi64
#   define strtoull _strtoui64

#   if( _MSC_VER < 1500 )
#       define vsnprintf _vsnprintf
#   else /* _MSC_VER >= 1500 */
#       ifndef strdup
#           define strdup _strdup
#       endif /* !strdup */
#   endif /* _MSC_VER >= 1500 */
#endif /* MSVC */

#ifdef MINGW
/*
 * Define localtime_r as a call to localtime.
 *
 * The page below says that MS's version of localtime uses
 * thread-specific storage, so this should not be a problem.
 *
 * http://msdn.microsoft.com/en-us/library/bf12f0hc(VS.80).aspx
 */
#   define localtime_r( x, y ) \
    ( (tm*)( memcpy( ( y ), localtime( x ), sizeof( tm ) ) ) )
#endif /* MINGW */

#ifdef WIN32
#  define ASCENT_FORCEINLINE __forceinline
#else
#  define ASCENT_FORCEINLINE inline
#endif

// fast int abs
static ASCENT_FORCEINLINE int int32abs( const int value )
{
    return (value ^ (value >> 31)) - (value >> 31);
}

// fast int abs and recast to unsigned
static ASCENT_FORCEINLINE uint32 int32abs2uint32( const int value )
{
    return (uint32)(value ^ (value >> 31)) - (value >> 31);
}

/// Fastest Method of float2int32
static ASCENT_FORCEINLINE int float2int32(const float value)
{
//#if !defined(X64) && ASCENT_COMPILER == COMPILER_MICROSOFT
    #if !defined(X64) && !defined(MINGW)
    int i;
    __asm {
        fld value
        frndint
        fistp i
    }
    return i;
#else
    union { int asInt[2]; double asDouble; } n;
    n.asDouble = value + 6755399441055744.0;

    return n.asInt [0];
#endif
}

/// Fastest Method of double2int32
static ASCENT_FORCEINLINE int double2int32(const double value)
{
#if !defined(X64) && ASCENT_COMPILER == COMPILER_MICROSOFT
    int i;
    __asm {
        fld value
        frndint
        fistp i
    }
    return i;
#else
  union { int asInt[2]; double asDouble; } n;
  n.asDouble = value + 6755399441055744.0;
  return n.asInt [0];
#endif
}

#endif /* !__COMMON_H__INCL__ */
