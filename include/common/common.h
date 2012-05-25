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

/*************************************************************************/
/* Header configuration                                                  */
/*************************************************************************/
#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef WIN32
#   define _WIN32_WINNT 0x0500 // building for Win2k
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#endif /* !WIN32 */

#ifdef MSVC
#   define _CRT_SECURE_NO_WARNINGS                  1
#   define _CRT_SECURE_NO_DEPRECATE                 1
#   define _CRT_SECURE_COPP_OVERLOAD_STANDARD_NAMES 1
#   define _SCL_SECURE_NO_WARNINGS                  1
#endif /* _MSC_VER */

#ifdef HAVE_CRTDBG_H
#   define _CRTDBG_MAP_ALLOC 1
#endif /* HAVE_CRTDBG_H */

#ifdef HAVE_INTTYPES_H
    // We must "explicitly request" the format strings ...
#   define __STDC_FORMAT_MACROS 1
#endif /* HAVE_INTTYPES_H */

/*************************************************************************/
/* Includes                                                              */
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

#ifdef WIN32
#   include <process.h>
#   include <windows.h>
#   include <winsock2.h>
#else /* !WIN32 */
#   include <dirent.h>
#   include <execinfo.h>
#   include <fcntl.h>
#   include <netdb.h>
#   include <pthread.h>
#   include <unistd.h>
#   include <arpa/inet.h>
#   include <netinet/in.h>
#   include <sys/socket.h>
#endif /* !WIN32 */

#ifdef MSVC
#   include <io.h>
#   include <sys/stat.h>
#   include <sys/timeb.h>
#else /* !MSVC */
#   include <sys/stat.h>
#   include <sys/time.h>
#   include <sys/types.h>
#endif /* !MSVC */

#ifdef HAVE_CRTDBG_H
#   include <crtdbg.h>
#endif /* HAVE_CRTDBG_H */

#ifdef HAVE_INTTYPES_H
#   include <inttypes.h>
#endif /* HAVE_INTTYPES_H */

#ifdef HAVE_VLD_H
#   include <vld.h>
#endif /* HAVE_VLD_H */

#ifndef HAVE_ASINH
#   include <boost/math/special_functions.hpp>
#endif /* !HAVE_ASINH */

/************************************************************************/
/* Dependencies                                                         */
/************************************************************************/
#include <asio.hpp>

#include <mysql.h>
#include <mysqld_error.h>
#include <errmsg.h>

#include <tinyxml.h>

#include <zlib.h>

/*************************************************************************/
/* Other stuff included by default                                       */
/*************************************************************************/
#include "compat.h"

#include "utils/FastInt.h"
#include "utils/SafeMem.h"

#endif /* !__COMMON_H__INCL__ */
