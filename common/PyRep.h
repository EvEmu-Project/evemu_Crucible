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


#ifndef EVE_PY_REP_H
#define EVE_PY_REP_H

#include "common.h"
#include "logsys.h"

#include <vector>
#include <map>
#include <string>
#include <stdio.h>

class PyVisitor;

class PyRep {
public:
	typedef enum {
		Integer,
		Real,
		Boolean,
		Buffer,
		String,
		Tuple,
		List,
		Dict,
		None,
		SubStruct,
		SubStream,
		ChecksumedStream,
		Object,
		PackedRow,
		PackedObject1,
		PackedObject2
	} Type;
	
	PyRep(Type t) : m_type(t) { }
	virtual ~PyRep() {}

	bool CheckType(Type t) const { return(m_type == t); }
	bool CheckType(Type t, Type t2) const { return(m_type == t || m_type == t2); }
	const char *TypeString() const;
	
	virtual void Dump(FILE *into, const char *pfx) const = 0;
	virtual void Dump(LogType type, const char *pfx) const = 0;
	virtual PyRep *Clone() const = 0;
	virtual void visit(PyVisitor *v) const = 0;

	//using this method is discouraged, it generally means your doing something wrong... CheckType() should cover almost all needs
	Type youreallyshouldentbeusingthis_GetType() const { return(m_type); }
protected:
	const Type m_type;
};

//storing all integers (and bools) as uint64s is a lot of craziness right now
//but its better than a ton of virtual functions to achieve the same thing.
class PyRepInteger : public PyRep {
public:
	PyRepInteger(uint64 i) : PyRep(PyRep::Integer), value(i) {}
	virtual ~PyRepInteger() {}
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	PyRepInteger *TypedClone() const;
	
	uint64 value;
};

class PyRepReal : public PyRep {
public:
	PyRepReal(double i) : PyRep(PyRep::Real), value(i) {}
	virtual ~PyRepReal() {}
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	PyRepReal *TypedClone() const;
	
	double value;
};

class PyRepBoolean : public PyRep {
public:
	PyRepBoolean(bool i) : PyRep(PyRep::Boolean), value(i) {}
	virtual ~PyRepBoolean() {}
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	PyRepBoolean *TypedClone() const;
	
	bool value;
};

class PyRepNone : public PyRep {
public:
	PyRepNone() : PyRep(PyRep::None) {}
	virtual ~PyRepNone() {}
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	PyRepNone *TypedClone() const;
};

class PyRepSubStream;
class PyRepBuffer : public PyRep {
public:
	PyRepBuffer(const byte *buffer, uint32 length);
	PyRepBuffer(byte **buffer, uint32 length);
	PyRepBuffer(uint32 length);
	virtual ~PyRepBuffer();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	PyRepBuffer *TypedClone() const;
	
	//treat it as a buffer:
	byte *GetBuffer() const { return(m_value); }
	uint32 GetLength() const { return(m_length); }

	PyRepSubStream *CreateSubStream() const;
	
protected:
	byte *const m_value;
	const uint32 m_length;
};

class EVEStringTable;
class PyRepString : public PyRep {
public:
	PyRepString(const char *str = "", bool type_1=false) : PyRep(PyRep::String), value(str), is_type_1(type_1) {}
	PyRepString(const std::string &str, bool type_1=false) : PyRep(PyRep::String), is_type_1(type_1) { value.assign(str.c_str(), str.length()); } //to deal with non-string buffers poorly placed in strings (CCP)
	PyRepString(const byte *data, uint32 len, bool type_1=false) : PyRep(PyRep::String), value((const char *) data, len), is_type_1(type_1) {}
	virtual ~PyRepString() {}
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	PyRepString *TypedClone() const;

	std::string value;
	bool is_type_1;	//true if this is an Op_PyByteString instead of the default Op_PyByteString2


//string table stuff:
	static bool LoadStringFile(const char *file);
	static EVEStringTable *GetStringTable();	//always returns a valid pointer

protected:
	static EVEStringTable *s_stringTable;
};

class PyRepTuple : public PyRep {
public:
	typedef std::vector<PyRep *> storage_type;
	typedef std::vector<PyRep *>::iterator iterator;
	typedef std::vector<PyRep *>::const_iterator const_iterator;
	
	PyRepTuple(uint32 item_count) : PyRep(PyRep::Tuple), items(item_count, NULL) {}
	virtual ~PyRepTuple();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;

	void CloneFrom(const PyRepTuple *from);
	PyRepTuple *TypedClone() const;
	
	storage_type items;
	
	iterator begin() { return(items.begin()); }
	iterator end() { return(items.end()); }
	const_iterator begin() const { return(items.begin()); }
	const_iterator end() const { return(items.end()); }
	bool empty() const { return(items.empty()); }
	void clear();
};

class PyRepList : public PyRep {
public:
	typedef std::vector<PyRep *> storage_type;
	typedef std::vector<PyRep *>::iterator iterator;
	typedef std::vector<PyRep *>::const_iterator const_iterator;
	
	PyRepList() : PyRep(PyRep::List) {}
	virtual ~PyRepList();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	void CloneFrom(const PyRepList *from);
	PyRepList *TypedClone() const;
	
	void addInt(uint64 intval);
	void addReal(double realval);
	void addStr(const char *str);
	void add(const char *str);
	void add(PyRep *i);
	
	storage_type items;
	
	iterator begin() { return(items.begin()); }
	iterator end() { return(items.end()); }
	const_iterator begin() const { return(items.begin()); }
	const_iterator end() const { return(items.end()); }
	bool empty() const { return(items.empty()); }
	void clear();
};

class PyRepDict : public PyRep {
public:
	typedef std::map<PyRep *, PyRep *> storage_type;
	typedef std::map<PyRep *, PyRep *>::iterator iterator;
	typedef std::map<PyRep *, PyRep *>::const_iterator const_iterator;
	
	PyRepDict() : PyRep(PyRep::Dict) {}
	virtual ~PyRepDict();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	void CloneFrom(const PyRepDict *from);
	PyRepDict *TypedClone() const;

	void add(const char *key, PyRep *value);
	void add(PyRep *key, PyRep *value);
	void add(const char *key, const char *value);
	
	//BE CAREFUL, this map does NOT facilitate overwritting an existing
	//key, because there is no key comparison done (ptr compare only)!
	storage_type items;
	
	iterator begin() { return(items.begin()); }
	iterator end() { return(items.end()); }
	const_iterator begin() const { return(items.begin()); }
	const_iterator end() const { return(items.end()); }
	bool empty() const { return(items.empty()); }
	void clear();
};

class PyRepObject : public PyRep {
public:
	PyRepObject(const std::string &t = "", PyRep *a = NULL) : PyRep(PyRep::Object), type(t), arguments(a) {}
	virtual ~PyRepObject();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	PyRepObject *TypedClone() const;
	
	std::string type;
	PyRep *arguments;	//should be a tuple or a dict
};


class PyRepPackedRow : public PyRep {
public:
	typedef std::vector<byte> buffer;
	typedef std::vector<PyRep *> rep_list;

	PyRepPackedRow(const PyRep *header, bool own_header, const byte *data = NULL, const uint32 len = 0);
	virtual ~PyRepPackedRow();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	PyRepPackedRow *TypedClone() const;
	void CloneFrom(const PyRepPackedRow *from);

	//integers
	void PushInt8(const int8 v) { Push(&v, sizeof(int8)); }
	void PushUInt8(const uint8 v) { Push(&v, sizeof(uint8)); }
	void PushInt16(const int16 v) { Push(&v, sizeof(int16)); }
	void PushUInt16(const uint16 v) { Push(&v, sizeof(uint16)); }
	void PushInt32(const int32 v) { Push(&v, sizeof(int32)); }
	void PushUInt32(const uint32 v) { Push(&v, sizeof(uint32)); }
	void PushInt64(const int64 v) { Push(&v, sizeof(int64)); }
	void PushUInt64(const uint64 v) { Push(&v, sizeof(uint64)); }

	//floating point
	void PushFloat(const float v) { Push(&v, sizeof(float)); }
	void PushDouble(const double v) { Push(&v, sizeof(double)); }

	//raw
	void Push(const void *data, uint32 len);

	//PyRep
	void PushPyRep(PyRep *const v) { m_reps.push_back(v); }

	//header access
	const PyRep *GetHeader() const { return(m_header); }
	bool OwnsHeader() const { return(m_ownsHeader); }

	//buffer access
	const byte *GetBuffer() const { return(m_buffer.empty() ? NULL : &m_buffer[0]); }
	uint32 GetBufferSize() const { return(uint32(m_buffer.size())); }

	//reps access
	rep_list::iterator begin() { return(m_reps.begin()); }
	rep_list::iterator end() { return(m_reps.end()); }
	rep_list::const_iterator begin() const { return(m_reps.begin()); }
	rep_list::const_iterator end() const { return(m_reps.end()); }

protected:
	buffer m_buffer;
	rep_list m_reps;

	const bool m_ownsHeader;
	const PyRep *const m_header;
};

//base for opcodes 0x22 and 0x23
class PyRepPackedObject : public PyRep {
public:
	//for list data (before first PackedTerminator)
	typedef std::vector<PyRep *> list_type;
	typedef std::vector<PyRep *>::iterator list_iterator;
	typedef std::vector<PyRep *>::const_iterator const_list_iterator;

	//for dict data (after first PackedTerminator)
	typedef std::map<PyRep *, PyRep *> dict_type;
	typedef std::map<PyRep *, PyRep *>::iterator dict_iterator;
	typedef std::map<PyRep *, PyRep *>::const_iterator const_dict_iterator;

	PyRepPackedObject(Type t) : PyRep(t) {}
	virtual ~PyRepPackedObject();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const = 0;
	virtual void visit(PyVisitor *v) const;

	void CloneFrom(const PyRepPackedObject *from);

	list_type list_data;
	dict_type dict_data;
};

//opcode 0x22
class PyRepPackedObject1 : public PyRepPackedObject {
public:
	PyRepPackedObject1(const char *_type = "", PyRepTuple *_args = NULL, const PyRepDict &_keywords = PyRepDict());
	~PyRepPackedObject1();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType ltype, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;

	PyRepPackedObject1 *TypedClone() const;
	void CloneFrom(const PyRepPackedObject1 *from);

	std::string type;
	PyRepTuple *args;
	PyRepDict keywords;
};

//opcode 0x23
//not 100% completed
class PyRepPackedObject2 : public PyRepPackedObject {
public:
	PyRepPackedObject2(const char *_type = "", PyRepTuple *_args1 = NULL, PyRep *_args2 = NULL);
	~PyRepPackedObject2();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;

	PyRepPackedObject2 *TypedClone() const;
	void CloneFrom(const PyRepPackedObject2 *from);

	std::string type;
	PyRepTuple *args1;
	PyRep *args2;
};

class PyRepSubStruct : public PyRep {
public:
	PyRepSubStruct() : PyRep(PyRep::SubStruct), sub(NULL) {}
	PyRepSubStruct(PyRep *t) : PyRep(PyRep::SubStruct), sub(t) {}
	virtual ~PyRepSubStruct();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	PyRepSubStruct *TypedClone() const;
	
	PyRep *sub;
};

class PyRepSubStream : public PyRep {
public:
	PyRepSubStream() : PyRep(PyRep::SubStream), length(0), data(NULL), decoded(NULL) {}
	PyRepSubStream(PyRep *t) : PyRep(PyRep::SubStream), length(0), data(NULL), decoded(t) {}
	PyRepSubStream(const byte *buffer, uint32 len);
	
	virtual ~PyRepSubStream();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	PyRepSubStream *TypedClone() const;

	//call to ensure that `data` represents `decoded` IF DATA IS NULL
	void EncodeData();
	
	//call to ensure that `decoded` represents `data`
	void DecodeData() const;
	
	uint32 length;
	byte *data;

	//set this and make data NULL to make somebody else marshal it
	//if both are non-NULL, they are considered to be equivalent
	mutable PyRep *decoded;
};

class PyRepChecksumedStream : public PyRep {
public:
	PyRepChecksumedStream() : PyRep(PyRep::ChecksumedStream), checksum(0), stream(NULL) {}
	PyRepChecksumedStream(uint32 sum, PyRep *t) : PyRep(PyRep::ChecksumedStream), checksum(sum), stream(t) {}
	virtual ~PyRepChecksumedStream();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	PyRepChecksumedStream *TypedClone() const;

	uint32 checksum;
	PyRep *stream;
};

//this object is rather hackish, and is used to allow the
//application layer to communicate with the presentation layer
//about the usage of PyRepSaveMask/Op_PySavedStreamElement
//and is mainly used to support the layer-violating blue.DBRowDescriptor
//construct
/*class PyRepReference : public PyRep {
public:
	PyRepReference(PyRep *to);
	virtual ~PyRepReference();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const;
	virtual void visit(PyVisitor *v) const;

	PyRep *const to;
};*/



//this prolly belongs in its own file
class EVEStringTable {
public:
	static const uint8 None = 0;	//returned for no match

	bool LoadFile(const char *file);
	
	uint8 LookupString(const std::string &str) const;
	const char *LookupIndex(uint8 index) const;

protected:
	//there are better implementations of this, but this works:
	std::vector<std::string> m_forwardLookup;
	std::map<std::string, uint8> m_reverseLookup;
};




#endif

