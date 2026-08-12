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
#include "network/ProtocolLimits.h"

namespace {

template<typename T>
bool ReadPackedValue(
    Buffer::const_iterator<uint8>& cursor,
    const Buffer::const_iterator<uint8>& end,
    T& value )
{
    if( cursor > end || sizeof( T ) > static_cast<size_t>( end - cursor ) )
        return false;

    memcpy( &value, &*cursor, sizeof( T ) );
    cursor += sizeof( T );
    return true;
}

bool SkipPackedBytes(
    Buffer::const_iterator<uint8>& cursor,
    const Buffer::const_iterator<uint8>& end,
    size_t count )
{
    if( cursor > end || count > static_cast<size_t>( end - cursor ) )
        return false;

    cursor += count;
    return true;
}

bool IsSupportedPackedColumnType( DBTYPE type )
{
    switch( type )
    {
        case DBTYPE_I8:
        case DBTYPE_CY:
        case DBTYPE_UI8:
        case DBTYPE_FILETIME:
        case DBTYPE_I4:
        case DBTYPE_UI4:
        case DBTYPE_I2:
        case DBTYPE_UI2:
        case DBTYPE_I1:
        case DBTYPE_UI1:
        case DBTYPE_R8:
        case DBTYPE_R4:
        case DBTYPE_BOOL:
        case DBTYPE_BYTES:
        case DBTYPE_STR:
        case DBTYPE_WSTR:
            return true;
        default:
            return false;
    }
}

bool IsHashableKey( PyRep* value )
{
    if( value == nullptr )
        return false;

    if( value->IsTuple() )
    {
        const PyTuple* tuple = value->AsTuple();
        for( PyTuple::const_iterator item = tuple->begin();
             item != tuple->end(); ++item )
        {
            if( !IsHashableKey( *item ) )
                return false;
        }
        return true;
    }

    return value->IsInt() || value->IsLong() || value->IsFloat() ||
        value->IsBool() || value->IsNone() || value->IsBuffer() ||
        value->IsString() || value->IsWString();
}

DBRowDescriptor* DecodePackedRowHeader(
    PyRep* encoded, size_t maxColumns )
{
    PyRep* descriptor = encoded;
    if( encoded->IsTuple() )
    {
        PyTuple* wrapper = encoded->AsTuple();
        if( wrapper->size() != 1 )
            return nullptr;
        descriptor = wrapper->GetItem( 0 );
    }

    if( descriptor == nullptr || !descriptor->IsObjectEx() )
        return nullptr;

    PyObjectEx* object = descriptor->AsObjectEx();
    if( object->isType2() || object->header() == nullptr ||
        !object->header()->IsTuple() )
        return nullptr;

    PyTuple* objectHeader = object->header()->AsTuple();
    if( objectHeader->size() < 2 )
        return nullptr;

    PyRep* type = objectHeader->GetItem( 0 );
    const std::string* typeName = nullptr;
    if( type->IsToken() )
        typeName = &type->AsToken()->content();
    else if( type->IsString() )
        typeName = &type->AsString()->content();

    if( typeName == nullptr || *typeName != "blue.DBRowDescriptor" )
        return nullptr;

    PyRep* argsRep = objectHeader->GetItem( 1 );
    if( !argsRep->IsTuple() )
        return nullptr;

    PyTuple* args = argsRep->AsTuple();
    if( args->size() != 1 || !args->GetItem( 0 )->IsTuple() )
        return nullptr;

    PyTuple* columns = args->GetItem( 0 )->AsTuple();
    if( columns->size() > maxColumns )
        return nullptr;

    DBRowDescriptor* result = new DBRowDescriptor();
    for( size_t index = 0; index < columns->size(); ++index )
    {
        PyRep* columnRep = columns->GetItem( index );
        if( !columnRep->IsTuple() )
        {
            PyDecRef( result );
            return nullptr;
        }

        PyTuple* column = columnRep->AsTuple();
        if( column->size() != 2 || !column->GetItem( 0 )->IsString() ||
            !column->GetItem( 1 )->IsInt() )
        {
            PyDecRef( result );
            return nullptr;
        }

        const DBTYPE typeValue = static_cast<DBTYPE>(
            column->GetItem( 1 )->AsInt()->value() );
        if( !IsSupportedPackedColumnType( typeValue ) )
        {
            PyDecRef( result );
            return nullptr;
        }

        result->AddColumn(
            column->GetItem( 0 )->AsString()->content().c_str(), typeValue );
    }

    return result;
}

}  // namespace

PyRep* Unmarshal( const Buffer& data )
{
    if (data.size() == 0 || data.size() > EveProtocol::MAX_PACKET_SIZE)
        return nullptr;

    UnmarshalStream stream;
    return stream.Load( data );
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
    DestroyObjectStore();
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

size_t UnmarshalStream::RemainingBytes() const
{
    if( mData == nullptr )
        return 0;

    return static_cast<size_t>( mPayloadEnd - mInItr );
}

bool UnmarshalStream::ReadBytes(
    size_t count, Buffer::const_iterator<uint8>& bytes )
{
    if( count > RemainingBytes() )
    {
        MarkFailed();
        return false;
    }

    bytes = mInItr;
    mInItr += count;
    return true;
}

bool UnmarshalStream::ReadSizeEx( uint32& size )
{
    const uint8 extendedSizeMarker = 0xFF;
    uint8 encodedSize = 0;
    if( !ReadValue( encodedSize ) )
        return false;

    if( encodedSize != extendedSizeMarker )
    {
        size = encodedSize;
        return true;
    }

    return ReadValue( size );
}

bool UnmarshalStream::BeginRep()
{
    if( mFailed || mDepth >= EveProtocol::MAX_MARSHAL_DEPTH ||
        mObjectCount >= EveProtocol::MAX_MARSHAL_OBJECT_COUNT )
    {
        MarkFailed();
        return false;
    }

    ++mDepth;
    ++mObjectCount;
    return true;
}

void UnmarshalStream::EndRep()
{
    if( mDepth == 0 )
    {
        MarkFailed();
        return;
    }

    --mDepth;
}

bool UnmarshalStream::ValidateContainerCount(
    uint32 count, size_t minimumBytesPerItem )
{
    if( count > EveProtocol::MAX_MARSHAL_CONTAINER_COUNT ||
        ( minimumBytesPerItem > 0 &&
          count > RemainingBytes() / minimumBytesPerItem ) )
    {
        MarkFailed();
        return false;
    }

    return true;
}

PyRep* UnmarshalStream::Load( const Buffer& data )
{
    DestroyObjectStore();
    mData = &data;
    mInItr = data.begin<uint8>();
    mEndItr = data.end<uint8>();
    mPayloadEnd = mEndItr;
    mFailed = false;
    mDepth = 0;
    mObjectCount = 0;

    PyRep* res = LoadStream( data.size() );
    mData = nullptr;
    mInItr = Buffer::const_iterator<uint8>();
    mEndItr = Buffer::const_iterator<uint8>();
    mPayloadEnd = Buffer::const_iterator<uint8>();

    return res;
}

PyRep* UnmarshalStream::LoadStream( size_t streamLength )
{
    uint8 header = 0;
    if( !ReadValue( header ) )
        return nullptr;

    if (MarshalHeaderByte != header) {
        sLog.Error( "Unmarshal", "Invalid stream received (header byte 0x%X).", header );
        MarkFailed();
        return nullptr;
    }

    uint32 saveCount = 0;
    if( !ReadValue( saveCount ) )
        return nullptr;

    const size_t headerLength = sizeof( uint8 ) + sizeof( uint32 );
    if( streamLength < headerLength ||
        !CreateObjectStore( streamLength - headerLength, saveCount ) )
        return nullptr;

    PyRep* rep = LoadRep();
    if( rep != nullptr && mInItr != mPayloadEnd )
    {
        PyDecRef( rep );
        rep = nullptr;
        MarkFailed();
    }

    DestroyObjectStore();
    return rep;
}

PyRep* UnmarshalStream::LoadRep()
{
    if( !BeginRep() )
        return nullptr;

    uint8 header = 0;
    if( !ReadValue( header ) )
    {
        EndRep();
        return nullptr;
    }

    const bool flagUnknown = ( header & PyRepUnknownMask ) != 0;
    const bool flagSave = ( header & PyRepSaveMask ) != 0;
    const uint8 opcode = ( header & PyRepOpcodeMask );

    if( flagUnknown )
        sLog.Warning( "Unmarshal", "Encountered flagUnknown in header 0x%X.", header );

    uint32 storageIndex = 0;
    if( flagSave && !GetStorageIndex( storageIndex ) )
    {
        EndRep();
        return nullptr;
    }

    PyRep* rep = ( this->*s_mLoadMap[ opcode ] )();
    EndRep();

    if( mFailed )
    {
        PySafeDecRef( rep );
        return nullptr;
    }

    if( rep == nullptr )
        return nullptr;

    if( 0 != storageIndex )
    {
        if( !StoreObject( storageIndex, rep ) )
        {
            PyDecRef( rep );
            return nullptr;
        }
    }

    return rep;
}

bool UnmarshalStream::CreateObjectStore( size_t streamLength, uint32 saveCount )
{
    DestroyObjectStore();

    if( streamLength > RemainingBytes() ||
        saveCount > EveProtocol::MAX_MARSHAL_SAVED_OBJECT_COUNT ||
        saveCount > streamLength / sizeof( uint32 ) )
    {
        MarkFailed();
        return false;
    }

    mEndItr = mInItr + streamLength;
    const size_t storeBytes = saveCount * sizeof( uint32 );
    mStoreIndexItr = mEndItr - storeBytes;
    mStoreIndexEnd = mEndItr;
    mPayloadEnd = mStoreIndexItr;

    if( 0 < saveCount )
        mStoredObjects = new PyList( saveCount );

    return true;
}

void UnmarshalStream::DestroyObjectStore()
{
    mStoreIndexItr = Buffer::const_iterator<uint8>();
    mStoreIndexEnd = Buffer::const_iterator<uint8>();

    if( mStoredObjects != nullptr )
    {
        for( PyList::iterator item = mStoredObjects->items.begin();
             item != mStoredObjects->items.end(); ++item )
        {
            PySafeDecRef( *item );
            *item = nullptr;
        }
        PyDecRef( mStoredObjects );
    }
    mStoredObjects = nullptr;
}

bool UnmarshalStream::GetStorageIndex( uint32& index )
{
    if( mStoredObjects == nullptr ||
        sizeof( uint32 ) > static_cast<size_t>(
            mStoreIndexEnd - mStoreIndexItr ) )
    {
        MarkFailed();
        return false;
    }

    memcpy( &index, &*mStoreIndexItr, sizeof( uint32 ) );
    mStoreIndexItr += sizeof( uint32 );
    return true;
}

PyRep* UnmarshalStream::GetStoredObject( uint32 index )
{
    if( mStoredObjects != nullptr && 0 < index &&
        index <= mStoredObjects->size() )
        return mStoredObjects->GetItem( index - 1 );

    return nullptr;
}

bool UnmarshalStream::StoreObject( uint32 index, PyRep* object )
{
    if( mStoredObjects == nullptr || index == 0 ||
        index > mStoredObjects->size() || object == nullptr )
    {
        MarkFailed();
        return false;
    }

    mStoredObjects->SetItem( index - 1, object );
    return true;
}

PyRep* UnmarshalStream::LoadIntegerVar()
{
    /* this is one of the stranger fields I have found, it seems to be a variable
     * length integer field (somewhat of a 'bigint' style data type), but it gets
     * used at times for integers which would fit into the other primitive data
     * types.... I would really like to see the criteria they use to determine
     * what gets marshaled as what...
     */

    uint32 len = 0;
    Buffer::const_iterator<uint8> data;
    if( !ReadSizeEx( len ) || len == 0 || !ReadBytes( len, data ) )
        return nullptr;

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
    Buffer::const_iterator<uint8> bytes;
    if( !ReadBytes( 1, bytes ) )
        return nullptr;

    const Buffer::const_iterator<char> str = bytes.As<char>();

    return new PyString( str, str + 1 );
}

PyRep* UnmarshalStream::LoadStringShort()
{
    uint8 len = 0;
    Buffer::const_iterator<uint8> bytes;
    if( !ReadValue( len ) || !ReadBytes( len, bytes ) )
        return nullptr;

    const Buffer::const_iterator<char> str = bytes.As<char>();

    return new PyString( str, str + len );
}

PyRep* UnmarshalStream::LoadStringLong()
{
    uint32 len = 0;
    Buffer::const_iterator<uint8> bytes;
    if( !ReadSizeEx( len ) || !ReadBytes( len, bytes ) )
        return nullptr;

    const Buffer::const_iterator<char> str = bytes.As<char>();

    return new PyString( str, str + len );
}

PyRep* UnmarshalStream::LoadStringTable()
{
    uint8 index = 0;
    if( !ReadValue( index ) )
        return nullptr;

    const char* str = sMarshalStringTable.LookupString( index );
    if( NULL == str )
    {
        sLog.Error( "Unmarshal", "String Table Item %u is out of range!", index );
        MarkFailed();
        return nullptr;
    }

    return new PyString( str );
}

PyRep* UnmarshalStream::LoadWStringUCS2Char()
{
    Buffer::const_iterator<uint8> bytes;
    if( !ReadBytes( sizeof( uint16 ), bytes ) )
        return nullptr;

    const Buffer::const_iterator<uint16> wstr = bytes.As<uint16>();

    // convert to UTF-8
    std::string str;
    utf8::utf16to8( wstr, wstr + 1, std::back_inserter( str ) );

    return new PyWString( str );
}

PyRep* UnmarshalStream::LoadWStringUCS2()
{
    uint32 len = 0;
    Buffer::const_iterator<uint8> bytes;
    if( !ReadSizeEx( len ) ||
        len > RemainingBytes() / sizeof( uint16 ) ||
        !ReadBytes( len * sizeof( uint16 ), bytes ) )
        return nullptr;

    const Buffer::const_iterator<uint16> wstr = bytes.As<uint16>();

    // convert to UTF-8
    std::string str;
    utf8::utf16to8( wstr, wstr + len, std::back_inserter( str ) );

    return new PyWString( str );
}

PyRep* UnmarshalStream::LoadWStringUTF8()
{
    uint32 len = 0;
    Buffer::const_iterator<uint8> bytes;
    if( !ReadSizeEx( len ) || !ReadBytes( len, bytes ) )
        return nullptr;

    const Buffer::const_iterator<char> wstr = bytes.As<char>();

    return new PyWString( wstr, wstr + len );
}

PyRep* UnmarshalStream::LoadToken()
{
    uint8 len = 0;
    Buffer::const_iterator<uint8> bytes;
    if( !ReadValue( len ) || !ReadBytes( len, bytes ) )
        return nullptr;

    const Buffer::const_iterator<char> str = bytes.As<char>();

    return new PyToken( str, str + len );
}

PyRep* UnmarshalStream::LoadBuffer()
{
    uint32 len = 0;
    Buffer::const_iterator<uint8> data;
    if( !ReadSizeEx( len ) || !ReadBytes( len, data ) )
        return nullptr;

    return new PyBuffer( data, data + len );
}

PyRep* UnmarshalStream::LoadTuple()
{
    uint32 count = 0;
    if( !ReadSizeEx( count ) || !ValidateContainerCount( count, 1 ) )
        return nullptr;

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
    uint32 count = 0;
    if( !ReadSizeEx( count ) || !ValidateContainerCount( count, 1 ) )
        return nullptr;

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
    uint32 count = 0;
    if( !ReadSizeEx( count ) || !ValidateContainerCount( count, 2 ) )
        return nullptr;

    PyDict* dict = new PyDict;

    for ( uint32 i(0); i < count; i++ )
    {
        PyRep* value = LoadRep();
        if( NULL == value )
        {
            PyDecRef( dict );
            return nullptr;
        }

        PyRep* key = LoadRep();
        if( NULL == key )
        {
            PyDecRef( value );
            PyDecRef( dict );
            return nullptr;
        }

        if( !IsHashableKey( key ) || key->hash() == -1 )
        {
            PyDecRef( key );
            PyDecRef( value );
            PyDecRef( dict );
            MarkFailed();
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
        MarkFailed();
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
    uint32 len = 0;
    Buffer::const_iterator<uint8> data;
    if( !ReadSizeEx( len ) || !ReadBytes( len, data ) )
        return nullptr;

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
    uint32 sum = 0;
    if( !ReadValue( sum ) )
        return nullptr;

    PyRep* ss = LoadRep();
    if( NULL == ss )
        return nullptr;

    return new PyChecksumedStream( ss, sum );
}

PyRep* UnmarshalStream::LoadPackedRow()
{
    // PyPackedRows are just a packed form of blue.DBRow
    // these take a DBRowDescriptor and the column data in different formats
    PyRep* headerElement = LoadRep();
    if( headerElement == nullptr )
        return nullptr;

    DBRowDescriptor* header = DecodePackedRowHeader(
        headerElement, EveProtocol::MAX_MARSHAL_CONTAINER_COUNT );
    PyDecRef( headerElement );
    if( header == nullptr )
    {
        MarkFailed();
        return nullptr;
    }

    const uint32 columnCount = header->ColumnCount();
    if( columnCount > EveProtocol::MAX_MARSHAL_CONTAINER_COUNT )
    {
        PyDecRef( header );
        MarkFailed();
        return nullptr;
    }

    // The row takes ownership of the decoded header.
    PyPackedRow* row = new PyPackedRow( header );
    const auto failRow = [this, row]() -> PyRep* {
        PyDecRef( row );
        MarkFailed();
        return nullptr;
    };

    // create the sizemap and sort it by bitsize, the value of the map indicates the index of the column
    // this can be used to identify things easily
    std::multimap< uint8, uint32, std::greater< uint8 > > sizeMap;
    std::map<uint32, size_t> booleanColumns;

    size_t byteDataBitLength = 0;
    size_t booleansBitLength = 0;
    size_t nullsBitLength = 0;

    for (uint32 i(0); i < columnCount; i++ )
    {
        DBTYPE columnType = row->header()->GetColumnType (i);
        uint8_t size = DBTYPE_GetSizeBits (columnType);

        // count booleans
        if (columnType == DBTYPE_BOOL)
        {
            booleanColumns.insert( std::make_pair( i, booleansBitLength ) );
            booleansBitLength++;
        }

        // count all columns as possible nulls
        nullsBitLength ++;

        // increase the bytedata length only if a column is longer than 7 bits
        // this is used as an indicator of what is written in the first, second or third part
        if (size >= 8)
            byteDataBitLength += size;

        // add the column to the list
        sizeMap.insert (std::make_pair (size, i));
    }

    const size_t flagBitLength = booleansBitLength + nullsBitLength;
    const size_t expectedByteSize =
        ( byteDataBitLength >> 3 ) + ( flagBitLength >> 3 ) + 1;
    if( expectedByteSize > EveProtocol::MAX_PACKET_SIZE )
        return failRow();

    // reserve enough space for the buffer
    Buffer unpacked (expectedByteSize, 0);

    if( !LoadRLE(unpacked) )
        return failRow();

    Buffer::const_iterator<uint8> unpackedItr = unpacked.begin<uint8>();

    std::multimap< uint8, uint32, std::greater< uint8 > >::iterator cur, end;
    cur = sizeMap.begin();
    end = sizeMap.end();
    for (; cur != end; ++cur)
    {
        const uint32 index = cur->second;
        const DBTYPE columnType = row->header ()->GetColumnType (index);

        const size_t nullBit =
            byteDataBitLength + booleansBitLength + cur->second;
        const size_t nullByte = nullBit >> 3;
        if( nullByte >= unpacked.size() )
            return failRow();

        const uint8 nullMask = static_cast<uint8>(
            1u << ( nullBit & 0x7 ) );
        if( ( unpacked[nullByte] & nullMask ) != 0 )
        {
            // PyNone value found! override it and increase the original iterator the required steps
            const size_t nullDataBytes =
                DBTYPE_GetSizeBits( columnType ) >> 3;
            if( !SkipPackedBytes(
                    unpackedItr, unpacked.end<uint8>(), nullDataBytes ) ||
                !row->SetField( index, new PyNone() ) )
                return failRow();

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
                int64 value = 0;
                if( !ReadPackedValue(
                        unpackedItr, unpacked.end<uint8>(), value ) ||
                    !row->SetField( index, new PyLong( value ) ) )
                    return failRow();
            } break;

            case DBTYPE_I4:
            {
                int32 value = 0;
                if( !ReadPackedValue(
                        unpackedItr, unpacked.end<uint8>(), value ) ||
                    !row->SetField( index, new PyInt( value ) ) )
                    return failRow();
            } break;
            case DBTYPE_UI4:
            {
                uint32 value = 0;
                if( !ReadPackedValue(
                        unpackedItr, unpacked.end<uint8>(), value ) ||
                    !row->SetField( index, new PyInt( value ) ) )
                    return failRow();
            } break;

            case DBTYPE_I2:
            {
                int16 value = 0;
                if( !ReadPackedValue(
                        unpackedItr, unpacked.end<uint8>(), value ) ||
                    !row->SetField( index, new PyInt( value ) ) )
                    return failRow();
            } break;
            case DBTYPE_UI2:
            {
                uint16 value = 0;
                if( !ReadPackedValue(
                        unpackedItr, unpacked.end<uint8>(), value ) ||
                    !row->SetField( index, new PyInt( value ) ) )
                    return failRow();
            } break;

            case DBTYPE_I1:
            {
                int8 value = 0;
                if( !ReadPackedValue(
                        unpackedItr, unpacked.end<uint8>(), value ) ||
                    !row->SetField( index, new PyInt( value ) ) )
                    return failRow();
            } break;

            case DBTYPE_UI1:
            {
                uint8 value = 0;
                if( !ReadPackedValue(
                        unpackedItr, unpacked.end<uint8>(), value ) ||
                    !row->SetField( index, new PyInt( value ) ) )
                    return failRow();
            } break;

            case DBTYPE_R8:
            {
                double value = 0.0;
                if( !ReadPackedValue(
                        unpackedItr, unpacked.end<uint8>(), value ) ||
                    !row->SetField( index, new PyFloat( value ) ) )
                    return failRow();
            } break;

            case DBTYPE_R4:
            {
                float value = 0.0f;
                if( !ReadPackedValue(
                        unpackedItr, unpacked.end<uint8>(), value ) ||
                    !row->SetField( index, new PyFloat( value ) ) )
                    return failRow();
            } break;

            case DBTYPE_BOOL:
            {
                // get the bit this boolean should be read from
                const std::map<uint32, size_t>::const_iterator boolColumn =
                    booleanColumns.find( index );
                if( boolColumn == booleanColumns.end() )
                    return failRow();

                const size_t boolBit =
                    byteDataBitLength + boolColumn->second;
                const size_t boolByte = boolBit >> 3;
                if( boolByte >= unpacked.size() )
                    return failRow();

                const uint8 boolMask = static_cast<uint8>(
                    1u << ( boolBit & 0x7 ) );
                if( !row->SetField(
                        index, new PyBool(
                            ( unpacked[boolByte] & boolMask ) != 0 ) ) )
                    return failRow();
            } break;

            // these objects are read directly from the end of the PyPackedRow
            // so they can be kept
            case DBTYPE_BYTES:
            case DBTYPE_STR:
            case DBTYPE_WSTR:
            {
                PyRep* el = LoadRep();
                if( NULL == el )
                    return failRow();

                if( !row->SetField( index, el ) )
                    return failRow();
            } break;

            case DBTYPE_EMPTY:
            case DBTYPE_ERROR:
            default:
                return failRow();
        }
    }

    return row;
}

PyRep* UnmarshalStream::LoadError()
{
    sLog.Error( "Unmarshal", "Invalid opcode encountered." );
    MarkFailed();

    return nullptr;
}

PyRep* UnmarshalStream::LoadSavedStreamElement()
{
    uint32 index = 0;
    if( !ReadSizeEx( index ) )
        return nullptr;

    PyRep* obj = GetStoredObject( index );
    if( NULL == obj )
    {
        sLog.Error( "Unmarshal", "SavedStreamElement: Got invalid stored object." );
        MarkFailed();
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

    uint8 next = 0;
    while( true )
    {
        if( !PeekValue( next ) )
        {
            PyDecRef( obj );
            MarkFailed();
            return nullptr;
        }
        if( next == Op_PackedTerminator )
            break;

        PyRep* el = LoadRep();
        if( NULL == el )
        {
            PyDecRef( obj );
            return nullptr;
        }

        obj->list().AddItem( el );
    }
    if( !ReadValue( next ) )
    {
        PyDecRef( obj );
        return nullptr;
    }

    while( true )
    {
        if( !PeekValue( next ) )
        {
            PyDecRef( obj );
            MarkFailed();
            return nullptr;
        }
        if( next == Op_PackedTerminator )
            break;

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

        if( !IsHashableKey( key ) || key->hash() == -1 )
        {
            PyDecRef( key );
            PyDecRef( value );
            PyDecRef( obj );
            MarkFailed();
            return nullptr;
        }

        obj->dict().SetItem( key, value );
    }
    if( !ReadValue( next ) )
    {
        PyDecRef( obj );
        return nullptr;
    }

    return obj;
}

bool UnmarshalStream::LoadRLE(Buffer& out)
{
    uint32 inSize = 0;
    Buffer::const_iterator<uint8> bytes;
    if( !ReadSizeEx( inSize ) || !ReadBytes( inSize, bytes ) )
        return false;

    const Buffer::const_iterator<uint8> end = bytes + inSize;
    Buffer::const_iterator<uint8> input = bytes;
    size_t outputIndex = 0;
    uint8 run = 0;
    bool lowNibble = true;

    while( input < end )
    {
        int count = 0;
        if( lowNibble )
        {
            run = *input++;
            count = static_cast<int>( run & 0x0F ) - 8;
        }
        else
        {
            count = static_cast<int>( run >> 4 ) - 8;
        }
        lowNibble = !lowNibble;

        if( count >= 0 )
        {
            const size_t zeroCount = static_cast<size_t>( count ) + 1;
            if( zeroCount > out.size() - outputIndex )
                return false;

            for( size_t index = 0; index < zeroCount; ++index )
                out[outputIndex++] = 0;
        }
        else
        {
            const size_t literalCount = static_cast<size_t>( -count );
            if( literalCount > static_cast<size_t>( end - input ) ||
                literalCount > out.size() - outputIndex )
                return false;

            for( size_t index = 0; index < literalCount; ++index )
                out[outputIndex++] = *input++;
        }
    }

    return true;
}
