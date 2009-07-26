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
    Author:     Zhur
*/

#ifndef EVE_PY_REP_H
#define EVE_PY_REP_H

#include <assert.h>

#include "common.h"
#include "logsys.h"
#include "packet_types.h"

class PyVisitor;
class PyVisitorLvl;
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
class PyRepObjectEx;
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
        PyTypeInteger           = 0,
        PyTypeReal              = 1,
        PyTypeBoolean           = 2,
        PyTypeBuffer            = 3,
        PyTypeString            = 4,
        PyTypeTuple             = 5,
        PyTypeList              = 6,
        PyTypeDict              = 7,
        PyTypeNone              = 8,
        PyTypeSubStruct         = 9,
        PyTypeSubStream         = 10,
        PyTypeChecksumedStream  = 11,
        PyTypeObject            = 12,
        PyTypeObjectEx          = 13,
        PyTypePackedRow         = 14,
        PyTypeError             = 15,
        PyTypeMax               = 15,
    } Type;

    PyRep(Type t);
    virtual ~PyRep();

    /** Type check functions
      */
    bool IsInteger() const          { return m_type == PyTypeInteger; }
    bool IsReal() const             { return m_type == PyTypeReal; }
    bool IsBool() const             { return m_type == PyTypeBoolean; }
    bool IsBuffer() const           { return m_type == PyTypeBuffer; }
    bool IsString() const           { return m_type == PyTypeString; }
    bool IsTuple() const            { return m_type == PyTypeTuple; }
    bool IsList() const             { return m_type == PyTypeList; }
    bool IsDict() const             { return m_type == PyTypeDict; }
    bool IsNone() const             { return m_type == PyTypeNone; }
    bool IsSubStruct() const        { return m_type == PyTypeSubStruct; }
    bool IsSubStream() const        { return m_type == PyTypeSubStream; }
    bool IsChecksumedStream() const { return m_type == PyTypeChecksumedStream; }
    bool IsObject() const           { return m_type == PyTypeObject; }
    bool IsObjectEx() const         { return m_type == PyTypeObjectEx; }
    bool IsPackedRow() const        { return m_type == PyTypePackedRow; }


    // tools for easy access... less typecasting, TODO assrt when wrong "as" is done
    PyRepInteger& AsInteger()                               { return *(PyRepInteger *)this; }
    const PyRepInteger& AsInteger() const                   { return *(const PyRepInteger *)this; }
    PyRepReal& AsReal()                                     { return *(PyRepReal *)this; }
    const PyRepReal& AsReal() const                         { return *(const PyRepReal *)this; }
    PyRepBoolean& AsBool()                                  { return *(PyRepBoolean *)this; }
    const PyRepBoolean& AsBool() const                      { return *(const PyRepBoolean *)this; }
    PyRepBuffer& AsBuffer()                                 { return *(PyRepBuffer *)this; }
    const PyRepBuffer& AsBuffer() const                     { return *(const PyRepBuffer *)this; }
    PyRepString& AsString()                                 { return *(PyRepString *)this; }
    const PyRepString& AsString() const                     { return *(const PyRepString *)this; }
    PyRepTuple& AsTuple()                                   { return *(PyRepTuple *)this; }
    const PyRepTuple& AsTuple() const                       { return *(const PyRepTuple *)this; }
    PyRepList& AsList()                                     { return *(PyRepList *)this; }
    const PyRepList& AsList() const                         { return *(const PyRepList *)this; }
    PyRepDict& AsDict()                                     { return *(PyRepDict *)this; }
    const PyRepDict& AsDict() const                         { return *(const PyRepDict *)this; }
    PyRepNone& AsNone()                                     { return *(PyRepNone *)this; }
    const PyRepNone& AsNone() const                         { return *(const PyRepNone *)this; }
    PyRepSubStruct& AsSubStruct()                           { return *(PyRepSubStruct *)this; }
    const PyRepSubStruct& AsSubStruct() const               { return *(const PyRepSubStruct *)this; }
    PyRepSubStream& AsSubStream()                           { return *(PyRepSubStream *)this; }
    const PyRepSubStream& AsSubStream() const               { return *(const PyRepSubStream *)this; }
    PyRepChecksumedStream& AsChecksumedStream()             { return *(PyRepChecksumedStream *)this; }
    const PyRepChecksumedStream& AsChecksumedStream() const { return *(const PyRepChecksumedStream *)this; }
    PyRepObject& AsObject()                                 { return *(PyRepObject *)this; }
    const PyRepObject& AsObject() const                     { return *(const PyRepObject *)this; }
    PyRepObjectEx& AsObjectEx()                             { return *(PyRepObjectEx *)this; }
    const PyRepObjectEx& AsObjectEx() const                 { return *(const PyRepObjectEx *)this; }
    PyRepPackedRow& AsPackedRow()                           { return *(PyRepPackedRow *)this; }
    const PyRepPackedRow& AsPackedRow() const               { return *(const PyRepPackedRow *)this; }

    const char *TypeString() const;

    virtual void Dump(FILE *into, const char *pfx) const = 0;
    virtual void Dump(LogType type, const char *pfx) const = 0;
    virtual PyRep *Clone() const = 0;
    virtual void visit(PyVisitor *v) const = 0;
    virtual void visit(PyVisitorLvl *v, int64 lvl) const = 0;

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
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitInteger(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitInteger(this, lvl);
    }

    PyRepInteger *TypedClone() const;
    int64 value;
};

class PyRepReal : public PyRep {
public:
    PyRepReal(const double &i) : PyRep(PyRep::PyTypeReal), value(i) {}
    virtual ~PyRepReal() {}
    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitReal(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitReal(this, lvl);
    }

    PyRepReal *TypedClone() const;

    double value;
};

class PyRepBoolean : public PyRep {
public:
    PyRepBoolean(bool i) : PyRep(PyRep::PyTypeBoolean), value(i) {}
    virtual ~PyRepBoolean() {}
    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitBoolean(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitBoolean(this, lvl);
    }

    PyRepBoolean *TypedClone() const;

    bool value;
};

class PyRepNone : public PyRep {
public:
    PyRepNone() : PyRep(PyRep::PyTypeNone) {}
    virtual ~PyRepNone() {}
    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitNone(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitNone(this, lvl);
    }

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
    EVEMU_INLINE PyRep *Clone() const { return TypedClone(); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitBuffer(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitBuffer(this, lvl);
    }

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
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitString(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitString(this, lvl);
    }

    PyRepString *TypedClone() const;

    std::string value;
    bool is_type_1; //true if this is an Op_PyByteString instead of the default Op_PyByteString2

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
    static EVEStringTable *GetStringTable();    //always returns a valid pointer

protected:
    static EVEStringTable *s_stringTable;
};

class PyRepTuple : public PyRep {
public:
    typedef std::vector<PyRep *>            storage_type;
    typedef storage_type::iterator          iterator;
    typedef storage_type::const_iterator    const_iterator;

    PyRepTuple(uint32 item_count) : PyRep(PyRep::PyTypeTuple), items(item_count, NULL) {}
    virtual ~PyRepTuple();
    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitTuple(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitTuple(this, lvl);
    }

    void CloneFrom(const PyRepTuple *from);
    PyRepTuple *TypedClone() const;

    storage_type items;

    inline iterator begin() { return(items.begin()); }
    inline iterator end() { return(items.end()); }
    inline const_iterator begin() const { return(items.begin()); }
    inline const_iterator end() const { return(items.end()); }
    bool empty() const { return(items.empty()); }
    void clear();
    uint32 size() const;

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
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitList(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitList(this, lvl);
    }

    void CloneFrom(const PyRepList *from);
    PyRepList *TypedClone() const;

    EVEMU_INLINE void addInt(uint64 intval) {
        items.push_back(new PyRepInteger(intval));
    }

    EVEMU_INLINE void addReal(double realval) {
        items.push_back(new PyRepReal(realval));
    }

    EVEMU_INLINE void addStr(const char *str) {
        items.push_back(new PyRepString(str));
    }

    EVEMU_INLINE void add(const char *str) {
        items.push_back(new PyRepString(str));
    }

    EVEMU_INLINE void add(PyRep *i) {
        items.push_back(i);
    }

    storage_type items;

    iterator begin() { return(items.begin()); }
    iterator end() { return(items.end()); }
    const_iterator begin() const { return(items.begin()); }
    const_iterator end() const { return(items.end()); }
    bool empty() const { return(items.empty()); }
    void clear();
    EVEMU_INLINE uint32 size() const {
        return (uint32)items.size();
    }
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
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitDict(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitDict(this, lvl);
    }

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
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitObject(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitObject(this, lvl);
    }

    PyRepObject *TypedClone() const;

    std::string type;
    PyRep *arguments;   //should be a tuple or a dict
};


//opcodes 0x22 and 0x23 (merged for simplicity)
class PyRepObjectEx : public PyRep {
public:
    //for list data (before first PackedTerminator)
    typedef std::vector<PyRep *> list_type;
    typedef std::vector<PyRep *>::iterator list_iterator;
    typedef std::vector<PyRep *>::const_iterator const_list_iterator;

    //for dict data (after first PackedTerminator)
    typedef std::map<PyRep *, PyRep *> dict_type;
    typedef std::map<PyRep *, PyRep *>::iterator dict_iterator;
    typedef std::map<PyRep *, PyRep *>::const_iterator const_dict_iterator;

    PyRepObjectEx(bool _is_type_1, PyRep *_header = NULL) : PyRep(PyRep::PyTypeObjectEx), header(_header), is_type_1(_is_type_1) {}
    virtual ~PyRepObjectEx();
    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitObjectEx(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitObjectEx(this, lvl);
    }

    PyRepObjectEx *TypedClone() const;
    void CloneFrom(const PyRepObjectEx *from);

    PyRep *header;
    const bool is_type_1;   // true if opcode is 0x26 instead of 0x25

    list_type list_data;
    dict_type dict_data;

};

class PyRepPackedRow : public PyRep {
public:
    // We silently assume here that header is blue.DBRowDescriptor.
    PyRepPackedRow(const PyRep &header, bool header_owner);
    virtual ~PyRepPackedRow();
    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitPackedRow(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitPackedRow(this, lvl);
    }

    PyRepPackedRow *TypedClone() const;
    void CloneFrom(const PyRepPackedRow *from);

    // Header:
    const PyRep &GetHeader() const { return mHeader; }
    bool IsHeaderOwner() const { return mHeaderOwner; }

    // Column info:
    uint32 ColumnCount() const { return mFields.size(); }
    const std::string &GetColumnName(uint32 index) const;
    uint32 GetColumnIndex(const char *name) const;
    DBTYPE GetColumnType(uint32 index) const;

    // Fields:
    PyRep *GetField(uint32 index) const { return mFields.at( index ); }

    bool SetField(uint32 index, PyRep *value);
    bool SetField(const char *colName, PyRep *value);

protected:
    const PyRep &mHeader;
    const bool mHeaderOwner;

    const PyRepTuple *mColumnInfo;

    std::vector<PyRep *> mFields;
};

class PyRepSubStruct : public PyRep {
public:
    PyRepSubStruct() : PyRep(PyRep::PyTypeSubStruct), sub(NULL) {}
    PyRepSubStruct(PyRep *t) : PyRep(PyRep::PyTypeSubStruct), sub(t) {}
    virtual ~PyRepSubStruct();
    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitSubStruct(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitSubStruct(this, lvl);
    }

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
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitSubStream(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitSubStream(this, lvl);
    }

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
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitChecksumedStream(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitChecksumedStream(this, lvl);
    }

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
    EVEMU_INLINE PyRep *Clone() const;
    void visit(PyVisitor *v) const;

    PyRep *const to;
};*/

//this probably belongs in its own file
class EVEStringTable
{
public:
    EVEStringTable();
    ~EVEStringTable();

    static const uint8 None = 0;    //returned for no match

    bool LoadFile(const char *file);

    uint8 LookupString(const std::string &str) const;
    const char *LookupIndex(uint8 index) const;

protected:
    //there are better implementations of this, but this works:
    typedef std::vector<char*>                          LookupVector;
    typedef LookupVector::iterator                      LookupVectorItr;
    typedef LookupVector::const_iterator                LookupVectorConstItr;

    typedef std::tr1::unordered_map<std::string, uint8> LookupMap;
    typedef LookupMap::iterator                         LookupMapItr;
    typedef LookupMap::const_iterator                   LookupMapConstItr;


    LookupVector    m_forwardLookup;
    LookupMap       m_reverseLookup;
    uint32          m_size;
};

#endif//EVE_PY_REP_H
