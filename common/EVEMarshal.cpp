/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

using std::string;


class MarshalVisitor : public PyVisitor {
public:
	virtual ~MarshalVisitor() {}
	
	virtual void VisitInteger(const PyRepInteger *rep) {
//printf("int\n");
		//logic assumes value is unsigned
		if(rep->value > 0xFFFFFFFF) {
			PutByte(Op_PyLongLong);
			uint64 value = rep->value;
			//TODO: byte swap.
			PutBytes(&value, sizeof(value));
		} else if(rep->value > 0x7FFF) {
			PutByte(Op_PyLong);
			uint32 value = rep->value;
			//TODO: byte swap.
			PutBytes(&value, sizeof(value));
		} else if(rep->value > 1) {
			PutByte(Op_PySignedShort);
			uint16 value = rep->value;
			//TODO: byte swap.
			PutBytes(&value, sizeof(value));
/*      } else if(rep->value > 0xFF) {
			PutByte(Op_PyShort);
			uint16 value = rep->value;
			//TODO: byte swap.
			PutBytes(&value, sizeof(value));
		} else if(rep->value > 1) {
			PutByte(Op_PyByte);
			PutByte(rep->value);*/
		} else if(rep->value == 1) {
			PutByte(Op_PyOneInteger);
		} else if(rep->value == 0) {
			PutByte(Op_PyZeroInteger);
		}
	}
	
	virtual void VisitBoolean(const PyRepBoolean *rep) {
//printf("bool\n");
		if(rep->value)
			PutByte(Op_PyTrue);
		else
			PutByte(Op_PyFalse);
	}
	
	virtual void VisitReal(const PyRepReal *rep) {
//printf("real\n");
		if(rep->value == 0.0) {
			PutByte(Op_PyZeroReal);
		} else {
			PutByte(Op_PyReal);
			PutBytes(&rep->value, sizeof(rep->value));
		}
	}
	
	virtual void VisitNone(const PyRepNone *rep) {
//printf("none\n");
		PutByte(Op_PyNone);
	}
	
	virtual void VisitBuffer(const PyRepBuffer *rep) {
//printf("buffer\n");
		PutByte(Op_PyBuffer);
		if(rep->GetLength() >= 0xFF) {
			PutByte(0xFF);
			//todo: swap bytes?
			uint32 len = rep->GetLength();
			PutBytes(&len, sizeof(len));
		} else {
			PutByte(rep->GetLength());
		}
		PutBytes(rep->GetBuffer(), rep->GetLength());
	}
	
	virtual void VisitPackedRow(const PyRepPackedRow *rep) {
		PutByte(Op_PyPackedRow);
		
		rep->GetHeader()->visit(this);
		
		//pack the bytes with the zero compression algorithm.
		std::vector<byte> packed;
		PackZeroCompressed(rep->GetBuffer(), rep->GetBufferSize(), packed);
		
		if(packed.size() >= 0xFF) {
			PutByte(0xFF);
			//todo: swap bytes
			uint32 len = (uint32)packed.size();
			PutUint32(len);
		} else {
			PutByte((byte)packed.size());
		}
		if(!packed.empty())
			//out goes the data...
			PutBytes(&packed[0], (uint32)packed.size());

		//PyReps follow packed data
		PyRepPackedRow::rep_list::const_iterator cur, end;
		cur = rep->begin();
		end = rep->end();
		for(; cur != end; cur++)
			(*cur)->visit(this);
	}
	
	virtual void VisitString(const PyRepString *rep) {
		uint32 len = (uint32)rep->value.length();
//printf("string\n");
		if(rep->is_type_1) {
			if(len < 0xFF) {
				PutByte(Op_PyByteString);
				PutByte(len);
				PutBytes(rep->value.c_str(), len);
			} else {
				//almost certain this isnt supported.
				_log(NET__MARSHAL_ERROR, "Error: Length extension on Type1 string is untested, and prolly wont work.");
				PutByte(Op_PyByteString);
				PutByte(0xFF);
				PutBytes(&len, sizeof(len));
				PutBytes(rep->value.c_str(), len);
			}
		} else {
			if(len == 0) {
				PutByte(Op_PyEmptyString);
			} else if(len == 1) {
				PutByte(Op_PyCharString);
				PutByte(rep->value.c_str()[0]);
			} else {
				//string is long enough for a string table entry, check it.
				uint8 sid = PyRepString::GetStringTable()->LookupString(rep->value);
				if(sid != EVEStringTable::None) {
					_log(NET__MARSHAL_TRACE, "Replaced string '%s' with string table index %d", rep->value.c_str(), sid);
					//we have a string table entry, send that out:
					PutByte(Op_PyStringTableItem);
					PutByte(sid);
				} else if(len < 0xFF) {
					PutByte(Op_PyByteString2);
					PutByte(len);
					PutBytes(rep->value.c_str(), len);
				} else {
					//well, this type does not seem to support a length
					//extension, so im hacking it to a buffer for now. We should
					//prolly use a unicode string in the future
					PutByte(Op_PyUnicodeString);
					PutByte(0xFF);
					PutBytes(&len, sizeof(len));
					PutBytes(rep->value.c_str(), len);
				}
			}
		}
	}
	
	virtual void VisitObject(const PyRepObject *rep) {
//printf("object\n");
		PutByte(Op_PyObject);
		//throw out the type string
		PyRepString s(rep->type);
		s.visit(this);
		//this will visit arguments
		PyVisitor::VisitObject(rep);
	}

	virtual void VisitPackedObjectList(const PyRepPackedObject *rep) {
		PyVisitor::VisitPackedObjectList(rep);
		PutByte(Op_PackedTerminator);
	}

	virtual void VisitPackedObjectDict(const PyRepPackedObject *rep) {
		PyVisitor::VisitPackedObjectDict(rep);
		PutByte(Op_PackedTerminator);
	}

	virtual void VisitPackedObject1(const PyRepPackedObject1 *rep) {
		PutByte(Op_PackedObject1);
		//this is little hackish, but we dont have to clone whole contents
		if(rep->keywords.empty())
			PutByte(Op_PyTwoTuple);
		else {
			PutByte(Op_PyTuple);
			PutByte(3);
		}
		PyRepString s(rep->type, true);
		s.visit(this);
		
		if(rep->args == NULL)
			PutByte(Op_PyEmptyTuple);
		else
			rep->args->visit(this);

		//if not empty, insert keywords
		if(!rep->keywords.empty())
			rep->keywords.visit(this);

		VisitPackedObject(rep);
	}

	virtual void VisitPackedObject2(const PyRepPackedObject2 *rep) {
		PutByte(Op_PackedObject2);
		//this is little hackish, but we don't have to clone whole contents
		if(rep->args2 == NULL)
			PutByte(Op_PyOneTuple);
		else
			PutByte(Op_PyTwoTuple);

		if(rep->args1 == NULL)
			PutByte(Op_PyOneTuple);
		else if(rep->args1->items.size() == 1)
			PutByte(Op_PyTwoTuple);
		else {
			PutByte(Op_PyTuple);
			PutByte(1 + (byte)rep->args1->items.size()); // possible overload (size is 32 bits and your sending a byte)
		}

		PyRepString s(rep->type, true);
		s.visit(this);

		if(rep->args1 != NULL)
			rep->args1->visit(this);

		if(rep->args2 != NULL)
			rep->args2->visit(this);

		VisitPackedObject(rep);
	}
	
	virtual void VisitSubStruct(const PyRepSubStruct *rep) {
//printf("struct\n");
		PutByte(Op_PySubStruct);
		PyVisitor::VisitSubStruct(rep);
	}
	
	virtual void VisitSubStream(const PyRepSubStream *rep) {
//printf("stream\n");
		PutByte(Op_PySubStream);
		
		if(rep->length == 0 || rep->data == NULL) {
			if(rep->decoded == NULL) {
				PutByte(0);
				_log(NET__MARSHAL_ERROR, "Error: substream with no data or rep being marshaled!");
				return;
			}
			
			//unmarshaled stream
			//we have to marshal the substream first to get its length.
			MarshalVisitor v;
			v.PutByte(SubStreamHeaderByte);
			
			uint32 unknown = 0;
			v.PutBytes(&unknown, sizeof(unknown));
			
			rep->decoded->visit(&v);
			
			uint32 length = (uint32)v.m_result.size();
			
			if(length >= 0xFF) {
				PutByte(0xFF);
				PutBytes(&length, sizeof(length));
			} else {
				PutByte(length);
			}

			m_result.insert(m_result.end(), v.m_result.begin(), v.m_result.end());
		} else {
			//else, we have the marshaled data already, use it.
			if(rep->length >= 0xFF) {
				PutByte(0xFF);
				uint32 len = rep->length;
				PutBytes(&len, sizeof(len));
			} else {
				PutByte(rep->length);
			}
			
			PutBytes(rep->data, rep->length);
		}
	}
	
	virtual void VisitChecksumedStream(const PyRepChecksumedStream *rep) {
//printf("checksumed\n");
		PutByte(Op_PyChecksumedStream);
		//TODO: swap bytes
		uint32 sum = 0;
		PutBytes(&sum, sizeof(sum));
		PyVisitor::VisitChecksumedStream(rep);
	}
	
	virtual void VisitDict(const PyRepDict *rep) {
//printf("dict\n");
		uint32 size = (uint32)rep->items.size();
		if(size >= 0xFF) {
			PutByte(Op_PyDict);
			PutByte(0xFF);
			PutBytes(&size, sizeof(size));
		} else {
			PutByte(Op_PyDict);
			PutByte(size);
		}
		PyVisitor::VisitDict(rep);
	}
	virtual void VisitDictElement(const PyRepDict *rep, uint32 index, const PyRep *key, const PyRep *value) {
//printf("dicte\n");
		//we have to reverse the order of key/value to be value/key, so do not call base class.
		value->visit(this);
		key->visit(this);
	}
	
	virtual void VisitList(const PyRepList *rep) {
//printf("list\n");
		uint32 size = (uint32)rep->items.size();
		if(size >= 0xFF) {
			PutByte(Op_PyList);
			PutByte(0xFF);
			PutBytes(&size, sizeof(size));
		} else if(size > 1) {
			PutByte(Op_PyList);
			PutByte(size);
		} else if(size == 1) {
			PutByte(Op_PyOneList);
		} else if(size == 0) {
			PutByte(Op_PyEmptyList);
		}
		PyVisitor::VisitList(rep);
	}
	
	virtual void VisitTuple(const PyRepTuple *rep) {
//printf("tuple\n");
		uint32 size = (uint32)rep->items.size();
		if(size >= 0xFF) {
			PutByte(Op_PyTuple);
			PutByte(0xFF);
			PutBytes(&size, sizeof(size));
		} else if(size > 2) {
			PutByte(Op_PyTuple);
			PutByte(size);
		} else if(size == 2) {
			PutByte(Op_PyTwoTuple);
		} else if(size == 1) {
			PutByte(Op_PyOneTuple);
		} else if(size == 0) {
			PutByte(Op_PyEmptyTuple);
		}
		PyVisitor::VisitTuple(rep);
	}
	
	//TODO: optimize this, it could be done so much better than a vector.
	inline void PutByte(byte b) {
//printf("Byte 0x%02x\n", b);
		m_result.push_back(b);
	}
	inline void PutBytes(const void *v, uint32 len) {
		const byte *b = (const byte *) v;
		while(len > 0) {
//printf("Byte 0x%02x -\n", *b);
			m_result.push_back(*b);
			b++;
			len--;
		}
	}

	inline void PutUint32(uint32 data) {
		PutBytes((void*)&data,sizeof(uint32));
	}
	
	std::vector<byte> m_result;
};

//returns ownership of the buffer
byte *Marshal(const PyRep *rep, uint32 &len, bool inlineSubStream) {
	MarshalVisitor v;

	if(inlineSubStream) {
		v.PutByte(SubStreamHeaderByte);
		v.PutByte(0);		//not sure what these zeros are about right now.
		v.PutByte(0);
		v.PutByte(0);
		v.PutByte(0);
	}

	rep->visit(&v);

	len = (uint32)v.m_result.size();
	byte *b = new byte[len];

	memcpy(b, &v.m_result[0], len);

	return(b);
}





