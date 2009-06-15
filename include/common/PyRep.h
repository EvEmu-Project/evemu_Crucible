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
	Author:		Zhur
*/

#ifndef EVE_PY_REP_H
#define EVE_PY_REP_H

#include "common.h"
#include "logsys.h"

#include <assert.h>

class PyVisitor;
class EVEStringTable;

class PyRepInteger;
class PyRepReal;
class PyRepBoolean;
class PyRepBuffer;
class PyRepString;
class PyRepTuple;
class PyRepList;
class PyRepDict;
class PyRepNone;
class PyRepSubStruct;
class PyRepSubStream;
class PyRepChecksumedStream;
class PyRepObject;
class PyRepNewObject;
class PyRepPackedRow;

/** Lookup table for PyRep type object type names.
  */
extern const char *PyRepTypeString[];

/** PyRep base Python wire object
  */
class PyRep {
public:
	/** PyRep Python wire object types
	*/
	typedef enum _Type {
		PyTypeInteger			= 0,
		PyTypeReal				= 1,
		PyTypeBoolean			= 2,
		PyTypeBuffer			= 3,
		PyTypeString			= 4,
		PyTypeTuple				= 5,
		PyTypeList				= 6,
		PyTypeDict				= 7,
		PyTypeNone				= 8,
		PyTypeSubStruct			= 9,
		PyTypeSubStream			= 10,
		PyTypeChecksumedStream	= 11,
		PyTypeObject			= 12,
		PyTypeNewObject			= 13,
		PyTypePackedRow			= 14,
		PyTypeError				= 15,
		PyTypeMax				= 15,
	} Type;

	PyRep(Type t);
	virtual ~PyRep();

	/** Type check functions
	  */
	bool IsInteger() const {return m_type == PyTypeInteger;}
	bool IsReal() const {return m_type == PyTypeReal;}
	bool IsBool() const {return m_type == PyTypeBoolean;}
	bool IsBuffer() const {return m_type == PyTypeBuffer;}
	bool IsString() const {return m_type == PyTypeString;}
	bool IsTuple() const {return m_type == PyTypeTuple;}
	bool IsList() const {return m_type == PyTypeList;}
	bool IsDict() const {return m_type == PyTypeDict;}
	bool IsNone() const {return m_type == PyTypeNone;}
	bool IsSubStruct() const {return m_type == PyTypeSubStruct;}
	bool IsSubStream() const {return m_type == PyTypeSubStream;}
	bool IsChecksumedStream() const {return m_type == PyTypeChecksumedStream;}
	bool IsObject() const {return m_type == PyTypeObject;}
	bool IsNewObject() const {return m_type == PyTypeNewObject;}
	bool IsPackedRow() const {return m_type == PyTypePackedRow;}


	// tools for easy access... less typecasting, TODO assrt when wrong "as" is done
	PyRepInteger& AsInteger() {return *((PyRepInteger*)this);}
	PyRepReal& AsReal() {return *((PyRepReal*)this);}
	PyRepBoolean& AsBool() {return *((PyRepBoolean*)this);}
	PyRepBuffer& AsBuffer() {return *((PyRepBuffer*)this);}
	PyRepString& AsString() {return *((PyRepString*)this);}
	PyRepTuple& AsTuple() {return *((PyRepTuple*)this);}
	PyRepList& AsLAst() {return *((PyRepList*)this);}
	PyRepDict& AsDict() {return *((PyRepDict*)this);}
	PyRepNone& AsNone() {return *((PyRepNone*)this);}
	PyRepSubStruct& AsSubStruct() {return *((PyRepSubStruct*)this);}
	PyRepSubStream& AsSubStream() {return *((PyRepSubStream*)this);}
	PyRepChecksumedStream& AsChecksumedStream() {return *((PyRepChecksumedStream*)this);}
	PyRepObject& AsObject() {return *((PyRepObject*)this);}
	PyRepNewObject& AsNewObject() {return *((PyRepNewObject*)this);}
	PyRepPackedRow& AsPackedRow() {return *((PyRepPackedRow*)this);}

	const char *TypeString() const;
		
	virtual void Dump(FILE *into, const char *pfx) const = 0;
	virtual void Dump(LogType type, const char *pfx) const = 0;
	virtual PyRep *Clone() const = 0;
	virtual void visit(PyVisitor *v) const = 0;

	//using this method is discouraged, it generally means your doing something wrong... CheckType() should cover almost all needs
	Type youreallyshouldentbeusingthis_GetType() const { return m_type; }

protected:
	const Type m_type;
};

//storing all integers (and booleans) as uint64s is a lot of craziness right now
//but its better than a ton of virtual functions to achieve the same thing.

class PyRepInteger : public PyRep {
public:
	PyRepInteger(const int64 &i) : PyRep(PyRep::PyTypeInteger), value(i) {}
	virtual ~PyRepInteger() {}
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return(TypedClone()); }
	void visit(PyVisitor *v) const;
	
	PyRepInteger *TypedClone() const;
	int64 value;
};

class PyRepReal : public PyRep {
public:
	PyRepReal(const double &i) : PyRep(PyRep::PyTypeReal), value(i) {}
	virtual ~PyRepReal() {}
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return(TypedClone()); }
	void visit(PyVisitor *v) const;
	
	PyRepReal *TypedClone() const;
	
	double value;
};

class PyRepBoolean : public PyRep {
public:
	PyRepBoolean(bool i) : PyRep(PyRep::PyTypeBoolean), value(i) {}
	virtual ~PyRepBoolean() {}
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return(TypedClone()); }
	void visit(PyVisitor *v) const;
	
	PyRepBoolean *TypedClone() const;
	
	bool value;
};

class PyRepNone : public PyRep {
public:
	PyRepNone() : PyRep(PyRep::PyTypeNone) {}
	virtual ~PyRepNone() {}
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return(TypedClone()); }
	void visit(PyVisitor *v) const;
	
	PyRepNone *TypedClone() const;
};

class PyRepBuffer : public PyRep
{
public:
	PyRepBuffer(const uint8 *buffer, uint32 length);
	PyRepBuffer(uint8 **buffer, uint32 length);
	PyRepBuffer(uint32 length);

	PyRepBuffer(std::string &buffer);

	virtual ~PyRepBuffer();
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return TypedClone(); }
	void visit(PyVisitor *v) const;
	
	PyRepBuffer *TypedClone() const;
	
	//treat it as a buffer:
	uint8 *GetBuffer() const { return m_value; }
	uint32 GetLength() const { return m_length; }

	/**
	 * @brief Get the PyBuffer size.
	 *
	 * 
	 *
	 * @return return the size of the buffer.
	 */
	size_t size() const {return (size_t)m_length;}

	/**
	 * @brief Get the PyBuffer content
	 *
	 * 
	 *
	 * @return the pointer to the PyBuffer content
	 */
	uint8* content() const {return m_value;}

	PyRepSubStream *CreateSubStream() const;
	
protected:
	uint8 *m_value;
	const uint32 m_length;
};

class PyRepString : public PyRep
{
public:
	PyRepString(const char *str = "", bool type_1=false) : PyRep(PyRep::PyTypeString), value(str), is_type_1(type_1)
	{

	}

	PyRepString(const std::string &str, bool type_1=false) : PyRep(PyRep::PyTypeString), is_type_1(type_1)
	{
		value.assign(str.c_str(), str.length());
	} //to deal with non-string buffers poorly placed in strings (CCP)

	PyRepString(const uint8 *data, uint32 len, bool type_1=false) : PyRep(PyRep::PyTypeString), value((const char *) data, len), is_type_1(type_1) {}
	virtual ~PyRepString() {}
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return(TypedClone()); }
	void visit(PyVisitor *v) const;

	PyRepString *TypedClone() const;

	std::string value;
	bool is_type_1;	//true if this is an Op_PyByteString instead of the default Op_PyByteString2

	/**
	 * @brief get the length of the PyString
	 *
	 * 
	 *
	 * @return the length of the string as size_t
	 */
	size_t size() { return value.size(); }

	/**
	* @brief Get the PyString content
	*
	* 
	*
	* @return the pointer to the char* array.
	*/
	const char* content() { return value.c_str(); }

	//string table stuff:
	static bool LoadStringFile(const char *file);
	static EVEStringTable *GetStringTable();	//always returns a valid pointer

protected:
	static EVEStringTable *s_stringTable;
};

class PyRepTuple : public PyRep {
public:
	typedef std::vector<PyRep *>			storage_type;
	typedef storage_type::iterator			iterator;
	typedef storage_type::const_iterator	const_iterator;
	
	PyRepTuple(uint32 item_count) : PyRep(PyRep::PyTypeTuple), items(item_count, NULL) {}
	virtual ~PyRepTuple();
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return(TypedClone()); }
	void visit(PyVisitor *v) const;

	void CloneFrom(const PyRepTuple *from);
	PyRepTuple *TypedClone() const;
	
	storage_type items;
	
	inline iterator begin() { return(items.begin()); }
	inline iterator end() { return(items.end()); }
	inline const_iterator begin() const { return(items.begin()); }
	inline const_iterator end() const { return(items.end()); }
	bool empty() const { return(items.empty()); }
	void clear();

	/* polymorphic overload to make the PyRepTuple do nice lookups. */
	
	/**
	 * @brief overloading the [] operator for reference lookup.
	 *
	 * 
	 *
	 * @param[in] pos the position of the required object.
	 * @return The reference to the indexed object.
	 */
	inline PyRep& operator[](const size_t pos) const {
		/* we should crash here if we are trying to lookup a object that isn't in our range */
		assert(pos < items.size());
		return *items[pos];
	}

	/**
	 * @brief overloading the [] operator for pointer lookup.
	 *
	 * 
	 *
	 * @param[in] pos the position of the required object.
	 * @return The reference to the indexed object.
	 */
	inline PyRep* operator[](const size_t pos) {
		/* we should crash here if we are trying to lookup a object that isn't in our range */
		assert(pos < items.size());
		return items[pos];
	}
};

class PyRepList : public PyRep {
public:
	typedef std::vector<PyRep *> storage_type;
	typedef std::vector<PyRep *>::iterator iterator;
	typedef std::vector<PyRep *>::const_iterator const_iterator;
	
	PyRepList() : PyRep(PyRep::PyTypeList) {}
	virtual ~PyRepList();
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return(TypedClone()); }
	void visit(PyVisitor *v) const;
	
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
	
	PyRepDict() : PyRep(PyRep::PyTypeDict) {}
	virtual ~PyRepDict();
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return(TypedClone()); }
	void visit(PyVisitor *v) const;
	
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
	PyRepObject(const std::string &t = "", PyRep *a = NULL) : PyRep(PyRep::PyTypeObject), type(t), arguments(a) {}
	virtual ~PyRepObject();
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return(TypedClone()); }
	void visit(PyVisitor *v) const;
	
	PyRepObject *TypedClone() const;
	
	std::string type;
	PyRep *arguments;	//should be a tuple or a dict
};


//opcodes 0x22 and 0x23 (merged for simplicity)
class PyRepNewObject : public PyRep {
public:
	//for list data (before first PackedTerminator)
	typedef std::vector<PyRep *> list_type;
	typedef std::vector<PyRep *>::iterator list_iterator;
	typedef std::vector<PyRep *>::const_iterator const_list_iterator;

	//for dict data (after first PackedTerminator)
	typedef std::map<PyRep *, PyRep *> dict_type;
	typedef std::map<PyRep *, PyRep *>::iterator dict_iterator;
	typedef std::map<PyRep *, PyRep *>::const_iterator const_dict_iterator;

	PyRepNewObject(bool _is_type_1, PyRep *_header = NULL) : PyRep(PyRep::PyTypeNewObject), header(_header), is_type_1(_is_type_1) {}
	virtual ~PyRepNewObject();
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return(TypedClone()); }
	void visit(PyVisitor *v) const;

	PyRepNewObject *TypedClone() const;
	void CloneFrom(const PyRepNewObject *from);

	PyRep *header;
	const bool is_type_1;	// true if opcode is 0x26 instead of 0x25

	list_type list_data;
	dict_type dict_data;

};

class PyRepPackedRow : public PyRep {
public:
	typedef std::vector<uint8> buffer_t;
	typedef std::vector<PyRep *> rep_list;

	PyRepPackedRow(const PyRep *header, bool own_header, const uint8 *data = NULL, const uint32 len = 0);
	virtual ~PyRepPackedRow();
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return(TypedClone()); }
	void visit(PyVisitor *v) const;
	
	PyRepPackedRow *TypedClone() const;
	void CloneFrom(const PyRepPackedRow *from);

	/*
	 * Push
	 */
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

	/*
	 * Get
	 */
	//integers
	int8 GetInt8(size_t offset) const { return(*(const int8 *)Get(offset)); }
	uint8 GetUInt8(size_t offset) const { return(*(const uint8 *)Get(offset)); }
	int16 GetInt16(size_t offset) const { return(*(const int16 *)Get(offset)); }
	uint16 GetUInt16(size_t offset) const { return(*(const uint16 *)Get(offset)); }
	int32 GetInt32(size_t offset) const { return(*(const int32 *)Get(offset)); }
	uint32 GetUInt32(size_t offset) const { return(*(const uint32 *)Get(offset)); }
	int64 GetInt64(size_t offset) const { return(*(const int64 *)Get(offset)); }
	uint64 GetUInt64(size_t offset) const { return(*(const uint64 *)Get(offset)); }

	//floating point
	float GetFloat(size_t offset) const { return(*(const float *)Get(offset)); }
	double GetDouble(size_t offset) const { return(*(const double *)Get(offset)); }

	//raw
	const uint8 *Get(size_t offset) const { return(m_buffer.empty() ? NULL : &m_buffer[offset]); }

	//PyRep
	PyRep *GetPyRep(size_t offset) const { return(m_reps[offset]); }

	/*
	 * Other
	 */
	//header
	const bool ownsHeader;
	const PyRep *const header;

	//buffer access
	const uint8 *buffer() const { return(Get(0)); }
	uint32 bufferSize() const { return(uint32(m_buffer.size())); }

	//reps
	rep_list::iterator begin() { return(m_reps.begin()); }
	rep_list::iterator end() { return(m_reps.end()); }
	rep_list::const_iterator begin() const { return(m_reps.begin()); }
	rep_list::const_iterator end() const { return(m_reps.end()); }

	rep_list m_reps;

protected:
	buffer_t m_buffer;
};

class PyRepSubStruct : public PyRep {
public:
	PyRepSubStruct() : PyRep(PyRep::PyTypeSubStruct), sub(NULL) {}
	PyRepSubStruct(PyRep *t) : PyRep(PyRep::PyTypeSubStruct), sub(t) {}
	virtual ~PyRepSubStruct();
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return(TypedClone()); }
	void visit(PyVisitor *v) const;
	
	PyRepSubStruct *TypedClone() const;
	
	PyRep *sub;
};

class PyRepSubStream : public PyRep {
public:
	PyRepSubStream() : PyRep(PyRep::PyTypeSubStream), length(0), data(NULL), decoded(NULL) {}
	PyRepSubStream(PyRep *t) : PyRep(PyRep::PyTypeSubStream), length(0), data(NULL), decoded(t) {}
	PyRepSubStream(const uint8 *buffer, uint32 len);
	
	virtual ~PyRepSubStream();
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return(TypedClone()); }
	void visit(PyVisitor *v) const;
	
	PyRepSubStream *TypedClone() const;

	//call to ensure that `data` represents `decoded` IF DATA IS NULL
	void EncodeData();
	
	//call to ensure that `decoded` represents `data`
	void DecodeData() const;
	
	uint32 length;
	uint8 *data;

	//set this and make data NULL to make somebody else marshal it
	//if both are non-NULL, they are considered to be equivalent
	mutable PyRep *decoded;
};

class PyRepChecksumedStream : public PyRep {
public:
	PyRepChecksumedStream() : PyRep(PyRep::PyTypeChecksumedStream), checksum(0), stream(NULL) {}
	PyRepChecksumedStream(uint32 sum, PyRep *t) : PyRep(PyRep::PyTypeChecksumedStream), checksum(sum), stream(t) {}
	virtual ~PyRepChecksumedStream();
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const { return(TypedClone()); }
	void visit(PyVisitor *v) const;
	
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
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	ASCENT_INLINE PyRep *Clone() const;
	void visit(PyVisitor *v) const;

	PyRep *const to;
};*/

//this probably belongs in its own file
class EVEStringTable
{
public:
	EVEStringTable();
	~EVEStringTable();

	static const uint8 None = 0;	//returned for no match

	bool LoadFile(const char *file);
	
	uint8 LookupString(const std::string &str) const;
	const char *LookupIndex(uint8 index) const;

protected:
	//there are better implementations of this, but this works:
	typedef std::vector<char*>							LookupVector;
	typedef LookupVector::iterator						LookupVectorItr;
	typedef LookupVector::const_iterator				LookupVectorConstItr;

	typedef std::tr1::unordered_map<std::string, uint8> LookupMap;
	typedef LookupMap::iterator							LookupMapItr;
	typedef LookupMap::const_iterator					LookupMapConstItr;

	
	LookupVector	m_forwardLookup;
	LookupMap		m_reverseLookup;
	uint32			m_size;
};

#endif//EVE_PY_REP_H
