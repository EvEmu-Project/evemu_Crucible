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

#include <stdio.h>
#include <string>
#include <zlib.h>
#include <assert.h>

#include "common.h"
#include "packet_dump.h"
#include "packet_functions.h"
#include "logsys.h"
#include "PyVisitor.h"
#include "PyRep.h"
#include "EVEMarshalOpcodes.h"
#include "EVEUtils.h"
#include "EVEZeroCompress.h"
#include "EVEMarshal.h"
#include "PyStringTable.h"

const uint32 EVEDeflationBytesLimit = 10000;    //every packet larger than this is deflated

/* Marshal Stream builder
 *\Note: it returns ownership of the buffer
 */
uint8 *Marshal(const PyRep *rep, uint32& len, bool inlineSubStream)
{
    if (rep==NULL)
        return NULL;

    MarshalVisitor v;

    if(inlineSubStream == true)
    {
        v.PutByte(SubStreamHeaderByte);

        /* Mapcount
         * the amount of referenced objects within a marshal stream.
         * Note: Atm not supported.
         */
        v.PutUint32(0); // Mapcount
    }

    rep->visit(&v);

    len = v.size();

    uint8 *packetBuff = NULL;

    // check if we need to compress the data
    if (len > EVEDeflationBytesLimit)
    {
        packetBuff = DeflatePacket(v.data(), &len);
    }
    else
    {
        packetBuff = v.data();
    }

    uint8 *b = new uint8[len];

    memcpy(b, packetBuff, len);

    if (len > EVEDeflationBytesLimit)
        SafeFree( packetBuff );

    return b;
}

void MarshalVisitor::VisitInteger(const PyRepInteger *rep)
{
    const uint64 & val = rep->value;
    if (val == (uint64)-1)
    {
        PutByte(Op_PyMinusOne);
        return;
    }
    if (val == 0)
    {
        PutByte(Op_PyZeroInteger);
        return;
    }
    if (val == 1)
    {
        PutByte(Op_PyOneInteger);
        return;
    }
    if ( val + 0x80u > 0xFF )
    {
        if ( val + 0x8000u > 0xFFFF )
        {
            if (val + 0x800000u > 0xFFFFFFFF)
            {
                _PyRepInteger_AsByteArray(rep);
                return;
            }
            else
            {
                PutByte(Op_PyLong);
                PutUint32(val);
                return;
            }
        }
        else
        {
            PutByte(Op_PySignedShort);
            PutUint16(val);
            return;
        }
    }
    else
    {
        PutByte(Op_PyByte);
        PutByte(val);
        return;
    }
}

void MarshalVisitor::VisitBoolean(const PyRepBoolean *rep)
{
    if(rep->value == true)
        PutByte(Op_PyTrue);
    else
        PutByte(Op_PyFalse);
}

void MarshalVisitor::VisitReal(const PyRepReal *rep)
{
    if(rep->value == 0.0)
    {
        PutByte(Op_PyZeroReal);
    }
    else
    {
        PutByte(Op_PyReal);
        PutDouble(rep->value);
    }
}

void MarshalVisitor::VisitNone(const PyRepNone *rep)
{
    PutByte(Op_PyNone);
}

void MarshalVisitor::VisitBuffer(const PyRepBuffer *rep)
{
    PutByte(Op_PyBuffer);
    uint32 len = rep->GetLength();
    PutSizeEx(len);
    PutBytes(rep->GetBuffer(), len);
}

void MarshalVisitor::VisitPackedRow(const PyRepPackedRow *rep)
{
    PutByte( Op_PyPackedRow );

    rep->GetHeader().visit( this );

    std::vector<uint8> unpacked;
    unpacked.reserve( 64 );

    // Create size map, sorted from the greatest to the smallest value:
    std::multimap< uint8, uint32, std::greater< uint8 > > sizeMap;
    uint32 cc = rep->ColumnCount();
    for(uint32 i = 0; i < cc; i++)
        sizeMap.insert( std::make_pair( DBTYPE_SizeOf( rep->GetColumnType( i ) ), i ) );

    std::multimap< uint8, uint32, std::greater< uint8 > >::iterator cur, end;
    cur = sizeMap.begin();
    end = sizeMap.lower_bound( 1 );
    for(; cur != end; cur++)
    {
        uint8 len = (cur->first >> 3);

        size_t off = unpacked.size();
        unpacked.resize( off + len );

        union
        {
            uint64 i;
            double r8;
            float r4;
        } v;
        v.i = 0;

        PyRep *r = rep->GetField( cur->second );
        if( r != NULL )
        {
            switch( rep->GetColumnType( cur->second ) )
            {
                case DBTYPE_I8:
                case DBTYPE_UI8:
                case DBTYPE_CY:
                case DBTYPE_FILETIME:
                case DBTYPE_I4:
                case DBTYPE_UI4:
                case DBTYPE_I2:
                case DBTYPE_UI2:
                case DBTYPE_I1:
                case DBTYPE_UI1:
                    if( r->IsInteger() )
                        v.i = r->AsInteger().value;
                    else if( r->IsReal() )
                        v.i = r->AsReal().value;
                    break;

                case DBTYPE_R8:
                    if( r->IsReal() )
                        v.r8 = r->AsReal().value;
                    else if( r->IsInteger() )
                        v.r8 = r->AsInteger().value;
                    break;

                case DBTYPE_R4:
                    if( r->IsReal() )
                        v.r4 = r->AsReal().value;
                    else if( r->IsInteger() )
                        v.r4 = r->AsInteger().value;
                    break;
                case DBTYPE_BOOL:
                case DBTYPE_BYTES:
                case DBTYPE_STR:
                case DBTYPE_WSTR:
                    //! TODO: handle this DB types.
                    break;
            }
        }

        memcpy( &unpacked[ off ], &v, len );
    }

    cur = sizeMap.lower_bound( 1 );
    end = sizeMap.lower_bound( 0 );
    for(uint8 off = 0; cur != end; cur++)
    {
        if( off > 7 )
            off = 0;
        if( off == 0 )
            unpacked.push_back( 0 );

        PyRep *r = rep->GetField( cur->second );

        if( r != NULL )
            if( r-> IsBool() )
                unpacked.back() |= (r->AsBool().value << off);

        off++;
    }

    //pack the bytes with the zero compression algorithm.
    std::vector<uint8> packed;
    PackZeroCompressed( &unpacked[ 0 ], unpacked.size(), packed );

    uint32 len = packed.size();
    if(len >= 0xFF)
    {
        PutByte(0xFF);
        PutUint32(len);
    }
    else
        PutByte(len);

    if( !packed.empty() )
        //out goes the data...
        PutBytes( &packed[0], len );

    // Append fields that are not packed:
    cur = sizeMap.lower_bound( 0 );
    end = sizeMap.end();
    for(; cur != end; cur++)
    {
        PyRep *r = rep->GetField( cur->second );
        if( r == NULL )
            r = new PyRepNone;

        r->visit( this );
    }
}

void MarshalVisitor::VisitString(const PyRepString *rep)
{
    uint32 len = (uint32)rep->value.length();
    if(rep->is_type_1)
    {
        PutByte(Op_PyByteString);
        PutSizeEx(len);
        PutBytes(rep->value.c_str(), len);
    }
    else
    {
        if(len == 0)
        {
            PutByte(Op_PyEmptyString);
        }
        else if(len == 1)
        {
            PutByte(Op_PyCharString);
            PutByte(rep->value.c_str()[0]);
        }
        else
        {
            //string is long enough for a string table entry, check it.
            size_t sid = sPyStringTable.LookupIndex(rep->value.c_str());
            if(sid != -1)
            {
                PutByte(Op_PyStringTableItem);
                PutByte(sid);
            }
            // NOTE: they seem to have stopped using Op_PyShortString
            else
            {
                PutByte(Op_PyLongString);
                PutSizeEx(len);
                PutBytes(rep->value.c_str(), len);
            }
            // TODO: use Op_PyUnicodeString?
        }
    }
}

void MarshalVisitor::VisitObject(const PyRepObject *rep)
{
    PutByte(Op_PyObject);
    //throw out the type string
    PyRepString s(rep->type);
    s.visit(this);
    //this will visit arguments
    rep->arguments->visit(this);
}

void MarshalVisitor::VisitObjectEx(const PyRepObjectEx *rep)
{
    if (rep->is_type_1 == true)
        PutByte(Op_ObjectEx2);
    else
        PutByte(Op_ObjectEx1);

    //! visit the rep header
    rep->header->visit(this);

    //! visit the rep list elements
    {
        PyRepObjectEx::const_list_iterator cur, end;
        cur = rep->list_data.begin();
        end = rep->list_data.end();
        for(; cur != end; ++cur)
           (*cur)->visit(this);
    }
    PutByte(Op_PackedTerminator);

    //! visit the rep Dict elements
    {
        PyRepObjectEx::const_dict_iterator cur, end;
        cur = rep->dict_data.begin();
        end = rep->dict_data.end();
        for(; cur != end; ++cur)
        {
            cur->first->visit(this);
            cur->second->visit(this);
        }
    }
    PutByte(Op_PackedTerminator);
}

void MarshalVisitor::VisitSubStruct(const PyRepSubStruct *rep)
{
    PutByte(Op_PySubStruct);
    rep->sub->visit(this);
}

void MarshalVisitor::VisitSubStream(const PyRepSubStream *rep)
{
    PutByte(Op_PySubStream);
    if(rep->length == 0 || rep->data == NULL)
    {
        if(rep->decoded == NULL)
        {
            PutByte(0);
            //_log(NET__MARSHAL_ERROR, "Error: substream with no data or rep being marshaled!");
            return;
        }

        //unmarshaled stream
        //we have to marshal the substream first to get its length.
        MarshalVisitor v;
        v.PutByte(SubStreamHeaderByte);

        uint32 SubStreamMapCount = 0;
        v.PutUint32(SubStreamMapCount);

        rep->decoded->visit(&v);

        uint32 length = v.size();
        // put in the size
        PutSizeEx(length);
        // put in the data
        PutBytes(v.data(), v.size());
    }
    else
    {
        //else, we have the marshaled data already, use it.
        PutSizeEx(rep->length);
        PutBytes(rep->data, rep->length);
    }
}

//! TODO: check the implementation of this...
// we should never visit a checksummed stream... NEVER...
void MarshalVisitor::VisitChecksumedStream(const PyRepChecksumedStream *rep)
{
    assert(false && "MarshalStream on the server size should never send checksummed objects");
    PutByte(Op_PyChecksumedStream);
    uint32 sum = 0;
    PutUint32(sum);
    rep->stream->visit(this);
}

/** Add dict content to the stream
  */
void MarshalVisitor::VisitDict(const PyRepDict *rep)
{
    uint32 size = (uint32)rep->items.size();
    PutByte(Op_PyDict);
    PutSizeEx(size);

    PyRepDict::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(; cur != end; ++cur) {
        //we have to reverse the order of key/value to be value/key, so do not call base class.
        cur->second->visit(this);
        cur->first->visit(this);
    }
}

/** Add a list object to the stream
 */
void MarshalVisitor::VisitList(const PyRepList *rep)
{
    uint32 size = rep->size();

    if(size == 0)
    {
        PutByte(Op_PyEmptyList);
    }
    else if(size == 1)
    {
        PutByte(Op_PyOneList);

        PyRepList::const_iterator cur, end;
        cur = rep->begin();
        end = rep->end();
        for(; cur != end; ++cur) {
            (*cur)->visit(this);
        }
    }
    else if(size > 1)
    {
        PutByte(Op_PyList);
        PutSizeEx(size);

        PyRepList::const_iterator cur, end;
        cur = rep->begin();
        end = rep->end();
        for(; cur != end; ++cur) {
            (*cur)->visit(this);
        }
    }
}

/** Add tuple to the stream
 */
void MarshalVisitor::VisitTuple(const PyRepTuple *tuple)
{
    uint32 size = tuple->size();
    if(size == 0)
    {
        PutByte(Op_PyEmptyTuple);
    }
    else if(size == 1)
    {
        PutByte(Op_PyOneTuple);

        PyRepTuple::const_iterator cur, end;
        cur = tuple->begin();
        end = tuple->end();
        for(; cur != end; ++cur) {
            (*cur)->visit(this);
        }
    }
    else if(size == 2)
    {
        PutByte(Op_PyTwoTuple);

        PyRepTuple::const_iterator cur, end;
        cur = tuple->begin();
        end = tuple->end();
        for(; cur != end; ++cur) {
            (*cur)->visit(this);
        }
    }
    else if(size > 2)
    {
        PutByte(Op_PyTuple);
        PutSizeEx(size);

        PyRepTuple::const_iterator cur, end;
        cur = tuple->begin();
        end = tuple->end();
        for(; cur != end; ++cur) {
            (*cur)->visit(this);
        }
    }
}

//! private members

void MarshalVisitor::_PyRepInteger_AsByteArray(const PyRepInteger* v) {
    reserve(8);
    uint8 integerSize = 0;
#define DoIntegerSizeCheck(x) if (((uint8*)&v->value)[x] != 0) integerSize = x + 1;
    DoIntegerSizeCheck(4);
    DoIntegerSizeCheck(5);
    DoIntegerSizeCheck(6);
#undef  DoIntegerSizeCheck

    if (integerSize != 0 && integerSize < 7)
    {
        PutByte(Op_PyVarInteger);
        PutByte(integerSize);
        PutBytes((char*)&v->value, integerSize);
    }
    else
    {
        PutByte(Op_PyLongLong);                     // 1
        PutUint64(v->value);                        // 8
    }
}
