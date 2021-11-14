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
    Author:     Zhur
    Updates:    Allan
*/

#ifndef EVE_PY_REP_H
#define EVE_PY_REP_H

#include "../../eve-core/eve-core.h"
#include "../../eve-core/memory/RefPtr.h"

class PyInt;
class PyLong;
class PyFloat;
class PyBool;
class PyBuffer;
class PyString;
class PyWString;
class PyToken;
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
class DBRowDescriptor;

/**
 * debug macros to ease the increase and decrease of references of a object
 * using this also increases the possibility of debugging it.
 */
#define PyIncRef(op) (op)->IncRef()
#define PyDecRef(op) (op)->DecRef()

/* Macros to use in case the object pointer may be NULL */
#define PySafeIncRef(op) if (op != nullptr) PyIncRef( op )
#define PySafeDecRef(op) if (op != nullptr) PyDecRef( op )

/**
 * @brief Base Python wire object
 */
class PyRep : public RefObject
{
public:
    /**
     * @brief Python wire object types
     */
    enum PyType
    {
        PyTypeMin               = 0,
        PyTypeInt               = 1,
        PyTypeLong              = 2,
        PyTypeFloat             = 3,
        PyTypeBool              = 4,
        PyTypeBuffer            = 5,
        PyTypeString            = 6,
        PyTypeWString           = 7,
        PyTypeToken             = 8,
        PyTypeTuple             = 9,
        PyTypeList              = 10,
        PyTypeDict              = 11,
        PyTypeNone              = 12,
        PyTypeSubStruct         = 13,
        PyTypeSubStream         = 14,
        PyTypeChecksumedStream  = 15,
        PyTypeObject            = 16,
        PyTypeObjectEx          = 17,
        PyTypePackedRow         = 18,
        PyTypeError             = 19
    };

    /* PyType functions */

    PyType GetType() const          { return mType; }

    bool IsInt() const              { return mType == PyTypeInt; }
    bool IsLong() const             { return mType == PyTypeLong; }
    bool IsFloat() const            { return mType == PyTypeFloat; }
    bool IsBool() const             { return mType == PyTypeBool; }
    bool IsBuffer() const           { return mType == PyTypeBuffer; }
    bool IsString() const           { return mType == PyTypeString; }
    bool IsWString() const          { return mType == PyTypeWString; }
    bool IsToken() const            { return mType == PyTypeToken; }
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
    PyInt* AsInt()                                       { assert( IsInt() ); return (PyInt*)this; }
    const PyInt* AsInt() const                           { assert( IsInt() ); return (const PyInt*)this; }
    PyLong* AsLong()                                     { assert( IsLong() ); return (PyLong*)this; }
    const PyLong* AsLong() const                         { assert( IsLong() ); return (const PyLong*)this; }
    PyFloat* AsFloat()                                   { assert( IsFloat() ); return (PyFloat*)this; }
    const PyFloat* AsFloat() const                       { assert( IsFloat() ); return (const PyFloat*)this; }
    PyBool* AsBool()                                     { assert( IsBool() ); return (PyBool*)this; }
    const PyBool* AsBool() const                         { assert( IsBool() ); return (const PyBool*)this; }
    PyBuffer* AsBuffer()                                 { assert( IsBuffer() ); return (PyBuffer*)this; }
    const PyBuffer* AsBuffer() const                     { assert( IsBuffer() ); return (const PyBuffer*)this; }
    PyString* AsString()                                 { assert( IsString() ); return (PyString*)this; }
    const PyString* AsString() const                     { assert( IsString() ); return (const PyString*)this; }
    PyWString* AsWString()                               { assert( IsWString() ); return (PyWString*)this; }
    const PyWString* AsWString() const                   { assert( IsWString() ); return (const PyWString*)this; }
    PyToken* AsToken()                                   { assert( IsToken() ); return (PyToken*)this; }
    const PyToken* AsToken() const                       { assert( IsToken() ); return (const PyToken*)this; }
    PyTuple* AsTuple()                                   { assert( IsTuple() ); return (PyTuple*)this; }
    const PyTuple* AsTuple() const                       { assert( IsTuple() ); return (const PyTuple*)this; }
    PyList* AsList()                                     { assert( IsList() ); return (PyList*)this; }
    const PyList* AsList() const                         { assert( IsList() ); return (const PyList*)this; }
    PyDict* AsDict()                                     { assert( IsDict() ); return (PyDict*)this; }
    const PyDict* AsDict() const                         { assert( IsDict() ); return (const PyDict*)this; }
    PyNone* AsNone()                                     { assert( IsNone() ); return (PyNone*)this; }
    const PyNone* AsNone() const                         { assert( IsNone() ); return (const PyNone*)this; }
    PySubStruct* AsSubStruct()                           { assert( IsSubStruct() ); return (PySubStruct*)this; }
    const PySubStruct* AsSubStruct() const               { assert( IsSubStruct() ); return (const PySubStruct*)this; }
    PySubStream* AsSubStream()                           { assert( IsSubStream() ); return (PySubStream*)this; }
    const PySubStream* AsSubStream() const               { assert( IsSubStream() ); return (const PySubStream*)this; }
    PyChecksumedStream* AsChecksumedStream()             { assert( IsChecksumedStream() ); return (PyChecksumedStream*)this; }
    const PyChecksumedStream* AsChecksumedStream() const { assert( IsChecksumedStream() ); return (const PyChecksumedStream*)this; }
    PyObject* AsObject()                                 { assert( IsObject() ); return (PyObject*)this; }
    const PyObject* AsObject() const                     { assert( IsObject() ); return (const PyObject*)this; }
    PyObjectEx* AsObjectEx()                             { assert( IsObjectEx() ); return (PyObjectEx*)this; }
    const PyObjectEx* AsObjectEx() const                 { assert( IsObjectEx() ); return (const PyObjectEx*)this; }
    PyPackedRow* AsPackedRow()                           { assert( IsPackedRow() ); return (PyPackedRow*)this; }
    const PyPackedRow* AsPackedRow() const               { assert( IsPackedRow() ); return (const PyPackedRow*)this; }

    using RefObject::IncRef;
    using RefObject::DecRef;
    //using RefObject::GetCount();
    //using RefObject::IsDeleted();

    PyRep(PyType t);
    // copy c'tor
    PyRep(const PyRep& oth );
    // move c'tor
    PyRep(PyRep&& oth) =delete;
    /*   : PyRep(oth.mType) {
     *        sLog.Cyan("PyRep()", "Calling Move C'tor.");
     *        std::swap(*this, oth);
     *        PyDecRef(&oth);
     * }
     */
    // copy assignment
    PyRep& operator= (const PyRep& oth) = default;
    // move assignment
    PyRep& operator= (PyRep&& oth) = default;


    /**
     * @brief Dumps object to file.
     *
     * @param[in] into File into which dump should be written.
     * @param[in] pfx  Prefix which is put in front of each line.
     */
    void Dump( FILE* into, const char* pfx ) const;
    /**
     * @brief Dumps object to console.
     *
     * @param[in] type Log type to use for dump.
     * @param[in] pfx  Prefix which is put in front of each line.
     */
    void Dump( LogType type, const char* pfx ) const;

    /**
     * @brief Clones object.
     *
     * @return Identical copy of object.
     */
    virtual PyRep* Clone() const;
    /**
     * @brief Visits object.
     *
     * @param[in] v Visitor to be used for visiting.
     *
     * @retval true  Visit successful.
     * @retval false Error during visit.
     */
    virtual bool visit( PyVisitor& v ) const;
    /**
     * @brief virtual function to generate a hash value of a object.
     *
     * virtual function to generate a hash value of a object to facilitate the various maps and checks.
     *
     * @return returns a uint32 containing a hash function that represents the object.
     */
    virtual int32 hash() const;

    // this is used when PyRep can be either String or WString.
    static std::string StringContent(PyRep* pRep);
    // this is used when PyRep can be either Int or Long
    // None returns 0, Float is converted to int64
    static int64 IntegerValue(PyRep* pRep);
    // this is used when PyRep can be either Int or Long
    // None returns 0. Returned as unsigned 32b int
    static uint32 IntegerValueU32(PyRep* pRep);

protected:
    virtual ~PyRep();
    const PyType mType;
};

/**
 * @brief Python integer.
 *
 * Class representing 32-bit signed integer.
 */
class PyInt : public PyRep
{
public:
    // default c'tor
    PyInt( const int32 i );
    // copy c'tor
    PyInt( const PyInt& oth );
    // move c'tor
    PyInt(PyInt&& oth) = delete;
    // copy assignment
    PyInt& operator= (const PyInt& oth) = delete;
    // move assignment
    PyInt& operator= (PyInt&& oth) = delete;

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    int32 value() const { return mValue; }

    int32 hash() const;

    virtual ~PyInt()    { /* do we need to do anything here? */ }
protected:
    const int32 mValue;
};

/**
 * @brief Python long integer.
 *
 * Class representing 64-bit signed integer.
 */
class PyLong : public PyRep
{
public:
    // default c'tor
    PyLong( const int64 i );
    // copy c'tor
    PyLong( const PyLong& oth );
    // move c'tor
    PyLong(PyLong&& oth) = delete;
    // copy assignment
    PyLong& operator= (const PyLong& oth) = delete;
    // move assignment
    PyLong& operator= (PyLong&& oth) = delete;


    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    int64 value() const { return mValue; }

    int32 hash() const;

    virtual ~PyLong()    { /* do we need to do anything here? */ }
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
    // default c'tor
    PyFloat( const double& i );
    // copy c'tor
    PyFloat( const PyFloat& oth );
    // move c'tor
    PyFloat(PyFloat&& oth) = delete;
    // copy assignment
    PyFloat& operator= (const PyFloat& oth) = delete;
    // move assignment
    PyFloat& operator= (PyFloat&& oth) = delete;


    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    double value() const { return mValue; }

    int32 hash() const;

    virtual ~PyFloat()    { /* do we need to do anything here? */ }
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
    // default c'tor
    PyBool( bool i );
    // copy c'tor
    PyBool( const PyBool& oth );
    // move c'tor
    PyBool(PyBool&& oth) = delete;
    // copy assignment
    PyBool& operator= (const PyBool& oth) = delete;
    // move assignment
    PyBool& operator= (PyBool&& oth) = delete;


    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    bool value() const { return mValue; }

    virtual ~PyBool()    { /* do we need to do anything here? */ }
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
    // default c'tor
    PyNone();
    // copy c'tor
    PyNone( const PyNone& oth );
    // move c'tor
    PyNone(PyNone&& oth) = delete;
    // copy assignment
    PyNone& operator= (const PyNone& oth) = delete;
    // move assignment
    PyNone& operator= (PyNone&& oth) = delete;


    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    int32 hash() const;

    virtual ~PyNone()    { /* do we need to do anything here? */ }
};

/**
 * @brief Python buffer.
 *
 * Usual binary buffer.
 * This buffer has immutable content; once allocated, it cannot
 * be changed.
 */
class PyBuffer : public PyRep
{
public:
    /** Calls Buffer::Buffer( size_t, const uint8& ). */
    PyBuffer( size_t len, const uint8& value );
    /** Calls Buffer::Buffer( Iter, Iter ). */
    template<typename Iter>
    PyBuffer( Iter first, Iter last );
    /** Calls Buffer::Buffer( const Buffer& ). */
    PyBuffer( const Buffer& buffer );
    /** Takes ownership of a passed Buffer. */
    PyBuffer( Buffer** buffer );
    /** Copy constructor. */
    PyBuffer( const PyString& str );
    /** Copy constructor. */
    PyBuffer( const PyBuffer& oth );

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    /**
     * @brief Get the const PyBuffer content
     *
     * @return const PyBuffer content
     */
    const Buffer& content() const { return *mValue; }

    int32 hash() const;

    /**
     * @brief Obtains length of the buffer.
     *
     * @return Length of buffer.
     */
    size_t size() const;

protected:
    virtual ~PyBuffer();

    const Buffer* const mValue;
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
    /** Calls std::string( const char* ). */
    PyString( const char* str );
    /** Calls std::string( const char*, size_t ). */
    PyString( const char* str, size_t len );
    /** Calls std::string( Iter, Iter ). */
    template<typename Iter>
    PyString( Iter first, Iter last );
    /** Calls std::string( const std::string& ). */
    PyString( const std::string& str );

    /** Copy constructor. */
    PyString( const PyBuffer& buf );
    /** Copy constructor. */
    PyString( const PyToken& token );
    /** Copy constructor. */
    PyString( const PyString& oth );

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    /**
     * @brief Get the PyString content
     *
     * @return the std::string reference.
     */
    const std::string& content() const { return mValue; }

    // updated to use std::hash for strings.  better checks without collision (so far)
    int32 hash() const;

protected:
    virtual ~PyString()                                 { /* do nothing here */ }
    const std::string mValue;
    mutable int32 mHashCache;
};

/**
 * @brief Python wide string.
 *
 * Stores the string encoded in UTF-8. We don't use UCS-2
 * because MySQL client doesn't support it.
 */
class PyWString : public PyRep
{
public:
    /** Calls std::string( const char*, size_t ). */
    PyWString( const char* str, size_t len );
    /** Calls std::string( Iter, Iter ). */
    template<typename Iter>
    PyWString( Iter first, Iter last );
    /** Calls std::string( const std::string& ). */
    PyWString( const std::string& str );

    /** Copy constructor. */
    PyWString( const PyString& str );
    /** Copy constructor. */
    PyWString( const PyWString& oth );

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    /**
     * @brief Get the PyWString content.
     *
     * @return the std::string reference.
     */
    const std::string& content() const { return mValue; }

    /**
     * @brief Obtains length of string.
     *
     * @return Length of string in UTF-8 characters.
     */
    size_t size() const;

    // updated to use std::hash for strings.  better checks without collision (so far)
    int32 hash() const;

protected:
    virtual ~PyWString()    { /* do we need to do anything here? */ }
    const std::string mValue;
    mutable int32 mHashCache;
};

/**
 * @brief Python token (eg. class name).
 *
 * Usually part of PyObject or PyObjectEx.
 */
class PyToken : public PyRep
{
public:
    /** Calls std::string( const char* ). */
    PyToken( const char* token );
    /** Calls std::string( const char*, size_t ). */
    PyToken( const char* token, size_t len );
    /** Calls std::string( Iter, Iter ). */
    template<typename Iter>
    PyToken( Iter first, Iter last );
    /** Calls std::string( const std::string& ). */
    PyToken( const std::string& token );

    /** Copy constructor. */
    PyToken( const PyString& token );
    /** Copy constructor. */
    PyToken( const PyToken& oth );
    // move c'tor
    PyToken(PyToken&& oth) = delete;
    // copy assignment
    PyToken& operator= (const PyToken& oth) = delete;
    // move assignment
    PyToken& operator= (PyToken&& oth) = delete;


    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    /**
     * @brief Obtain token.
     *
     * @return Token.
     */
    const std::string& content() const { return mValue; }

    virtual ~PyToken()    { /* do we need to do anything here? */ }
protected:
    const std::string mValue;
};

/**
 * @brief Python tuple.
 *
 * Tuple. Unlike Python's, this one is mutable.
 */
class PyTuple : public PyRep
{
public:
    typedef std::vector<PyRep*>::iterator          iterator;
    typedef std::vector<PyRep*>::const_iterator    const_iterator;

    // default c'tor
    PyTuple( size_t item_count= 0 );
    // copy c'tor
    PyTuple( const PyTuple& oth );
    // move c'tor
    PyTuple(PyTuple&& oth) = delete;
    // copy assignment
    PyTuple& operator= (const PyTuple& oth);
    // move assignment
    PyTuple& operator= (PyTuple&& oth) = delete;

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    const_iterator begin() const                        { return items.begin(); }
    const_iterator end() const                          { return items.end(); }

    size_t size() const                                 { return items.size(); }
    bool empty() const                                  { return items.empty(); }
    void clear();

    /**
     * @brief Returns Python object.
     *
     * @param[in] index Index at which is the desired object.
     *
     * @return Python object.
     */
    PyRep* GetItem( size_t index ) const                { return items.at( index ); }

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
        if (object == nullptr) {
            *rep = new PyNone();
        } else {
            *rep = object;
        }
        PyIncRef( *rep );
    }

    void SetItemInt( size_t index, int32 val )          { SetItem( index, new PyInt( val ) ); }
    void SetItemString( size_t index, const char* str ) { SetItem( index, new PyString( str ) ); }

    int32 hash() const;

    // This needs to be public for now.
    std::vector<PyRep*> items;

protected:
    virtual ~PyTuple()                                  { /* do nothing here */ }
};

/**
 * @brief Python list.
 *
 * Python's list, completely mutable.
 */
class PyList : public PyRep
{
public:
    typedef std::vector<PyRep*>::iterator          iterator;
    typedef std::vector<PyRep*>::const_iterator    const_iterator;

    // default c'tor
    PyList( size_t item_count = 0 );
    // copy c'tor
    PyList( const PyList& oth );
    // move c'tor
    PyList(PyList&& oth) = delete;
    // copy assignment
    PyList& operator= (const PyList& oth);
    // move assignment
    PyList& operator= (PyList&& oth) = delete;


    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    const_iterator begin() const                        { return items.begin(); }
    const_iterator end() const                          { return items.end(); }

    size_t size() const                                 { return items.size(); }
    bool empty() const                                  { return items.empty(); }
    void clear();

    /**
     * @brief Returns Python object.
     *
     * @param[in] index Index at which is the desired object.
     *
     * @return Python object.
     */
    PyRep* GetItem( size_t index ) const                { return items.at( index ); }

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
        if (object == nullptr) {
            *rep = new PyNone();
        } else {
            *rep = object;
        }
        PyIncRef( *rep );
    }
    /**
     * @brief Stores Python string.
     *
     * @param[in] index Index at which the object should be stored.
     * @param[in] str   String to be stored.
     */
    void SetItemString( size_t index, const char* str ) { SetItem( index, new PyString( str ) ); }

    void AddItem( PyRep* i )                            { items.push_back( i ); }
    void AddItemInt( int32 intval )                     { AddItem( new PyInt( intval ) ); }
    void AddItemLong( int64 intval )                    { AddItem( new PyLong( intval ) ); }
    void AddItemReal( double realval )                  { AddItem( new PyFloat( realval ) ); }
    void AddItemString( const char* str )               { AddItem( new PyString( str ) ); }

    // This needs to be public:
    std::vector<PyRep*> items;

protected:
    virtual ~PyList();
};

/**
 * @brief Python's dictionary.
 *
 * Dictionary; completely mutable associative container.
 */
class PyDict : public PyRep
{
protected:
    // hash function
    class _hash : public std::unary_function<PyRep*, size_t>
    {
    public:
        size_t operator()( const PyRep* _Keyval ) const
        {
            assert( _Keyval );

            return (size_t)_Keyval->hash();
        }
    };

    // comparison function
    class _comp : public std::binary_function<PyRep*, PyRep*, bool>
    {
    public:
        bool operator()( const PyRep* _Arg1, const PyRep* _Arg2 ) const
        {
            assert( _Arg1 );
            assert( _Arg2 );

            return ( _Arg1->hash() == _Arg2->hash() );
        }
    };

public:
    typedef std::unordered_map<PyRep*, PyRep*, _hash, _comp>   storage_type;
    typedef storage_type::iterator                             iterator;
    typedef storage_type::const_iterator                       const_iterator;

    // default c'tor
    PyDict();
    // copy c'tor
    PyDict( const PyDict& oth );
    // move c'tor
    PyDict(PyDict&& oth) =delete;
    // copy assignment
    PyDict& operator=(const PyDict& oth);
    // move assignment
    PyDict& operator=(PyDict&& oth) =delete;

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

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
    void SetItem( const char* key, PyRep* value )       { SetItem(new PyString(key), value); }
    void SetItem( const char* key, const char* value )  { SetItem(new PyString(key), new PyString(value)); }

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

    storage_type items;

protected:
    virtual ~PyDict();
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
    PyObject( const char* type, PyRep* args );
    PyObject( PyString* type, PyRep* args );
    // copy c'tor
    PyObject( const PyObject& oth );
    // move c'tor
    PyObject(PyObject&& oth) = delete;
    // copy assignment
    PyObject& operator= (const PyObject& oth);
    // move assignment
    PyObject& operator= (PyObject&& oth) = delete;


    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    PyString* type() const { return mType; }
    PyRep* arguments() const { return mArguments; }

protected:
    virtual ~PyObject();

    PyString* mType;
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
    typedef PyList::iterator             list_iterator;
    typedef PyList::const_iterator       const_list_iterator;

    typedef PyDict::iterator             dict_iterator;
    typedef PyDict::const_iterator       const_dict_iterator;

    // default c'tor
    PyObjectEx( bool is_type_2, PyRep* header );
    // copy c'tor
    PyObjectEx( const PyObjectEx& oth );
    // move c'tor
    PyObjectEx(PyObjectEx&& oth) = delete;
    // copy assignment
    PyObjectEx& operator= (const PyObjectEx& oth);
    // move assignment
    PyObjectEx& operator= (PyObjectEx&& oth) = delete;


    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    PyRep* header() const                               { return mHeader; }
    bool isType2() const                                { return mIsType2; }

    PyList& list()                                   { return *mList; }
    const PyList& list() const                       { return *mList; }

    PyDict& dict()                                   { return *mDict; }
    const PyDict& dict() const                       { return *mDict; }


protected:
    virtual ~PyObjectEx();

    PyRep* const mHeader;
    const bool mIsType2;

    PyList* const mList;
    PyDict* const mDict;
};

/**
 * @brief Wrapper class for PyObjectEx of type 1.
 *
 * @author Bloody.Rabbit
 */
class PyObjectEx_Type1 : public PyObjectEx
{
public:
    PyObjectEx_Type1( PyToken* type, PyTuple* args, bool enclosed= false );
    PyObjectEx_Type1( PyObjectEx_Type1* args1, PyTuple* args2, bool enclosed= false );
    PyObjectEx_Type1( PyToken* type, PyTuple* args, PyDict* keywords, bool enclosed= false );
    PyObjectEx_Type1( PyToken* type, PyTuple* args, PyList* keywords, bool enclosed= false );

    PyToken* GetType() const;
    PyTuple* GetArgs() const;
    PyDict* GetKeywords() const;

    PyRep* FindKeyword( const char* keyword ) const;

protected:
    virtual ~PyObjectEx_Type1()    { /* do we need to do anything here? */ }
    static PyTuple* _CreateHeader( PyToken* type, PyTuple* args, bool enclosed= false );
    static PyTuple* _CreateHeader( PyObjectEx_Type1* args1, PyTuple* args2, bool enclosed= false );
    static PyTuple* _CreateHeader( PyToken* type, PyTuple* args, PyDict* keywords, bool enclosed= false );
    static PyTuple* _CreateHeader( PyToken* type, PyTuple* args, PyList* keywords, bool enclosed= false );
};

/**
 * @brief Wrapper class for PyObjectEx of type 2.
 *
 * @author Bloody.Rabbit
 */
class PyObjectEx_Type2 : public PyObjectEx
{
public:
    PyObjectEx_Type2( PyTuple* args, PyDict* keywords, bool enclosed= false );
    PyObjectEx_Type2( PyToken* args, PyDict* keywords, bool enclosed= false );

    PyTuple* GetArgs() const;
    PyDict* GetKeywords() const;

    PyRep* FindKeyword( const char* keyword ) const;

protected:
    virtual ~PyObjectEx_Type2()    { /* do we need to do anything here? */ }
    static PyTuple* _CreateHeader( PyTuple* args, PyDict* keywords, bool enclosed= false );
    static PyTuple* _CreateHeader( PyToken* args, PyDict* keywords, bool enclosed= false );
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
    typedef PyList::iterator          iterator;
    typedef PyList::const_iterator    const_iterator;

    // default c'tor
    PyPackedRow( DBRowDescriptor* header );
    // copy c'tor
    PyPackedRow( const PyPackedRow& oth );
    // move c'tor
    PyPackedRow(PyPackedRow&& oth) = delete;
    // copy assignment
    PyPackedRow& operator= (const PyPackedRow& oth);
    // move assignment
    PyPackedRow& operator= (PyPackedRow&& oth) = delete;



    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    // Header:
    DBRowDescriptor* header() const { return mHeader; }

    // Fields:
    const_iterator begin() const { return mFields->begin(); }
    const_iterator end() const { return mFields->end(); }
    void clear() { mFields->clear(); }

    PyRep* GetField( size_t index ) const { return mFields->GetItem( index ); }

    bool SetField( uint32 index, PyRep* value );
    bool SetField( const char* colName, PyRep* value );

    int32 hash() const;

protected:
    virtual ~PyPackedRow();

    DBRowDescriptor* const mHeader;
    PyList* const mFields;
};

class PySubStruct : public PyRep
{
public:
    // default c'tor
    PySubStruct( PyRep* t );
    // copy c'tor
    PySubStruct( const PySubStruct& oth );
    // move c'tor
    PySubStruct(PySubStruct&& oth) =delete;
    // copy assignment
    PySubStruct& operator=(const PySubStruct& oth) =delete;
    // move assignment
    PySubStruct& operator=(PySubStruct&& oth) =delete;


    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    PyRep* sub() const { return mSub; }

protected:
    virtual ~PySubStruct();

    PyRep* const mSub;
};

class PySubStream : public PyRep
{
public:
    // default c'tor
    PySubStream( PyRep* rep );
    // default c'tor
    PySubStream( PyBuffer* buffer );
    // copy c'tor
    PySubStream( const PySubStream& oth );
    // move c'tor
    PySubStream(PySubStream&& oth) = delete;
    // copy assignment
    PySubStream& operator= (const PySubStream& oth) = delete;
    // move assignment
    PySubStream& operator= (PySubStream&& oth) = delete;


    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    PyBuffer* data() const { return mData; }
    PyRep* decoded() const { return mDecoded; }

    //call to ensure that `data` represents `decoded` IF DATA IS NULL
    void EncodeData() const;

    //call to ensure that `decoded` represents `data` IF DECODED IS NULL
    void DecodeData() const;

protected:
    virtual ~PySubStream();

    //if both are non-NULL, they are considered to be equivalent
    mutable PyBuffer* mData;
    mutable PyRep* mDecoded;
};

class PyChecksumedStream : public PyRep
{
public:
    // default c'tor
    PyChecksumedStream( PyRep* t, uint32 sum );
    // copy c'tor
    PyChecksumedStream( const PyChecksumedStream& oth );
    // move c'tor
    PyChecksumedStream(PyChecksumedStream&& oth) = delete;
    // copy assignment
    PyChecksumedStream& operator= (const PyChecksumedStream& oth) = delete;
    // move assignment
    PyChecksumedStream& operator= (PyChecksumedStream&& oth) = delete;


    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    PyRep* stream() const { return mStream; }
    uint32 checksum() const { return mChecksum; }

protected:
    virtual ~PyChecksumedStream();

    PyRep* const mStream;
    const uint32 mChecksum;
};


/* note: these need to be in header since they are templates ... we don't mess
 * the class definitions up with them, we stick them here instead to have them
 * all together.
 */
template<typename Iter>
inline PyBuffer::PyBuffer( Iter first, Iter last ) : PyRep( PyRep::PyTypeBuffer ), mValue( new Buffer( first, last ) ), mHashCache( -1 ) {}
template<typename Iter>
inline PyString::PyString( Iter first, Iter last ) : PyRep( PyRep::PyTypeString ), mValue( first, last ), mHashCache( -1 ) {}
template<typename Iter>
inline PyWString::PyWString( Iter first, Iter last ) : PyRep( PyRep::PyTypeWString ), mValue( first, last ), mHashCache( -1 ) {}
template<typename Iter>
inline PyToken::PyToken( Iter first, Iter last ) : PyRep( PyRep::PyTypeToken ), mValue( first, last ) {}


/************************************************************************/
/* tuple helper functions                                               */
/************************************************************************/
PyTuple* new_tuple(int64 arg1);
PyTuple* new_tuple(int64 arg1, int64 arg2);
/* strings */
PyTuple* new_tuple(const char* arg1);
PyTuple* new_tuple(const char* arg1, const char* arg2);
PyTuple* new_tuple(const char* arg1, const char* arg2, const char* arg3);
PyTuple* new_tuple(const char* arg1, const char* arg2, PyTuple* arg3);
/* mixed */
PyTuple* new_tuple(const char* arg1, PyRep* arg2, PyRep* arg3);
PyTuple* new_tuple(PyRep* arg1);
PyTuple* new_tuple(PyRep* arg1, PyRep* arg2);
PyTuple* new_tuple(PyRep* arg1, PyRep* arg2, PyRep* arg3);


class BuiltinSet : public PyObjectEx_Type1
{
public:
    BuiltinSet() : PyObjectEx_Type1( new PyToken("collections.defaultdict"), new_tuple(new PyToken("__builtin__.set")) ) {}

protected:
    virtual ~BuiltinSet()    { /* do we need to do anything here? */ }
};

class CacheOK : public PyObjectEx_Type1
{
public:
    CacheOK() : PyObjectEx_Type1( new PyToken("objectCaching.CacheOK"), new_tuple("CacheOK") ) {}

protected:
    virtual ~CacheOK()    { /* do we need to do anything here? */ }
};


/**
 * @name PyStatic.h
 *   static memory object caching/tracking system for oft-used Python objects
 *
 * @Author:         Allan
 * @date:          13 December 17
 * @update:     15 February 21 (added mt objects)
 *
 */

#include "../../eve-core/utils/Singleton.h"

class pyStatic
: public Singleton< pyStatic >
{
public:
    pyStatic()
    {
        m_none = new PyNone();
        m_zero = new PyInt(0);
        m_one = new PyInt(1);
        m_negone = new PyInt(-1);
        m_true = new PyBool(true);
        m_false = new PyBool(false);
        m_dict = new PyDict();
        m_list = new PyList();
        m_tuple = new PyTuple(0);
    }

   ~pyStatic()
   {
       PyDecRef(m_none);
       PyDecRef(m_zero);
       PyDecRef(m_one);
       PyDecRef(m_negone);
       PyDecRef(m_true);
       PyDecRef(m_false);
       PyDecRef(m_dict);
       PyDecRef(m_list);
       PyDecRef(m_tuple);
    }

    PyRep* NewNone()            { PyIncRef(m_none); return m_none; }
    PyRep* NewZero()            { PyIncRef(m_zero); return m_zero; }
    PyRep* NewOne()             { PyIncRef(m_one); return m_one; }
    PyRep* NewNegOne()          { PyIncRef(m_negone); return m_negone; }
    PyRep* NewTrue()            { PyIncRef(m_true); return m_true; }
    PyRep* NewFalse()           { PyIncRef(m_false); return m_false; }

    PyDict* mtDict()            { PyIncRef(m_dict); return m_dict; }
    PyList* mtList()            { PyIncRef(m_list); return m_list; }
    PyTuple* mtTuple()          { PyIncRef(m_tuple); return m_tuple; }

private:
    PyRep* m_none;
    PyRep* m_zero;
    PyRep* m_one;
    PyRep* m_negone;
    PyRep* m_true;
    PyRep* m_false;

    PyDict* m_dict;
    PyList* m_list;
    PyTuple* m_tuple;
};

//Singleton
#define PyStatic \
    ( pyStatic::get() )

#endif//EVE_PY_REP_H
