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
#include "PyVisitor.h"

/* note: this will decrease memory use with 50% but increase load time with 50%
 * enabling this would have to wait until references work properly. Or when
 * you operate the server using the cache store system this can also be enabled.
 * note: this will have influence on the overall server performance.
 */
//#pragma pack(push,1)

class PyInt;
class PyFloat;
class PyBool;
class PyBuffer;
class PyString;
class PyTuple;
class PyList;
class PyDict;
class PyNone;
class PySubStruct;
class PySubStream;
class PyChecksumedStream;
class PyObject;
class PyObjectEx;
class PyPackedRow;

class DBRowDescriptor;

/** Lookup table for PyRep type object type names.
  */
extern const char *PyRepTypeString[];

/**
 * debug macro's to ease the increase and decrease of references of a object
 * using this also increases the possibility of debugging it.
 */
#define PyIncRef(op) (op)->IncRef()
#define PyDecRef(op) (op)->DecRef()

/* Macros to use in case the object pointer may be NULL */
#define PySafeIncRef(op) if( (op) == NULL ) ; else PyIncRef( op )
#define PySafeDecRef(op) if( (op) == NULL ) ; else PyDecRef( op )

/** PyRep base Python wire object
 */
class PyRep
{
public:
    /** PyRep Python wire object types
     */
    enum PyType {
        PyTypeInt               = 0,
        PyTypeLong              = 1,
        PyTypeFloat             = 2,
        PyTypeBool              = 3,
        PyTypeBuffer            = 4,
        PyTypeString            = 5,
        PyTypeTuple             = 6,
        PyTypeList              = 7,
        PyTypeDict              = 8,
        PyTypeNone              = 9,
        PyTypeSubStruct         = 10,
        PyTypeSubStream         = 11,
        PyTypeChecksumedStream  = 12,
        PyTypeObject            = 13,
        PyTypeObjectEx          = 14,
        PyTypePackedRow         = 15,
        PyTypeError             = 16,
        PyTypeMax               = 16,
    };

    PyRep(PyType t);
    virtual ~PyRep();

    /** PyType check functions
      */
    bool IsInt() const              { return m_type == PyTypeInt; }
	bool IsLong() const             { return m_type == PyTypeLong; }
    bool IsFloat() const            { return m_type == PyTypeFloat; }
    bool IsBool() const             { return m_type == PyTypeBool; }
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


    // tools for easy access, less typecasting...
    PyInt& AsInt()                                       { assert( IsInt() ); return *(PyInt *)this; }
    const PyInt& AsInt() const                           { assert( IsInt() ); return *(const PyInt *)this; }
	PyLong& AsLong()                                     { assert( IsLong() ); return *(PyLong *)this; }
	const PyLong& AsLong() const                         { assert( IsLong() ); return *(const PyLong *)this; }
    PyFloat& AsFloat()                                   { assert( IsFloat() ); return *(PyFloat *)this; }
    const PyFloat& AsFloat() const                       { assert( IsFloat() ); return *(const PyFloat *)this; }
    PyBool& AsBool()                                     { assert( IsBool() ); return *(PyBool *)this; }
    const PyBool& AsBool() const                         { assert( IsBool() ); return *(const PyBool *)this; }
    PyBuffer& AsBuffer()                                 { assert( IsBuffer() ); return *(PyBuffer *)this; }
    const PyBuffer& AsBuffer() const                     { assert( IsBuffer() ); return *(const PyBuffer *)this; }
    PyString& AsString()                                 { assert( IsString() ); return *(PyString *)this; }
    const PyString& AsString() const                     { assert( IsString() ); return *(const PyString *)this; }
    PyTuple& AsTuple()                                   { assert( IsTuple() ); return *(PyTuple *)this; }
    const PyTuple& AsTuple() const                       { assert( IsTuple() ); return *(const PyTuple *)this; }
    PyList& AsList()                                     { assert( IsList() ); return *(PyList *)this; }
    const PyList& AsList() const                         { assert( IsList() ); return *(const PyList *)this; }
    PyDict& AsDict()                                     { assert( IsDict() ); return *(PyDict *)this; }
    const PyDict& AsDict() const                         { assert( IsDict() ); return *(const PyDict *)this; }
    PyNone& AsNone()                                     { assert( IsNone() ); return *(PyNone *)this; }
    const PyNone& AsNone() const                         { assert( IsNone() ); return *(const PyNone *)this; }
    PySubStruct& AsSubStruct()                           { assert( IsSubStruct() ); return *(PySubStruct *)this; }
    const PySubStruct& AsSubStruct() const               { assert( IsSubStruct() ); return *(const PySubStruct *)this; }
    PySubStream& AsSubStream()                           { assert( IsSubStream() ); return *(PySubStream *)this; }
    const PySubStream& AsSubStream() const               { assert( IsSubStream() ); return *(const PySubStream *)this; }
    PyChecksumedStream& AsChecksumedStream()             { assert( IsChecksumedStream() ); return *(PyChecksumedStream *)this; }
    const PyChecksumedStream& AsChecksumedStream() const { assert( IsChecksumedStream() ); return *(const PyChecksumedStream *)this; }
    PyObject& AsObject()                                 { assert( IsObject() ); return *(PyObject *)this; }
    const PyObject& AsObject() const                     { assert( IsObject() ); return *(const PyObject *)this; }
    PyObjectEx& AsObjectEx()                             { assert( IsObjectEx() ); return *(PyObjectEx *)this; }
    const PyObjectEx& AsObjectEx() const                 { assert( IsObjectEx() ); return *(const PyObjectEx *)this; }
    PyPackedRow& AsPackedRow()                           { assert( IsPackedRow() ); return *(PyPackedRow *)this; }
    const PyPackedRow& AsPackedRow() const               { assert( IsPackedRow() ); return *(const PyPackedRow *)this; }

    const char *TypeString() const;

    virtual void Dump(FILE *into, const char *pfx) const = 0;
    virtual void Dump(LogType type, const char *pfx) const = 0;
    virtual PyRep *Clone() const = 0;
    virtual void visit(PyVisitor *v) const = 0;
    virtual void visit(PyVisitorLvl *v, int64 lvl) const = 0;

    /**
     * @brief virtual function to generate a hash value of a object.
     * 
     * virtual function to generate a hash value of a object to facilitate the various maps and checks.
     *
     * @return returns a uint32 containing a hash function that represents the object.
     */
    virtual int32 hash() const;

    //using this method is discouraged, it generally means your doing something wrong... CheckType() should cover almost all needs
    PyType GetType() const { return m_type; }

    void IncRef()
    {
        mRefcnt++;
        //printf("PyRep:0x%p | ref:%u inc\n", this, mRefcnt);
    }

    void DecRef()
    {
        mRefcnt--;
        //printf("PyRep:0x%p | ref:%u dec\n", this, mRefcnt);
        if (mRefcnt <= 0)
        {
            //printf("PyRep:0x%p | deleted\n", this);
            delete this;
        }
    }
protected:
    const PyType m_type;
    size_t mRefcnt;
};

/**
 * @brief Python integer.
 *
 * Class representing 32-bit integer.
 * Longer integers may be stored in PyLong.
 */
class PyInt : public PyRep
{
public:
    PyInt(const int32 i);
	PyInt(const PyInt &oth);
    virtual ~PyInt();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    int32 GetValue() const { return value; }
    int32 hash() const;

//private:
    int32 value;
};

/**
 * @brief Python long integer.
 *
 * Class representing 64-bit integer.
 */
class PyLong : public PyRep
{
public:
    PyLong(const int64 i);
	PyLong(const PyLong &oth);
    virtual ~PyLong();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    int64 GetValue() { return value; }
    int32 hash() const;

//private:
    int64 value;
};

/**
 * @brief Python floating point number.
 *
 * Class representing floating point number.
 */
class PyFloat : public PyRep
{
public:
    PyFloat(const double &i);
	PyFloat(const PyFloat &oth);
    virtual ~PyFloat();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    double GetValue() { return value; }
    int32 hash() const;

//private:
    double value;
};

/**
 * @brief Python boolean.
 *
 * Class represeting boolean.
 */
class PyBool : public PyRep
{
public:
    PyBool(bool i);
	PyBool(const PyBool &oth);
    virtual ~PyBool();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

//private:
    bool value;
};

/**
 * @brief Python's "none".
 *
 * Class which represents Python's "nothing".
 */
class PyNone : public PyRep
{
public:
    PyNone();
    virtual ~PyNone();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    int32 hash() const;
};

/**
 * @brief Python buffer.
 *
 * Usual binary buffer. Conversion to PyString & vice versa is
 * implemented.
 * This buffer also has immutable size; once allocated, it cannot
 * be changed.
 */
class PyBuffer : public PyRep
{
public:
    PyBuffer(const uint8 *buffer, size_t length);
    PyBuffer(uint8 **buffer, size_t length);
    PyBuffer(size_t length);
    PyBuffer(const PyString &str);
	PyBuffer(const PyBuffer &oth);
    virtual ~PyBuffer();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    /**
     * @brief Get the PyBuffer content
     *
     * @return the pointer to the PyBuffer content
     */
    uint8* content() { return m_value; }
    /**
     * @brief Get the const PyBuffer content
     *
     * @return the pointer to the const PyBuffer content
     */
    const uint8* content() const { return m_value; }

	/**
	 * @brief Overload of operator[] for indexed access.
	 *
	 * @param[in] index Index of byte.
	 * @return Byte in PyBuffer content with specified index.
	 */
	uint8& operator[](size_t index) { assert( index < size() ); return content()[ index ]; }
	/**
	 * @brief Overload of operator[] for const indexed access.
	 *
	 * @param[in] index Index of byte.
	 * @return Const byte in PyBuffer content with specified index.
	 */
	const uint8& operator[](size_t index) const { assert( index < size() ); return content()[ index ]; }

    /**
     * @brief Get the PyBuffer size.
     *
     * @return return the size of the buffer.
     */
    size_t size() const { return m_length; }

    PySubStream *CreateSubStream() const;
    int32 hash() const;

protected:
    uint8 *const m_value;
    const size_t m_length;

	mutable int32 m_hash_cache;
};

/**
 * @brief Python string.
 *
 * Usual string. Unlike Python's string, our one is mutable.
 */
class PyString : public PyRep
{
public:
    PyString(const char *str = "", bool type_1=false);
    PyString(const std::string &str, bool type_1=false);
	PyString(const PyBuffer &buf, bool type_1=false); //to deal with non-string buffers poorly placed in strings (CCP)
	PyString(const PyString &oth);
    virtual ~PyString();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    /**
     * @brief Get the PyString content
     *
     * @return the pointer to the char* array.
     */
    const char* content() const { return m_value.c_str(); }

	/**
	 * @brief Overload of operator[] to allow indexed access.
	 *
	 * @param[in] index Index of character to return.
	 * @return Const character with given index.
	 */
	const char& operator[](size_t index) const { return m_value[ index ]; }

    /**
     * @brief Get the length of the PyString
     *
     * @return the length of the string as size_t
     */
    size_t size() const { return m_value.size(); }

	/**
	 * @brief Checks if string is empty or not.
	 *
	 * @return True if string is empty, false if not.
	 */
	bool empty() const { return m_value.empty(); }

	/**
	 * @brief Checks if string is of type 1.
	 *
	 * @return True if string is of type 1, false if not.
	 */
	bool isType1() const { return m_is_type_1; }

	/**
	 * @brief Overload of operator== for easier and more performant (hash) comparisons.
	 *
	 * @param[in] oth The other string for comparison.
	 * @return True if strings are equal, false otherwise.
	 */
	bool operator==(const PyString &oth) const { return ( hash() == oth.hash() ); }

	/**
	 * @brief Assigment operator.
	 *
	 * This needs to be defined by hand because of const m_is_type_1.
	 * Once is this member removed, this function may also be
	 * removed, passing its job to compiler-generated one.
	 *
	 * @param[in] oth String content of which should be assigned.
	 * @return Itself.
	 */
	PyString& operator=(const PyString &oth);

    int32 hash() const;

protected:
    std::string m_value;
    const bool m_is_type_1; //true if this is an Op_PyByteString instead of the default Op_PyByteString2

    mutable int32 m_hash_cache;
};

/**
 * @brief Python tuple.
 *
 * Tuple. Unlike Python's, this one is mutable.
 */
class PyTuple : public PyRep
{
public:
    typedef std::vector<PyRep *>            storage_type;
    typedef storage_type::iterator          iterator;
    typedef storage_type::const_iterator    const_iterator;

    PyTuple(size_t item_count);
	PyTuple(const PyTuple& oth);
    virtual ~PyTuple();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    const_iterator begin() const { return items.begin(); }
    const_iterator end() const { return items.end(); }

	size_t size() const { return items.size(); }
    bool empty() const { return items.empty(); }
    void clear();

    void SetItem( size_t index, PyRep* object )
	{
		PyRep*& rep = items.at( index );

		PySafeDecRef( rep );
		rep = object;
	}

    /**
     * @brief overloading the [] operator for reference lookup.
     *
     * @param[in] index the position of the required object.
     * @return The pointer to the indexed object.
     */
    PyRep* operator[]( size_t index ) const { return items.at( index ); }

	/**
	 * @brief Overload of assigment operator to handle object ownership.
	 *
	 * @param[in] oth Tuple the content of which is to be copied.
	 * @return Itself.
	 */
	PyTuple& operator=(const PyTuple& oth);

    int32 hash() const;

	// This needs to be public for now.
    storage_type items;
};

/**
 * @brief Python list.
 *
 * Python's list, compeletely mutable.
 */
class PyList : public PyRep
{
public:
    typedef std::vector<PyRep *>            storage_type;
    typedef storage_type::iterator          iterator;
    typedef storage_type::const_iterator    const_iterator;

    PyList(size_t item_count = 0);
	PyList(const PyList& oth);
    virtual ~PyList();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    const_iterator begin() const { return items.begin(); }
    const_iterator end() const { return items.end(); }

    size_t size() const { return items.size(); }
    bool empty() const { return items.empty(); }
    void clear();

    void add(PyRep* i) { items.push_back( i ); }

    void addInt(int32 intval) { add( new PyInt( intval ) ); }
    void addLong(int64 intval) { add( new PyLong( intval ) ); }
    void addReal(double realval) { add( new PyFloat( realval ) ); }
    void addStr(const char* str) { add( new PyString( str ) ); }

    void SetItem(size_t index, PyRep* i)
	{
		PyRep*& rep = items.at( index );

		PySafeDecRef( rep );
		rep = i;
    }

    void SetItemString(size_t index, const char* str) { SetItem( index, new PyString( str ) ); }

	/**
	 * @brief Overload of operator[] for easier access.
	 *
	 * @param[in] index Index of required object.
	 * @return Pointer to object.
	 */
	PyRep* operator[]( size_t index ) const { return items.at( index ); }

	/**
	 * @brief Overload of assigment operator to handle object ownership.
	 *
	 * @param[in] oth List the content of which is to be copied.
	 * @return Itself.
	 */
	PyList& operator=(const PyList& oth);

	// This needs to be public:
    storage_type items;
};

/**
 * @brief Python's dictionary.
 *
 * Dictionary; completely mutable associative container.
 */
class PyDict : public PyRep
{
protected:
    class _hasher
    : public unary_function<PyRep*, size_t>
    {	// hash functor
    public:
        size_t operator()(const PyRep* _Keyval) const
        {
			assert( _Keyval );

            return (size_t)_Keyval->hash();
        }
    };

public:
    typedef std::tr1::unordered_map<PyRep*, PyRep*, _hasher>        storage_type;
    typedef storage_type::iterator                                  iterator;
    typedef storage_type::const_iterator                            const_iterator;

    PyDict();
	PyDict(const PyDict& oth);
    virtual ~PyDict();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    const_iterator begin() const { return items.begin(); }
    const_iterator end() const { return items.end(); }

    size_t size() const { return items.size(); }
    bool empty() const { return items.empty(); }
    void clear();

    /**
     * \brief SetItem adds or sets a database entry.
     *
     * PyDict::SetItem handles the adding and setting of object in
     * mapped and non mapped python dictionary's.
     *
     * @param[in] key contains the key object which the value needs to be filed under.
     * @param[in] value is the object that needs to be filed under key.
     */
    void SetItem( PyRep* key, PyRep* value );

    /**
     * \brief SetItemString adds or sets a database entry.
     *
     * PyDict::SetItemString handles the adding and setting of object in
     * mapped and non mapped python dictionary's.
     *
     * @param[in] key contains the key string which the value needs to be filed under.
     * @param[in] value is the object that needs to be filed under key.
     */
	void SetItemString( const char* key, PyRep* value ) { SetItem( new PyString( key ), value ); }

	/**
	 * @brief Overload of assigment operator to handle object ownership.
	 *
	 * @param[in] oth PyDict the content of which is to be copied.
	 * @return Itself.
	 */
	PyDict& operator=(const PyDict& oth);

    storage_type items;
};

/**
 * @brief Python object.
 *
 * Consists of typename (string) and arguments,
 * which is usually PyTuple or PyDict.
 */
class PyObject : public PyRep
{
public:
    PyObject(const char* _type = "", PyRep* _args = NULL);
	PyObject(const PyObject& oth);
    virtual ~PyObject();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

	/**
	 * @brief Assigment operator, handles arguments ownership.
	 *
	 * @param[in] oth PyObject the content of which should be copied.
	 * @return Itself.
	 */
	PyObject& operator=(const PyObject& oth);

    std::string type;
    PyRep* arguments;
};

/**
 * @brief Python extended object.
 *
 * This represents some kind of extended Python object.
 * Used for opcodes 0x22 (Type1) and 0x23 (Type2),
 * since the only difference is in header.
 */
class PyObjectEx : public PyRep
{
public:
    typedef PyList                          list_type;
    typedef list_type::iterator             list_iterator;
    typedef list_type::const_iterator       const_list_iterator;

    typedef PyDict                          dict_type;
    typedef dict_type::iterator             dict_iterator;
    typedef dict_type::const_iterator       const_dict_iterator;

    PyObjectEx(bool _is_type_2, PyRep* _header = NULL);
	PyObjectEx(const PyObjectEx& oth);
    virtual ~PyObjectEx();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

	/**
	 * @brief Assigment operator to handle ownership things.
	 *
	 * @param[in] oth PyObjectEx the content of which should be copied.
	 * @return Itself.
	 */
	PyObjectEx& operator=(const PyObjectEx& oth);

    PyRep* header;
    const bool is_type_2;

    list_type list_data;
    dict_type dict_data;
};

/**
 * \brief Wrapper class for PyObjectEx of type 1.
 *
 * \author Bloody.Rabbit
 */
class PyObjectEx_Type1
: public PyObjectEx
{
public:
	PyObjectEx_Type1(const char *type, PyTuple *args, PyDict *keywords);

	PyString &GetType() const;
	PyTuple &GetArgs() const;
	PyDict &GetKeywords() const;
	PyRep *FindKeyword(const char *keyword) const;

protected:
	static PyTuple *_CreateHeader(const char *type, PyTuple *args, PyDict *keywords);
};

/**
 * \brief Wrapper class for PyObjectEx of type 2.
 *
 * \author Bloody.Rabbit
 */
class PyObjectEx_Type2
: public PyObjectEx
{
public:
	PyObjectEx_Type2(PyTuple *args, PyDict *keywords);

	PyTuple &GetArgs() const;
	PyDict &GetKeywords() const;
	PyRep *FindKeyword(const char *keyword) const;

protected:
	static PyTuple *_CreateHeader(PyTuple *args, PyDict *keywords);
};

class PyPackedRow : public PyRep {
public:
    PyPackedRow(DBRowDescriptor &header, bool header_owner);
    virtual ~PyPackedRow();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitPackedRow(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitPackedRow(this, lvl);
    }

    PyPackedRow *TypedClone() const;
    void CloneFrom(const PyPackedRow *from);

    // Header:
    DBRowDescriptor &GetHeader() const { return mHeader; }
    bool IsHeaderOwner() const { return mHeaderOwner; }

    // Fields:
    PyRep *GetField(uint32 index) const { return mFields.at( index ); }

    bool SetField(uint32 index, PyRep *value);
    bool SetField(const char *colName, PyRep *value);

    int32 hash() const;

protected:
    DBRowDescriptor &mHeader;
    const bool mHeaderOwner;

    std::vector<PyRep *> mFields;
};

class PySubStruct : public PyRep {
public:
    PySubStruct(PyRep *t = NULL) : PyRep(PyRep::PyTypeSubStruct), sub(t) {}
    virtual ~PySubStruct();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitSubStruct(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitSubStruct(this, lvl);
    }

    PySubStruct *TypedClone() const;

    PyRep *sub;
};

class PySubStream : public PyRep {
public:
    PySubStream() : PyRep(PyRep::PyTypeSubStream), length(0), data(NULL), decoded(NULL) {}
    PySubStream(PyRep *t) : PyRep(PyRep::PyTypeSubStream), length(0), data(NULL), decoded(t) {}
    PySubStream(const uint8 *buffer, uint32 len);
    virtual ~PySubStream();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitSubStream(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitSubStream(this, lvl);
    }

    PySubStream *TypedClone() const;

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

class PyChecksumedStream : public PyRep {
public:
    PyChecksumedStream() : PyRep(PyRep::PyTypeChecksumedStream), checksum(0), stream(NULL) {}
    PyChecksumedStream(uint32 sum, PyRep *t) : PyRep(PyRep::PyTypeChecksumedStream), checksum(sum), stream(t) {}
    virtual ~PyChecksumedStream();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    EVEMU_INLINE PyRep *Clone() const { return(TypedClone()); }
    EVEMU_INLINE void visit(PyVisitor *v) const {
        v->VisitChecksumedStream(this);
    }
    EVEMU_INLINE void visit(PyVisitorLvl *v, int64 lvl) const {
        v->VisitChecksumedStream(this, lvl);
    }

    PyChecksumedStream *TypedClone() const;

    uint32 checksum;
    PyRep *stream;
};

/* note: this will decrease mem use with 50% but increase load time with 50%
 * enabling this would have to wait until references work properly.
 */
//#pragma pack(pop)

#endif//EVE_PY_REP_H
