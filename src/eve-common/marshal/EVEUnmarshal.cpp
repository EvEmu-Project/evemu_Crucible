﻿/*
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
    Author:     Bloody.Rabbit, Captnoord, Zhur
*/

#include "eve-common.h"

#include "python/classes/PyDatabase.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"

#include "marshal/EVEUnmarshal.h"
#include "marshal/EVEMarshalOpcodes.h"
#include "marshal/EVEMarshalStringTable.h"

#include "utils/EVEUtils.h"

PyRep* Unmarshal( const Buffer& data )
{
    UnmarshalStream* pUMS = new UnmarshalStream();
    PyRep* res = pUMS->Load( data );
    SafeDelete(pUMS);
    return res;
}

PyRep* InflateUnmarshal( const Buffer& data )
{
    if (IsDeflated(data)) {
        Buffer inflatedData;
        if (!InflateData(data, inflatedData))
            return nullptr;
        return Unmarshal(inflatedData);
    }

    return Unmarshal(data);
}

UnmarshalStream::~UnmarshalStream()
{
    //PySafeDecRef(mStoredObjects);
}

/************************************************************************/
/* UnmarshalStream                                                      */
/************************************************************************/
PyRep* ( UnmarshalStream::* const UnmarshalStream::s_mLoadMap[ PyRepOpcodeMask + 1 ] )() =
{
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadNone,                 //Op_PyNone
    &UnmarshalStream::LoadToken,                //Op_PyToken
    &UnmarshalStream::LoadIntegerLongLong,      //Op_PyLongLong
    &UnmarshalStream::LoadIntegerLong,          //Op_PyLong
    &UnmarshalStream::LoadIntegerSignedShort,   //Op_PySignedShort
    &UnmarshalStream::LoadIntegerByte,          //Op_PyByte
    &UnmarshalStream::LoadIntegerMinusOne,      //Op_PyMinusOne
    &UnmarshalStream::LoadIntegerZero,          //Op_PyZeroInteger
    &UnmarshalStream::LoadIntegerOne,           //Op_PyOneInteger
    &UnmarshalStream::LoadReal,                 //Op_PyReal
    &UnmarshalStream::LoadRealZero,             //Op_PyZeroReal
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadBuffer,               //Op_PyBuffer
    &UnmarshalStream::LoadStringEmpty,          //Op_PyEmptyString
    &UnmarshalStream::LoadStringChar,           //Op_PyCharString
    &UnmarshalStream::LoadStringShort,          //Op_PyShortString
    &UnmarshalStream::LoadStringTable,          //Op_PyStringTableItem
    &UnmarshalStream::LoadWStringUCS2,          //Op_PyWStringUCS2
    &UnmarshalStream::LoadStringLong,           //Op_PyLongString
    &UnmarshalStream::LoadTuple,                //Op_PyTuple
    &UnmarshalStream::LoadList,                 //Op_PyList
    &UnmarshalStream::LoadDict,                 //Op_PyDict
    &UnmarshalStream::LoadObject,               //Op_PyObject
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadSubStruct,            //Op_PySubStruct
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadSavedStreamElement,   //Op_PySavedStreamElement
    &UnmarshalStream::LoadChecksumedStream,     //Op_PyChecksumedStream
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadBoolTrue,             //Op_PyTrue
    &UnmarshalStream::LoadBoolFalse,            //Op_PyFalse
    &UnmarshalStream::LoadError,                //Op_cPicked
    &UnmarshalStream::LoadObjectEx1,            //Op_PyObjectEx1
    &UnmarshalStream::LoadObjectEx2,            //Op_PyObjectEx2
    &UnmarshalStream::LoadTupleEmpty,           //Op_PyEmptyTuple
    &UnmarshalStream::LoadTupleOne,             //Op_PyOneTuple
    &UnmarshalStream::LoadListEmpty,            //Op_PyEmptyList
    &UnmarshalStream::LoadListOne,              //Op_PyOneList
    &UnmarshalStream::LoadWStringEmpty,         //Op_PyEmptyWString
    &UnmarshalStream::LoadWStringUCS2Char,      //Op_PyWStringUCS2Char
    &UnmarshalStream::LoadPackedRow,            //Op_PyPackedRow
    &UnmarshalStream::LoadSubStream,            //Op_PySubStream
    &UnmarshalStream::LoadTupleTwo,             //Op_PyTwoTuple
    &UnmarshalStream::LoadError,                //Op_PackedTerminator
    &UnmarshalStream::LoadWStringUTF8,          //Op_PyWStringUTF8
    &UnmarshalStream::LoadIntegerVar,           //Op_PyVarInteger
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError
};

PyRep* UnmarshalStream::Load( const Buffer& data )
{
    mInItr = data.begin<uint8>();
    PyRep* res = LoadStream( data.size() );
    mInItr = Buffer::const_iterator<uint8>();

    return res;
}

PyRep* UnmarshalStream::LoadStream( size_t streamLength )
{
    const uint8 header = Read<uint8>();
    if (MarshalHeaderByte != header) {
        sLog.Error( "Unmarshal", "Invalid stream received (header byte 0x%X).", header );
        return nullptr;
    }

    const uint32 saveCount = Read<uint32>();
    CreateObjectStore( streamLength - sizeof( uint8 ) - sizeof( uint32 ), saveCount );

    PyRep* rep = LoadRep();

    DestroyObjectStore();
    return rep;
}

PyRep* UnmarshalStream::LoadRep()
{
    const uint8 header = Read<uint8>();

    const bool flagUnknown = ( header & PyRepUnknownMask ) != 0;
    const bool flagSave = ( header & PyRepSaveMask ) != 0;
    const uint8 opcode = ( header & PyRepOpcodeMask );

    if( flagUnknown )
        sLog.Warning( "Unmarshal", "Encountered flagUnknown in header 0x%X.", header );

    const uint32 storageIndex = ( flagSave ? GetStorageIndex() : 0 );

    PyRep* rep = ( this->*s_mLoadMap[ opcode ] )();

    if( 0 != storageIndex )
        StoreObject( storageIndex, rep );

    return rep;
}

void UnmarshalStream::CreateObjectStore( size_t streamLength, uint32 saveCount )
{
    DestroyObjectStore();

    if( 0 < saveCount )
    {
        mStoreIndexItr = ( ( mInItr + streamLength ).As<uint32>() - saveCount );
        mStoredObjects = new PyList( saveCount );
    }
}

void UnmarshalStream::DestroyObjectStore()
{
    mStoreIndexItr = Buffer::const_iterator<uint32>();
    PySafeDecRef( mStoredObjects );
}

PyRep* UnmarshalStream::GetStoredObject( uint32 index )
{
    if( 0 < index )
        return mStoredObjects->GetItem( --index );
    return nullptr;
}

void UnmarshalStream::StoreObject( uint32 index, PyRep* object )
{
    if( 0 < index )
    {
        PyIncRef( object );
        mStoredObjects->SetItem( --index, object );
    }
}

PyRep* UnmarshalStream::LoadIntegerVar()
{
    /* this is one of the stranger fields I have found, it seems to be a variable
     * length integer field (somewhat of a 'bigint' style data type), but it gets
     * used at times for integers which would fit into the other primitive data
     * types.... I would really like to see the criteria they use to determine
     * what gets marshaled as what...
     */

    const uint32 len = ReadSizeEx();
    const Buffer::const_iterator<uint8> data = Read<uint8>( len );

    if( sizeof( int32 ) >= len )
    {
        int32 intval(0);
        memcpy( &intval, &*data, len );

        return new PyInt( intval );
    }
    else if( sizeof( int64 ) >= len )
    {
        int64 intval(0);
        memcpy( &intval, &*data, len );

        return new PyLong( intval );
    }
    else
    {
        //int64 is not big enough
        //just pass it up to the application layer as a buffer...
        return new PyBuffer( data, data + len );
    }
}

PyRep* UnmarshalStream::LoadStringChar()
{
    const Buffer::const_iterator<char> str = Read<char>( 1 );

    return new PyString( str, str + 1 );
}

PyRep* UnmarshalStream::LoadStringShort()
{
    const uint8 len = Read<uint8>();
    const Buffer::const_iterator<char> str = Read<char>( len );

    return new PyString( str, str + len );
}

PyRep* UnmarshalStream::LoadStringLong()
{
    const uint32 len = ReadSizeEx();
    const Buffer::const_iterator<char> str = Read<char>( len );

    return new PyString( str, str + len );
}

PyRep* UnmarshalStream::LoadStringTable()
{
    const uint8 index = Read<uint8>();

    const char* str = sMarshalStringTable.LookupString( index );
    if( NULL == str )
    {
        assert( false );
        sLog.Error( "Unmarshal", "String Table Item %u is out of range!", index );

        char ebuf[64];
        snprintf( ebuf, 64, "Invalid String Table Item %u", index );
        return new PyString( ebuf );
    }
    else
        return new PyString( str );
}

PyRep* UnmarshalStream::LoadWStringUCS2Char()
{
    const Buffer::const_iterator<uint16> wstr = Read<uint16>( 1 );

    // convert to UTF-8
    std::string str;
    utf8::utf16to8( wstr, wstr + 1, std::back_inserter( str ) );

    return new PyWString( str );
}

PyRep* UnmarshalStream::LoadWStringUCS2()
{
    const uint32 len = ReadSizeEx();
    const Buffer::const_iterator<uint16> wstr = Read<uint16>( len );

    // convert to UTF-8
    std::string str;
    utf8::utf16to8( wstr, wstr + len, std::back_inserter( str ) );

    return new PyWString( str );
}

PyRep* UnmarshalStream::LoadWStringUTF8()
{
    const uint32 len = ReadSizeEx();
    const Buffer::const_iterator<char> wstr = Read<char>( len );

    return new PyWString( wstr, wstr + len );
}

PyRep* UnmarshalStream::LoadToken()
{
    const uint8 len = Read<uint8>();
    const Buffer::const_iterator<char> str = Read<char>( len );

    return new PyToken( str, str + len );
}

PyRep* UnmarshalStream::LoadBuffer()
{
    const uint32 len = ReadSizeEx();
    const Buffer::const_iterator<uint8> data = Read<uint8>( len );

    return new PyBuffer( data, data + len );
}

PyRep* UnmarshalStream::LoadTuple()
{
    const uint32 count = ReadSizeEx();
    PyTuple* tuple = new PyTuple( count );

    for ( uint32 i(0); i < count; ++i ) {
        PyRep* rep = LoadRep();
        if (rep == nullptr) {
            PyDecRef( tuple );
            return nullptr;
        }

        tuple->SetItem( i, rep );
    }

    return tuple;
}

PyRep* UnmarshalStream::LoadTupleOne()
{
    PyRep* i = LoadRep();
    if( NULL == i )
        return nullptr;

    PyTuple* tuple = new PyTuple( 1 );
    tuple->SetItem( 0, i );

    return tuple;
}

PyRep* UnmarshalStream::LoadTupleTwo()
{
    PyRep* i = LoadRep();
    if( NULL == i )
        return nullptr;

    PyRep* j = LoadRep();
    if( NULL == j )
    {
        PyDecRef( i );
        return nullptr;
    }

    PyTuple *tuple = new PyTuple( 2 );
    tuple->SetItem( 0, i );
    tuple->SetItem( 1, j );

    return tuple;
}

PyRep* UnmarshalStream::LoadList()
{
    const uint32 count = ReadSizeEx();
    PyList* list = new PyList( count );

    for ( uint32 i(0); i < count; i++ )
    {
        PyRep* rep = LoadRep();
        if (rep == nullptr)
        {
            PyDecRef( list );
            return nullptr;
        }

        list->SetItem( i, rep );
    }

    return list;
}

PyRep* UnmarshalStream::LoadListOne()
{
    PyRep* i = LoadRep();
    if( NULL == i )
        return nullptr;

    PyList* list = new PyList();
    list->AddItem( i );

    return list;
}

PyRep* UnmarshalStream::LoadDict()
{
    const uint32 count = ReadSizeEx();
    PyDict* dict = new PyDict;

    for ( uint32 i(0); i < count; i++ )
    {
        PyRep* value = LoadRep();
        if( NULL == value )
            return nullptr;

        PyRep* key = LoadRep();
        if( NULL == key )
        {
            PyDecRef( value );
            return nullptr;
        }

        dict->SetItem( key, value );
    }

    return dict;
}

PyRep* UnmarshalStream::LoadObject()
{
    PyRep* type = LoadRep();
    if( NULL == type )
        return nullptr;

    if( !type->IsString() )
    {
        sLog.Error( "Unmarshal", "Object: Expected 'String' as type, got '%s'.", type->TypeString() );

        PyDecRef( type );
        return nullptr;
    }

    PyRep* arguments = LoadRep();
    if( NULL == arguments )
    {
        PyDecRef( type );
        return nullptr;
    }

    return new PyObject( type->AsString(), arguments );
}

PyRep* UnmarshalStream::LoadObjectEx1()
{
    return LoadObjectEx( false );
}

PyRep* UnmarshalStream::LoadObjectEx2()
{
    return LoadObjectEx( true );
}

PyRep* UnmarshalStream::LoadSubStream()
{
    const uint32 len = ReadSizeEx();
    const Buffer::const_iterator<uint8> data = Read<uint8>( len );

    return new PySubStream( new PyBuffer( data, data + len ) );
}

PyRep* UnmarshalStream::LoadSubStruct()
{
    // This is actually a remote object specification

    PyRep* ss = LoadRep();
    if( NULL == ss )
        return nullptr;

    return new PySubStruct( ss );
}

PyRep* UnmarshalStream::LoadChecksumedStream()
{
    const uint32 sum = Read<uint32>();

    PyRep* ss = LoadRep();
    if( NULL == ss )
        return nullptr;

    return new PyChecksumedStream( ss, sum );
}

PyRep* UnmarshalStream::LoadPackedRow()
{
    // PyPackedRows are just a packed form of blue.DBRow
    // these take a DBRowDescriptor and the column data in different formats
    PyRep* header_element = LoadRep();
    if (NULL == header_element)
        return nullptr;

    // create the base packed row to be filled with data
    PyPackedRow* row = new PyPackedRow((DBRowDescriptor*)header_element);

    // create the sizemap and sort it by bitsize, the value of the map indicates the index of the column
    // this can be used to identify things easily
    std::multimap<uint8, uint32, std::greater<uint8>> sizeMap;
    std::map<uint8, uint8> booleanColumns;

    uint32 columnCount = row->header()->ColumnCount();
    uint8 booleansBitLength = 0;  // 使用 uint8 类型
    size_t byteDataBitLength = 0;
    size_t nullsBitLength = 0;

    // go through all the columns to gather the required information
    for (uint32 i = 0; i < columnCount; i++)
    {
        DBTYPE columnType = row->header()->GetColumnType(i);
        uint8 size = DBTYPE_GetSizeBits(columnType);

        // count booleans
        if (columnType == DBTYPE_BOOL)
        {
            // 检查是否超出 uint8 范围
            if (booleansBitLength == UINT8_MAX) {
                sLog.Error("UnmarshalStream", "Boolean columns count exceeds maximum allowed value");
                PyDecRef(row);
                return nullptr;
            }

            // register the boolean in the list and increase the length
            booleanColumns.insert(std::make_pair(static_cast<uint8>(i), booleansBitLength));
            booleansBitLength++;
        }

        // also count all columns as possible nulls
        nullsBitLength++;

        // increase the bytedata length only if a column is longer than 7 bits
         // this is used as an indicator of what is written in the first, second or third part
        if (size >= 8)
            byteDataBitLength += size;

        // add the column to the list
        sizeMap.insert(std::make_pair(size, i));
    }

    size_t expectedByteSize = (byteDataBitLength >> 3) + ((booleansBitLength + nullsBitLength) >> 3) + 1;

    // 检查大小是否超出范围
    if (expectedByteSize > static_cast<size_t>(ULONG_MAX)) {
        sLog.Error("UnmarshalStream", "Expected byte size %zu exceeds maximum allowed value", expectedByteSize);
        PyDecRef(header_element);
        return nullptr;
    }

    // reserve enough space for the buffer
    Buffer unpacked(static_cast<unsigned long>(expectedByteSize), 0);

    if (!LoadRLE(unpacked)) {
        PyDecRef(header_element);
        return nullptr;
    }

    Buffer::const_iterator<uint8> unpackedItr = unpacked.begin<uint8>();
    Buffer::const_iterator<uint8> bitIterator = unpacked.begin<uint8>();

    std::multimap< uint8, uint32, std::greater< uint8 > >::iterator cur, end;
    cur = sizeMap.begin();
    end = sizeMap.end();
    for (; cur != end; ++cur)
    {
        const uint32 index = cur->second;
        const DBTYPE columnType = row->header ()->GetColumnType (index);

        size_t nullBitOffset = byteDataBitLength + booleansBitLength + cur->second;
        if (nullBitOffset > static_cast<size_t>(ULONG_MAX)) {
            sLog.Error("UnmarshalStream", "Bit offset %zu exceeds maximum allowed value", nullBitOffset);
            PyDecRef(row);
            return nullptr;
        }

        size_t maxByteOffset = nullBitOffset >> 3;
        if (maxByteOffset > static_cast<size_t>(ULONG_MAX)) {
            sLog.Error("UnmarshalStream", "Byte offset %zu exceeds maximum allowed value", maxByteOffset);
            PyDecRef(row);
            return nullptr;
        }

        unsigned long nullBit = static_cast<unsigned long>(nullBitOffset + cur->second);
        unsigned long nullByte = nullBit >> 3;

        unsigned long boolBit = static_cast<unsigned long>(byteDataBitLength + booleanColumns.find(cur->second)->second);
        unsigned long boolByte = boolBit >> 3;

        // setup the iterator to the proper byte
        // first check for nulls
        bitIterator = unpacked.begin<uint8>() + nullByte;

        if ((*bitIterator & (1 << (nullBit & 0x7))) == (1 << (nullBit & 0x7)))
        {
            // PyNone value found! override it and increase the original iterator the required steps
            unpackedItr += DBTYPE_GetSizeBits (columnType) >> 3;
            row->SetField (index, new PyNone ());

            // continue should only be performed if the columns are not normal marshal objects
            if (columnType != DBTYPE_BYTES && columnType != DBTYPE_STR && columnType != DBTYPE_WSTR)
                continue;
        }

        switch (columnType)
        {
            case DBTYPE_I8:
            case DBTYPE_CY:
            case DBTYPE_UI8:
            case DBTYPE_FILETIME:
            {
                Buffer::const_iterator<int64> v = unpackedItr.As<int64>();
                row->SetField( index, new PyLong( *v++ ) );
                unpackedItr = v.As<uint8>();
            } break;

            case DBTYPE_I4:
            {
                Buffer::const_iterator<int32> v = unpackedItr.As<int32>();
                row->SetField( index, new PyInt( *v++ ) );
                unpackedItr = v.As<uint8>();
            } break;
            case DBTYPE_UI4:
            {
                Buffer::const_iterator<uint32> v = unpackedItr.As<uint32>();
                row->SetField( index, new PyInt( *v++ ) );
                unpackedItr = v.As<uint8>();
            } break;

            case DBTYPE_I2:
            {
                Buffer::const_iterator<int16> v = unpackedItr.As<int16>();
                row->SetField( index, new PyInt( *v++ ) );
                unpackedItr = v.As<uint8>();
            } break;
            case DBTYPE_UI2:
            {
                Buffer::const_iterator<uint16> v = unpackedItr.As<uint16>();
                row->SetField( index, new PyInt( *v++ ) );
                unpackedItr = v.As<uint8>();
            } break;

            case DBTYPE_I1:
            {
                Buffer::const_iterator<int8> v = unpackedItr.As<int8>();
                row->SetField( index, new PyInt( *v++ ) );
                unpackedItr = v.As<uint8>();
            } break;

            case DBTYPE_UI1:
            {
                Buffer::const_iterator<uint8> v = unpackedItr.As<uint8>();
                row->SetField( index, new PyInt( *v++ ) );
                unpackedItr = v.As<uint8>();
            } break;

            case DBTYPE_R8:
            {
                Buffer::const_iterator<double> v = unpackedItr.As<double>();
                row->SetField( index, new PyFloat( *v++ ) );
                unpackedItr = v.As<uint8>();
            } break;

            case DBTYPE_R4:
            {
                Buffer::const_iterator<float> v = unpackedItr.As<float>();
                row->SetField( index, new PyFloat( *v++ ) );
                unpackedItr = v.As<uint8>();
            } break;

            case DBTYPE_BOOL:
            {
                // get the bit this boolean should be read from
                size_t boolBitOffset = byteDataBitLength + booleanColumns.find(cur->second)->second;
                if (boolBitOffset > static_cast<size_t>(ULONG_MAX)) {
                    sLog.Error("UnmarshalStream", "Boolean bit offset %zu exceeds maximum allowed value", boolBitOffset);
                    PyDecRef(row);
                    return nullptr;
                }

                size_t boolByteOffset = boolBitOffset >> 3;
                if (boolByteOffset > static_cast<size_t>(ULONG_MAX)) {
                    sLog.Error("UnmarshalStream", "Boolean byte offset %zu exceeds maximum allowed value", boolByteOffset);
                    PyDecRef(row);
                    return nullptr;
                }

                unsigned long boolBit = static_cast<unsigned long>(boolBitOffset);
                unsigned long boolByte = static_cast<unsigned long>(boolByteOffset);

                // setup the iterator to the proper byte
                bitIterator = unpacked.begin<uint8>() + boolByte;

                row->SetField (index, new PyBool ((*bitIterator & (1 << (boolBit & 0x7))) == (1 << (boolBit & 0x7))));
            } break;

            // these objects are read directly from the end of the PyPackedRow
            // so they can be kept
            case DBTYPE_BYTES:
            case DBTYPE_STR:
            case DBTYPE_WSTR:
            {
                PyRep* el = LoadRep();
                if( NULL == el )
                {
                    PyDecRef( row );
                    return nullptr;
                }

                row->SetField( index, el );
            } break;

            case DBTYPE_EMPTY:
            case DBTYPE_ERROR:
                return nullptr;
        }
    }

    return row;
}

PyRep* UnmarshalStream::LoadError()
{
    sLog.Error( "Unmarshal", "Invalid opcode encountered." );

    return nullptr;
}

PyRep* UnmarshalStream::LoadSavedStreamElement()
{
    const uint32 index = ReadSizeEx();

    PyRep* obj = GetStoredObject( index );
    if( NULL == obj )
    {
        sLog.Error( "Unmarshal", "SavedStreamElement: Got invalid stored object." );
        return nullptr;
    }

    return obj->Clone();
}

PyObjectEx* UnmarshalStream::LoadObjectEx( bool is_type_2 )
{
    PyRep* header = LoadRep();
    if( NULL == header )
        return nullptr;

    PyObjectEx* obj = new PyObjectEx( is_type_2, header );

    while( Op_PackedTerminator != Peek<uint8>() )
    {
        PyRep* el = LoadRep();
        if( NULL == el )
        {
            PyDecRef( obj );
            return nullptr;
        }

        obj->list().AddItem( el );
    }
    //skip Op_PackedTerminator
    Read<uint8>();

    while( Op_PackedTerminator != Peek<uint8>() )
    {
        PyRep* key = LoadRep();
        if( NULL == key )
        {
            PyDecRef( obj );
            return nullptr;
        }

        PyRep* value = LoadRep();
        if( NULL == value )
        {
            PyDecRef( key );
            PyDecRef( obj );
            return nullptr;
        }

        obj->dict().SetItem( key, value );
    }
    //skip Op_PackedTerminator
    Read<uint8>();

    return obj;
}

bool UnmarshalStream::LoadRLE(Buffer& out)
{
    // 获取输入大小并检查范围
    size_t in_size = ReadSizeEx();
    if (in_size > static_cast<size_t>(ULONG_MAX)) {
        sLog.Error("UnmarshalStream", "Input size %zu exceeds maximum allowed value", in_size);
        return false;
    }

    Buffer::const_iterator<uint8> cur, end;
    cur = Read<uint8>(static_cast<unsigned long>(in_size));
    end = cur + in_size;
    Buffer::const_iterator<uint8> in_ix = cur;
    int out_ix = 0;
    int count;
    int run = 0;
    int nibble = 0;

    while(in_ix < end)
    {
        nibble = !nibble;
        if(nibble)
        {
            run = (unsigned char)*in_ix++;
            count = (run & 0x0f) - 8;
        }
        else
            count = (run >> 4) - 8;

        if(count >= 0)
        {
            if (out_ix + count + 1 > static_cast<int>(out.size()))
                return false;

            while(count-- >= 0)
                out[out_ix++] = 0;
        }
        else
        {
            // 检查输出缓冲区大小
            if (out_ix - count > static_cast<int>(out.size()))
                return false;

            while(count++ && in_ix < end)
                out[out_ix++] = *in_ix++;
        }
    }

    // no need to set the rest of the buffer to zero as the output is already
    // set to 0
    // while(out_ix < out.size())
    //    out[out_ix++] = 0;
    return true;
}
