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
	Author:		Zhur
*/

#ifndef EVE_UNMARSHAL_H
#define EVE_UNMARSHAL_H

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
    : mBuffer( NULL ),
      mStoreIndex( 0 ),
      mStoredObjects( NULL )
    {
    }

    /**
     * @brief Loads Python object from given bytecode.
     *
     * @param[in] data Buffer containing marshal bytecode.
     *
     * @return Loaded Python object.
     */
    PyRep* Load( const Buffer& data );

protected:
    /** Peeks element from stream. */
    template<typename X>
    const X& Peek() const { return mBuffer->Peek<X>(); }
    /** Peeks elements from stream. */
    template<typename X>
    const X* Peek( size_t count ) const { return mBuffer->Peek<X>( count ); }

    /** Reads element from stream. */
    template<typename X>
    const X& Read() const { return mBuffer->Read<X>(); }
    /** Reads elements from stream. */
    template<typename X>
    const X* Read( size_t count ) const { return mBuffer->Read<X>( count ); }

    /** Reads extended size from stream. */
    uint32 ReadSizeEx() const
    {
        uint32 size = Read<uint8>();
        if( 0xFF == size )
            size = Read<uint32>();

        return size;
    }

    /** Initializes loading and loads rep from stream. */
    PyRep* LoadStream();

    /** Loads rep from stream. */
    PyRep* LoadRep();

    /**
     * @brief Initializes object store.
     *
     * @param[in] saveCount Number of saved objects within the stream.
     */
    void CreateObjectStore( uint32 saveCount );
    /**
     * @brief Destroys object store.
     */
    void DestroyObjectStore();

    /**
     * @brief Obtains previously stored object.
     *
     * @param[in] index Index of stored object.
     *
     * @return The stored object.
     */
    PyRep* GetStoredObject( uint32 index );
    /**
     * @brief Obtains storage index for StoreObject.
     *
     * @return Storage index.
     */
    uint32 GetStorageIndex();
    /**
     * @brief Stores object.
     *
     * @param[in] index  Index of object.
     * @param[in] object The object to be stored.
     */
    void StoreObject( uint32 index, PyRep* object );

private:
    /** Loads none from stream. */
    PyRep* LoadNone() { return new PyNone; }

    /** Loads true boolean from stream. */
    PyRep* LoadBoolTrue() { return new PyBool( true ); }
    /** Loads false boolean from stream. */
    PyRep* LoadBoolFalse() { return new PyBool( false ); }

    /** Loads long long integer from stream. */
    PyRep* LoadIntegerLongLong() { return new PyLong( Read<int64>() ); }
    /** Loads long integer from stream. */
    PyRep* LoadIntegerLong() { return new PyInt( Read<int32>() ); }
    /** Loads signed short from stream. */
    PyRep* LoadIntegerSignedShort() { return new PyInt( Read<int16>() ); }
    /** Loads byte integer from stream. */
    PyRep* LoadIntegerByte() { return new PyInt( Read<int8>() ); }
    /** Loads variable length integer from stream. */
    PyRep* LoadIntegerVar();
    /** Loads minus one integer from stream. */
    PyRep* LoadIntegerMinusOne() { return new PyInt( -1 ); }
    /** Loads zero integer from stream. */
    PyRep* LoadIntegerZero() { return new PyInt( 0 ); }
    /** Loads one integer from stream. */
    PyRep* LoadIntegerOne() { return new PyInt( 1 ); }

    /** Loads real from stream. */
    PyRep* LoadReal() { return new PyFloat( Read<double>() ); }
    /** Loads zero real from stream. */
    PyRep* LoadRealZero() { return new PyFloat( 0.0 ); }

    /** Loads empty string from stream. */
    PyRep* LoadStringEmpty() { return new PyString( "" ); }
    /** Loads single character string from stream. */
    PyRep* LoadStringChar() { return new PyString( Read<char>( 1 ), (size_t)1 ); }
    /** Loads short (up to 255 chars) string from stream. */
    PyRep* LoadStringShort();
    /** Loads long (no limit) string from stream. */
    PyRep* LoadStringLong();
    /** Loads table string from stream. */
    PyRep* LoadStringTable();

    /** Loads empty wide string from stream. */
    PyRep* LoadWStringEmpty() { return new PyString( "" ); }
    /** Loads single UCS-2 character string from stream. */
    PyRep* LoadWStringUCS2Char() { return new PyString( (const char*)Read<uint16>( 1 ), sizeof( uint16 ) ); }
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
    /** Loads sub struct from stream. */
    PyRep* LoadSubStruct();
    /** Loads sub stream from stream. */
    PyRep* LoadSubStream();
    /** Loads checksumed stream from stream. */
    PyRep* LoadChecksumedStream();

    /** Loads extended object of type 1 from stream. */
    PyRep* LoadObjectEx1();
    /** Loads extended object of type 2 from stream. */
    PyRep* LoadObjectEx2();
    /** Loads packed row from stream. */
    PyRep* LoadPackedRow();

    /** Prints error and returns NULL. */
    PyRep* LoadError();
    /** Loads saved stream element from stream. */
    PyRep* LoadSavedStreamElement();

    /** Helper; loads extended object from stream. */
    PyObjectEx* LoadObjectEx( bool is_type_2 );
    /** Helper; loads zero-compressed buffer from stream. */
    bool LoadZeroCompressed( Buffer& into );

    /** Buffer which we are processing. */
    const Buffer* mBuffer;
    /** Next store index for referencing in the buffer. */
    size_t mStoreIndex;
    /** Referenced objects within the buffer. */
    PyRep** mStoredObjects;

    /** Load function map. */
    static PyRep* ( UnmarshalStream::* const s_mLoadMap[] )();
};

#endif

