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
    Authors:    BloodyRabbit, Captnoord, Zhur
*/

#include "EVECommonPCH.h"

#include "marshal/EVEMarshal.h"
#include "marshal/EVEMarshalOpcodes.h"
#include "marshal/EVEMarshalStringTable.h"
#include "marshal/EVEZeroCompress.h"
#include "python/classes/DBRowDescriptor.h"
#include "python/PyRep.h"
#include "python/PyVisitor.h"
#include "utils/EVEUtils.h"

uint8* Marshal(const PyRep* rep, uint32& len)
{
    if( rep == NULL )
        return NULL;

    MarshalVisitor v;
    rep->visit( &v );

    uint8* packetBuf;
    len = v.data( &packetBuf );

    return packetBuf;
}

uint8* MarshalDeflate(const PyRep* rep, uint32& len, const uint32 deflationLimit)
{
    uint8* data = Marshal( rep, len );
    if( data == NULL )
    {
        sLog.Error( "Marshal", "Failed to marshal Python object." );
        return NULL;
    }

    if( len >= deflationLimit )
    {
        // Deflate the buffer
        uint8* dataDeflated = DeflateData( data, &len );
        SafeDeleteArray( data );
        data = dataDeflated;
    }

    return data;
}

void MarshalVisitor::VisitInteger(const PyInt *rep)
{
    const int32 val = rep->value();

    if( val == -1 )
    {
        Put<uint8>(Op_PyMinusOne);
    }
    else if( val == 0 )
    {
        Put<uint8>(Op_PyZeroInteger);
    }
    else if( val == 1 )
    {
        Put<uint8>(Op_PyOneInteger);
    }
    else if( val + 0x8000u > 0xFFFF )
    {
        Put<uint8>(Op_PyLong);
        Put<int32>(val);
    }
    else if( val + 0x80u > 0xFF )
    {
        Put<uint8>(Op_PySignedShort);
        Put<int16>(val);
    }
    else
    {
        Put<uint8>(Op_PyByte);
        Put<int8>(val);
    }
}

void MarshalVisitor::VisitLong( const PyLong* rep )
{
    const int64 val = rep->value();

    if( val == -1 )
    {
        Put<uint8>(Op_PyMinusOne);
    }
    else if( val == 0 )
    {
        Put<uint8>(Op_PyZeroInteger);
    }
    else if( val == 1 )
    {
        Put<uint8>(Op_PyOneInteger);
    }
    else if( val + 0x800000u > 0xFFFFFFFF )
    {
        _PyInt_AsByteArray(rep);
    }
    else if( val + 0x8000u > 0xFFFF )
    {
        Put<uint8>(Op_PyLong);
        Put<int32>(val);
    }
    else if( val + 0x80u > 0xFF )
    {
        Put<uint8>(Op_PySignedShort);
        Put<int16>(val);
    }
    else
    {
        Put<uint8>(Op_PyByte);
        Put<int8>(val);
    }
}

void MarshalVisitor::VisitBoolean(const PyBool *rep)
{
    if(rep->value() == true)
        Put<uint8>(Op_PyTrue);
    else
        Put<uint8>(Op_PyFalse);
}

void MarshalVisitor::VisitReal(const PyFloat *rep)
{
    if(rep->value() == 0.0)
    {
        Put<uint8>(Op_PyZeroReal);
    }
    else
    {
        Put<uint8>(Op_PyReal);
        Put<double>(rep->value());
    }
}

void MarshalVisitor::VisitNone(const PyNone *rep)
{
    Put<uint8>(Op_PyNone);
}

void MarshalVisitor::VisitBuffer(const PyBuffer *rep)
{
    Put<uint8>(Op_PyBuffer);

    uint32 len = rep->size();
    PutSizeEx(len);
    Put(rep->content(), len);
}

void MarshalVisitor::VisitPackedRow(const PyPackedRow *rep)
{
    Put<uint8>( Op_PyPackedRow );

    DBRowDescriptor& header = rep->header();
    header.visit( this );

    // Create size map, sorted from the greatest to the smallest value:
    std::multimap< uint8, uint32, std::greater< uint8 > > sizeMap;
    uint32 cc = header.ColumnCount();

    size_t unpackedBlobSize = 0;
    for(uint32 i = 0; i < cc; i++)
    {
        uint8 fieldSize = DBTYPE_SizeOf( header.GetColumnType( i ) );

        sizeMap.insert( std::make_pair( fieldSize, i ) );
        unpackedBlobSize += (fieldSize >> 3);
    }

    std::vector<uint8> unpacked;
    unpacked.resize( unpackedBlobSize );

    size_t writeIndex = 0;

    std::multimap< uint8, uint32, std::greater< uint8 > >::iterator cur, end;
    cur = sizeMap.begin();
    end = sizeMap.lower_bound( 1 );
    for(; cur != end; cur++)
    {
        const PyRep* r = rep->GetField( cur->second );
        if( r == NULL )
            continue;

/* util macro's for faster buffer setting
 * TODO this needs to be reworked into a more generic sollution.
 */
#define setchunk(type, x) do{(*((type*)&unpacked[ writeIndex ])) = x; writeIndex+=sizeof(type);}while(0)
#define setu8(x) setchunk(uint8, x)
#define seti8(x) setchunk(int8, x)
#define setu16(x) setchunk(uint16, x)
#define seti16(x) setchunk(int16, x)
#define setu32(x) setchunk(uint32, x)
#define seti32(x) setchunk(int32, x)
#define setu64(x) setchunk(uint64, x)
#define seti64(x) setchunk(int64, x)
#define setfloat(x) setchunk(float, x)
#define setdouble(x) setchunk(double, x)

        /* note the assert are disabled because of performance flows */
        switch( header.GetColumnType( cur->second ) )
        {
        case DBTYPE_I8:
            //assert( r->IsLong() );
            seti64( ((PyLong*)r)->value());
            break;
        case DBTYPE_UI8:
            //assert( r->IsLong() );
            setu64( ((PyLong*)r)->value());
            break;
        case DBTYPE_CY:
        case DBTYPE_FILETIME:
            //assert( r->IsLong() );
            seti64( ((PyLong*)r)->value());
            break;
        case DBTYPE_I4:
            //assert( r->IsInt() );
            seti32( ((PyInt*)r)->value());
            break;
        case DBTYPE_UI4:
            //assert( r->IsInt() );
            setu32( ((PyInt*)r)->value());
            break;
        case DBTYPE_I2:
            //assert( r->IsInt() );
            seti16( ((PyInt*)r)->value());
            break;
        case DBTYPE_UI2:
            //assert( r->IsInt() );
            setu16( ((PyInt*)r)->value());
            break;
        case DBTYPE_I1:
            //assert( r->IsInt() );
            seti8( ((PyInt*)r)->value());
            break;
        case DBTYPE_UI1:
            //assert( r->IsInt() );
            setu8( ((PyInt*)r)->value());
            break;
        case DBTYPE_R8:
            //assert( r->IsFloat() );
            setdouble( ((PyFloat*)r)->value());
            break;
        case DBTYPE_R4:
            //assert( r->IsFloat() );
            setfloat( ((float)((PyFloat*)r)->value()) );
            break;
        case DBTYPE_BOOL:
            /* in correct implemented so we make sure we crash here */
            assert(false);
            break;
        case DBTYPE_BYTES:
        case DBTYPE_STR:
        case DBTYPE_WSTR:
            break;
        }

#undef setchunk
#undef setu8
#undef seti8
#undef setu16
#undef seti16
#undef setu32
#undef seti32
#undef setu64
#undef seti64
#undef setfloat
#undef setdouble
    }

    cur = sizeMap.lower_bound( 1 );
    end = sizeMap.lower_bound( 0 );
    for(uint8 off = 0; cur != end; cur++)
    {
        if( off > 7 )
            off = 0;
        if( off == 0 )
            unpacked.push_back( 0 );

        const PyRep* r = rep->GetField( cur->second );

        if( r != NULL )
            if( r-> IsBool() )
                unpacked.back() |= (r->AsBool().value() << off);

        off++;
    }

    //pack the bytes with the zero compression algorithm.
    std::vector<uint8> packed;
    PackZeroCompressed( &unpacked[ 0 ], unpacked.size(), packed );

    uint32 len = packed.size();
    PutSizeEx( len );

    if( !packed.empty() )
        //out goes the data...
        Put( &packed[0], len );

    // Append fields that are not packed:
    cur = sizeMap.lower_bound( 0 );
    end = sizeMap.end();
    for(; cur != end; cur++)
    {
        const PyRep* r = rep->GetField( cur->second );
        if( r == NULL )
            r = new PyNone;

        r->visit( this );
    }
}

void MarshalVisitor::VisitString(const PyString *rep)
{
    uint32 len = (uint32)rep->content().size();
    if(rep->isType1())
    {
        Put<uint8>(Op_PyByteString);
        PutSizeEx(len);
        Put(rep->content().c_str(), len);
    }
    else
    {
        if(len == 0)
        {
            Put<uint8>(Op_PyEmptyString);
        }
        else if(len == 1)
        {
            Put<uint8>(Op_PyCharString);
            Put<uint8>(rep->content()[0]);
        }
        else
        {
            //string is long enough for a string table entry, check it.
            size_t sid = sMarshalStringTable.LookupIndex(rep->content());
            if(sid != STRING_TABLE_ERROR)
            {
                Put<uint8>(Op_PyStringTableItem);
                Put<uint8>(sid);
            }
            // NOTE: they seem to have stopped using Op_PyShortString
            else
            {
                Put<uint8>(Op_PyLongString);
                PutSizeEx(len);
                Put(rep->content().c_str(), len);
            }
            // TODO: use Op_PyUnicodeString?
        }
    }
}

void MarshalVisitor::VisitObject(const PyObject *rep)
{
    Put<uint8>(Op_PyObject);
    //throw out the type string
    rep->type()->visit(this);
    //this will visit arguments
    rep->arguments()->visit(this);
}

void MarshalVisitor::VisitObjectEx(const PyObjectEx *rep)
{
    if (rep->isType2() == true)
        Put<uint8>(Op_ObjectEx2);
    else
        Put<uint8>(Op_ObjectEx1);

    //! visit the rep header
    rep->header()->visit(this);

    //! visit the rep list elements
    {
        PyObjectEx::const_list_iterator cur, end;
        cur = rep->list().begin();
        end = rep->list().end();
        for(; cur != end; ++cur)
           (*cur)->visit(this);
    }
    Put<uint8>(Op_PackedTerminator);

    //! visit the rep Dict elements
    {
        PyObjectEx::const_dict_iterator cur, end;
        cur = rep->dict().begin();
        end = rep->dict().end();
        for(; cur != end; ++cur)
        {
            cur->first->visit(this);
            cur->second->visit(this);
        }
    }
    Put<uint8>(Op_PackedTerminator);
}

void MarshalVisitor::VisitSubStruct(const PySubStruct *rep)
{
    Put<uint8>(Op_PySubStruct);
    rep->sub()->visit(this);
}

void MarshalVisitor::VisitSubStream(const PySubStream *rep)
{
    Put<uint8>(Op_PySubStream);

    if(rep->data() == NULL)
    {
        if(rep->decoded() == NULL)
        {
            Put<uint8>(0);
            //_log(NET__MARSHAL_ERROR, "Error: substream with no data or rep being marshaled!");
            return;
        }

        //unmarshaled stream
        //we have to marshal the substream.
        rep->EncodeData();
        if( rep->data() == NULL )
        {
            Put<uint8>(0);
            //_log( NET__MARSHAL_ERROR, "Failed to marshal substream." );
            return;
        }
    }

    //we have the marshaled data, use it.
    PutSizeEx(rep->data()->size());
    Put(rep->data()->content(), rep->data()->size());
}

//! TODO: check the implementation of this...
// we should never visit a checksummed stream... NEVER...
void MarshalVisitor::VisitChecksumedStream(const PyChecksumedStream *rep)
{
    assert(false && "MarshalStream on the server size should never send checksummed objects");
    Put<uint8>(Op_PyChecksumedStream);
    uint32 sum = 0;
    Put<uint32>(sum);
    rep->stream()->visit(this);
}

/** Add dict content to the stream
  */
void MarshalVisitor::VisitDict(const PyDict *rep)
{
    uint32 size = rep->size();

    Put<uint8>(Op_PyDict);
    PutSizeEx(size);

    PyDict::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(; cur != end; ++cur)
    {
        //we have to reverse the order of key/value to be value/key, so do not call base class.
        cur->second->visit(this);
        cur->first->visit(this);
    }
}

/** Add a list object to the stream
 */
void MarshalVisitor::VisitList(const PyList *rep)
{
    uint32 size = rep->size();

    if( size == 0 )
    {
        Put<uint8>(Op_PyEmptyList);
    }
    else if( size == 1 )
    {
        Put<uint8>(Op_PyOneList);
    }
    else
    {
        Put<uint8>(Op_PyList);
        PutSizeEx(size);
    }

    PyList::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(; cur != end; ++cur)
        (*cur)->visit(this);
}

/** Add tuple to the stream
 */
void MarshalVisitor::VisitTuple(const PyTuple *tuple)
{
    uint32 size = tuple->size();

    if( size == 0 )
    {
        Put<uint8>(Op_PyEmptyTuple);
    }
    else if( size == 1 )
    {
        Put<uint8>(Op_PyOneTuple);
    }
    else if( size == 2 )
    {
        Put<uint8>(Op_PyTwoTuple);
    }
    else
    {
        Put<uint8>(Op_PyTuple);
        PutSizeEx(size);
    }

    PyTuple::const_iterator cur, end;
    cur = tuple->begin();
    end = tuple->end();
    for(; cur != end; ++cur)
        (*cur)->visit(this);
}

//! private members

void MarshalVisitor::_PyInt_AsByteArray( const PyLong* v )
{
    const uint64 value = v->value();
    uint8 integerSize = 0;
    reserve(8);

#define DoIntegerSizeCheck(x) if( ( (uint8*)&value )[x] != 0 ) integerSize = x + 1;
    DoIntegerSizeCheck(4);
    DoIntegerSizeCheck(5);
    DoIntegerSizeCheck(6);
#undef  DoIntegerSizeCheck

    if (integerSize != 0 && integerSize < 7)
    {
        Put<uint8>(Op_PyVarInteger);
        Put<uint8>(integerSize);
        Put(&value, integerSize);
    }
    else
    {
        Put<uint8>(Op_PyLongLong);                    // 1
        Put<int64>(value);                           // 8
    }
}

