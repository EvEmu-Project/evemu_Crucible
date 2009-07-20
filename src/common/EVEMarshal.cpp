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
	Authors:	BloodyRabbit, Captnoord, Zhur
*/

#include <stdio.h>
#include <string>
#include <zlib.h>
#include <assert.h>

#include "common.h"
#include "packet_dump.h"
#include "packet_functions.h"
#include "logsys.h"

#include "PyRep.h"
#include "PyVisitor.h"
#include "EVEMarshalOpcodes.h"
#include "EVEUtils.h"
#include "EVEZeroCompress.h"

static const uint32 EVEDeflationBytesLimit = 10000;	//every packet larger than this is deflated

class MarshalVisitor : public PyVisitor
{
public:
	MarshalVisitor() : mWriteIndex(0), mAllocatedMem( 0x1000 )
	{
		mBuffer = (uint8*)malloc( mAllocatedMem );
	}
	virtual ~MarshalVisitor()
	{
		free( mBuffer );
	}
	
	/** adds a integer to the data stream
	  *  Note:	assuming the value is unsigned
	  *			research shows that Op_PyByte can be negative
	  */
	virtual void VisitInteger(const PyRepInteger *rep)
	{
		const uint64 & val = rep->value;
		if (val == -1)
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

	/** Adds a boolean to the stream
	  */
	virtual void VisitBoolean(const PyRepBoolean *rep)
	{
		if(rep->value == true)
			PutByte(Op_PyTrue);
		else
			PutByte(Op_PyFalse);
	}
	
	/** Adds a double to the stream
	  */
	virtual void VisitReal(const PyRepReal *rep)
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
	
	/** Adds a None object to the stream
	  */
	virtual void VisitNone(const PyRepNone *rep)
	{
		PutByte(Op_PyNone);
	}

	/** Adds a buffer to the stream
	  */
	virtual void VisitBuffer(const PyRepBuffer *rep)
	{
		PutByte(Op_PyBuffer);
		uint32 len = rep->GetLength();
        PutSizeEx(len);
		PutBytes(rep->GetBuffer(), len);
	}
	
	/** Adds a packed row to the stream
	  */
	virtual void VisitPackedRow(const PyRepPackedRow *rep)
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
	
	/** add a string object to the data stream
	  */
	virtual void VisitString(const PyRepString *rep)
	{
		uint32 len = (uint32)rep->value.length();
		if(rep->is_type_1)
		{
			PutByte(Op_PyByteString);
            PutSizeEx(len);
            PutBytes(rep->value.c_str(), len);
            return;
		}
		else
		{
			if(len == 0)
			{
				PutByte(Op_PyEmptyString);
				return;
			}
			else if(len == 1)
			{
				PutByte(Op_PyCharString);
				PutByte(rep->value.c_str()[0]);
				return;
			}
			else
			{
				//string is long enough for a string table entry, check it.
				uint8 sid = PyRepString::GetStringTable()->LookupString(rep->value);
				if(sid != EVEStringTable::None)
				{
					//_log(NET__MARSHAL_TRACE, "Replaced string '%s' with string table index %d", rep->value.c_str(), sid);
					//we have a string table entry, send that out:
					PutByte(Op_PyStringTableItem);
					PutByte(sid);
					return;
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
	
	/** Adds a object to the stream
	  */
	virtual void VisitObject(const PyRepObject *rep)
	{
		PutByte(Op_PyObject);
		//throw out the type string
		PyRepString s(rep->type);
		s.visit(this);
		//this will visit arguments
		PyVisitor::VisitObject(rep);
	}

	/** Adds a New object to the stream
	  */
	virtual void VisitObjectEx(const PyRepObjectEx *rep)
	{
		if (rep->is_type_1 == true)
			PutByte(Op_ObjectEx2);
		else
			PutByte(Op_ObjectEx1);

		PyVisitor::VisitObjectEx(rep);
	}

	/** Adds a new object list
	  */
	virtual void VisitObjectExList(const PyRepObjectEx *rep)
	{
		PyVisitor::VisitObjectExList(rep);
		PutByte(Op_PackedTerminator);
	}

	/** Adds a new object dict
	  */
	virtual void VisitObjectExDict(const PyRepObjectEx *rep)
	{
		PyVisitor::VisitObjectExDict(rep);
		PutByte(Op_PackedTerminator);
	}
	
	/** Adds a sub structure to the stream
	  */
	virtual void VisitSubStruct(const PyRepSubStruct *rep)
	{
		PutByte(Op_PySubStruct);
		PyVisitor::VisitSubStruct(rep);
	}
	
	/** Adds a sub stream to the stream
	  */
	virtual void VisitSubStream(const PyRepSubStream *rep)
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
	
	// we should never visit a checksummed stream... NEVER...
	virtual void VisitChecksumedStream(const PyRepChecksumedStream *rep)
	{
		assert(false && "MarshalStream on the server size should never send checksummed objects");
		PutByte(Op_PyChecksumedStream);
		uint32 sum = 0;
		PutUint32(sum);
		PyVisitor::VisitChecksumedStream(rep);
	}
	
	/** Add dict content to the stream
	  */
	virtual void VisitDict(const PyRepDict *rep)
	{
		uint32 size = (uint32)rep->items.size();
        PutByte(Op_PyDict);
        PutSizeEx(size);
		PyVisitor::VisitDict(rep);
	}

	/** Add dict elements to the stream
	 */
	virtual void VisitDictElement(const PyRepDict *rep, uint32 index, const PyRep *key, const PyRep *value)
	{
		//we have to reverse the order of key/value to be value/key, so do not call base class.
		value->visit(this);
		key->visit(this);
	}
	
	/** Add a list object to the stream
	 */
	virtual void VisitList(const PyRepList *rep)
	{
		uint32 size = rep->size();
        if(size > 1)
		{
			PutByte(Op_PyList);
			PutSizeEx(size);
			PyVisitor::VisitList(rep);
			return;
		}
		else if(size == 1)
		{
			PutByte(Op_PyOneList);
			PyVisitor::VisitList(rep);
			return;
		}
		else if(size == 0)
		{
			PutByte(Op_PyEmptyList);
		}
	}
	
	/** Add tuple to the stream
	 */
	virtual void VisitTuple(const PyRepTuple *tuple)
	{
		uint32 size = tuple->size();
		if(size > 2)
		{
			PutByte(Op_PyTuple);
			PutSizeEx(size);
			PyVisitor::VisitTuple(tuple);
			return;
		}
		else if(size == 2)
		{
			PutByte(Op_PyTwoTuple);
			PyVisitor::VisitTuple(tuple);
			return;
		}
		else if(size == 1)
		{
			PutByte(Op_PyOneTuple);
			PyVisitor::VisitTuple(tuple);
			return;
		}
		else if(size == 0)
		{
			PutByte(Op_PyEmptyTuple);
			return;
		}
	}

private:
	// utility to handle Op_PyVarInteger (a bit hacky......)
	EVEMU_INLINE void _PyRepInteger_AsByteArray(const PyRepInteger* v)
	{
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
			return;
		}
		else
		{
			PutByte(Op_PyLongLong);						// 1
			PutUint64(v->value);						// 8
			return;
		}
	}

	// not very efficient but it will do for now
	EVEMU_INLINE void reserve(uint32 size)
	{
        uint32 neededMem = mWriteIndex + size;
        if (neededMem > mAllocatedMem)
        {
            mBuffer = (uint8*)realloc(mBuffer, neededMem + 0x1000);
            mAllocatedMem = neededMem + 0x1000;
        }
	}
	
public:

	EVEMU_INLINE void PutByte(uint8 b)
	{
		reserve(1);
		mBuffer[mWriteIndex] = b;
		mWriteIndex++;
	}

    EVEMU_INLINE void PutSizeEx(uint32 size)
    {
        if (size < 0xFF)
        {
            reserve(1);
            mBuffer[mWriteIndex] = (uint8)size;
            mWriteIndex++;
        }
        else
        {
            reserve(5);
            mBuffer[mWriteIndex++] = 0xFF;
            (*((uint32*)&mBuffer[mWriteIndex])) = size;
            mWriteIndex+=4;
        }
    }

	EVEMU_INLINE void PutBytes(const void *v, uint32 len)
	{
		reserve( len );
        memcpy( &mBuffer[mWriteIndex], v, len );
        mWriteIndex += len;
	}

	/** adds a double do the data stream
	*/
	EVEMU_INLINE void PutDouble(const double& value)
	{
		reserve(8);
        (*((double*)&mBuffer[mWriteIndex])) = value;
        mWriteIndex+=8;
	}

	/**	adds a uint64 do the data stream
	*/
	EVEMU_INLINE void PutUint64(const uint64& value)
	{
		reserve(8);
        (*((uint64*)&mBuffer[mWriteIndex])) = value;
        mWriteIndex+=8;
	}

	/**	adds a uint32 do the data stream
	*/
	EVEMU_INLINE void PutUint32(const uint32 value)
	{
		reserve(4);
        (*((uint32*)&mBuffer[mWriteIndex])) = value;
        mWriteIndex+=4;
	}

	/**	adds a uint16 do the data stream
	  */
	EVEMU_INLINE void PutUint16(const uint16 value)
	{
		reserve(2);
        (*((uint16*)&mBuffer[mWriteIndex])) = value;
        mWriteIndex+=2;
	}

	/**	adds a uint8 do the data stream
	  */
	EVEMU_INLINE void PutUint8(uint8 value)
	{
		reserve(1);
		mBuffer[mWriteIndex] = value; mWriteIndex++;
	}

	uint32 size()
	{
		return mWriteIndex;
	}

	uint8* data()
	{
		return mBuffer;
	}

protected:

	uint32				mAllocatedMem;
	uint32				mWriteIndex;
	uint8				*mBuffer;
};

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
        free( packetBuff );

	return b;
}
