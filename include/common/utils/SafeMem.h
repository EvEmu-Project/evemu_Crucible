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
    Author:        Bloody.Rabbit
*/

#ifndef __UTILS__SAFE_MEM_H__INCL__
#define __UTILS__SAFE_MEM_H__INCL__

/**
 * @brief Allocates and zero-initializes memory.
 *
 * Useful for allocating constructor-less C structs.
 *
 * @return A pointer to the allocated memory.
 */
template< typename T >
inline T* SafeAlloc()
{
    T* p = new T;
    ::memset( p, 0, sizeof( T ) );

    return p;
}

/**
 * @brief Allocates and zero-initializes an array.
 *
 * Useful for allocating constructor-less C struct arrays.
 *
 * @param[in] size Desired size of the array.
 *
 * @return A pointer to the array.
 */
template< typename T >
inline T* SafeAllocArray( size_t size )
{
    T* p = new T[ size ];
    ::memset( p, 0, size * sizeof( T ) );

    return p;
}

/**
 * @brief Deletes and nullifies a pointer.
 *
 * Basic programming tips
 * URL: http://nedprod.com/programs/index.html
 * Note: always nullify pointers after deletion, why? because its safer on a MT application
 *
 * @param[in] p The pointer.
 */
template< typename T >
inline void SafeDelete( T*& p )
{
    delete p;
    p = NULL;
}

/**
 * @brief Deletes and nullifies an array pointer.
 *
 * @param[in] p The array pointer.
 *
 * @see safeDelete< T >( T*& p )
 */
template< typename T >
inline void SafeDeleteArray( T*& p )
{
    delete[] p;
    p = NULL;
}

/**
 * @brief Frees and nullifies a pointer.
 *
 * @param[in] p The pointer.
 *
 * @see safeDelete< T >( T*& p )
 */
template< typename T >
inline void SafeFree( T*& p )
{
    ::free( p );
    p = NULL;
}

/**
 * @brief Releases pointer and nullifies it.
 *
 * @param[in] p The pointer.
 * @param[in] f The release function.
 */
template< typename T, typename F >
inline void SafeRelease( T*& p, F f )
{
    /* We don't know if the function accepts
       NULL pointers, like delete or free. */
    if( NULL != p )
        f( p );

    p = NULL;
}

#endif /* !__UTILS__SAFE_MEM_H__INCL__ */
