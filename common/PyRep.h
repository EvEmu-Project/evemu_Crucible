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
		PackedHeader,
		PackedRow,
		PackedResultSet
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

//storing all integers (and bools) as uint62s is a lot of crazyness right now
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
	PyRepPackedRow(const byte *buffer, uint32 length, bool own_header, const PyRep *header);
	virtual ~PyRepPackedRow();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	PyRepPackedRow *TypedClone() const;
	
	//treat it as a buffer:
	byte *GetBuffer() const { return(m_value); }
	uint32 GetLength() const { return(m_length); }
	
	const PyRep *GetHeader() const { return(m_header); }

	bool OwnsHeader() const { return(m_ownsHeader); }
	
protected:
	byte *const m_value;
	const uint32 m_length;
	const bool m_ownsHeader;
	const PyRep *const m_header;
};

//TODO: make a seperate object for RowDict, because we need to keep
//key value pairs in `rows`, not just values.
class PyRepPackedRowHeader : public PyRep {
public:
	typedef std::vector<PyRepPackedRow *> storage_type;
	typedef std::vector<PyRepPackedRow *>::iterator iterator;
	typedef std::vector<PyRepPackedRow *>::const_iterator const_iterator;
	
	typedef enum {
		RowList,
		RowDict
	} Format;
	
	PyRepPackedRowHeader(Format format_ = RowList, PyRep *t = NULL, PyRep *a = NULL) : PyRep(PyRep::Object), format(format_), header_type(t), arguments(a) {}
	virtual ~PyRepPackedRowHeader();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	PyRepPackedRowHeader *TypedClone() const;

	Format format;	//determines how the packed data is stored following this object.
	PyRep *header_type;		//should always be "blue.DBRowDescriptor" right now.
	PyRep *arguments;	//should be a tuple or a dict
	
	//Packed data included after this header:
	storage_type rows;	//we own these.
	
	iterator begin() { return(rows.begin()); }
	iterator end() { return(rows.end()); }
	const_iterator begin() const { return(rows.begin()); }
	const_iterator end() const { return(rows.end()); }
	bool empty() const { return(rows.empty()); }
	void clear();
};

//TODO: make a seperate object for RowDict, because we need to keep
//key value pairs in `rows`, not just values. Then we can be more 
// specific with the header fields.
class PyRepPackedResultSet : public PyRep {
public:
	typedef std::vector<PyRepPackedRow *> storage_type;
	typedef std::vector<PyRepPackedRow *>::iterator iterator;
	typedef std::vector<PyRepPackedRow *>::const_iterator const_iterator;
	
	typedef enum {
		RowList,
		RowDict
	} Format;
	
	PyRepPackedResultSet(Format format_ = RowList, PyRep *h = NULL) : PyRep(PyRep::PackedResultSet), format(format_), header(h) {}
	virtual ~PyRepPackedResultSet();
	virtual void Dump(FILE *into, const char *pfx) const;
	virtual void Dump(LogType type, const char *pfx) const;
	virtual PyRep *Clone() const { return(TypedClone()); }
	virtual void visit(PyVisitor *v) const;
	
	PyRepPackedResultSet *TypedClone() const;
	
	Format format;	//determines how the packed data is stored following this object.
	
	PyRep *header;
		//contains this for a RowList:
		// ( ( "dbutil.RowList" ), [ "header" => PackedRowHeader, "columns" => list_of_col_names ] )
		// 	Represented as a dbutil_RowList_header in XML
		//contains this for a RowDict:
		// ( ( "dbutil.RowDict" ), [ "header" => PackedRowHeader, "columns" => list_of_col_names, "key" => key_column_name ] )
		// 	Represented as a dbutil_RowDict_header in XML
	
	//Packed data included in this result set:
	storage_type rows;	//we own these.
	
	iterator begin() { return(rows.begin()); }
	iterator end() { return(rows.end()); }
	const_iterator begin() const { return(rows.begin()); }
	const_iterator end() const { return(rows.end()); }
	bool empty() const { return(rows.empty()); }
	void clear();
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

