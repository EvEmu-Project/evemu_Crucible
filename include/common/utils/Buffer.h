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
	Author:		Bloody.Rabbit
*/

#ifndef __BUFFER_H__INCL__
#define __BUFFER_H__INCL__

#include "utils/misc.h"

/**
 * @brief Generic class for buffers.
 *
 * This class incorporates all stuff which
 * may be handy when manipulating with buffers.
 *
 * @author Bloody.Rabbit
 */
class Buffer
{
public:
    /**
     * @brief Creates buffer of given length.
     *
     * @param[in] len   Length of buffer to be created.
     * @param[in] value Value to fill the buffer with.
     */
    Buffer( size_t len = 0, const uint8& value = 0 )
    : mBuffer( NULL ),
      mSize( 0 ),
      mCapacity( 0 ),
      mPosition( 0 )
    {
        Resize( len, value );
    }
    /**
     * @brief Create buffer with given content.
     *
     * @param[in] data New content of buffer.
     * @param[in] len  Length of buffer.
     */
    Buffer( const uint8* data, size_t len )
    : mBuffer( NULL ),
      mSize( 0 ),
      mCapacity( 0 ),
      mPosition( 0 )
    {
        Set( data, len );
    }
    /**
     * @brief Create buffer with given content.
     *
     * Takes ownership of data; invalidates
     * given pointer.
     *
     * @param[in] data New content of buffer.
     * @param[in] len  Length of buffer.
     */
    Buffer( uint8** data, size_t len )
    : mBuffer( *data ),
      mSize( len ),
      mCapacity( len ),
      mPosition( 0 )
    {
        // Invalidate the pointer
        *data = NULL;
    }
    /**
     * @brief Create buffer with duplicated content.
     *
     * @param[in] oth The other buffer; source of data.
     */
    Buffer( const Buffer& oth )
    : mBuffer( NULL ),
      mSize( 0 ),
      mCapacity( 0 ),
      mPosition( 0 )
    {
        // Use assigment operator
        *this = oth;
    }
    /** Deletes buffer. */
    ~Buffer()
    {
        // Free buffer
        SafeFree( mBuffer );
    }

    /********************************************************************/
    /* Read methods                                                     */
    /********************************************************************/
    /**
     * @brief Checks if given element can be read from stream.
     *
     * @retval true  Element can be read.
     * @retval false Not enough data for element.
     */
    template<typename X>
    bool isAvailable() const
    {
        return isAvailable( sizeof( X ) );
    }
    /**
     * @brief Checks whether stream contains given number of bytes.
     *
     * @param[in] len Number of bytes to be checked.
     *
     * @retval true  Stream contains at least given number of bytes.
     * @retval false Stream contains less than given number of bytes.
     */
    bool isAvailable( size_t len ) const
    {
        return ( mPosition + len <= size() );
    }

    /**
     * @brief Peeks element from buffer.
     *
     * @return Peeked element.
     */
    template<typename X>
    const X& Peek() const
    {
        return Peek<X>( 1 )[0];
    }
    /**
     * @brief Peeks multiple elements from buffer.
     *
     * @param[in] count Count of elements to peek.
     *
     * @return Array of peeked elements.
     */
    template<typename X>
    const X* Peek( size_t count ) const
    {
        assert( mPosition + count * sizeof( X ) <= size() );

        return &Get<X>( mPosition );
    }

    /**
     * @brief Reads element from buffer.
     *
     * @return Read element.
     */
    template<typename X>
    const X& Read() const
    {
        return Read<X>( 1 )[0];
    }
    /**
     * @brief Reads multiple elements from buffer.
     *
     * @param[in] count Count of elements to read.
     *
     * @return Array of read elements.
     */
    template<typename X>
    const X* Read( size_t count ) const
    {
        const X* res = Peek<X>( count );
        mPosition += count * sizeof( X );

        return res;
    }

    /**
     * @brief Gets element from buffer.
     *
     * @param[in] index Index of byte the element starts at.
     *
     * @return Reference to element.
     */
    template<typename X>
    X& Get( size_t index )
    {
        assert( index + sizeof( X ) <= size() );

        return *(X*)&mBuffer[ index ];
    }
    /**
     * @brief Gets const element from buffer.
     *
     * @param[in] index Index of byte the element starts at.
     *
     * @return Const reference to element.
     */
    template<typename X>
    const X& Get( size_t index ) const
    {
        assert( index + sizeof( X ) <= size() );

        return *(const X*)&mBuffer[ index ];
    }

    /**
     * @brief Overload of access operator[].
     *
     * @param[in] index Index of byte to be returned.
     *
     * @return Reference to required byte.
     */
    uint8& operator[]( size_t index ) { return Get<uint8>( index ); }
    /**
     * @brief Overload of const access operator[].
     *
     * @param[in] index Index of byte to be returned.
     *
     * @return Const reference to required byte.
     */
    const uint8& operator[]( size_t index ) const { return Get<uint8>( index ); }
    /**
     * @brief Reads element from buffer.
     *
     * @param[in] value Destination of read value.
     */
    template<typename X>
    const Buffer& operator>>( X& value ) const
    {
        value = Read<X>();
        return *this;
    }

    /********************************************************************/
    /* Write methods                                                    */
    /********************************************************************/
    /** Drops already read data. */
    void DropRead()
    {
        Set( &mBuffer[ mPosition ], size() - mPosition );

        mPosition = 0;
    }

    /**
     * @brief Sets content of buffer.
     *
     * @param[in] data Pointer to new content.
     * @param[in] len  Length of new content.
     */
    void Set( const uint8* data, size_t len )
    {
        Write( 0, data, len );
        Resize( len );
    }
    /**
     * @brief Sets content of buffer.
     *
     * @param[in] value New content.
     */
    template<typename X>
    void Set( const X& value )
    {
        Write( 0, value );
        Resize( sizeof( X ) );
    }

    /**
     * @brief Adds data into buffer.
     *
     * @param[in] data Pointer to data.
     * @param[in] len  Length of data.
     */
    void Write( const uint8* data, size_t len )
    {
        Write( size(), data, len );
    }
    /**
     * @brief Adds value into buffer.
     *
     * @param[in] value Value to be added into buffer.
     */
    template<typename X>
    void Write( const X& value )
    {
        Write( size(), value );
    }

    /**
     * @brief Writes data to buffer.
     *
     * @param[in] index Index at which data should be written.
     * @param[in] data  Pointer to data.
     * @param[in] len   Length of data.
     */
    void Write( size_t index, const uint8* data, size_t len )
    {
        assert( index <= size() );
        Reserve( index + len );

        // Use memmove because it allows overlapping
        memmove( &mBuffer[ index ], data, len );
        mSize = std::max( size(), index + len );
    }
    /**
     * @brief Writes value to buffer.
     *
     * @param[in] index Index at which value should be written.
     * @param[in] value Value to be written to buffer.
     */
    template<typename X>
    void Write( size_t index, const X& value )
    {
        assert( index <= size() );
        Reserve( index + sizeof( X ) );

        *(X*)&mBuffer[ index ] = value;
        mSize = std::max( size(), index + sizeof( X ) );
    }

    /**
     * @brief Adds value into buffer.
     *
     * @param[in] value Value to be added into buffer.
     */
    template<typename X>
    Buffer& operator<<( const X& value )
    {
        Write( value );
        return *this;
    }
    /**
     * @brief Sets content of buffer.
     *
     * @param[in] value New content.
     */
    template<typename X>
    Buffer& operator=( const X& value )
    {
        Set( value );
        return *this;
    }
    /**
     * Template above doesn't overload compiler-generated
     * assigment operator ... we must do it manually.
     */
    Buffer& operator=( const Buffer& value )
    {
        return operator=<Buffer>( value );
    }

    /********************************************************************/
    /* Size methods                                                     */
    /********************************************************************/
    /** @return Current size of buffer. */
    size_t size() const { return mSize; }
    /** @return Current capacity of buffer. */
    size_t capacity() const { return mCapacity; }

    /**
     * @brief Reserves (pre-allocates) memory for buffer.
     *
     * Should be used in cases where lazy reallocating
     * can negatively affect performance.
     *
     * @param[in] requiredSize The least size of buffer.
     */
    void Reserve( size_t requiredSize )
    {
        if( requiredSize > capacity() )
            _Reallocate( requiredSize );
    }
    /**
     * @brief Resizes buffer.
     *
     * Changes size of buffer, possibly reallocating it.
     *
     * If read position during shrink goes out of range,
     * it is set to the end of the buffer.
     *
     * @param[in] requiredSize The new size of buffer.
     * @param[in] value        When expanding buffer the gap will be filled by this value.
     */
    void Resize( size_t requiredSize, const uint8& value = 0 )
    {
        _Reallocate( requiredSize );

        if( requiredSize > size() )
            // A gap was created, fill it with value
            memset( &mBuffer[ size() ], value, requiredSize - size() );
        else if( requiredSize < mPosition )
            // Position went out of range,
            // set it to end of buffer.
            mPosition = requiredSize;

        mSize = requiredSize;
    }

protected:
    /** Pointer to start of buffer. */
    uint8* mBuffer;
    /** Current size of buffer. */
    size_t mSize;
    /** Current capacity of buffer. */
    size_t mCapacity;

    /** Current read position in stream. */
    mutable size_t mPosition;

    /**
     * @brief Reallocates buffer.
     *
     * Reallocates buffer so it can efficiently store
     * given amount of data.
     *
     * @param[in] requiredSize Least required new size of buffer.
     */
    void _Reallocate( size_t requiredSize )
    {
        size_t newCapacity = _CalcBufferCapacity( capacity(), requiredSize );
        assert( newCapacity >= requiredSize );

        if( newCapacity != capacity() )
        {
            mBuffer = (uint8*)realloc( mBuffer, newCapacity );
            mCapacity = newCapacity;
        }
    }

    /**
     * @brief Calculates buffer capacity.
     *
     * Based on current capacity and required size of the buffer,
     * this function calculates capacity of buffer to be allocated.
     *
     * @param[in] currentCapacity Current capacity of buffer.
     * @param[in] requiredSize    Required size of buffer.
     *
     * @return Capacity to be allocated.
     */
    static size_t _CalcBufferCapacity( size_t currentCapacity, size_t requiredSize )
    {
        size_t newCapacity = 0;

        if( requiredSize > 0x100 )
            newCapacity = (size_t)npowof2( requiredSize );
        else if( requiredSize > 0 )
            newCapacity = 0x100;

        if( requiredSize <= currentCapacity && currentCapacity < newCapacity )
            // This saves some memory ...
            return currentCapacity;
        else
            return newCapacity;
    }
};

/**
 * @brief Specialization for setting other buffers.
 *
 * @param[in] value Buffer with new content.
 */
template<>
EVEMU_INLINE void Buffer::Set<Buffer>( const Buffer& value )
{
    Set( &value[0], value.size() );
}

/**
 * @brief Specialization for adding other buffers.
 *
 * @param[in] value Other buffer the content of which
 *                  should be added.
 */
template<>
EVEMU_INLINE void Buffer::Write<Buffer>( const Buffer& value )
{
    Write( &value[0], value.size() );
}

/**
 * @brief Specialization for writing other buffers.
 *
 * @param[in] index Index at which buffer should be written.
 * @param[in] value Other buffer the content of which
 *                  should be written to this buffer.
 */
template<>
EVEMU_INLINE void Buffer::Write<Buffer>( size_t index, const Buffer& value )
{
    Write( index, &value[0], value.size() );
}

#endif /* !__BUFFER_H__INCL__ */
