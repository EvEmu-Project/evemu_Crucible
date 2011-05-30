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
	Author:		Aim, Captnoord, Zhur, Bloody.Rabbit
*/

#ifndef __COMMON_H__INCL__
#define __COMMON_H__INCL__

/*
 * Custom config include.
 *
 * config.h (along with command-line parameters) should
 * describe the following (using defines listed below;
 * the default is in parenthesis):
 *
 *   a) the target platform
 *        X64, (x86)
 *        WIN32, CYGWIN, FREE_BSD, APPLE, (other)
 *   b) the compiler
 *        MSVC, GNUC, MINGW, (other)
 *   c) the configuration
 *        NDEBUG, (debug)
 *        EVEMU_ROOT_DIR
 *        EVEMU_VERSION
 *        TINYXML_USE_STL
 *
 * The rest of code relies on these defines.
 */
#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif /* HAVE_CONFIG_H */

/*************************************************************************/
/* Pre-include                                                           */
/*************************************************************************/

/*
 * Pre-include platform-specific
 */

// Define basic Windows configuration
#ifdef WIN32
#   define WIN32_LEAN_AND_MEAN
#   define _WIN32_WINNT 0x0500 // building for Win2k
#   define NOMINMAX
#endif /* !WIN32 */

/*
 * Pre-include compiler-specific
 */

// Visual Studio configuration
#ifdef MSVC
// 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
//#   pragma warning( disable : 4251 )
// 'function': was declared deprecated
//#   pragma warning( disable : 4996 )

#   define _CRT_SECURE_NO_WARNINGS                  1
#   define _CRT_SECURE_NO_DEPRECATE                 1
#   define _CRT_SECURE_COPP_OVERLOAD_STANDARD_NAMES 1

#   ifndef NDEBUG
#       define _CRTDBG_MAP_ALLOC 1
#   endif /* !NDEBUG */

#   define _SCL_SECURE_NO_WARNINGS 1
#endif /* _MSC_VER */

/*************************************************************************/
/* Include                                                               */
/*************************************************************************/

/*
 * Standard library includes
 */

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

/*
 * Standard Template Library includes
 */

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
#   include <tuple>
#   include <unordered_map>
#   include <unordered_set>
#else /* !MSVC */
#   include <tr1/tuple>
#   include <tr1/unordered_map>
#   include <tr1/unordered_set>
#endif /* !MSVC */

/*
 * Platform-dependent includes
 */

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

/*
 * Compiler-dependent includes.
 */

#ifdef MSVC
#   include <io.h>
#   include <sys/stat.h>
#   include <sys/timeb.h>
#else /* !MSVC */
#   include <sys/stat.h>
#   include <sys/time.h>
#   include <sys/types.h>
#endif /* !MSVC */

// Visual Studio memory leak detection includes
#ifdef MSVC
#   ifndef NDEBUG
#       include <crtdbg.h>
//#       include <vld.h>  // Only enable if you have VLD installed
#       define new new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#   endif /* !NDEBUG */
#endif /* MSVC */

/*************************************************************************/
/* Post-include                                                          */
/*************************************************************************/

// 'undefine' min / max so that there won't be major conflicts regarding std
#ifdef min
#   undef min
#endif /* min */

#ifdef max
#  undef max
#endif /* max */

// if not defined, define va_copy
// using memcpy() because of possible array implementation
#ifndef va_copy
#   define va_copy( a, b ) \
    ( memcpy( &( a ), &( b ), sizeof( va_list ) ) )
#endif /* !va_copy */

// define finite() and isnan()
#if defined( MSVC ) || defined( MINGW )
#   define finite _finite
#   define isnan  _isnan
#elif defined( FREE_BSD ) || defined( APPLE )
using std::finite;
using std::isnan;
#else /* !MSVC && !MINGW && !APPLE && !FREE_BSD */
#   define finite __finite
#   define isnan  __isnan
#endif /* !MSVC && !MINGW && !APPLE && !FREE_BSD */

// dll interface stuff
#ifdef MSVC
#  define DLLFUNC extern "C" __declspec( dllexport )
#else /* !WIN32 */
#  define DLLFUNC extern "C"
#endif /* !WIN32 */

// Typedefs for integers
#if defined( MSVC ) || defined( MINGW )

typedef signed __int64 int64;
typedef signed __int32 int32;
typedef signed __int16 int16;
typedef signed __int8  int8;

typedef unsigned __int64 uint64;
typedef unsigned __int32 uint32;
typedef unsigned __int16 uint16;
typedef unsigned __int8  uint8;

#else /* !MSVC && !MINGW */

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t  int8;

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t  uint8;

#endif /* !MSVC && !MINGW */

// for printf() compatibility, I don't like this but its needed.
#if defined( WIN32 )
#   define I64d "%I64d"
#   define I64u "%I64u"
#   define I64x "%I64x"
#   define I64X "%I64X"
#elif defined( X64 )
#   define I64d "%ld"
#   define I64u "%lu"
#   define I64x "%lx"
#   define I64X "%lX"
#else /* !WIN32 && !X64 */
#   define I64d "%lld"
#   define I64u "%llu"
#   define I64x "%llx"
#   define I64X "%llX"
#endif /* !WIN32 && !X64 */

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
#define ENABLE_SAFE_DELETE       1       // only delete and NULL after
#define ENABLE_EXTRA_SAFE_DELETE 1       // check the array for NULL pointer then delete and NULL after
//#define ENABLE_ULTRA_SAFE_DELETE 1       // check object and array for NULL pointer then delete and NULL after

#if defined( ENABLE_ULTRA_SAFE_DELETE )
#   define SafeDelete( p )      { if( p != NULL ) { delete p; p = NULL; } }
#   define SafeDeleteArray( p ) { if( p != NULL ) { delete[] p; p = NULL; } }
#   define SafeFree( p )        { if( p != NULL ) { free( p ); p = NULL; } }
#elif defined( ENABLE_EXTRA_SAFE_DELETE )
#   define SafeDelete( p )      { delete p; p = NULL; }
#   define SafeDeleteArray( p ) { if( p != NULL ) { delete[] p; p = NULL; } }
#   define SafeFree( p )        { if( p != NULL ) { free( p ); p = NULL; } }
#elif defined( ENABLE_SAFE_DELETE )
#   define SafeDelete( p )      { delete p; p = NULL; }
#   define SafeDeleteArray( p ) { delete[] p; p = NULL; }
#   define SafeFree( p )        { free( p ); p = NULL; }
#else
#   define SafeDelete( p )      { delete p; }
#   define SafeDeleteArray( p ) { delete[] p; }
#   define SafeFree( p )        { free( p ); }
#endif

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

#   ifndef PATH_MAX
#       define MAX_PATH 1024
#   else /* PATH_MAX */
#       define MAX_PATH PATH_MAX
#   endif /* PATH_MAX */

#   ifndef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#       define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP \
            { 0, 0, 0, PTHREAD_MUTEX_RECURSIVE_NP, __LOCK_INITIALIZER }
#   endif /* !PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP */

typedef int SOCKET;

void Sleep( uint32 x );
uint32 GetTickCount();

#   ifndef CYGWIN

char* strupr( char* tmp );
char* strlwr( char* tmp );

#   endif /* !CYGWIN */
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

#endif /* !__COMMON_H__INCL__ */
