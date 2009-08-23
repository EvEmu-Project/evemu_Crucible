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

/** Build platform defines
  */
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#  ifndef WIN32
#    define WIN32
#  endif//WIN32
#endif


/** Visual Studio 'errors'/'warnings'
  */
#ifdef WIN32
#  ifdef _MSC_VER
//#    pragma warning(disable:4996)
//#    pragma warning(disable:4251) // dll-interface bullshit
#    define _CRT_SECURE_NO_WARNINGS 1
#    define _CRT_SECURE_NO_DEPRECATE 1
#    define _CRT_SECURE_COPP_OVERLOAD_STANDARD_NAMES 1
#    if _DEBUG
#      define _CRTDBG_MAP_ALLOC 1
#    endif// _DEBUG
#  else
#  endif//_MSC_VER
#endif//WIN32


/** 'inlined' functions 'can' improve performance, the compiler will judge how this will be handled.
  * '__forceinline' functions can improve performance but only under certain circumstances
  * url: http://msdn.microsoft.com/en-us/library/z8y1yy88(VS.80).aspx
  */
#if WIN32
#  ifdef _DEBUG
#    define EVEMU_INLINE
#    define ASCENT_FORCEINLINE __forceinline
#  else
#    define EVEMU_INLINE inline
#    define ASCENT_FORCEINLINE __forceinline
#  endif//_DEBUG
#else
#  define EVEMU_INLINE inline
#  define ASCENT_FORCEINLINE inline
#endif//WIN32


/** custom config include
  * Note: mainly for 'UNIX' builds
  */
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif//HAVE_CONFIG_H


/** Platform in depended includes
  */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <errno.h>


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


/** TR1 STL includes
  * Note: my fellow developers please read 'http://en.wikipedia.org/wiki/Technical_Report_1'. I know its a wiki page
  *       but it gives you the general idea.
  */
#ifdef WIN32
#  include <tuple>
#  include <unordered_map>
#  include <unordered_set>
#else
#  include <tr1/tuple>
#  include <tr1/unordered_map>
#  include <tr1/unordered_set>
#endif//WIN32

using namespace std;

/** Visual Studio memory leak detection
  */
#ifdef WIN32
#  ifdef _DEBUG
#    include <crtdbg.h>
#    include <new>
#    include <memory>
#    define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#  endif//_DEBUG
#endif//WIN32


/** visual leak detection, only enable when you know what your doing and have installed vlc
  */
/*
#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC_NEW
#include <stdlib.h>
#include <crtdbg.h>
#include <vld.h>
#endif//WIN32
*/


/** Platform 'critical' defines and includes
  */
#if WIN32
#  define WIN32_LEAN_AND_MEAN
#  define _WIN32_WINNT 0x0500
#  define NOMINMAX
#  include <windows.h>
#else
#  include <string.h>
#  ifndef PATH_MAX
#    define MAX_PATH 1024
#  else
#    define MAX_PATH PATH_MAX
#  endif//PATH_MAX
#endif//WIN32


/** 'undefine' min / max so that there won't be major conflicts regarding std
  */
#ifdef min
#  undef min
#endif//min

#ifdef max
#  undef max
#endif//max


/** fix Visual Studio "winsock2 vs. winsock issues"
  * Note: Packet Collector on win32 requires winsock.h due to latest pcap.h
  *       winsock.h must come before windows.h
  * Commented because: as the eve data stream is encrypted there is no reason to do this
  */
//#define _WINSOCKAPI_
//#if defined(WIN32) && ( defined(PACKETCOLLECTOR) || defined(COLLECTOR) )
//#  include <winsock.h>
//#endif


/** include platform depended headers
  */
#ifdef WIN32
#  include <winsock2.h>
#  include <process.h>
#else
#  include <unistd.h>
#  include <pthread.h>
#  include <sys/time.h>
#endif//WIN32


/** set platform depended macros
  */
#if WIN32
#  define snprintf _snprintf
#  if _MSC_VER < 1500
#    define vsnprintf _vsnprintf
#  else
#    ifndef strdup
#      define strdup _strdup
#    endif//strdup
#  endif
#  define strncasecmp _strnicmp
#  define strcasecmp _stricmp
#  ifndef va_copy
#    define va_copy(a, b) ((va_list)((a) = (b)))
#  endif
#else
#  ifndef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#    define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP {0, 0, 0, PTHREAD_MUTEX_RECURSIVE_NP, __LOCK_INITIALIZER}
#  endif
#endif


/** platform depended typedefs and prototypes
  */
#if !WIN32
  typedef int SOCKET;

  void Sleep(unsigned int x);
#  ifndef __CYGWIN__
    char* strupr(char* tmp);
    char* strlwr(char* tmp);
#  endif//__CYGWIN__
#endif//WIN32


#include "./types.h"

/* unix hack for GetTickCount */
#ifndef WIN32
inline uint32 GetTickCount()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
#endif//!WIN32
	
#endif
