/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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

#include <assert.h>

#ifndef EVE_PY_REP_H
#define EVE_PY_REP_H

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
 * debug macro's to ease the increase and decrease of references of a object
 * using this also increases the possibility of debugging it.
 */
#define PyIncRef(op) (op)->IncRef()
#define PyDecRef(op) (op)->DecRef()

/* Macros to use in case the object pointer may be NULL */
#define PySafeIncRef(op) if( NULL == (op) ) ; else PyIncRef( op )
#define PySafeDecRef(op) if( NULL == (op) ) ; else PyDecRef( op )

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
        PyTypeInt               = 0,
        PyTypeLong              = 1,
        PyTypeFloat             = 2,
        PyTypeBool              = 3,
        PyTypeBuffer            = 4,
        PyTypeString            = 5,
        PyTypeWString           = 6,
        PyTypeToken             = 7,
        PyTypeTuple             = 8,
        PyTypeList              = 9,
        PyTypeDict              = 10,
        PyTypeNone              = 11,
        PyTypeSubStruct         = 12,
        PyTypeSubStream         = 13,
        PyTypeChecksumedStream  = 14,
        PyTypeObject            = 15,
        PyTypeObjectEx          = 16,
        PyTypePackedRow         = 17,
        PyTypeError             = 18,
        PyTypeMax               = 18,
    };

    /** PyType check functions
      */
    //using this method is discouraged, it generally means your doing something wrong... Is<type>() should cover almost all needs
    PyType GetType() const          { return mType; }

    bool IsInt() const
    {
#ifdef WIN32
        /* crash when we missed a convertion... lol */
        if (mType == PyTypeLong)
            __asm{int 3};
#endif//WIN32
        return mType == PyTypeInt;
    }

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
     * @return Indentical copy of object.
     */
    virtual PyRep* Clone() const = 0;
    /**
     * @brief Visits object.
     *
     * @param[in] v Visitor to be used for visiting.
     *
     * @retval true  Visit successful.
     * @retval false Error during visit.
     */
    virtual bool visit( PyVisitor& v ) const = 0;
    /**
     * @brief virtual function to generate a hash value of a object.
     *
     * virtual function to generate a hash value of a object to facilitate the various maps and checks.
     *
     * @return returns a uint32 containing a hash function that represents the object.
     */
    virtual int32 hash() const;

protected:
    PyRep( PyType t );
    virtual ~PyRep();

    const PyType mType;

    /** Lookup table for PyRep type object type names. */
    static const char* const s_mTypeString[];
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
    PyInt( const int32 i );
	PyInt( const PyInt& oth );

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

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
    PyLong( const int64 i );
	PyLong( const PyLong& oth );

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

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
    PyFloat( const double& i );
	PyFloat( const PyFloat& oth );

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

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
    PyBool( bool i );
	PyBool( const PyBool& oth );

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

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
    PyNone( const PyNone& oth );

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    int32 hash() const;
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

    int32 hash() const;

protected:
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

    int32 hash() const;

protected:
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

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    /**
     * @brief Obtain token.
     *
     * @return Token.
     */
    const std::string& content() const { return mValue; }

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
    typedef std::vector<PyRep*>             storage_type;
    typedef storage_type::iterator          iterator;
    typedef storage_type::const_iterator    const_iterator;

    PyTuple( size_t item_count );
    PyTuple( const PyTuple& oth );

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

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
    PyTuple& operator=( const PyTuple& oth );

    int32 hash() const;

	// This needs to be public for now.
	storage_type items;

protected:
    virtual ~PyTuple();
};

/**
 * @brief Python list.
 *
 * Python's list, completely mutable.
 */
class PyList : public PyRep
{
public:
    typedef std::vector<PyRep*>             storage_type;
    typedef storage_type::iterator          iterator;
    typedef storage_type::const_iterator    const_iterator;

    PyList( size_t item_count = 0 );
    PyList( const PyList& oth );

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

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

    void AddItem( PyRep* i ) { items.push_back( i ); }
    void AddItemInt( int32 intval ) { AddItem( new PyInt( intval ) ); }
    void AddItemLong( int64 intval ) { AddItem( new PyLong( intval ) ); }
    void AddItemReal( double realval ) { AddItem( new PyFloat( realval ) ); }
    void AddItemString( const char* str ) { AddItem( new PyString( str ) ); }

    /**
     * @brief Overload of assigment operator to handle object ownership.
     *
     * @param[in] oth List the content of which is to be copied.
     * @return Itself.
     */
    PyList& operator=( const PyList& oth );

    // This needs to be public:
    storage_type items;

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
    PyDict( const PyDict& oth );

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

    void SetItem( const char* key, PyRep* value );

    void SetItem( const char* key, const char* value );

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
    PyDict& operator=( const PyDict& oth );

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
    PyObject( const PyObject& oth );

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
    typedef PyList                          list_type;
    typedef list_type::iterator             list_iterator;
    typedef list_type::const_iterator       const_list_iterator;

    typedef PyDict                          dict_type;
    typedef dict_type::iterator             dict_iterator;
    typedef dict_type::const_iterator       const_dict_iterator;

    PyObjectEx( bool is_type_2, PyRep* header );
    PyObjectEx( const PyObjectEx& oth );

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

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
    PyObjectEx& operator=( const PyObjectEx& oth );

protected:
    virtual ~PyObjectEx();

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
class PyObjectEx_Type1 : public PyObjectEx
{
public:
    PyObjectEx_Type1( PyToken* type, PyTuple* args, PyDict* keywords );

    PyToken* GetType() const;
    PyTuple* GetArgs() const;
    PyDict* GetKeywords() const;

    PyRep* FindKeyword( const char* keyword ) const;

protected:
    static PyTuple* _CreateHeader( PyToken* type, PyTuple* args, PyDict* keywords );
};

/**
 * @brief Wrapper class for PyObjectEx of type 2.
 *
 * @author Bloody.Rabbit
 */
class PyObjectEx_Type2 : public PyObjectEx
{
public:
    PyObjectEx_Type2( PyTuple* args, PyDict* keywords );

    PyTuple* GetArgs() const;
    PyDict* GetKeywords() const;

    PyRep* FindKeyword( const char* keyword ) const;

protected:
    static PyTuple* _CreateHeader( PyTuple* args, PyDict* keywords );
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

    PyPackedRow( DBRowDescriptor* header );
    PyPackedRow( const PyPackedRow& oth );

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

    /**
     * @brief Assigment operator to handle ownership things.
     *
     * @param[in] oth PyPackedRow the content of which should be copied.
     * @return Itself.
     */
    PyPackedRow& operator=( const PyPackedRow& oth );

    int32 hash() const;

protected:
    virtual ~PyPackedRow();

    DBRowDescriptor* const mHeader;
    storage_type* const mFields;
};

class PySubStruct : public PyRep
{
public:
    PySubStruct( PyRep* t );
	PySubStruct( const PySubStruct& oth );

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
    PySubStream( PyRep* rep );
    PySubStream( PyBuffer* buffer );
	PySubStream( const PySubStream& oth );

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
    PyChecksumedStream( PyRep* t, uint32 sum );
	PyChecksumedStream( const PyChecksumedStream& oth );

    PyRep* Clone() const;
    bool visit( PyVisitor& v ) const;

    PyRep* stream() const { return mStream; }
    uint32 checksum() const { return mChecksum; }

protected:
    virtual ~PyChecksumedStream();

    PyRep* const mStream;
    const uint32 mChecksum;
};

/* note: this will decrease memory use with 50% but increase load time with 50%
 * enabling this would have to wait until references work properly.
 */
//#pragma pack(pop)

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
PyTuple * new_tuple(uint64 arg1);
PyTuple * new_tuple(uint64 arg1, uint64 arg2);
PyTuple * new_tuple(const char* arg1);
PyTuple * new_tuple(const char* arg1, const char* arg2);
PyTuple * new_tuple(const char* arg1, const char* arg2, const char* arg3);
PyTuple * new_tuple(const char* arg1, const char* arg2, PyTuple* arg3);
/* mixed */
PyTuple * new_tuple(const char* arg1, PyRep* arg2, PyRep* arg3);
PyTuple * new_tuple(PyRep* arg1);
PyTuple * new_tuple(PyRep* arg1, PyRep* arg2);

#endif//EVE_PY_REP_H
