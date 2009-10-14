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
     * @brief Creates (possibly pre-allocated) empty buffer.
     *
     * @param[in] len Length to pre-allocate.
     */
    explicit Buffer( size_t len = 0 )
    : mBuffer( NULL ),
      mSize( 0 ),
      mSizeAllocated( 0 )
    {
        // Reserve required memory
        Reserve( len );
    }
    /**
     * @brief Creates buffer of given length.
     *
     * @param[in] len   Length of buffer to be created.
     * @param[in] value Value to fill the buffer with.
     */
    Buffer( size_t len, const uint8& value )
    : mBuffer( NULL ),
      mSize( 0 ),
      mSizeAllocated( 0 )
    {
        // Resize to required size
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
      mSizeAllocated( 0 )
    {
        // Add data into buffer
        Add( data, len );
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
      mSizeAllocated( len )
    {
        // Invalidate the pointer
        *data = NULL;
    }
    /**
     * @brief Deletes buffer.
     */
    ~Buffer()
    {
        // Free buffer
        SafeFree( mBuffer );
    }

    /********************************************************************/
    /* Read methods                                                     */
    /********************************************************************/
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

    /********************************************************************/
    /* Write methods                                                    */
    /********************************************************************/
    /**
     * @brief Adds data into buffer.
     *
     * @param[in] data Pointer to data.
     * @param[in] len  Length of data.
     */
    void Add( const uint8* data, size_t len )
    {
        // Append data
        Write( size(), data, len );
    }
    /**
     * @brief Adds value into buffer.
     *
     * @param[in] value Value to be added into buffer.
     */
    template<typename X>
    void Add( const X& value )
    {
        // Append value
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
        // Add value
        Add( value );

        // Return ourselves
        return *this;
    }

    /********************************************************************/
    /* Size methods                                                     */
    /********************************************************************/
    /** @return Current size of buffer. */
    size_t size() const { return mSize; }

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
        if( requiredSize > mSizeAllocated )
            _Reallocate( requiredSize );
    }
    /**
     * @brief Resizes buffer.
     *
     * Changes size of buffer, possibly reallocating it.
     *
     * @param[in] requiredSize The new size of buffer.
     * @param[in] value        When expanding buffer the gap will be filled by this value.
     */
    void Resize( size_t requiredSize, const uint8& value )
    {
        // Reallocate
        _Reallocate( requiredSize );

        // If a gap was created, fill it with value
        if( requiredSize > size() )
            memset( &mBuffer[ size() ], value, requiredSize - size() );

        // Set new size
        mSize = requiredSize;
    }

protected:
    /** Pointer to start of buffer. */
    uint8* mBuffer;
    /** Current size of buffer. */
    size_t mSize;
    /** Actual allocated memory for buffer. */
    size_t mSizeAllocated;

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
        // Calculate new size
        size_t newSize = _CalcBufferSize( mSizeAllocated, requiredSize );
        assert( newSize >= requiredSize );
        if( newSize != mSizeAllocated )
        {
            // Reallocate
            mBuffer = (uint8*)realloc( mBuffer, newSize );
            mSizeAllocated = newSize;
        }
    }

    /**
     * @brief Calculates buffer size.
     *
     * Based on current and required size of the buffer,
     * this function calculates size of buffer to be allocated.
     *
     * @param[in] currentSize  Current size of buffer.
     * @param[in] requiredSize Required size of buffer.
     *
     * @return Size of buffer to be allocated.
     */
    static size_t _CalcBufferSize( size_t currentSize, size_t requiredSize )
    {
        // BUFFER_SIZE_SNAP must consist of 1 bit, otherwise stuff below won't work!
        static const size_t BUFFER_SIZE_SNAP = ( 1L << 10 ); // 1 kB

        // Calculate multiple of BUFFER_SIZE_SNAP equal or greater than requiredSize:
        size_t newSize = ( requiredSize + BUFFER_SIZE_SNAP - 1 ) & ~( BUFFER_SIZE_SNAP - 1 );

        if( requiredSize <= currentSize && currentSize < newSize )
            // This saves some memory ...
            return currentSize;
        else
            return newSize;
    }
};

/**
 * @brief Specialization for adding other buffers.
 *
 * @param[in] value Other buffer the content of which
 *                  should be added.
 */
template<>
EVEMU_INLINE void Buffer::Add<Buffer>( const Buffer& value )
{
    // Append data
    Write( size(), &value[0], value.size() );
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
