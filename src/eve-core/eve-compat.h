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
    Author:        Aim, Captnoord, Zhur, Bloody.Rabbit
*/

#ifndef __EVE_COMPAT_H__INCL__
#define __EVE_COMPAT_H__INCL__

/*************************************************************************/
/* inttypes.h                                                            */
/*************************************************************************/
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
//typedef uint64_t uint64;
#else /* !HAVE_INTTYPES_H */
typedef   signed __int8   int8;
typedef unsigned __int8  uint8;
typedef   signed __int16  int16;
typedef unsigned __int16 uint16;
typedef   signed __int32  int32;
typedef unsigned __int32 uint32;
typedef   signed __int64  int64;
//typedef unsigned __int64 uint64;
#endif /* !HAVE_INTTYPES_H */

/*************************************************************************/
/* windows.h                                                             */
/*************************************************************************/
#ifndef HAVE_WINDOWS_H
void   Sleep( uint32 x );
uint32 GetTickCount();
int    CreateDirectory( const char* name, void* );
#endif /* !HAVE_WINDOWS_H */

/*************************************************************************/
/* winsock2.h                                                            */
/*************************************************************************/
#ifdef HAVE_WINSOCK2_H
typedef int SOCKLEN_T;
typedef SOCKLEN_T* PSOCKLEN_T;

#   define SHUT_RD      SD_RECEIVE
#   define SHUT_WR      SD_SEND
#   define SHUT_RDWR    SD_BOTH

#   define MSG_DONTWAIT 0
#   define MSG_MORE     MSG_PARTIAL
#else
typedef int SOCKET;
typedef unsigned int SOCKLEN_T;
typedef SOCKLEN_T* PSOCKLEN_T;

#   define INVALID_SOCKET -1
#   define SOCKET_ERROR   -1
#endif /* !HAVE_WINSOCK2_H */

#ifndef MSG_NOSIGNAL
#   ifdef SO_NOSIGPIPE
#       define MSG_NOSIGNAL SO_NOSIGPIPE
#   else /* !SO_NOSIGPIPE */
#       define MSG_NOSIGNAL 0
#   endif /* !SO_NOSIGPIPE */
#endif /* !MSG_NOSIGNAL */

/*************************************************************************/
/* cfloat, cmath                                                         */
/*************************************************************************/
#ifndef HAVE_ASINH
#   define asinh boost::math::asinh
#endif /* !HAVE_ASINH */

#ifndef HAVE_ISNAN
//#   if defined( HAVE_STD_ISNAN )
//#       define isnan std::isnan
//#   if defined( HAVE__ISNAN )
//#       define isnan _isnan
//#   elif defined( HAVE___ISNAN )
//#       define isnan __isnan
//#   endif
#endif /* !HAVE_ISNAN */

/*************************************************************************/
/* cstdarg                                                               */
/*************************************************************************/
#ifndef HAVE_VA_COPY
    // Use memcpy() because of possible array implementation.
#   define va_copy( a, b ) ::memcpy( &( a ), &( b ), sizeof( va_list ) )
#endif /* !HAVE_VA_COPY */

/*************************************************************************/
/* cstdio                                                                */
/*************************************************************************/
#ifndef HAVE_SNPRINTF
#   define snprintf _snprintf
#endif /* !HAVE_SNPRINTF */

#ifndef HAVE_VSNPRINTF
#   define vsnprintf _vsnprintf
#endif /* !HAVE_VSNPRINTF */

#ifndef HAVE_ASPRINTF
int asprintf( char** strp, const char* fmt, ... );
#endif /* !HAVE_ASPRINTF */

#ifndef HAVE_VASPRINTF
int vasprintf( char** strp, const char* fmt, va_list ap );
#endif /* !HAVE_VASPRINTF */

/**
 * @brief <code>sprintf</code> for <code>std::string</code>.
 *
 * @param[in] fmt The format string.
 * @param[in] ... Arguments.
 *
 * @return The printed string.
 */
std::string sprintf( const char* fmt, ... );
/**
 * @brief <code>vsprintf</code> for <code>std::string</code>.
 *
 * @param[in] fmt The format string.
 * @param[in] ap  Arguments.
 *
 * @return The printed string.
 */
std::string vsprintf( const char* fmt, va_list ap );

/**
 * @brief <code>sprintf</code> for <code>std::string</code>.
 *
 * @param[out] str Where to store the result.
 * @param[in]  fmt The format string.
 * @param[in]  ... Arguments.
 *
 * @return A value returned by <code>vsprintf</code>.
 */
int sprintf( std::string& str, const char* fmt, ... );
/**
 * @brief <code>vsprintf</code> for <code>std::string</code>.
 *
 * @param[out] str Where to store the result.
 * @param[in]  fmt The format string.
 * @param[in]  ap  Arguments.
 *
 * @return A value returned by <code>vsnprintf</code>.
 */
int vsprintf( std::string& str, const char* fmt, va_list ap );

/*************************************************************************/
/* cstdlib                                                               */
/*************************************************************************/
#ifndef HAVE_STRTOLL
#   define strtoll _strtoi64
#endif /* !HAVE_STRTOLL */

#ifndef HAVE_STRTOULL
#   define strtoull _strtoui64
#endif /* !HAVE_STRTOULL */

#ifndef HAVE_STRTOF
float strtof( const char* nptr, char** endptr );
#endif /* !HAVE_STRTOF */

/*************************************************************************/
/* cstring                                                               */
/*************************************************************************/
#ifndef HAVE_STRDUP
#   define strdup _strdup
#endif /* !HAVE_STRDUP */

#ifndef HAVE_STRCASECMP
#   define strcasecmp _stricmp
#endif /* !HAVE_STRCASECMP */

#ifndef HAVE_STRNCASECMP
#   define strncasecmp _strnicmp
#endif /* !HAVE_STRNCASECMP */

/*************************************************************************/
/* ctime                                                                 */
/*************************************************************************/
#ifndef HAVE_LOCALTIME_R
tm* localtime_r( const time_t* timep, tm* result );
#endif /* !HAVE_LOCALTIME_R */

#endif /* !__EVE_COMPAT_H__INCL__ */
