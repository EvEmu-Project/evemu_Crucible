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
    Author:        Zhur
*/

#ifndef EVE_UNMARSHAL_H
#define EVE_UNMARSHAL_H

#include <cstring>

#include "python/PyRep.h"


/**
 * @brief Turns marshal stream into Python object.
 *
 * @param[in] data Marshal stream.
 *
 * @return Ownership of Python object.
 */
extern PyRep* Unmarshal( const Buffer& data );
/**
 * @brief Turns possibly inflated marshal stream into Python object.
 *
 * @param[in] data Possibly inflated marshal stream.
 *
 * @return Ownership of Python object.
*/
extern PyRep* InflateUnmarshal( const Buffer& data );

/**
 * @brief Class which turns marshal bytecode into Python object.
 *
 * @author Zhur, Bloody.Rabbit
 */
class UnmarshalStream
{
public:
    UnmarshalStream()
    : mStoredObjects( nullptr )
    {
    }

    ~UnmarshalStream();

    /**
     * @brief Loads Python object from given bytecode.
     *
     * @param[in] data Buffer containing marshal bytecode.
     *
     * @return Loaded Python object.
     */
    PyRep* Load( const Buffer& data );

protected:
    /** Reads extended size from stream. */
    bool ReadSizeEx( uint32& size );

    /** Reads a primitive value without assuming alignment. */
    template<typename T>
    bool ReadValue( T& value )
    {
        if( sizeof( T ) > RemainingBytes() )
        {
            mFailed = true;
            return false;
        }

        memcpy( &value, &*mInItr, sizeof( T ) );
        mInItr += sizeof( T );
        return true;
    }

    /** Peeks a primitive value without assuming alignment. */
    template<typename T>
    bool PeekValue( T& value ) const
    {
        if( sizeof( T ) > RemainingBytes() )
            return false;

        memcpy( &value, &*mInItr, sizeof( T ) );
        return true;
    }

    /** Reads a byte range and advances the input cursor. */
    bool ReadBytes( size_t count,
                    Buffer::const_iterator<uint8>& bytes );

    /** Returns the unread payload length. */
    size_t RemainingBytes() const;

    /** Rejects malformed or excessively expensive object graphs. */
    bool BeginRep();
    void EndRep();

    /** Validates a counted container before allocation. */
    bool ValidateContainerCount( uint32 count,
                                 size_t minimumBytesPerItem );

    /** Marks the current stream invalid. */
    void MarkFailed() { mFailed = true; }

    /** Initializes loading and loads rep from stream. */
    PyRep* LoadStream( size_t streamLength );

    /** Loads rep from stream. */
    PyRep* LoadRep();

    /**
     * @brief Initializes object store.
     *
     * @param[in] streamLength Length of stream.
     * @param[in] saveCount    Number of saved objects within the stream.
     */
    bool CreateObjectStore( size_t streamLength, uint32 saveCount );
    /**
     * @brief Destroys object store.
     */
    void DestroyObjectStore();

    /**
     * @brief Obtains storage index for StoreObject.
     *
     * @return Storage index.
     */
    bool GetStorageIndex( uint32& index );
    /**
     * @brief Obtains previously stored object.
     *
     * @param[in] index Index of stored object.
     *
     * @return The stored object.
     */
    PyRep* GetStoredObject( uint32 index );
    /**
     * @brief Stores object.
     *
     * @param[in] index  Index of object.
     * @param[in] object The object to be stored.
     */
    bool StoreObject( uint32 index, PyRep* object );

private:
    /** Loads none from stream. */
    PyRep* LoadNone() { return PyStatic.NewNone(); }

    /** Loads true boolean from stream. */
    PyRep* LoadBoolTrue() { return PyStatic.NewTrue(); }
    /** Loads false boolean from stream. */
    PyRep* LoadBoolFalse() { return PyStatic.NewFalse(); }

    /** Loads long long integer from stream. */
    PyRep* LoadIntegerLongLong()
    {
        int64 value = 0;
        if( !ReadValue( value ) )
            return nullptr;
        return new PyLong( value );
    }
    /** Loads long integer from stream. */
    PyRep* LoadIntegerLong()
    {
        int32 value = 0;
        if( !ReadValue( value ) )
            return nullptr;
        return new PyInt( value );
    }
    /** Loads signed short from stream. */
    PyRep* LoadIntegerSignedShort()
    {
        int16 value = 0;
        if( !ReadValue( value ) )
            return nullptr;
        return new PyInt( value );
    }
    /** Loads byte integer from stream. */
    PyRep* LoadIntegerByte()
    {
        int8 value = 0;
        if( !ReadValue( value ) )
            return nullptr;
        return new PyInt( value );
    }
    /** Loads variable length integer from stream. */
    PyRep* LoadIntegerVar();
    /** Loads minus one integer from stream. */
    PyRep* LoadIntegerMinusOne() { return new PyInt( -1 ); }
    /** Loads zero integer from stream. */
    PyRep* LoadIntegerZero() { return new PyInt( 0 ); }
    /** Loads one integer from stream. */
    PyRep* LoadIntegerOne() { return new PyInt( 1 ); }

    /** Loads real from stream. */
    PyRep* LoadReal()
    {
        double value = 0.0;
        if( !ReadValue( value ) )
            return nullptr;
        return new PyFloat( value );
    }
    /** Loads zero real from stream. */
    PyRep* LoadRealZero() { return new PyFloat( 0.0 ); }

    /** Loads empty string from stream. */
    PyRep* LoadStringEmpty() { return new PyString( "" ); }
    /** Loads single character string from stream. */
    PyRep* LoadStringChar();
    /** Loads short (up to 255 chars) string from stream. */
    PyRep* LoadStringShort();
    /** Loads long (no limit) string from stream. */
    PyRep* LoadStringLong();
    /** Loads table string from stream. */
    PyRep* LoadStringTable();

    /** Loads empty wide string from stream. */
    PyRep* LoadWStringEmpty() { return new PyWString( "", 0 ); }
    /** Loads single UCS-2 character string from stream. */
    PyRep* LoadWStringUCS2Char();
    /** Loads UCS-2 string from stream. */
    PyRep* LoadWStringUCS2();
    /** Loads UTF-8 string from stream. */
    PyRep* LoadWStringUTF8();

    /** Loads token from stream. */
    PyRep* LoadToken();

    /** Loads buffer from stream. */
    PyRep* LoadBuffer();

    /** Loads empty tuple from stream. */
    PyRep* LoadTupleEmpty() { return new PyTuple( 0 ); }
    /** Loads tuple from stream. */
    PyRep* LoadTuple();
    /** Loads one-element tuple from stream. */
    PyRep* LoadTupleOne();
    /** Loads two-element tuple from stream. */
    PyRep* LoadTupleTwo();

    /** Loads empty list from stream. */
    PyRep* LoadListEmpty() { return new PyList( 0 ); }
    /** Loads list from stream. */
    PyRep* LoadList();
    /** Loads one-element list from stream. */
    PyRep* LoadListOne();

    /** Loads dict from stream. */
    PyRep* LoadDict();

    /** Loads object from stream. */
    PyRep* LoadObject();
    /** Loads extended object of type 1 from stream. */
    PyRep* LoadObjectEx1();
    /** Loads extended object of type 2 from stream. */
    PyRep* LoadObjectEx2();

    /** Loads sub stream from stream. */
    PyRep* LoadSubStream();
    /** Loads sub struct from stream. */
    PyRep* LoadSubStruct();
    /** Loads checksumed stream from stream. */
    PyRep* LoadChecksumedStream();

    /** Loads packed row from stream. */
    PyRep* LoadPackedRow();

    /** Prints error and returns NULL. */
    PyRep* LoadError();
    /** Loads saved stream element from stream. */
    PyRep* LoadSavedStreamElement();

    /** Helper; loads extended object from stream. */
    PyObjectEx* LoadObjectEx( bool is_type_2 );
    /** Helper; loads zero-compressed buffer from stream. */
    bool LoadRLE(Buffer& out );

    /** Buffer iterator we are processing. */
    Buffer::const_iterator<uint8> mInItr;
    /** End of the complete input buffer. */
    Buffer::const_iterator<uint8> mEndItr;
    /** End of payload, excluding the saved-object index table. */
    Buffer::const_iterator<uint8> mPayloadEnd;
    /** Input buffer identity used to validate iterator state. */
    const Buffer* mData = nullptr;

    /** Next store index for referencing in the buffer. */
    Buffer::const_iterator<uint8> mStoreIndexItr;
    /** End of saved-object index table. */
    Buffer::const_iterator<uint8> mStoreIndexEnd;
    /** Referenced objects within the buffer. */
    PyList* mStoredObjects;

    /** Stream validation state. */
    bool mFailed = false;
    size_t mDepth = 0;
    size_t mObjectCount = 0;

    /** Load function map. */
    static PyRep* ( UnmarshalStream::* const s_mLoadMap[] )();
};

#endif

