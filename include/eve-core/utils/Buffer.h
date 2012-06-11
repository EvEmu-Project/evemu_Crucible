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
    Author:     Bloody.Rabbit
*/

#ifndef __UTILS__BUFFER_H__INCL__
#define __UTILS__BUFFER_H__INCL__

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
    /// Typedef for size type.
    typedef size_t size_type;

    /**
     * @brief Buffer's const iterator.
     *
     * @author Bloody.Rabbit
     */
    template< typename T >
    class const_iterator
    : public std::iterator< std::random_access_iterator_tag, T >
    {
        /// Typedef for our base due to readibility.
        typedef std::iterator< std::random_access_iterator_tag, T > _Base;

    public:
        /// Typedef for iterator category.
        typedef typename _Base::iterator_category iterator_category;
        /// Typedef for value type.
        typedef typename _Base::value_type        value_type;
        /// Typedef for difference type.
        typedef typename _Base::difference_type   difference_type;
        /// Typedef for pointer.
        typedef typename _Base::pointer           pointer;
        /// Typedef for reference.
        typedef typename _Base::reference         reference;

        /// Typedef for const pointer.
        typedef const T* const_pointer;
        /// Typedef for const reference.
        typedef const T& const_reference;

        /**
         * @brief Default constructor.
         *
         * @param[in] buffer The parent Buffer.
         * @param[in] index  The index.
         */
        const_iterator( const Buffer* buffer = NULL, size_type index = 0 )
        : mIndex( index ),
          mBuffer( buffer )
        {
        }
        /// Copy constructor.
        const_iterator( const const_iterator& oth )
        : mIndex( oth.mIndex ),
          mBuffer( oth.mBuffer )
        {
        }

        /// Copy operator.
        const_iterator& operator=( const const_iterator& oth )
        {
            mIndex = oth.mIndex;
            mBuffer = oth.mBuffer;
            return *this;
        }

        /**
         * @brief Converts const_iterator to another const_iterator
         *        with different type.
         *
         * @return The new const_iterator.
         */
        template< typename T2 >
        const_iterator< T2 > As() const { return const_iterator< T2 >( mBuffer, mIndex ); }

        /// Dereference operator.
        const_reference operator*() const
        {
            // make sure we have valid buffer
            assert( mBuffer );
            // make sure we're not going off the bounds
            assert( 1 <= mBuffer->end< value_type >() - *this );

            // obtain the value and return
            return *(const_pointer)&( mBuffer->mBuffer )[ mIndex ];
        }
        /// Dereference operator.
        const_pointer operator->() const { return &**this; }
        /// Subscript operator.
        const_reference operator[]( difference_type diff ) const { return *( *this + diff ); }

        /// Sum operator.
        const_iterator operator+( difference_type diff ) const
        {
            const_iterator res( *this );
            return ( res += diff );
        }
        /// Add operator.
        const_iterator& operator+=( difference_type diff )
        {
            // turn the diff into byte diff
            const difference_type res = ( diff * sizeof( value_type ) );

            // make sure we have valid buffer
            assert( mBuffer );
            // make sure we won't go negative
            assert( 0 <= mIndex + res );
            // make sure we won't go past end
            assert( mIndex + res <= mBuffer->size() );

            // set new index
            mIndex += res;

            return *this;
        }
        /// Preincrement operator.
        const_iterator& operator++() { return ( *this += 1 );  }
        /// Postincrement operator.
        const_iterator operator++( int )
        {
            const_iterator res( *this );
            ++*this;
            return res;
        }

        /// Diff operator.
        const_iterator operator-( difference_type diff ) const
        {
            const_iterator res( *this );
            return ( res -= diff );
        }
        /// Subtract operator.
        const_iterator& operator-=( difference_type diff ) { return ( *this += ( -diff ) ); }
        /// Predecrement operator.
        const_iterator& operator--() { return ( *this -= 1 ); }
        /// Postdecrement operator.
        const_iterator operator--( int )
        {
            const_iterator res( *this );
            --*this;
            return res;
        }

        /// Diff operator.
        difference_type operator-( const const_iterator& oth ) const
        {
            // make sure we have same parent buffer
            assert( oth.mBuffer == mBuffer );
            // return difference in element offset
            return ( ( mIndex - oth.mIndex ) / sizeof( value_type ) );
        }

        /// Equal operator.
        bool operator==( const const_iterator& oth ) const
        {
            // make sure we have same parent buffer
            assert( oth.mBuffer == mBuffer );
            // return the result
            return ( mIndex == oth.mIndex );
        }
        /// Non-equal operator.
        bool operator!=( const const_iterator& oth ) const { return !( *this == oth ); }

        /// Less-than operator.
        bool operator<( const const_iterator& oth ) const
        {
            // make sure we have same parent buffer
            assert( oth.mBuffer == mBuffer );
            // return the result
            return ( mIndex < oth.mIndex );
        }
        /// Greater-than operator.
        bool operator>( const const_iterator& oth ) const
        {
            // make sure we have same parent buffer
            assert( oth.mBuffer == mBuffer );
            // return the result
            return ( mIndex > oth.mIndex );
        }
        /// Less-equal operator.
        bool operator<=( const const_iterator& oth ) const { return !( *this > oth ); }
        /// Greater-equal operator.
        bool operator>=( const const_iterator& oth ) const { return !( *this < oth ); }

    protected:
        /// Index in buffer, in bytes.
        size_type mIndex;
        /// The parent Buffer.
        const Buffer* mBuffer;
    };

    /**
     * @brief Buffer's iterator.
     *
     * @author Bloody.Rabbit
     */
    template< typename T >
    class iterator
    : public const_iterator< T >
    {
        /// Typedef for our base due to readibility.
        typedef const_iterator< T > _Base;

    public:
        /// Typedef for iterator category.
        typedef typename _Base::iterator_category iterator_category;
        /// Typedef for value type.
        typedef typename _Base::value_type        value_type;
        /// Typedef for difference type.
        typedef typename _Base::difference_type   difference_type;
        /// Typedef for pointer.
        typedef typename _Base::pointer           pointer;
        /// Typedef for const pointer.
        typedef typename _Base::const_pointer     const_pointer;
        /// Typedef for reference.
        typedef typename _Base::reference         reference;
        /// Typedef for const reference.
        typedef typename _Base::const_reference   const_reference;

        /**
         * @brief Default constructor.
         *
         * @param[in] buffer The parent Buffer.
         * @param[in] index  The index.
         */
        iterator( Buffer* buffer = NULL, size_type index = 0 ) : _Base( buffer, index ) {}
        /// Copy constructor.
        iterator( const iterator& oth ) : _Base( oth ) {}

        /// Copy operator.
        iterator& operator=( const iterator& oth ) { *(_Base*)this = oth; return *this; }

        /**
         * @brief Converts iterator to another iterator
         *        with different type.
         *
         * @return The new iterator.
         */
        template< typename T2 >
        iterator< T2 > As() const { return iterator< T2 >( _Base::mBuffer, _Base::mIndex ); }

        /// Dereference operator.
        reference operator*() const { return const_cast< reference >( **(_Base*)this ); }
        /// Dereference operator.
        pointer operator->() const { return &**this; }
        /// Subscript operator.
        reference operator[]( difference_type diff ) const { return *( *this + diff ); }

        /// Sum operator.
        iterator operator+( difference_type diff ) const
        {
            iterator res( *this );
            return ( res += diff );
        }
        /// Add operator.
        iterator operator+=( difference_type diff ) { *(_Base*)this += diff; return *this; }
        /// Preincrement operator.
        iterator& operator++() { ++*(_Base*)this; return *this; }
        /// Postincrement operator.
        iterator operator++( int )
        {
            iterator res( *this );
            ++*this;
            return res;
        }

        /// Diff operator.
        iterator operator-( difference_type diff ) const
        {
            iterator res( *this );
            return ( res -= diff );
        }
        /// Subtract operator.
        iterator& operator-=( difference_type diff ) { *(_Base*)this -= diff; return *this; }
        /// Predecrement operator.
        iterator& operator--() { --*(_Base*)this; return *this; }
        /// Postdecrement operator.
        iterator operator--( int )
        {
            iterator res( *this );
            --*this;
            return res;
        }

        /// Diff operator.
        difference_type operator-( const _Base& oth ) const { return ( *(_Base*)this - oth ); }
    };

    /**
     * @brief Creates buffer of given length.
     *
     * Resizes buffer to be @a len bytes long, filling
     * it with @a value.
     *
     * @param[in] len  Length of buffer to be created.
     * @param[in] fill Value to fill the buffer with.
     */
    Buffer( size_type len = 0, const uint8& fill = 0 )
    : mBuffer( NULL ),
      mSize( 0 ),
      mCapacity( 0 )
    {
        /* unfortunately, we cannot use template here
           since it's not possible to explicitly instantiate
           constructor .... assuming uint8 */
        Resize< uint8 >( len, fill );
    }
    /**
     * @brief Creates buffer with given content.
     *
     * Fills buffer with content determined by
     * iterators @a first and @a last.
     *
     * @param[in] first Iterator pointing to first element.
     * @param[in] last  Iterator pointing to element after the last one.
     */
    template< typename Iter >
    Buffer( Iter first, Iter last )
    : mBuffer( NULL ),
      mSize( 0 ),
      mCapacity( 0 )
    {
        // assign the content
        AssignSeq( first, last );
    }
    /// Copy constructor.
    Buffer( const Buffer& oth )
    : mBuffer( NULL ),
      mSize( 0 ),
      mCapacity( 0 )
    {
        // Use assigment operator
        *this = oth;
    }
    /// Destructor; deletes buffer.
    ~Buffer()
    {
        // Free buffer
        SafeFree( mBuffer );
    }

    /********************************************************************/
    /* Read methods                                                     */
    /********************************************************************/
    /// @return iterator to begin.
    template< typename T >
    iterator< T > begin() { return iterator< T >( this, 0 ); }
    /// @return const_iterator to begin.
    template< typename T >
    const_iterator< T > begin() const { return const_iterator< T >( this, 0 ); }
    /// @return iterator to end.
    template< typename T >
    iterator< T > end() { return iterator< T >( this, size() ); }
    /// @return const_iterator to end.
    template< typename T >
    const_iterator< T > end() const { return const_iterator< T >( this, size() ); }

    /**
     * @brief Gets element from buffer.
     *
     * @param[in] index Index of element in the buffer.
     *
     * @return Reference to element.
     */
    template< typename T >
    T& Get( size_type index ) { return *( begin< T >() + index ); }
    /**
     * @brief Gets const element from buffer.
     *
     * @param[in] index Index of element in the buffer.
     *
     * @return Const reference to element.
     */
    template< typename T >
    const T& Get( size_type index ) const { return *( begin< T >() + index ); }

    /**
     * @brief Overload of access operator[].
     *
     * @param[in] index Index of byte to be returned.
     *
     * @return Reference to required byte.
     */
    uint8& operator[]( size_type index ) { return Get< uint8 >( index ); }
    /**
     * @brief Overload of const access operator[].
     *
     * @param[in] index Index of byte to be returned.
     *
     * @return Const reference to required byte.
     */
    const uint8& operator[]( size_type index ) const { return Get< uint8 >( index ); }

    /********************************************************************/
    /* Write methods                                                    */
    /********************************************************************/
    /**
     * @brief Appends a single value to buffer.
     *
     * @param[in] value Value to be appended.
     */
    template< typename T >
    void Append( const T& value )
    {
        // we wish to append to the end
        const const_iterator< T > index = end< T >();

        // make enough room; we're going to fill the gap immediately
        _ResizeAt< T >( index, 1 );

        // assign the value, filling the gap
        AssignAt< T >( index, value );
    }
    /**
     * @brief Appends a sequence of elements to buffer.
     *
     * @param[in] first Iterator pointing to first element.
     * @param[in] last  Iterator pointing to element after the last one.
     */
    template< typename Iter >
    void AppendSeq( Iter first, Iter last )
    {
        // we wish to append to the end
        const const_iterator< typename std::iterator_traits< Iter >::value_type >
            index = end< typename std::iterator_traits< Iter >::value_type >();

        // make enough room; we're going to fill the gap immediately
        _ResizeAt< typename std::iterator_traits< Iter >::value_type >( index, last - first );

        // assign the value, filling the gap
        AssignSeqAt< Iter >( index, first, last );
    }

    /**
     * @brief Assigns a single value to buffer.
     *
     * @param[in] value New content.
     */
    template< typename T >
    void Assign( const T& value )
    {
        // we wish to assign to beginning
        const const_iterator< T > index = begin< T >();

        // do we have enough space?
        if( 1 <= end< T >() - index )
        {
            // yes, we do: assign the value
            AssignAt< T >( index, value );

            // shrink the buffer; no gap will be created
            _ResizeAt< T >( index, 1 );
        }
        else
        {
            // no, we don't: make enough room; we're going to fill the gap immediately
            _ResizeAt< T >( index, 1 );

            // assign the value, filling the gap
            AssignAt< T >( index, value );
        }
    }
    /**
     * @brief Assigns a sequence of elements to buffer.
     *
     * @param[in] first Iterator pointing to first element.
     * @param[in] last  Iterator pointing to element after the last one.
     */
    template< typename Iter >
    void AssignSeq( Iter first, Iter last )
    {
        // we wish to assign to beginning
        const const_iterator< typename std::iterator_traits< Iter >::value_type >
            index = begin< typename std::iterator_traits< Iter >::value_type >();

        // do we have enough space?
        if( last - first <= end< typename std::iterator_traits< Iter >::value_type >() - index )
        {
            // yes, we do: assign the value
            AssignSeqAt< Iter >( index, first, last );

            // shrink the buffer; no gap will be created
            _ResizeAt< typename std::iterator_traits< Iter >::value_type >( index, last - first );
        }
        else
        {
            // no, we don't: make enough room; we're going to fill the gap immediately
            _ResizeAt< typename std::iterator_traits< Iter >::value_type >( index, last - first );

            // assign the value, filling the gap
            AssignSeqAt< Iter >( index, first, last );
        }
    }

    /**
     * @brief Assigns a single value to buffer at specific point.
     *
     * @param[in] index Point at which the value should be assigned.
     * @param[in] value New content.
     */
    template< typename T >
    void AssignAt( const_iterator< T > index, const T& value )
    {
        // make sure we're not going off the bounds
        assert( 1 <= end< T >() - index );

        // turn iterator into byte offset
        const size_type _index = ( index.template As< uint8 >() - begin< uint8 >() );
        // assign the value
        *(T*)&mBuffer[ _index ] = value;
    }
    /**
     * @brief Assigns a sequence of elements to buffer at specific point.
     *
     * @param[in] index Point at which the sequence of elements should be assigned.
     * @param[in] first Iterator pointing to first element.
     * @param[in] last  Iterator pointing to element after the last one.
     */
    template< typename Iter >
    void AssignSeqAt( const_iterator< typename std::iterator_traits< Iter >::value_type > index, Iter first, Iter last )
    {
        // make sure we're not going off the bounds
        assert( last - first <= end< typename std::iterator_traits< Iter >::value_type >() - index );

        // is there anything to assign?
        if( first != last )
        {
            // turn the iterator into byte offset
            const size_type _index = ( index.template As< uint8 >() - begin< uint8 >() );
            // obtain byte length of input data
            const size_type _len = sizeof( typename std::iterator_traits< Iter >::value_type ) * ( last - first );
            // assign the content
            memmove( &mBuffer[ _index ], &*first, _len );
        }
    }

    /**
     * @brief Appends a value to buffer.
     *
     * @param[in] value Value to be appended.
     */
    template< typename T >
    Buffer& operator<<( const T& value )
    {
        // append the value
        Append< T >( value );
        // return ourselves
        return *this;
    }
    /**
     * @brief Assigns new value to buffer.
     *
     * @param[in] value New content.
     */
    template< typename T >
    Buffer& operator=( const T& value )
    {
        // assign the value
        Assign< T >( value );
        // return ourselves
        return *this;
    }
    /// Copy operator.
    Buffer& operator=( const Buffer& value )
    {
        // assign new content
        AssignSeq( value.begin< uint8 >(), value.end< uint8 >() );
        // return ourselves
        return *this;
    }

    /********************************************************************/
    /* Size methods                                                     */
    /********************************************************************/
    /// @return Current size of buffer, in bytes.
    size_type size() const { return mSize; }
    /// @return Current capacity of buffer, in bytes.
    size_type capacity() const { return mCapacity; }

    /**
     * @brief Reserves (pre-allocates) memory for buffer.
     *
     * Pre-allocates memory for buffer to hold at least
     * @a requiredCount number of elements.
     *
     * Should be used in cases where lazy reallocating
     * can negatively affect performance.
     *
     * @param[in] requiredCount The least reserved number of elements.
     */
    template< typename T >
    void Reserve( size_type requiredCount )
    {
        // reserve at beginning
        ReserveAt< T >( begin< T >(), requiredCount );
    }

    /**
     * @brief Reserves (pre-allocates) memory for buffer at specific point.
     *
     * Pre-allocates memory for buffer to hold at least
     * @a requiredCount number of elements, counting from @a index.
     *
     * Should be used in cases where lazy reallocating
     * can negatively affect performance.
     *
     * @param[in] index         The point at which the memory should be reserved.
     * @param[in] requiredCount The least reserved number of elements.
     */
    template< typename T >
    void ReserveAt( const_iterator< T > index, size_type requiredCount )
    {
        // make sure we're not going off the bounds
        assert( index <= end< T >() );

        // turn iterator into byte offset
        const size_type _index = ( index.template As< uint8 >() - begin< uint8 >() );
        // obtain required size in bytes
        const size_type _requiredSize = sizeof( T ) * requiredCount;

        // reallocate if necessary
        if( _index + _requiredSize > capacity() )
            _Reallocate( _index + _requiredSize );
    }

    /**
     * @brief Resizes buffer.
     *
     * Changes size of buffer to hold @a requiredCount
     * number of elements, possibly reallocating it.
     *
     * @param[in] requiredCount The number of elements to hold.
     * @param[in] fill          During buffer expansion the gap will be filled by this value.
     */
    template< typename T >
    void Resize( size_type requiredCount, const uint8& fill = 0 )
    {
        // Resize at beginning
        ResizeAt< T >( begin< T >(), requiredCount, fill );
    }

    /**
     * @brief Resizes buffer.
     *
     * Changes size of buffer to hold @a requiredCount
     * number of elements, counting from @a index,
     * possibly reallocating it.
     *
     * @param[in] index         The point at which the buffer should be resized.
     * @param[in] requiredCount The number of elements to hold.
     * @param[in] fill          During buffer expansion the gap will be filled by this value.
     */
    template< typename T >
    void ResizeAt( const_iterator< T > index, size_type requiredCount, const uint8& fill = 0 )
    {
        // make sure we're not going off the bounds
        assert( index <= end< T >() );

        // keep old size
        const size_type _oldSize = size();
        // do actual resize
        _ResizeAt< T >( index, requiredCount );

        // turn iterator into byte offset
        const size_type _index = ( index.template As< uint8 >() - begin< uint8 >() );
        // obtain required size in bytes
        const size_type _requiredSize = sizeof( T ) * requiredCount;

        // has a gap been created?
        if( _index + _requiredSize > _oldSize )
            // fill it with value
            memset( &mBuffer[ _oldSize ], fill, _index + _requiredSize - _oldSize );
    }

protected:
    /// Pointer to start of buffer.
    uint8* mBuffer;
    /// Current size of buffer, in bytes.
    size_type mSize;
    /// Current capacity of buffer, in bytes.
    size_type mCapacity;

    /**
     * @brief Resizes buffer.
     *
     * Similar to Resize, but does not care
     * about the gaps that may be created.
     *
     * @param[in] requiredCount The number of elements to hold.
     */
    template< typename T >
    void _Resize( size_type requiredCount )
    {
        // resize at beginning
        _ResizeAt< T >( begin< T >(), requiredCount );
    }

    /**
     * @brief Resizes buffer.
     *
     * Similar to ResizeAt, but does not care
     * about the gaps that may be created.
     *
     * @param[in] index         The point at which the buffer should be resized.
     * @param[in] requiredCount The number of elements to hold.
     */
    template< typename T >
    void _ResizeAt( const_iterator< T > index, size_type requiredCount )
    {
        // make sure we're not going off the bounds
        assert( index <= end< T >() );

        // turn index into byte offset
        const size_type _index = ( index.template As< uint8 >() - begin< uint8 >() );
        // obtain required size in bytes
        const size_type _requiredSize = sizeof( T ) * requiredCount;

        // reallocate
        _Reallocate( _index + _requiredSize );
        // set new size
        mSize = ( _index + _requiredSize );
    }

    /**
     * @brief Reallocates buffer.
     *
     * Reallocates buffer so it can efficiently store
     * given amount of data.
     *
     * @param[in] requiredSize The least required new size of buffer, in bytes.
     */
    void _Reallocate( size_type requiredSize )
    {
        // calculate new capacity for required size
        size_type newCapacity = _CalcBufferCapacity( capacity(), requiredSize );
        // make sure new capacity is bigger than required size
        assert( requiredSize <= newCapacity );

        // has the capacity changed?
        if( newCapacity != capacity() )
        {
            // reallocate
            mBuffer = (uint8*)realloc( mBuffer, newCapacity );
            // set new capacity
            mCapacity = newCapacity;
        }
    }

    /**
     * @brief Calculates buffer capacity.
     *
     * Based on current capacity and required size of the buffer,
     * this function calculates capacity of buffer to be allocated.
     *
     * @param[in] currentCapacity Current capacity of buffer, in bytes.
     * @param[in] requiredSize    Required size of buffer, in bytes.
     *
     * @return Capacity to be allocated.
     */
    static size_type _CalcBufferCapacity( size_type currentCapacity, size_type requiredSize )
    {
        size_type newCapacity = 0;

        // if more than 0x100 bytes required, return next power of 2
        if( 0x100 < requiredSize )
            newCapacity = (size_type)npowof2( requiredSize );
        // else if non-zero, return 0x100 bytes
        else if( 0 < requiredSize )
            newCapacity = 0x100;
        // else return 0 bytes

        /* if current capacity is sufficient and at the same time smaller
           than the new capacity, return current one ... saves resources */
        if( requiredSize <= currentCapacity && currentCapacity < newCapacity )
            return currentCapacity;
        else
            return newCapacity;
    }
};

#endif /* !__UTILS__BUFFER_H__INCL__ */
