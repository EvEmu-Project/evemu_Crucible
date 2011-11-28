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
    Authors:    BloodyRabbit, Captnoord, Zhur
*/

#include "EVECommonPCH.h"

#include "marshal/EVEMarshal.h"
#include "marshal/EVEMarshalOpcodes.h"
#include "marshal/EVEMarshalStringTable.h"
#include "python/classes/PyDatabase.h"
#include "python/PyRep.h"
#include "python/PyVisitor.h"
#include "utils/EVEUtils.h"

bool Marshal( const PyRep* rep, Buffer& into )
{
    MarshalStream v;
    return v.Save( rep, into );
}

bool MarshalDeflate( const PyRep* rep, Buffer& into, const uint32 deflationLimit )
{
    Buffer data;
    if( !Marshal( rep, data ) )
        return false;

    if( data.size() >= deflationLimit )
        return DeflateData( data, into );
    else
    {
        into.AppendSeq( data.begin<uint8>(), data.end<uint8>() );
        return true;
    }
}

/************************************************************************/
/* MarshalStream                                                        */
/************************************************************************/
MarshalStream::MarshalStream()
: mBuffer( NULL )
{
}

bool MarshalStream::Save( const PyRep* rep, Buffer& into )
{
    mBuffer = &into;
    bool res = SaveStream( rep );
    mBuffer = NULL;

    return res;
}

bool MarshalStream::SaveStream( const PyRep* rep )
{
    if( rep == NULL )
        return false;

    Put<uint8>( MarshalHeaderByte );
    /*
     * Mapcount
     * the amount of referenced objects within a marshal stream.
     * Note: Atm not supported.
     */
    Put<uint32>( 0 ); // Mapcount

    return rep->visit( *this );
}

bool MarshalStream::VisitInteger( const PyInt* rep )
{
    const int32 val = rep->value();

    if( val == -1 )
    {
        Put<uint8>( Op_PyMinusOne );
    }
    else if( val == 0 )
    {
        Put<uint8>( Op_PyZeroInteger );
    }
    else if( val == 1 )
    {
        Put<uint8>( Op_PyOneInteger );
    }
    else if( val + 0x8000u > 0xFFFF )
    {
        Put<uint8>( Op_PyLong );
        Put<int32>( val );
    }
    else if( val + 0x80u > 0xFF )
    {
        Put<uint8>( Op_PySignedShort );
        Put<int16>( val );
    }
    else
    {
        Put<uint8>( Op_PyByte );
        Put<int8>( val );
    }

    return true;
}

bool MarshalStream::VisitLong( const PyLong* rep )
{
    const int64 val = rep->value();

    if( val == -1 )
    {
        Put<uint8>( Op_PyMinusOne );
    }
    else if( val == 0 )
    {
        Put<uint8>( Op_PyZeroInteger );
    }
    else if( val == 1 )
    {
        Put<uint8>( Op_PyOneInteger );
    }
    else if( val + 0x800000u > 0xFFFFFFFF )
    {
        SaveVarInteger( rep );
    }
    else if( val + 0x8000u > 0xFFFF )
    {
        Put<uint8>( Op_PyLong );
        Put<int32>(static_cast<int32>(val));
    }
    else if( val + 0x80u > 0xFF )
    {
        Put<uint8>( Op_PySignedShort );
        Put<int16>(static_cast<int16>(val));
    }
    else
    {
        Put<uint8>( Op_PyByte );
        Put<int8>(static_cast<int8>(val));
    }

    return true;
}

bool MarshalStream::VisitBoolean( const PyBool* rep )
{
    if( rep->value() == true )
        Put<uint8>( Op_PyTrue );
    else
        Put<uint8>( Op_PyFalse );

    return true;
}

bool MarshalStream::VisitReal( const PyFloat* rep )
{
    if( rep->value() == 0.0 )
    {
        Put<uint8>( Op_PyZeroReal );
    }
    else
    {
        Put<uint8>( Op_PyReal );
        Put<double>( rep->value() );
    }

    return true;
}

bool MarshalStream::VisitNone( const PyNone* rep )
{
    Put<uint8>( Op_PyNone );
    return true;
}

bool MarshalStream::VisitBuffer( const PyBuffer* rep )
{
    Put<uint8>( Op_PyBuffer );

    const Buffer& buf = rep->content();

    PutSizeEx( buf.size() );
    Put( buf.begin<uint8>(), buf.end<uint8>() );

    return true;
}

bool MarshalStream::VisitString( const PyString* rep )
{
    size_t len = rep->content().size();

    if( len == 0 )
    {
        Put<uint8>( Op_PyEmptyString );
    }
    else if( len == 1 )
    {
        Put<uint8>( Op_PyCharString );
        Put<uint8>( rep->content()[0] );
    }
    else
    {
        //string is long enough for a string table entry, check it.
        const uint8 index = sMarshalStringTable.LookupIndex( rep->content() );
        if( STRING_TABLE_ERROR != index )
        {
            Put<uint8>( Op_PyStringTableItem );
            Put<uint8>( index );
        }
        // NOTE: they seem to have stopped using Op_PyShortString
        else
        {
            Put<uint8>( Op_PyLongString );
            PutSizeEx( len );
            Put( rep->content().begin(), rep->content().end() );
        }
    }

    return true;
}

bool MarshalStream::VisitWString( const PyWString* rep )
{
    size_t len = rep->content().size();

    if( 0 == len )
    {
        Put<uint8>( Op_PyEmptyWString );
    }
    else
    {
        // We don't have to consider any conversions because
        // UTF-8 is more space-efficient than UCS-2.

        Put<uint8>( Op_PyWStringUTF8 );
        PutSizeEx( len );
        Put( rep->content().begin(), rep->content().end() );
    }

    return true;
}

bool MarshalStream::VisitToken( const PyToken* rep )
{
    Put<uint8>( Op_PyToken );

    const std::string& str = rep->content();

    PutSizeEx( str.size() );
    Put( str.begin(), str.end() );

    return true;
}

bool MarshalStream::VisitTuple( const PyTuple* rep )
{
    uint32 size = rep->size();
    if( size == 0 )
    {
        Put<uint8>( Op_PyEmptyTuple );
    }
    else if( size == 1 )
    {
        Put<uint8>( Op_PyOneTuple );
    }
    else if( size == 2 )
    {
        Put<uint8>( Op_PyTwoTuple );
    }
    else
    {
        Put<uint8>( Op_PyTuple );
        PutSizeEx( size );
    }

    return PyVisitor::VisitTuple( rep );
}

bool MarshalStream::VisitList( const PyList* rep )
{
    uint32 size = rep->size();
    if( size == 0 )
    {
        Put<uint8>( Op_PyEmptyList );
    }
    else if( size == 1 )
    {
        Put<uint8>( Op_PyOneList );
    }
    else
    {
        Put<uint8>( Op_PyList );
        PutSizeEx( size );
    }

    return PyVisitor::VisitList( rep );
}

bool MarshalStream::VisitDict( const PyDict* rep )
{
    uint32 size = rep->size();

    Put<uint8>( Op_PyDict );
    PutSizeEx( size );

    //we have to reverse the order of key/value to be value/key, so do not call base class.
    PyDict::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(; cur != end; ++cur)
    {
        if( !cur->second->visit( *this ) )
            return false;
        if( !cur->first->visit( *this ) )
            return false;
    }

    return true;
}

bool MarshalStream::VisitObject( const PyObject* rep )
{
    Put<uint8>( Op_PyObject );
    return PyVisitor::VisitObject( rep );
}

bool MarshalStream::VisitObjectEx( const PyObjectEx* rep )
{
    if( rep->isType2() == true )
        Put<uint8>( Op_PyObjectEx2 );
    else
        Put<uint8>( Op_PyObjectEx1 );

    if( !rep->header()->visit( *this ) )
        return false;

    {
        PyObjectEx::const_list_iterator cur, end;
        cur = rep->list().begin();
        end = rep->list().end();
        for(; cur != end; ++cur)
        {
           if( !(*cur)->visit( *this ) )
               return false;
        }
    }
    Put<uint8>( Op_PackedTerminator );

    {
        PyObjectEx::const_dict_iterator cur, end;
        cur = rep->dict().begin();
        end = rep->dict().end();
        for(; cur != end; ++cur)
        {
            if( !cur->first->visit( *this ) )
                return false;
            if( !cur->second->visit( *this ) )
                return false;
        }
    }
    Put<uint8>( Op_PackedTerminator );

    return true;
}

bool MarshalStream::VisitPackedRow( const PyPackedRow* rep )
{
    Put<uint8>( Op_PyPackedRow );

    DBRowDescriptor* header = rep->header();
    header->visit( *this );

    // Create size map, sorted from the greatest to the smallest value:
    std::multimap< uint8, uint32, std::greater< uint8 > > sizeMap;
    uint32 cc = header->ColumnCount();
    size_t sum = 0;

    for( uint32 i = 0; i < cc; i++ )
    {
        uint8 size = DBTYPE_GetSizeBits( header->GetColumnType( i ) );

        sizeMap.insert( std::make_pair( size, i ) );
        sum += size;
    }

    Buffer unpacked;
    sum = ( ( sum + 7 ) >> 3 );
    unpacked.Reserve<uint8>( sum );

    std::multimap< uint8, uint32, std::greater< uint8 > >::iterator cur, end;
    cur = sizeMap.begin();
    end = sizeMap.lower_bound( 1 );
    for(; cur != end; cur++)
    {
        const uint32 index = cur->second;
        const PyRep* r = rep->GetField( index );

        /* note the assert are disabled because of performance flows */
        switch( header->GetColumnType( index ) )
        {
            case DBTYPE_I8:
            case DBTYPE_UI8:
            case DBTYPE_CY:
            case DBTYPE_FILETIME:
            {
                unpacked.Append<int64>( r->IsNone() ? 0 : r->AsLong()->value() );
            } break;

            case DBTYPE_I4:
            case DBTYPE_UI4:
            {
                unpacked.Append<int32>( r->IsNone() ? 0 : r->AsInt()->value() );
            } break;

            case DBTYPE_I2:
            case DBTYPE_UI2:
            {
                unpacked.Append<int16>( r->IsNone() ? 0 : r->AsInt()->value() );
            } break;

            case DBTYPE_I1:
            case DBTYPE_UI1:
            {
                unpacked.Append<int8>( r->IsNone() ? 0 : r->AsInt()->value() );
            } break;

            case DBTYPE_R8:
            {
                unpacked.Append<double>( r->IsNone() ? 0.0 : r->AsFloat()->value() );
            } break;

            case DBTYPE_R4:
            {
                unpacked.Append<float>(static_cast<float>(r->IsNone() ? 0.0 : r->AsFloat()->value()));
            } break;

            case DBTYPE_BOOL:
            case DBTYPE_BYTES:
            case DBTYPE_STR:
            case DBTYPE_WSTR:
            {
                /* incorrect implemented so we make sure we crash here */
                assert( false );
            } break;
        }
    }

    uint8 bitOffset = 0;
    Buffer::iterator<uint8> bitByte;

    cur = sizeMap.lower_bound( 1 );
    end = sizeMap.lower_bound( 0 );
    for(; cur != end; ++cur)
    {
        const uint32 index = cur->second;
        const PyBool* r = rep->GetField( index )->AsBool();

        if( 7 < bitOffset )
            bitOffset = 0;
        if( 0 == bitOffset )
        {
            bitByte = unpacked.end<uint8>();
            unpacked.ResizeAt( bitByte, 1 );
        }

        *bitByte |= ( r->value() << bitOffset++ );
    }

    //pack the bytes with the zero compression algorithm.
    if( !SaveZeroCompressed( unpacked ) )
        return false;

    // Append fields that are not packed:
    cur = sizeMap.lower_bound( 0 );
    end = sizeMap.end();
    for(; cur != end; cur++)
    {
        const uint32 index = cur->second;
        const PyRep* r = rep->GetField( index );

        if( !r->visit( *this ) )
            return false;
    }

    return true;
}

bool MarshalStream::VisitSubStruct( const PySubStruct* rep )
{
    Put<uint8>(Op_PySubStruct);
    return PyVisitor::VisitSubStruct( rep );
}

bool MarshalStream::VisitSubStream( const PySubStream* rep )
{
    Put<uint8>(Op_PySubStream);

    if(rep->data() == NULL)
    {
        if(rep->decoded() == NULL)
        {
            Put<uint8>(0);
            return false;
        }

        //unmarshaled stream
        //we have to marshal the substream.
        rep->EncodeData();
        if( rep->data() == NULL )
        {
            Put<uint8>(0);
            return false;
        }
    }

    //we have the marshaled data, use it.
    const Buffer& data = rep->data()->content();

    PutSizeEx( data.size() );
    Put( data.begin<uint8>(), data.end<uint8>() );

    return true;
}

//! TODO: check the implementation of this...
// we should never visit a checksummed stream... NEVER...
bool MarshalStream::VisitChecksumedStream( const PyChecksumedStream* rep )
{
    assert(false && "MarshalStream on the server size should never send checksummed objects");

    Put<uint8>(Op_PyChecksumedStream);

    Put<uint32>( rep->checksum() );
    return PyVisitor::VisitChecksumedStream( rep );
}

void MarshalStream::SaveVarInteger( const PyLong* v )
{
    const uint64 value = v->value();
    uint8 integerSize = 0;

#define DoIntegerSizeCheck(x) if( ( (uint8*)&value )[x] != 0 ) integerSize = x + 1;
    DoIntegerSizeCheck(4);
    DoIntegerSizeCheck(5);
    DoIntegerSizeCheck(6);
#undef  DoIntegerSizeCheck

    if( integerSize > 0 && integerSize < 7 )
    {
        Put<uint8>(Op_PyVarInteger);
        Put<uint8>(integerSize);
        Put( &( (uint8*)&value )[0], &( (uint8*)&value )[integerSize] );
    }
    else
    {
        Put<uint8>(Op_PyLongLong);                    // 1
        Put<int64>(value);                           // 8
    }
}

bool MarshalStream::SaveZeroCompressed( const Buffer& data )
{
    Buffer packed;

    Buffer::const_iterator<uint8> cur, end;
    cur = data.begin<uint8>();
    end = data.end<uint8>();
    while( cur < end )
    {
        // Insert opcode
        Buffer::iterator<ZeroCompressOpcode> opcode = packed.end<ZeroCompressOpcode>();
        packed.ResizeAt( opcode, 1 );

        // Encode first part
        if( 0x00 == *cur )
        {
            opcode->firstIsZero = true;
            opcode->firstLen = -1;

            do
            {
                ++cur;
                ++opcode->firstLen;
            } while( 7 > opcode->firstLen && ( cur < end ? 0x00 == *cur : false ) );
        }
        else
        {
            opcode->firstIsZero = false;
            opcode->firstLen = 8;

            do
            {
                packed.Append<uint8>( *cur++ );
                --opcode->firstLen;
            } while( 0 < opcode->firstLen && ( cur < end ? 0x00 != *cur : false ) );
        }

        // Check whether we have data for second part
        if( cur >= end )
        {
            opcode->secondIsZero = true;
            opcode->secondLen = 0;
        }
        // Encode second part
        else if( 0x00 == *cur )
        {
            opcode->secondIsZero = true;
            opcode->secondLen = -1;

            do
            {
                ++cur;
                ++opcode->secondLen;
            } while( 7 > opcode->secondLen && ( cur < end ? 0x00 == *cur : false ) );
        }
        else
        {
            opcode->secondIsZero = false;
            opcode->secondLen = 8;

            do
            {
                packed.Append<uint8>( *cur++ );
                --opcode->secondLen;
            } while( 0 < opcode->secondLen && ( cur < end ? 0x00 != *cur : false ) );
        }
    }

    PutSizeEx( packed.size() );
    if( 0 < packed.size() )
        Put( packed.begin<uint8>(), packed.end<uint8>() );

    return true;
}

