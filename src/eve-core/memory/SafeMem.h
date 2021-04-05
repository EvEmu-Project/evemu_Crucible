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
    Author:        Bloody.Rabbit
*/

#ifndef __UTILS__SAFE_MEM_H__INCL__
#define __UTILS__SAFE_MEM_H__INCL__

#include <cstddef>
#include <type_traits>
#include <memory>
#include <utility>
#include <memory.h>
#include <stdlib.h>


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
 * (Allan) by setting a pointer to the null pointer, you have successfully eliminated the only chance to detect double deletion
 *
 * @param[in] p The pointer.
 */
template< typename T >
inline void SafeDelete( T*& p )
{
    delete p;
    p = nullptr;
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
    p = nullptr;
}

/**
 * @brief Frees and nullifies an array pointer.
 *
 * @param[in] p The pointer.
 *
 * @see safeDelete< T >( T*& p )
 */
template< typename T >
inline void SafeFree( T*& p )
{
    ::free( p );
    p = nullptr;
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

    p = nullptr;
}

/*  make_unique code taken from Stephan T. Lavavej's proposal for adding to C++0x14 (N3656 revision) */
/* unique_ptr for single items   -allan 7Mar16  */
template<class T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/* unique_ptr for arrays  -allan 9Mar16  */
template<class T>
std::unique_ptr<T> make_uniquea(size_t n) {
    //typedef typename remove_extent<T>::type U;
    return std::unique_ptr<T>(new T[n]());
}

template<class T>
class myPtr
{
public:
    explicit myPtr(T* pointer) : ptr(pointer) { }
    ~myPtr() { delete ptr; }
    T* get() const { return ptr; }
    T* operator->() const { return ptr; }
    T* operator*() const { return *ptr; }

private:
    T* ptr;
};

#endif /* !__UTILS__SAFE_MEM_H__INCL__ */
