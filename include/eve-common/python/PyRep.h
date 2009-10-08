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

/* note: python object behavior tracing.
 */
//#define PY_OBJ_TRACE
#ifdef PY_OBJ_TRACE
#  define _py_obj_trace(str, ...) printf(str, __VA_ARGS__)
#else
#  define _py_obj_trace(str, ...) do {} while(0)
#endif//PY_OBJ_TRACE

/* note: this will decrease memory use with 50% but increase load time with 50%
 * enabling this would have to wait until references work properly. Or when
 * you operate the server using the cache store system this can also be enabled.
 * note: this will have influence on the overall server performance.
 */
//#pragma pack(push,1)

class PyInt;
class PyLong;
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

class PyVisitor;
class PyVisitorLvl;
class DBRowDescriptor;

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
    //using this method is discouraged, it generally means your doing something wrong... CheckType() should cover almost all needs
    PyType GetType() const          { return mType; }

    bool IsInt() const              { return mType == PyTypeInt; }
    bool IsLong() const             { return mType == PyTypeLong; }
    bool IsFloat() const            { return mType == PyTypeFloat; }
    bool IsBool() const             { return mType == PyTypeBool; }
    bool IsBuffer() const           { return mType == PyTypeBuffer; }
    bool IsString() const           { return mType == PyTypeString; }
    bool IsTuple() const            { return mType == PyTypeTuple; }
    bool IsList() const             { return mType == PyTypeList; }
    bool IsDict() const             { return mType == PyTypeDict; }
    bool IsNone() const             { return mType == PyTypeNone; }
    bool IsSubStruct() const        { return mType == PyTypeSubStruct; }
    bool IsSubStream() const        { return mType == PyTypeSubStream; }
    bool IsChecksumedStream() const { return mType == PyTypeChecksumedStream; }
    bool IsObject() const           { return mType == PyTypeObject; }
    bool IsObjectEx() const         { return mType == PyTypeObjectEx; }
    bool IsPackedRow() const        { return mType == PyTypePackedRow; }

    const char* TypeString() const;

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

    void IncRef() const
    {
        ++mRefCnt;
        _py_obj_trace( "PyRep:0x%p | ref:%u inc\n", this, mRefCnt );
    }

    void DecRef() const
    {
        --mRefCnt;
        _py_obj_trace( "PyRep:0x%p | ref:%u dec\n", this, mRefCnt );

        if( mRefCnt <= 0 )
        {
            _py_obj_trace( "PyRep:0x%p | deleted\n", this );
            delete this;
        }
    }

protected:
    const PyType mType;

    mutable size_t mRefCnt;

    /** Lookup table for PyRep type object type names. */
    static const char* s_mTypeString[];
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

    int32 value() const { return mValue; }

    int32 hash() const;

protected:
    const int32 mValue;
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

    int64 value() const { return mValue; }

    int32 hash() const;

protected:
    const int64 mValue;
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

    double value() const { return mValue; }
    int32 hash() const;

protected:
    const double mValue;
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

	bool value() const { return mValue; }

protected:
    const bool mValue;
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
    PyBuffer(const uint8* buffer, size_t size);
    PyBuffer(uint8** buffer, size_t size);
    PyBuffer(size_t size);
    PyBuffer(const PyString& str);
    PyBuffer(const PyBuffer& oth);
    virtual ~PyBuffer();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    /**
     * @brief Get the const PyBuffer content
     *
     * @return the pointer to the const PyBuffer content
     */
    const uint8* content() const { return mValue; }
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
    size_t size() const { return mSize; }

    int32 hash() const;

protected:
    uint8* const mValue;
    const size_t mSize;

    mutable int32 mHashCache;
};

/**
 * @brief Python string.
 *
 * Usual string.
 */
class PyString : public PyRep
{
public:
    PyString(const char* str, bool type_1=false);
    PyString(const std::string& str, bool type_1=false);
    PyString(const PyBuffer& buf, bool type_1=false); //to deal with non-string buffers poorly placed in strings (CCP)
    PyString(const PyString& oth);
    virtual ~PyString();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    /**
     * @brief Get the PyString content
     *
     * @return the std::string reference.
     */
    const std::string& content() const { return mValue; }

    /**
     * @brief Checks if string is of type 1.
     *
     * @return True if string is of type 1, false if not.
     */
    bool isType1() const { return mIsType1; }

    int32 hash() const;

protected:
    const std::string mValue;
    const bool mIsType1; //true if this is an Op_PyByteString instead of the default Op_PyByteString2

    mutable int32 mHashCache;
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

    /**
     * @brief Returns Python object.
     *
     * @param[in] index Index at which is the desired object.
     *
     * @return Python object.
     */
    PyRep* GetItem( size_t index ) const { return items.at( index ); }

    /**
     * @brief Stores Python object.
     *
     * @param[in] index  Index at which the object should be stored.
     * @param[in] object Object to be stored.
     */
    void SetItem( size_t index, PyRep* object )
    {
        PyRep** rep = &items.at( index );

        PySafeDecRef( *rep );
        *rep = object;
    }

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
 * Python's list, completely mutable.
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

    /**
     * @brief Returns Python object.
     *
     * @param[in] index Index at which is the desired object.
     *
     * @return Python object.
     */
    PyRep* GetItem( size_t index ) const { return items.at( index ); }

    /**
     * @brief Stores Python object.
     *
     * @param[in] index  Index at which the object should be stored.
     * @param[in] object Object to be stored.
     */
    void SetItem( size_t index, PyRep* object )
    {
        PyRep** rep = &items.at( index );

        PySafeDecRef( *rep );
        *rep = object;
    }
    /**
     * @brief Stores Python string.
     *
     * @param[in] index Index at which the object should be stored.
     * @param[in] str   String to be stored.
     */
    void SetItemString( size_t index, const char* str ) { SetItem( index, new PyString( str ) ); }

    void AddItem(PyRep* i) { items.push_back( i ); }
    void AddItemInt(int32 intval) { AddItem( new PyInt( intval ) ); }
    void AddItemLong(int64 intval) { AddItem( new PyLong( intval ) ); }
    void AddItemReal(double realval) { AddItem( new PyFloat( realval ) ); }
    void AddItemString(const char* str) { AddItem( new PyString( str ) ); }

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
    class _hash
    : public std::unary_function<PyRep*, size_t>
    {	// hash functor
    public:
        size_t operator()( const PyRep* _Keyval ) const
        {
			assert( _Keyval );

            return (size_t)_Keyval->hash();
        }
    };

    class _comp
    : public std::binary_function<PyRep*, PyRep*, bool>
    {   // comparison functor
    public:
        bool operator()( const PyRep* _Arg1, const PyRep* _Arg2 ) const
        {
            assert( _Arg1 );
            assert( _Arg2 );

            // Suffers from hash collision but whatever ... still
            // better than raw pointer comparison
            return ( _Arg1->hash() == _Arg2->hash() );
        }
    };

public:
    typedef std::tr1::unordered_map<PyRep*, PyRep*, _hash, _comp>   storage_type;
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
     * @brief Obtains a database entry based on given key object.
     *
     * @param[in] key is the key object of the database entry.
     *
     * @return Desired database entry.
     */
    PyRep* GetItem( PyRep* key ) const;
    /**
     * @brief Obtains database entry based on given key string.
     *
     * @param[in] key is the key string of the database entry.
     *
     * @return Desired database entry.
     */
    PyRep* GetItemString( const char* key ) const;

    /**
     * @brief SetItem adds or sets a database entry.
     *
     * PyDict::SetItem handles the adding and setting of object in
     * mapped and non mapped python dictionary's.
     *
     * @param[in] key contains the key object which the value needs to be filed under.
     * @param[in] value is the object that needs to be filed under key.
     */
    void SetItem( PyRep* key, PyRep* value );
    /**
     * @brief SetItemString adds or sets a database entry.
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
    PyObject(PyString* type, PyRep* args);
    PyObject(const PyObject& oth);
    virtual ~PyObject();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    PyString* type() const { return mType; }
    PyRep* arguments() const { return mArguments; }

protected:
    PyString* const mType;
    PyRep* const mArguments;
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

    PyObjectEx(bool is_type_2, PyRep* header);
    PyObjectEx(const PyObjectEx& oth);
    virtual ~PyObjectEx();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    PyRep* header() const { return mHeader; }
    bool isType2() const { return mIsType2; }

    list_type& list() { return *mList; }
    const list_type& list() const { return *mList; }

    dict_type& dict() { return *mDict; }
    const dict_type& dict() const { return *mDict; }

    /**
     * @brief Assigment operator to handle ownership things.
     *
     * @param[in] oth PyObjectEx the content of which should be copied.
     * @return Itself.
     */
    PyObjectEx& operator=(const PyObjectEx& oth);

protected:
    PyRep* const mHeader;
    const bool mIsType2;

    list_type* const mList;
    dict_type* const mDict;
};

/**
 * @brief Wrapper class for PyObjectEx of type 1.
 *
 * @author Bloody.Rabbit
 */
class PyObjectEx_Type1
: public PyObjectEx
{
public:
    PyObjectEx_Type1(const char* type, PyTuple* args, PyDict* keywords);

    PyString& GetType() const
    {
        assert( header() );
        return header()->AsTuple().GetItem( 0 )->AsString();
    }
    PyTuple& GetArgs() const
    {
        assert( header() );
        return header()->AsTuple().GetItem( 1 )->AsTuple();
    }
    PyDict& GetKeywords() const;

    PyRep* FindKeyword(const char* keyword) const;

protected:
    static PyTuple* _CreateHeader(const char* type, PyTuple* args, PyDict* keywords);
};

/**
 * @brief Wrapper class for PyObjectEx of type 2.
 *
 * @author Bloody.Rabbit
 */
class PyObjectEx_Type2
: public PyObjectEx
{
public:
    PyObjectEx_Type2(PyTuple* args, PyDict* keywords);

    PyTuple& GetArgs() const
    {
        assert( header() );
        return header()->AsTuple().GetItem( 0 )->AsTuple();
    }
    PyDict& GetKeywords() const
    {
        assert( header() );
        return header()->AsTuple().GetItem( 1 )->AsDict();
    }

    PyRep* FindKeyword(const char* keyword) const;

protected:
    static PyTuple* _CreateHeader(PyTuple* args, PyDict* keywords);
};

/**
 * @brief Packed row.
 *
 * Special row which packs all its values
 * with zero-compression algorithm.
 */
class PyPackedRow : public PyRep
{
public:
    typedef PyList                          storage_type;
    typedef storage_type::iterator          iterator;
    typedef storage_type::const_iterator    const_iterator;

    PyPackedRow(DBRowDescriptor* header);
    PyPackedRow(const PyPackedRow& oth);
    virtual ~PyPackedRow();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    // Header:
    DBRowDescriptor& header() const { return *mHeader; }

    // Fields:
    const_iterator begin() const { return mFields->begin(); }
    const_iterator end() const { return mFields->end(); }
    void clear() { mFields->clear(); }

    PyRep* GetField( size_t index ) const { return mFields->GetItem( index ); }

    bool SetField(uint32 index, PyRep* value);
    bool SetField(const char* colName, PyRep* value);

    /**
     * @brief Assigment operator to handle ownership things.
     *
     * @param[in] oth PyPackedRow the content of which should be copied.
     * @return Itself.
     */
    PyPackedRow& operator=(const PyPackedRow& oth);

    int32 hash() const;

protected:
    DBRowDescriptor* const mHeader;
    storage_type* const mFields;
};

class PySubStruct : public PyRep
{
public:
    PySubStruct( PyRep *t );
	PySubStruct( const PySubStruct& oth );
    virtual ~PySubStruct();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    PyRep* sub() const { return mSub; }

protected:
    PyRep* const mSub;
};

class PySubStream : public PyRep
{
public:
    PySubStream(PyRep* rep);
    PySubStream(PyBuffer* buffer);
	PySubStream(const PySubStream& oth);
    virtual ~PySubStream();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    PyBuffer* data() const { return mData; }
    PyRep* decoded() const { return mDecoded; }

    //call to ensure that `data` represents `decoded` IF DATA IS NULL
    void EncodeData() const;

    //call to ensure that `decoded` represents `data`
    void DecodeData() const;

protected:
    //if both are non-NULL, they are considered to be equivalent
    mutable PyBuffer* mData;
    mutable PyRep* mDecoded;
};

class PyChecksumedStream : public PyRep
{
public:
    PyChecksumedStream(PyRep* t, uint32 sum);
	PyChecksumedStream(const PyChecksumedStream& oth);
    virtual ~PyChecksumedStream();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    PyRep *Clone() const;
    void visit(PyVisitor *v) const;
    void visit(PyVisitorLvl *v, int64 lvl) const;

    PyRep* stream() const { return mStream; }
    uint32 checksum() const { return mChecksum; }

protected:
    PyRep* const mStream;
    const uint32 mChecksum;
};

/* note: this will decrease memory use with 50% but increase load time with 50%
 * enabling this would have to wait until references work properly.
 */
//#pragma pack(pop)

#endif//EVE_PY_REP_H
