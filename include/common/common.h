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
	Author:		Aim, Captnoord, Zhur
*/

#ifndef __COMMON_H
#define __COMMON_H

/** custom config include
  * Note: mainly for 'UNIX' builds
  */
#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif//HAVE_CONFIG_H


/** Build platform defines
  */
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#   ifndef WIN32
#       define WIN32
#   endif//WIN32
#endif


/** Define NDEBUG according to _DEBUG.
  */
#ifdef _DEBUG
#   undef NDEBUG
#else /* !_DEBUG */
#   define NDEBUG 1
#endif /* !_DEBUG */

/** Visual Studio 'errors'/'warnings'
  */
#ifdef WIN32
#   ifdef _MSC_VER
//#    pragma warning(disable:4996)
//#    pragma warning(disable:4251) // dll-interface bullshit
#       define _CRT_SECURE_NO_WARNINGS 1
#       define _CRT_SECURE_NO_DEPRECATE 1
#       define _CRT_SECURE_COPP_OVERLOAD_STANDARD_NAMES 1
#       if _DEBUG
#           define _CRTDBG_MAP_ALLOC 1
#       endif// _DEBUG
#   endif//_MSC_VER
#endif//WIN32


/** Platform in depended includes
  */
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


/** STL includes
  */
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <list>
#include <map>
#include <set>
#include <stack>


/** TR1 STL includes
  * Note: my fellow developers please read 'http://en.wikipedia.org/wiki/Technical_Report_1'. I know its a wiki page
  *       but it gives you the general idea.
  */
#ifdef WIN32
#   include <tuple>
#   include <unordered_map>
#   include <unordered_set>
#else
#   include <tr1/tuple>
#   include <tr1/unordered_map>
#   include <tr1/unordered_set>
#endif//WIN32

/** Visual Studio memory leak detection
  */
#ifdef WIN32
#   ifdef _DEBUG
#       include <crtdbg.h>
#       include <new>
#       include <memory>
#       define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#   endif//_DEBUG
#endif//WIN32


/** visual leak detection, only enable when you know what your doing and have installed vld
  */
/*
#ifdef WIN32
#   define _CRTDBG_MAP_ALLOC
#   define _CRTDBG_MAP_ALLOC_NEW
#   include <stdlib.h>
#   include <crtdbg.h>
#   include <vld.h>
#endif//WIN32
*/


/** Platform dependent defines and includes
  */
#if WIN32
#   define WIN32_LEAN_AND_MEAN
#   define _WIN32_WINNT 0x0500
#   define NOMINMAX
#   include <windows.h>
#   include <winsock2.h>
#   include <sys/stat.h>
#   include <sys/timeb.h>
#   include <io.h>
#   include <process.h>
#else
#   include <arpa/inet.h>
#   include <netinet/in.h>
#   include <sys/socket.h>
#   include <sys/stat.h>
#   include <sys/time.h>
#   include <sys/types.h>
#   include <dirent.h>
#   include <execinfo.h>
#   include <fcntl.h>
#   include <netdb.h>
#   include <pthread.h>
#   include <unistd.h>
#endif//WIN32


/** set platform depended macros
  */
#if WIN32
#   define snprintf _snprintf
#   if _MSC_VER < 1500
#       define vsnprintf _vsnprintf
#   else /* _MSC_VER >= 1500 */
#       ifndef strdup
#           define strdup _strdup
#       endif
#   endif /* _MSC_VER >= 1500 */
#   define strncasecmp _strnicmp
#   define strcasecmp _stricmp
#   ifndef va_copy
#       define va_copy(a, b) ((va_list)((a) = (b)))
#   endif
#   define localtime_r(x, y) localtime_s(y, x)
#else
#   ifndef PATH_MAX
#       define MAX_PATH 1024
#   else
#       define MAX_PATH PATH_MAX
#   endif//PATH_MAX
#   define INVALID_SOCKET -1
#   define SOCKET_ERROR -1
#   ifndef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#       define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP {0, 0, 0, PTHREAD_MUTEX_RECURSIVE_NP, __LOCK_INITIALIZER}
#   endif
#   define _finite __finite
#   define _isnan __isnan
#endif


/** 'undefine' min / max so that there won't be major conflicts regarding std
  */
#ifdef min
#   undef min
#endif//min

#ifdef max
#  undef max
#endif//max


/** 'inlined' functions 'can' improve performance, the compiler will judge how this will be handled.
  * '__forceinline' functions can improve performance but only under certain circumstances
  * url: http://msdn.microsoft.com/en-us/library/z8y1yy88(VS.80).aspx
  */
#ifdef WIN32
#   define EVEMU_INLINE inline
#   define EVEMU_FORCEINLINE __forceinline
#else
#   define EVEMU_INLINE inline
#   define EVEMU_FORCEINLINE __attribute__((always_inline))
#endif//WIN32


/** dll interface stuff
  */
#ifdef WIN32
#  define DLLFUNC extern "C" __declspec(dllexport)
#else
#  define DLLFUNC extern "C"
#endif//WIN32

/** Use correct types for x64 platforms, too
  */
#ifdef WIN32
typedef signed __int64 int64;
typedef signed __int32 int32;
typedef signed __int16 int16;
typedef signed __int8 int8;

typedef unsigned __int64 uint64;
typedef unsigned __int32 uint32;
typedef unsigned __int16 uint16;
typedef unsigned __int8 uint8;
#else /* !WIN32 */
typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
#endif /* !WIN32 */

/** for printf() compatibility, I don't like this but its needed.
  */
#ifdef WIN32
#  define I64d "%I64d"
#  define I64u "%I64u"
#  define I64x "%I64x"
#  define I64X "%I64X"
#else
#  ifdef X64
#    define I64d "%ld"
#    define I64u "%lu"
#    define I64x "%lx"
#    define I64X "%lX"
#  else
#    define I64d "%lld"
#    define I64u "%llu"
#    define I64x "%llx"
#    define I64X "%llX"
#  endif//X64
#endif//WIN32

/** Return thread macro's
  * URL: http://msdn.microsoft.com/en-us/library/hw264s73(VS.80).aspx
  * Important Quote: "_endthread and _endthreadex cause C++ destructors pending in the thread not to be called."
  * Result: mem leaks under windows
  */
#ifdef WIN32
typedef void ThreadReturnType;
#  define THREAD_RETURN( x ) return
#else
typedef void* ThreadReturnType;
#  define THREAD_RETURN( x ) return ( x )
#endif


/** Basic programming tips
  * URL: http://nedprod.com/programs/index.html
  * Note: always nullify pointers after deletion, why? because its safer on a MT application
  */
#define ASCENT_ENABLE_SAFE_DELETE 1             // only delete and NULL after
#define ASCENT_ENABLE_EXTRA_SAFE_DELETE 1       // check the array for NULL pointer then delete and NULL after
//#define ASCENT_ENABLE_ULTRA_SAFE_DELETE 1     // check object and array for NULL pointer then delete and NULL after

#if defined ( ASCENT_ENABLE_ULTRA_SAFE_DELETE )
#   define SafeDelete( p )      { if( p != NULL ) { delete p; p = NULL; } }
#   define SafeDeleteArray( p ) { if( p != NULL ) { delete[] p; p = NULL; } }
#   define SafeFree( p )        { if( p != NULL ) { free( p ); p = NULL; } }
#elif defined ( ASCENT_ENABLE_EXTRA_SAFE_DELETE )
#   define SafeDelete( p )      { delete p; p = NULL; }
#   define SafeDeleteArray( p ) { if( p != NULL ) { delete[] p; p = NULL; } }
#   define SafeFree( p )        { if( p != NULL ) { free( p ); p = NULL; } }
#elif defined ( ASCENT_ENABLE_SAFE_DELETE )
#   define SafeDelete( p )      { delete p; p = NULL; }
#   define SafeDeleteArray( p ) { delete[] p; p = NULL; }
#   define SafeFree( p )        { free( p ); p = NULL; }
#else
#   define SafeDelete( p )      { delete p; }
#   define SafeDeleteArray( p ) { delete[] p; }
#   define SafeFree( p )        { free( p ); }
#endif


/** platform depended typedefs and prototypes
  */
#ifdef WIN32
int gettimeofday( timeval* tv, void* reserved );
int	asprintf( char** strp, const char* fmt, ... );
int	vasprintf( char** strp, const char* fmt, va_list ap );
#else /* !WIN32 */
typedef int SOCKET;
void Sleep( uint32 x );
uint32 GetTickCount();
#   ifndef __CYGWIN__
char* strupr( char* tmp );
char* strlwr( char* tmp );
#   endif//__CYGWIN__
#endif /* !WIN32 */

#endif
