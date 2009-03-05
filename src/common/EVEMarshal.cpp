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

#include "common.h"
#include "PyRep.h"
#include "packet_dump.h"
#include "packet_functions.h"
#include "EVEMarshalOpcodes.h"
#include "EVEZeroCompress.h"
#include "PyVisitor.h"
#include "logsys.h"
#include <stdio.h>
#include <string>
#include <zlib.h>
#include <assert.h>

static const uint32 EVEDeflationBytesLimit = 10000;	//every packet larger than this is deflated

class MarshalVisitor : public PyVisitor
{
public:
	MarshalVisitor() : mWriteIndex(0)
	{
		mBuffer = (uint8*)malloc(0x100000);
		mAllocatedMem = 0x100000;
	}
	virtual ~MarshalVisitor()
	{
		free(mBuffer);
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
			return;
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
		if(len >= 0xFF)
		{
			PutByte(0xFF);
			PutUint32(len);
		}
		else
		{
			PutByte(len);
		}
		PutBytes(rep->GetBuffer(), len);
	}
	
	/** Adds a packed row to the stream
	  */
	virtual void VisitPackedRow(const PyRepPackedRow *rep)
	{
		PutByte(Op_PyPackedRow);
		
		rep->header->visit(this);
		
		//pack the bytes with the zero compression algorithm.
		std::vector<uint8> packed;
		PackZeroCompressed(rep->buffer(), rep->bufferSize(), packed);
		
		uint32 len = (uint32)packed.size();
		if(len >= 0xFF)
		{
			PutByte(0xFF);
			PutUint32(len);
		}
		else
		{
			PutByte(len);
		}

		if(!packed.empty())
		{
			//out goes the data...
			PutBytes(&packed[0], len);
		}

		//PyReps follow packed data
		PyRepPackedRow::rep_list::const_iterator cur, end;
		cur = rep->begin();
		end = rep->end();
		for(; cur != end; cur++)
		{
			(*cur)->visit(this);
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
			if(len < 0xFF)
			{
				PutByte(len);
				PutBytes(rep->value.c_str(), len);
				return;
			}
			else
			{
				/* large string support
				 * old comment: almost certain this isn't supported.
				 * Note: when I check the asm i'm almost certain its supported.				
				 */
				PutByte(0xFF);
				PutUint32(len);
				PutBytes(rep->value.c_str(), len);
				return;
			}
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
				else if(len < 0xFF)
				{
					PutByte(Op_PyByteString2);
					PutByte(len);
					PutBytes(rep->value.c_str(), len);
					return;
				}
				else
				{
					//well, this type does not seem to support a length
					//extension, so I'm hacking it to a buffer for now. We should
					//probably use a unicode string in the future
					PutByte(Op_PyUnicodeString);
					PutByte(0xFF);
					PutUint32(len);
					PutBytes(rep->value.c_str(), len);
					return;
				}
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
	virtual void VisitNewObject(const PyRepNewObject *rep)
	{
		if (rep->is_type_1 == true)
			PutByte(Op_NewObject2);
		else
			PutByte(Op_NewObject1);

		PyVisitor::VisitNewObject(rep);
	}

	/** Adds a new object list
	  */
	virtual void VisitNewObjectList(const PyRepNewObject *rep)
	{
		PyVisitor::VisitNewObjectList(rep);
		PutByte(Op_PackedTerminator);
	}

	/** Adds a new object dict
	  */
	virtual void VisitNewObjectDict(const PyRepNewObject *rep)
	{
		PyVisitor::VisitNewObjectDict(rep);
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
			if(length >= 0xFF)
			{
				PutByte(0xFF);
				PutUint32(length);
			}
			else
			{
				PutByte(length);
			}

			CheckSize(v.size());
			
			// Fast but many coders consider hacky methode
			memcpy(&mBuffer[mWriteIndex], v.data(), v.size());
			mWriteIndex+=v.size();

			// slow but considered to be safer ( why in the hack would this be safer?:S )
			//PutBytes(v.data(), v.size());
		}
		else
		{
			//else, we have the marshaled data already, use it.
			if(rep->length >= 0xFF)
			{
				PutByte(0xFF);
				PutUint32(rep->length);
			}
			else
			{
				PutByte(rep->length);
			}
			
			PutBytes(rep->data, rep->length);
		}
	}
	
	// we should never visit a checksummed stream... NEVER...
	virtual void VisitChecksumedStream(const PyRepChecksumedStream *rep)
	{
		assert(false && "MarshalStream on the server size should never send checksummed objects");
		PutByte(Op_PyChecksumedStream);
		uint32 sum = 0;
		PutBytes(&sum, sizeof(sum));
		PyVisitor::VisitChecksumedStream(rep);
	}
	
	/** Add dict content to the stream
	  */
	virtual void VisitDict(const PyRepDict *rep)
	{
		uint32 size = (uint32)rep->items.size();
		if(size >= 0xFF)
		{
			PutByte(Op_PyDict);
			PutByte(0xFF);
			PutUint32(size);
		}
		else
		{
			PutByte(Op_PyDict);
			PutByte(size);
		}
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
		uint32 size = (uint32)rep->items.size();
		if(size >= 0xFF)
		{
			PutByte(Op_PyList);
			PutByte(0xFF);
			PutUint32(size);
			PyVisitor::VisitList(rep);
			return;
		}
		else if(size > 1)
		{
			PutByte(Op_PyList);
			PutByte(size);
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
	virtual void VisitTuple(const PyRepTuple *rep)
	{
		uint32 size = (uint32)rep->items.size();
		if(size >= 0xFF)
		{
			PutByte(Op_PyTuple);
			PutByte(0xFF);
			PutBytes(&size, sizeof(size));
			PyVisitor::VisitTuple(rep);
			return;
		}
		else if(size > 2)
		{
			PutByte(Op_PyTuple);
			PutByte(size);
			PyVisitor::VisitTuple(rep);
			return;
		}
		else if(size == 2)
		{
			PutByte(Op_PyTwoTuple);
			PyVisitor::VisitTuple(rep);
			return;
		}
		else if(size == 1)
		{
			PutByte(Op_PyOneTuple);
			PyVisitor::VisitTuple(rep);
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
	ASCENT_INLINE void _PyRepInteger_AsByteArray(const PyRepInteger* v)
	{
		CheckSize(8);
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
	ASCENT_INLINE void CheckSize(uint32 size)
	{
		uint32 rSize = mAllocatedMem - mWriteIndex;
		if (rSize < size)
		{
			//printf("VisitBuff: 0x%p is incr size from: %u to %u | check size was: %u\n", this, mAllocatedMem, mAllocatedMem + 0x10000, size);
			
			// realloc on itself is quite slow..... but it will do for now..
			mBuffer = (uint8*)realloc(mBuffer, mAllocatedMem + 0x10000);
			mAllocatedMem += 0x10000;
		}
	}
	
public:

	ASCENT_INLINE void PutByte(uint8 b)
	{
		CheckSize(1);
		mBuffer[mWriteIndex] = b;
		mWriteIndex++;
	}
	ASCENT_INLINE void PutBytes(const void *v, uint32 len)
	{
		CheckSize(len);
		const uint8 *b = (const uint8 *) v;
		while(len > 0)
		{
			mBuffer[mWriteIndex] = *b;
			mWriteIndex++;
			b++;
			len--;
		}
	}

	ASCENT_INLINE void PutDouble(const double& value)
	{
		CheckSize(8);
		mBuffer[mWriteIndex] = (((uint8*)&value)[0]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[1]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[2]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[3]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[4]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[5]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[6]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[7]); mWriteIndex++;
	}

	ASCENT_INLINE void PutUint64(const uint64& value)
	{
		CheckSize(8);
		mBuffer[mWriteIndex] = (((uint8*)&value)[0]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[1]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[2]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[3]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[4]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[5]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[6]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[7]); mWriteIndex++;
	}

	/**	adds a uint64 do the data stream
	*/
	ASCENT_INLINE void PutUint64(uint64& value)
	{
		CheckSize(8);
		mBuffer[mWriteIndex] = (((uint8*)&value)[0]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[1]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[2]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[3]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[4]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[5]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[6]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[7]); mWriteIndex++;
	}

	/**	adds a uint32 do the data stream
	*/
	ASCENT_INLINE void PutUint32(uint32 value)
	{
		CheckSize(4);
		mBuffer[mWriteIndex] = (((uint8*)&value)[0]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[1]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[2]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[3]); mWriteIndex++;
	}

	/**	adds a uint16 do the data stream
	  */
	ASCENT_INLINE void PutUint16(uint16 value)
	{
		CheckSize(2);
		mBuffer[mWriteIndex] = (((uint8*)&value)[0]); mWriteIndex++;
		mBuffer[mWriteIndex] = (((uint8*)&value)[1]); mWriteIndex++;
	}

	/**	adds a uint8 do the data stream
	  */
	ASCENT_INLINE void PutUint8(uint8 value)
	{
		CheckSize(1);
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

	return b;
}
