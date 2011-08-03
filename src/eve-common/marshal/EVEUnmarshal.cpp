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
    Author:     Bloody.Rabbit, Captnoord, Zhur
*/

#include "EVECommonPCH.h"

#include "python/classes/PyDatabase.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"

#include "marshal/EVEUnmarshal.h"
#include "marshal/EVEMarshalOpcodes.h"
#include "marshal/EVEMarshalStringTable.h"

#include "utils/EVEUtils.h"

PyRep* Unmarshal( const Buffer& data )
{
    UnmarshalStream v;
    return v.Load( data );
}

PyRep* InflateUnmarshal( const Buffer& data )
{
    if( IsDeflated( data ) )
    {
        Buffer inflatedData;
        if( !InflateData( data, inflatedData ) )
            return NULL;

        return Unmarshal( inflatedData );
    }
    else
        return Unmarshal( data );
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
    if( MarshalHeaderByte != header )
    {
        sLog.Error( "Unmarshal", "Invalid stream received (header byte 0x%X).", header );
        return NULL;
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
    return NULL;
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
     * what gets marshales as what...
     */

    const uint32 len = ReadSizeEx();
    const Buffer::const_iterator<uint8> data = Read<uint8>( len );

	if( sizeof( int32 ) >= len )
	{
        int32 intval = 0;
        memcpy( &intval, &*data, len );

        return new PyInt( intval );
	}
	else if( sizeof( int64 ) >= len )
	{
        int64 intval = 0;
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

    for( uint32 i = 0; i < count; i++ )
    {
        PyRep* rep = LoadRep();
        if( NULL == rep )
        {
            PyDecRef( tuple );
            return NULL;
        }

        tuple->SetItem( i, rep );
    }

    return tuple;
}

PyRep* UnmarshalStream::LoadTupleOne()
{
    PyRep* i = LoadRep();
    if( NULL == i )
        return NULL;

    PyTuple* tuple = new PyTuple( 1 );
    tuple->SetItem( 0, i );

    return tuple;
}

PyRep* UnmarshalStream::LoadTupleTwo()
{
    PyRep* i = LoadRep();
    if( NULL == i )
        return NULL;

    PyRep* j = LoadRep();
    if( NULL == j )
    {
        PyDecRef( i );
        return NULL;
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

    for( uint32 i = 0; i < count; i++ )
    {
        PyRep* rep = LoadRep();
        if( NULL == rep )
        {
            PyDecRef( list );
            return NULL;
        }

        list->SetItem( i, rep );
    }

    return list;
}

PyRep* UnmarshalStream::LoadListOne()
{
    PyRep* i = LoadRep();
    if( NULL == i )
        return NULL;

    PyList* list = new PyList();
    list->AddItem( i );

    return list;
}

PyRep* UnmarshalStream::LoadDict()
{
    const uint32 count = ReadSizeEx();
    PyDict* dict = new PyDict;

    for( uint32 i = 0; i < count; i++ )
    {
        PyRep* value = LoadRep();
        if( NULL == value )
            return NULL;

        PyRep* key = LoadRep();
        if( NULL == key )
        {
            PyDecRef( value );
            return NULL;
        }

        dict->SetItem( key, value );
    }

    return dict;
}

PyRep* UnmarshalStream::LoadObject()
{
    PyRep* type = LoadRep();
    if( NULL == type )
        return NULL;

    if( !type->IsString() )
    {
        sLog.Error( "Unmarshal", "Object: Expected 'String' as type, got '%s'.", type->TypeString() );

        PyDecRef( type );
        return NULL;
    }

    PyRep* arguments = LoadRep();
    if( NULL == arguments )
    {
        PyDecRef( type );
        return NULL;
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
        return NULL;

    return new PySubStruct( ss );
}

PyRep* UnmarshalStream::LoadChecksumedStream()
{
	const uint32 sum = Read<uint32>();

    PyRep* ss = LoadRep();
    if( NULL == ss )
        return NULL;

    return new PyChecksumedStream( ss, sum );
}

PyRep* UnmarshalStream::LoadPackedRow()
{
    /*
     * As far as I can tell, this opcode is really a packed
     * form of blue.DBRow, which takes a row descriptor and a
     * data stream (the work_buffer here)
     *
     */
    PyRep* header_element = LoadRep();
    if( NULL == header_element )
        return NULL;

    Buffer unpacked;
    if( !LoadZeroCompressed( unpacked ) )
    {
        PyDecRef( header_element );
        return NULL;
    }

	// This is only an assumption, though PyPackedRow does not
	// support anything else ....
    PyPackedRow* row = new PyPackedRow( (DBRowDescriptor*)header_element );

    // Create size map, sorted from the greatest to the smallest value
    std::multimap< uint8, uint32, std::greater< uint8 > > sizeMap;
    uint32 cc = row->header()->ColumnCount();
	size_t sum = 0;

    for( uint32 i = 0; i < cc; i++ )
	{
		uint8 size = DBTYPE_GetSizeBits( row->header()->GetColumnType( i ) );

        sizeMap.insert( std::make_pair( size, i ) );
		sum += size;
	}

	// make sure there is enough data in buffer
	sum = ( ( sum + 7 ) >> 3 );
	unpacked.Resize<uint8>( sum );

    Buffer::const_iterator<uint8> unpackedItr = unpacked.begin<uint8>();
    uint8 bitOffset = 0;

    std::multimap< uint8, uint32, std::greater< uint8 > >::iterator cur, end;
    cur = sizeMap.begin();
    end = sizeMap.end();
    for(; cur != end; ++cur)
    {
        const uint32 index = cur->second;

        switch( row->header()->GetColumnType( index ) )
        {
            case DBTYPE_I8:
            case DBTYPE_UI8:
            case DBTYPE_CY:
            case DBTYPE_FILETIME:
            {
                Buffer::const_iterator<int64> v = unpackedItr.As<int64>();
			    row->SetField( index, new PyLong( *v++ ) );
                unpackedItr = v.As<uint8>();
            } break;

            case DBTYPE_I4:
            case DBTYPE_UI4:
            {
                Buffer::const_iterator<int32> v = unpackedItr.As<int32>();
                row->SetField( index, new PyInt( *v++ ) );
                unpackedItr = v.As<uint8>();
            } break;

            case DBTYPE_I2:
            case DBTYPE_UI2:
            {
                Buffer::const_iterator<int16> v = unpackedItr.As<int16>();
                row->SetField( index, new PyInt( *v++ ) );
                unpackedItr = v.As<uint8>();
            } break;

            case DBTYPE_I1:
            case DBTYPE_UI1:
            {
                Buffer::const_iterator<int8> v = unpackedItr.As<int8>();
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
                if( 7 < bitOffset )
                {
                    bitOffset = 0;
                    ++unpackedItr;
                }

                row->SetField( index, new PyBool( ( *unpackedItr >> bitOffset++ ) & 0x01 ) );
            } break;

            case DBTYPE_BYTES:
            case DBTYPE_STR:
            case DBTYPE_WSTR:
            {
                PyRep* el = LoadRep();
                if( NULL == el )
                {
                    PyDecRef( row );
                    return NULL;
                }

                row->SetField( index, el );
            } break;
        }
    }

    return row;
}

PyRep* UnmarshalStream::LoadError()
{
    sLog.Error( "Unmarshal", "Invalid opcode encountered." );

    return NULL;
}

PyRep* UnmarshalStream::LoadSavedStreamElement()
{
    const uint32 index = ReadSizeEx();

	PyRep* obj = GetStoredObject( index );
	if( NULL == obj )
	{
        sLog.Error( "Unmarshal", "SavedStreamElement: Got invalid stored object." );
        return NULL;
	}

    return obj->Clone();
}

PyObjectEx* UnmarshalStream::LoadObjectEx( bool is_type_2 )
{
    PyRep* header = LoadRep();
    if( NULL == header )
        return NULL;

    PyObjectEx* obj = new PyObjectEx( is_type_2, header );

    while( Op_PackedTerminator != Peek<uint8>() )
	{
        PyRep* el = LoadRep();
        if( NULL == el )
        {
            PyDecRef( obj );
            return NULL;
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
            return NULL;
        }

        PyRep* value = LoadRep();
        if( NULL == value )
        {
            PyDecRef( key );
            PyDecRef( obj );
            return NULL;
        }

		obj->dict().SetItem( key, value );
    }
    //skip Op_PackedTerminator
    Read<uint8>();

    return obj;
}

bool UnmarshalStream::LoadZeroCompressed( Buffer& into )
{
    const uint32 packedLen = ReadSizeEx();

    Buffer::const_iterator<uint8> cur, end;
    cur = Read<uint8>( packedLen );
    end = cur + packedLen;
    while( cur < end )
    {
        // Load opcode
        const Buffer::const_iterator<ZeroCompressOpcode> opcode = cur.As<ZeroCompressOpcode>();
        ++cur;

        // Decode first part
        if( opcode->firstIsZero )
        {
            uint8 len = ( opcode->firstLen + 1 );
            while( 0 < len-- )
                into.Append<uint8>( 0 );
        }
        else
        {
            const Buffer::const_iterator<uint8> dataEnd = std::min( end, cur + 8 - opcode->firstLen );

            into.AppendSeq( cur, dataEnd );
            cur = dataEnd;
        }

        // Decode second part
        if( opcode->secondIsZero )
        {
            uint8 len = ( opcode->secondLen + 1 );
            while( 0 < len-- )
                into.Append<uint8>( 0 );
        }
        else
        {
            const Buffer::const_iterator<uint8> dataEnd = std::min( end, cur + 8 - opcode->secondLen );

            into.AppendSeq( cur, dataEnd );
            cur = dataEnd;
        }
    }

    return true;
}


