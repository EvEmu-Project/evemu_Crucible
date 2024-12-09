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
    Authors:    BloodyRabbit, Captnoord, Zhur
*/

#include "eve-common.h"

#include "marshal/EVEMarshal.h"
#include "marshal/EVEMarshalOpcodes.h"
#include "marshal/EVEMarshalStringTable.h"
#include "python/classes/PyDatabase.h"
#include "python/PyRep.h"
#include "python/PyVisitor.h"
#include "utils/EVEUtils.h"

bool Marshal( const PyRep* rep, Buffer& into )
{
    MarshalStream* pMS(new MarshalStream());
    bool ret(pMS->Save(rep, into));
    SafeDelete(pMS);
    return ret;
}

bool MarshalDeflate( const PyRep* rep, Buffer& into, const uint32 deflationLimit )
{
    Buffer* data(new Buffer());
    bool ret(false);
    if (Marshal(rep, *data)) {
        if ( data->size() >= deflationLimit ) {
            ret = DeflateData( *data, into );
        } else {
            into.AppendSeq( data->begin<uint8>(), data->end<uint8>() );
            ret = true;
        }
    }

    SafeDelete(data);
    return ret;
}

/************************************************************************/
/* MarshalStream                                                        */
/************************************************************************/
MarshalStream::MarshalStream()
: mBuffer( nullptr )
{
}

bool MarshalStream::Save( const PyRep* rep, Buffer& into )
{
    mBuffer = &into;
    bool res(SaveStream(rep));
    mBuffer = nullptr;

    return res;
}

bool MarshalStream::SaveStream( const PyRep* rep )
{
    Put<uint8>( MarshalHeaderByte );
    /*
     * Mapcount
     * the amount of referenced objects within a marshal stream.
     * Note: Atm not supported.
     * (allan)  have not found any information on this, so no idea how/when to implement it (or even if we need to)
     */
    Put<uint32>( 0 ); // Mapcount

    return rep->visit( *this );
}

bool MarshalStream::VisitInteger( const PyInt* rep )
{
    const int32 val(rep->value());

    if ( val == -1 ) {
        Put<uint8>( Op_PyMinusOne );
    } else if ( val == 0 ) {
        Put<uint8>( Op_PyZeroInteger );
    } else if ( val == 1 ) {
        Put<uint8>( Op_PyOneInteger );
    } else if ( val + 0x8000u > 0xFFFF ) {
        Put<uint8>( Op_PyLong );
        Put<int32>( val );
    } else if ( val + 0x80u > 0xFF ) {
        Put<uint8>( Op_PySignedShort );
        Put<int16>( val );
    } else {
        Put<uint8>( Op_PyByte );
        Put<int8>( val );
    }

    return true;
}

bool MarshalStream::VisitLong( const PyLong* rep )
{
    SaveVarInteger (rep);

    return true;
}

bool MarshalStream::VisitBoolean( const PyBool* rep )
{
    if (rep->value())
        Put<uint8>( Op_PyTrue );
    else
        Put<uint8>( Op_PyFalse );

    return true;
}

bool MarshalStream::VisitReal( const PyFloat* rep )
{
    if ( rep->value() == 0.0 ) {
        Put<uint8>( Op_PyZeroReal );
    } else {
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
    size_t size = buf.size();

    // 检查缓冲区大小是否超过限制
    if (size > static_cast<size_t>(UINT32_MAX)) {
        sLog.Error("MarshalStream", "Buffer size %zu exceeds maximum allowed value", size);
        return false;
    }

    PutSizeEx(static_cast<uint32>(size));
    Put(buf.begin<uint8>(), buf.end<uint8>());

    return true;
}

bool MarshalStream::VisitString( const PyString* rep )
{
    size_t len = rep->content().size();
    
    if (len == 0) {
        Put<uint8>(Op_PyEmptyString);
    } else if (len == 1) {
        Put<uint8>(Op_PyCharString);
        Put<uint8>(rep->content()[0]);
    } else {
        // 检查字符串长度是否超过限制
        if (len > static_cast<size_t>(UINT32_MAX)) {
            sLog.Error("MarshalStream", "String length %zu exceeds maximum allowed value", len);
            return false;
        }

        //string is long enough for a string table entry, check it.
        const uint8 index = sMarshalStringTable.LookupIndex(rep->content());
        if (index > STRING_TABLE_ERROR) {
            Put<uint8>(Op_PyStringTableItem);
            Put<uint8>(index);
        } else {
            Put<uint8>(Op_PyLongString);
            PutSizeEx(static_cast<uint32>(len));
            Put(rep->content().begin(), rep->content().end());
        }
    }

    return true;
}

bool MarshalStream::VisitWString( const PyWString* rep )
{
    size_t len = rep->content().size();

    if (len == 0) {
        Put<uint8>(Op_PyEmptyWString);
    } else {
        // 检查字符串长度是否超过限制
        if (len > static_cast<size_t>(UINT32_MAX)) {
            sLog.Error("MarshalStream", "WString length %zu exceeds maximum allowed value", len);
            return false;
        }

        // We don't have to consider any conversions because
        // UTF-8 is more space-efficient than UCS-2.
        Put<uint8>(Op_PyWStringUTF8);
        PutSizeEx(static_cast<uint32>(len));
        Put(rep->content().begin(), rep->content().end());
    }

    return true;
}

bool MarshalStream::VisitToken( const PyToken* rep )
{
    Put<uint8>( Op_PyToken );

    const std::string& str = rep->content();
    size_t size = str.size();

    // 检查字符串长度是否超过限制
    if (size > static_cast<size_t>(UINT32_MAX)) {
        sLog.Error("MarshalStream", "Token string length %zu exceeds maximum allowed value", size);
        return false;
    }

    PutSizeEx(static_cast<uint32>(size));
    Put(str.begin(), str.end());

    return true;
}

bool MarshalStream::VisitTuple( const PyTuple* rep )
{
    size_t size = rep->size();
    if (size > static_cast<size_t>(UINT32_MAX)) {
        sLog.Error("MarshalStream", "Tuple size %zu exceeds maximum allowed value", size);
        return false;
    }
    
    uint32 usize = static_cast<uint32>(size);
    if (usize == 0) {
        Put<uint8>( Op_PyEmptyTuple );
    } else if (usize == 1) {
        Put<uint8>( Op_PyOneTuple );
    } else if (usize == 2) {
        Put<uint8>( Op_PyTwoTuple );
    } else {
        Put<uint8>( Op_PyTuple );
        PutSizeEx(usize);
    }

    return PyVisitor::VisitTuple( rep );
}

bool MarshalStream::VisitList( const PyList* rep )
{
    size_t size = rep->size();
    if (size > static_cast<size_t>(UINT32_MAX)) {
        sLog.Error("MarshalStream", "List size %zu exceeds maximum allowed value", size);
        return false;
    }
    
    uint32 usize = static_cast<uint32>(size);
    if (usize == 0) {
        Put<uint8>( Op_PyEmptyList );
    } else if (usize == 1) {
        Put<uint8>( Op_PyOneList );
    } else {
        Put<uint8>( Op_PyList );
        PutSizeEx(usize);
    }

    return PyVisitor::VisitList( rep );
}

bool MarshalStream::VisitDict( const PyDict* rep )
{
    size_t size = rep->size();
    if (size > static_cast<size_t>(UINT32_MAX)) {
        sLog.Error("MarshalStream", "Dict size %zu exceeds maximum allowed value", size);
        return false;
    }
    
    Put<uint8>( Op_PyDict );
    PutSizeEx(static_cast<uint32>(size));

    //we have to reverse the order of key/value to be value/key, so do not call base class.
    PyDict::const_iterator cur = rep->begin(), end = rep->end();
    for (; cur != end; ++cur) {
        if ( !cur->second->visit( *this ) )
            return false;
        if ( !cur->first->visit( *this ) )
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
    if (rep->isType2())
        Put<uint8>( Op_PyObjectEx2 );
    else
        Put<uint8>( Op_PyObjectEx1 );

    if ( !rep->header()->visit( *this ) )
        return false;

    PyList::const_iterator lItr = rep->list().begin(), lEnd = rep->list().end();
    for (; lItr != lEnd; ++lItr ) {
        if ( !(*lItr )->visit( *this ) )
            return false;
    }

    Put<uint8>( Op_PackedTerminator );

    PyDict::const_iterator dItr = rep->dict().begin(), dEnd = rep->dict().end();
    for (; dItr != dEnd; ++dItr ) {
        if ( !dItr->first->visit( *this ) )
            return false;
        if ( !dItr->second->visit( *this ) )
            return false;
    }

    Put<uint8>( Op_PackedTerminator );

    return true;
}

bool MarshalStream::VisitPackedRow( const PyPackedRow* pyPackedRow )
{
    Put<uint8>( Op_PyPackedRow );

    DBRowDescriptor* header(pyPackedRow->header());
    header->visit( *this );

    // create the sizemap and sort it by bitsize, the value of the map indicates the index of the column
   // this can be used to identify things easily
    std::multimap< uint8, uint32, std::greater< uint8 > > sizeMap;
    std::map<uint8,uint8> booleanColumns;

    uint32 columnCount = header->ColumnCount();
    uint8 booleansBitLength = 0;
    size_t byteDataBitLength = 0;
    size_t nullsBitLength = 0;

    // go through all the columns to gather the required information
    for (uint32_t i = 0; i < columnCount; i++)
    {
        DBTYPE columnType = header->GetColumnType(i);
        uint8_t size = DBTYPE_GetSizeBits(columnType);

        // count booleans
        if (columnType == DBTYPE_BOOL)
        {
            // 检查是否超出 uint8 范围
            if (booleansBitLength == UINT8_MAX) {
                sLog.Error("MarshalStream", "Boolean columns count exceeds maximum allowed value");
                return false;
            }
            
            // register the boolean in the list and increase the length
            booleanColumns.insert(std::make_pair(static_cast<uint8>(i), booleansBitLength));
            booleansBitLength++;
        }

        // also count all columns as possible nulls
        nullsBitLength++;

        // increase the bytedata length only if a column is longer than 7 bits
        // this is used as an indicator of what is written in the first second, or third part
        if (size >= 8)
            byteDataBitLength += size;

        // add the column to the list
        sizeMap.insert(std::make_pair(size, i));
    }

    // reserve the space for the buffers
    Buffer rowData;
    rowData.Reserve<uint8> ((byteDataBitLength >> 3) + ((booleansBitLength + nullsBitLength) >> 3) + 1);
    // to ease working with the bit data, reserve a fixed-size buffer
    // and fill it with 0s
    Buffer bitData(((booleansBitLength + nullsBitLength) >> 3) + 1, 0);

    std::multimap< uint8, uint32, std::greater< uint8 > >::iterator cur, end;
    cur = sizeMap.begin();
    // limit the search to booleans, the rest of the values are encoded differently
    end = sizeMap.lower_bound( 1 );
    PyRep* value(nullptr);
    for (; cur != end; ++cur)
    {
        value = pyPackedRow->GetField(cur->second);

        // handle the column being none
        if (value->IsNone() == true)
        {
            // get the bit this column should be written at
            unsigned long nullBit = cur->second + booleansBitLength;
            unsigned long nullByte = nullBit >> 3;
            // setup the iterator to the proper byte
            Buffer::iterator<uint8> bitIterator = bitData.begin<uint8>() + nullByte;
            // update the proper bit
            *bitIterator |= (1 << (nullBit & 0x7));
        }

        // ensure that the proper value is written
        // the values will be ignored if a none flag is set, but they must be present
        switch (header->GetColumnType (cur->second))
        {
            case DBTYPE_CY:
            case DBTYPE_I8:
            case DBTYPE_UI8:
            case DBTYPE_FILETIME:
                rowData.Append<int64>(value->IsNone() ? 0 : value->AsLong()->value() );
                break;
            case DBTYPE_I4:
                rowData.Append<int32>(value->IsNone() ? 0 : value->AsInt()->value() );
                break;
            case DBTYPE_UI4:
                rowData.Append<uint32>(value->IsNone() ? 0 : value->AsInt()->value() );
                break;
            case DBTYPE_I2:
                rowData.Append<int16>(value->IsNone() ? 0 : value->AsInt()->value() );
                break;
            case DBTYPE_UI2:
                rowData.Append<uint16>(value->IsNone() ? 0 : value->AsInt()->value() );
                break;
            case DBTYPE_I1:
                rowData.Append<int8>(value->IsNone() ? 0 : value->AsInt()->value() );
                break;
            case DBTYPE_UI1:
                rowData.Append<uint8>(value->IsNone() ? 0 : value->AsInt()->value() );
                break;
            case DBTYPE_R8:
                rowData.Append<double>(value->IsNone() ? 0.0 : value->AsFloat()->value() );
                break;
            case DBTYPE_R4:
                rowData.Append<float>(static_cast<float>(value->IsNone() ? 0.0f : value->AsFloat()->value()));
                break;
            // FIXME nothing should hit here ever but better implement some error-handling just in case
            default:
                assert( false );
                EvE::traceStack();
                break;
        }
    }

    cur = sizeMap.lower_bound( 1 );
    end = sizeMap.lower_bound( 0 );
    PyBool* b(nullptr);
    for (; cur != end; ++cur)
    {
        b = pyPackedRow->GetField(cur->second)->AsBool();

        // false values do not need anything to be done
        if (b->value() == false)
            continue;

        // get the bit this boolean should be written at
        unsigned long boolBit = booleanColumns.find (cur->second)->second;
        unsigned long boolByte = boolBit >> 3;
        // setup the iterator to the proper byte
        Buffer::iterator<uint8> bitIterator = bitData.begin<uint8>() + boolByte;
        // update the proper bit
        *bitIterator |= (1 << (boolBit & 0x7));
    }

    // concatenate the bit data to the rowData
    rowData.AppendSeq(bitData.begin<uint8>(), bitData.end<uint8>());

    // run the data through the zero compression algorithm
    if (!SaveRLE(rowData))
        return false;

    // finally append items that are not packed like strings or byte buffers
    cur = sizeMap.lower_bound( 0 );
    end = sizeMap.end();
    for (; cur != end; ++cur) {
        value = pyPackedRow->GetField(cur->second );
        if (!value->visit(*this))
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
    if (rep->data() == nullptr) {
        if (rep->decoded() == nullptr) {
            Put<uint8>(0);
            return false;
        }

        //unmarshaled stream
        //we have to marshal the substream.
        rep->EncodeData();
        if (rep->data() == nullptr) {
            Put<uint8>(0);
            return false;
        }
    }

    //we have the marshaled data, use it.
    const Buffer& data = rep->data()->content();
    size_t size = data.size();

    // 检查数据大小是否超过限制
    if (size > static_cast<size_t>(UINT32_MAX)) {
        sLog.Error("MarshalStream", "SubStream data size %zu exceeds maximum allowed value", size);
        return false;
    }

    PutSizeEx(static_cast<uint32>(size));
    Put(data.begin<uint8>(), data.end<uint8>());

    return true;
}

//! TODO: check the implementation of this...
// we should never visit a checksummed stream... NEVER...
bool MarshalStream::VisitChecksumedStream( const PyChecksumedStream* rep )
{
    assert(false && "MarshalStream on the server side should never send checksummed objects");

    Put<uint8>(Op_PyChecksumedStream);

    Put<uint32>( rep->checksum() );
    return PyVisitor::VisitChecksumedStream( rep );
}

void MarshalStream::SaveVarInteger( const PyLong* v )
{
    const int64 value(v->value());
    uint8 integerSize(0);

#define DoIntegerSizeCheck(x) if ( ( (uint8*)&value )[x] != 0 ) integerSize = x + 1;
    DoIntegerSizeCheck(4);
    DoIntegerSizeCheck(5);
    DoIntegerSizeCheck(6);
#undef  DoIntegerSizeCheck

    if ( integerSize > 0 && integerSize < 7 ) {
        Put<uint8>(Op_PyVarInteger);
        PutSizeEx(integerSize);
        Put( &( (uint8*)&value )[0], &( (uint8*)&value )[integerSize] );
    } else {
        Put<uint8>(Op_PyLongLong);                    // 1
        Put<int64>(value);                           // 8
    }
}

bool MarshalStream::SaveRLE(const Buffer& in)
{
    size_t in_size = in.size();
    // TODO: REWRITE THIS, AS IT IS RIGHT NOW IS INEFFICIENT, I'VE CONVERTED THE BUFFER CLASS TO A BASTARDIZED VERSION OF A NORMAL BYTE ARRAY
    // ALMAMU - 2021/04/22 - After many years the buggy "SaveZeroCompressed" function has been laid to rest
    //                       may this todo be a way to remind us how unstable and fragile the EVEmu core is
    //                       this code has been validated against the disassembly of Apocrypha's blue.dll
    //                       for those interested, the function lives in .text:10082D10 of that DLL
    //                       "hopefully" this brings our marshaller closer to fully featured

    // reserve double the buffer size just in case, we do not want to run out of space or else the iterators will start to complain
    Buffer out(in_size * 2, 0);

    // this code has been used and ported through different projects
    // both ntt's reverence and Captnoord's re-implementation of evemu core have the exact same code
    // after validation against the disassembly (and unless I've missed anything) it seems to be 100% accurate
    // so no real modification to it was done
    int nibble = 0;
    int nibble_ix = 0;
    int in_ix = 0;
    int out_ix = 0;
    int start, end, count;
    int zerochains = 0;
    size_t buffer_size = in_size;

    while(in_ix < static_cast<int>(buffer_size))
    {
        if(!nibble)
        {
            nibble_ix = out_ix++;
            out[nibble_ix] = 0;
        }

        start = in_ix;
        end = std::min(in_ix + 8, static_cast<int>(buffer_size));

        if(in[in_ix])
        {
            zerochains = 0;
            do {
                out[out_ix++] = in[in_ix++];
            } while(in_ix<end && in[in_ix]);
            count = (start - in_ix) + 8;
        }
        else
        {
            zerochains++;
            while(in_ix<end && !in[in_ix])
                in_ix++;
            count = (in_ix - start) + 7;
        }

        if(nibble)
            out[nibble_ix] |= (count << 4);
        else
            out[nibble_ix] = count;
        nibble = !nibble;
    }

    if(nibble && zerochains)
        zerochains++;

    while(zerochains>1)
    {
        zerochains -= 2;
        out_ix -= 1;
    }

    // Write the packed in
    if (out_ix < 0 || static_cast<uint32>(out_ix) > UINT32_MAX) {
        sLog.Error("MarshalStream", "Packed size %d exceeds maximum allowed value", out_ix);
        return false;
    }
    
    PutSizeEx(static_cast<uint32>(out_ix));
    if (out_ix > 0)
        Put(out.begin<uint8>(), out.begin<uint8>() + out_ix);

    return true;
}
