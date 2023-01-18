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
    Rewrite:    Allan
*/

#include "../../eve-common/eve-common.h"

#include "marshal/EVEMarshal.h"
#include "marshal/EVEUnmarshal.h"
//#include "marshal/EVEMarshalOpcodes.h"
#include "python/classes/PyDatabase.h"
#include "python/PyDumpVisitor.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"
#include "utils/EVEUtils.h"

/** Lookup table for PyRep type object type names. */
const char* const s_mTypeString[] =
{
    "Min",
    "Integer",          //1
    "Long",             //2
    "Real",             //3
    "Boolean",          //4
    "Buffer",           //5
    "String",           //6
    "WString",          //7
    "Token",            //8
    "Tuple",            //9
    "List",             //10
    "Dict",             //11
    "None",             //12
    "SubStruct",        //13
    "SubStream",        //14
    "ChecksumedStream", //15
    "Object",           //16
    "ObjectEx",         //17
    "PackedRow",        //18
    "Invalid Type"      //19
};

/** @todo  this entire class should be better thought-out for class d'tors and object lifetime
 *         however, i dont fully understand it enough to implement better memMgmt at this time
 *              -allan 10Jan19
 *
 *      -- possibly rewrite to use shared_ptr/unique_ptr, then we can remove the RefObject inheritance here.
 */

/************************************************************************/
/* PyRep base Class                                                     */
/************************************************************************/
PyRep::PyRep(PyType t) : RefObject(1), mType( t ) { }

PyRep::PyRep(const PyRep& oth) : RefObject(1), mType(oth.mType)
{
    //sLog.Cyan("PyRep()", "Copy C'tor.");
}
PyRep::~PyRep()
{
    //sLog.Error("PyRep()", "D'tor. count: %u", GetCount());
}

const char* PyRep::TypeString() const
{
    if ((mType >= PyTypeMin) and (mType < PyTypeError))
        return s_mTypeString[ mType ];

    return s_mTypeString[PyTypeError];
}

void PyRep::Dump( FILE* into, const char* pfx ) const
{
    PyFileDumpVisitor dumper( into, pfx );
    visit( dumper );
}

void PyRep::Dump( LogType type, const char* pfx ) const
{
    PyLogDumpVisitor dumper( type, type, pfx );
    visit( dumper );
}

int32 PyRep::hash() const
{
    sLog.Error( "Hash", "Unhashable type: '%s'", TypeString() );
    return -1;
}

PyRep* PyRep::Clone() const
{
    //sLog.Magenta("PyRep()", "Clone.");
    return new PyRep( *this );
}

bool PyRep::visit(PyVisitor& v) const
{
    return true;
}

std::string PyRep::StringContent(PyRep* pRep)
{
    if (pRep == nullptr)
        return "";
    else if (pRep->IsString())
        return pRep->AsString()->content();
    else if (pRep->IsWString())
        return pRep->AsWString()->content();
    else if (pRep->IsNone())
        return "";

    sLog.Error("PyRep::StringContent()", "Expected PyString or PyWString but got %s.", pRep->TypeString());
    return "";
}

int64 PyRep::IntegerValue(PyRep* pRep)
{
    if (pRep == nullptr)
        return 0;
    else if (pRep->IsInt())
        return pRep->AsInt()->value();
    else if (pRep->IsLong())
        return pRep->AsLong()->value();
    else if (pRep->IsFloat())
        return pRep->AsFloat()->value();
    else if (pRep->IsBool())
        return pRep->AsBool()->value();

    return 0;
}

uint32 PyRep::IntegerValueU32(PyRep* pRep) {
    if (pRep == nullptr)
        return 0;
    else if (pRep->IsInt())
        return pRep->AsInt()->value();
    else if (pRep->IsLong())
        return pRep->AsLong()->value();
    else if (pRep->IsFloat())
        return pRep->AsFloat()->value();
    else if (pRep->IsBool())
        return pRep->AsBool()->value();

    return 0;
}

double PyRep::FloatValue(PyRep* pRep)
{
    if (pRep == nullptr)
        return 0.0;
    else if (pRep->IsInt())
        return pRep->AsInt()->value();
    else if (pRep->IsLong())
        return pRep->AsLong()->value();
    else if (pRep->IsFloat())
        return pRep->AsFloat()->value();
    else if (pRep->IsBool())
        return pRep->AsBool()->value();

    return 0.0;
}


/************************************************************************/
/* PyRep Integer Class                                                  */
/************************************************************************/
PyInt::PyInt( const int32 i ) : PyRep( PyRep::PyTypeInt ), mValue( i ) {}
PyInt::PyInt( const PyInt& oth ) : PyRep( PyRep::PyTypeInt ), mValue( oth.value() )
{
    //sLog.Cyan("PyInt()", "Copy C'tor.");
}

PyRep* PyInt::Clone() const
{
    //sLog.Magenta("PyInt()", "Clone.");
    return new PyInt( *this );
}

bool PyInt::visit( PyVisitor& v ) const
{
    return v.VisitInteger( this );
}

int32 PyInt::hash() const
{
    /* XXX If this is changed, you also need to change the way
     *    Python's long, float and complex types are hashed. */
    int32 x = mValue;
    if (x == -1 )
        x = -2;
    return x;
}

/************************************************************************/
/* PyRep Long Class                                                     */
/************************************************************************/
PyLong::PyLong( const int64 i ) : PyRep( PyRep::PyTypeLong ), mValue( i ) {}
PyLong::PyLong( const PyLong& oth ) : PyRep( PyRep::PyTypeLong ), mValue( oth.value() )
{
    //sLog.Cyan("PyLong()", "Copy C'tor.");
}

PyRep* PyLong::Clone() const
{
    //sLog.Magenta("PyLong()", "Clone.");
    return new PyLong( *this );
}

bool PyLong::visit( PyVisitor& v ) const
{
    return v.VisitLong( this );
}

int32 PyLong::hash() const
{
#define PyLong_SHIFT    15
#define PyLong_BASE     (1 << PyLong_SHIFT)
#define PyLong_MASK     ((int)(PyLong_BASE - 1))

#define LONG_BIT_PyLong_SHIFT    (8*sizeof(long) - PyLong_SHIFT)

    long x=0;
    int i=8, sign=1;

    /* This is designed so that Python ints and longs with the
    same value hash to the same value, otherwise comparisons
    of mapping keys will turn out weird */
    if (i < 0 ) {
        sign = -1;
        i = -(i);
    }
    /* The following loop produces a C long x such that (unsigned long)x
    is congruent to the absolute value of v modulo ULONG_MAX.  The
    resulting x is nonzero if and only if v is. */
    while( --i >= 0 ) {
        /* Force a native long #-bits (32 or 64) circular shift */
        x = ((x << PyLong_SHIFT) & ~PyLong_MASK) | ((x >> LONG_BIT_PyLong_SHIFT) & PyLong_MASK);
        x += ((uint8*)&mValue)[i];// v->ob_digit[i];
        /* If the addition above overflowed (thinking of x as
        unsigned), we compensate by incrementing.  This preserves
        the value modulo ULONG_MAX. */
        if ((unsigned long)x < ((uint8*)&mValue)[i] )//v->ob_digit[i])
            x++;
    }
    x = x * sign;
    if (x == -1 )
        x = -2;
    return x;

#undef PyLong_SHIFT
#undef PyLong_BASE
#undef PyLong_MASK

#undef LONG_BIT_PyLong_SHIFT
}

/************************************************************************/
/* PyRep Real/float/double Class                                        */
/************************************************************************/
PyFloat::PyFloat( const double& i ) : PyRep( PyRep::PyTypeFloat ), mValue( i ) { }
PyFloat::PyFloat( const PyFloat& oth ) : PyRep( PyRep::PyTypeFloat ), mValue( oth.value() )
{
    //sLog.Cyan("PyFloat()", "Copy C'tor.");
}

PyRep* PyFloat::Clone() const
{
    //sLog.Magenta("PyFloat()", "Clone.");
    return new PyFloat( *this );
}

bool PyFloat::visit( PyVisitor& v ) const
{
    return v.VisitReal( this );
}

// maximum int32 value
#ifndef INT32_MAX
#   define INT32_MAX 2147483647L
#endif /* !INT32_MAX */

int32 PyFloat::hash() const
{
#define Py_IS_INFINITY( X ) \
    ( !finite( X ) && !isnan( X ) )

    double v = mValue;
    double intpart=0.0, fractpart=0.0;
    int expo=0;
    long hipart=0, x=0;        /* x is the final hash value */
    /* This is designed so that Python numbers of different types
    * that compare equal hash to the same value; otherwise comparisons
    * of mapping keys will turn out weird.
    */

    fractpart = modf(v, &intpart);
    if (fractpart == 0.0) {
        /* This must return the same hash as an equal int or long. */
        if ((intpart > INT32_MAX) or (-intpart > INT32_MAX)) {
            /* Convert to long and use its hash. */
            if (Py_IS_INFINITY(intpart))
                /* can't convert to long int -- arbitrary */
                v = v < 0 ? -271828.0 : 314159.0;
            //plong = PyLong_FromDouble(v);

            PyRep *plong = new PyLong( (int64)v ); // this is a hack
            if (plong == nullptr)
                return -1;
            x = plong->hash();
            PyDecRef( plong );
            return x;
        }
        /* Fits in a C long == a Python int, so is its own hash. */
        x = (long)intpart;
        if (x == -1)
            x = -2;
        return x;
    }
    /* The fractional part is non-zero, so we don't have to worry about
    * making this match the hash of some other type.
    * Use frexp to get at the bits in the double.
    * Since the VAX D double format has 56 mantissa bits, which is the
    * most of any double format in use, each of these parts may have as
    * many as (but no more than) 56 significant bits.
    * So, assuming sizeof(long) >= 4, each part can be broken into two
    * longs; frexp and multiplication are used to do that.
    * Also, since the Cray double format has 15 exponent bits, which is
    * the most of any double format in use, shifting the exponent field
    * left by 15 won't overflow a long (again assuming sizeof(long) >= 4).
    */
    v = frexp(v, &expo);
    v *= 2147483648.0;    /* 2**31 */
    hipart = (long)v;    /* take the top 32 bits */
    v = (v - (double)hipart) * 2147483648.0; /* get the next 32 bits */
    x = hipart + (long)v + (expo << 15);
    if (x == -1)
        x = -2;
    return x;

#undef Py_IS_INFINITY
}

/************************************************************************/
/* PyRep Boolean Class                                                  */
/************************************************************************/
PyBool::PyBool( bool i ) : PyRep( PyRep::PyTypeBool ), mValue( i ) { }
PyBool::PyBool( const PyBool& oth ) : PyRep( PyRep::PyTypeBool ), mValue( oth.value() )
{
    //sLog.Cyan("PyBool()", "Copy C'tor.");
}

PyRep* PyBool::Clone() const
{
    //sLog.Magenta("PyBool()", "Clone.");
    return new PyBool( *this );
}

bool PyBool::visit( PyVisitor& v ) const
{
    return v.VisitBoolean( this );
}

/************************************************************************/
/* PyRep None Class                                                     */
/************************************************************************/
PyNone::PyNone() : PyRep( PyRep::PyTypeNone ) {}
PyNone::PyNone( const PyNone& oth ) : PyRep( PyRep::PyTypeNone )
{
    //sLog.Cyan("PyNone()", "Copy C'tor.");
}

PyRep* PyNone::Clone() const
{
    //sLog.Magenta("PyNone()", "Clone.");
    return new PyNone( *this );
}

bool PyNone::visit( PyVisitor& v ) const
{
    return v.VisitNone( this );
}

int32 PyNone::hash() const
{
    /* damn hack... bleh.. but its done like this... in python a PyNone is a static singleton....*/
    int32* hash = (int32*)this;
    return *((int32*)&hash);
}

/************************************************************************/
/* PyRep Buffer Class                                                   */
/************************************************************************/
PyBuffer::PyBuffer( size_t len, const uint8& value )
: PyRep( PyRep::PyTypeBuffer ), mValue( new Buffer( len, value ) ), mHashCache( -1 ) {}
PyBuffer::PyBuffer( const Buffer& buffer )
: PyRep( PyRep::PyTypeBuffer ), mValue( new Buffer( buffer ) ), mHashCache( -1 ) {}

PyBuffer::PyBuffer( Buffer** buffer )
: PyRep( PyRep::PyTypeBuffer ), mValue( *buffer ), mHashCache( -1 )
{
    *buffer = nullptr;
}
PyBuffer::PyBuffer( const PyString& str )
: PyRep( PyRep::PyTypeBuffer ), mValue( new Buffer( str.content().begin(), str.content().end() ) ), mHashCache( -1 )
{
    //sLog.Cyan("PyBuffer(string)", "Copy C'tor.");
}
PyBuffer::PyBuffer( const PyBuffer& buffer )
: PyRep( PyRep::PyTypeBuffer ), mValue( new Buffer( buffer.content() ) ), mHashCache( buffer.mHashCache )
{
    //sLog.Cyan("PyBuffer(buffer)", "Copy C'tor.");
}

PyBuffer::~PyBuffer()
{
    delete mValue;
}

PyRep* PyBuffer::Clone() const
{
    //sLog.Magenta("PyBuffer()", "Clone.");
    return new PyBuffer( *this );
}

bool PyBuffer::visit( PyVisitor& v ) const
{
    return v.VisitBuffer( this );
}

int32 PyBuffer::hash() const
{
    if (mHashCache != -1 )
        return mHashCache;

    int32 len=0, x=0;
    unsigned char* p(nullptr);

    /* XXX potential bugs here, a readonly buffer does not imply that the
     * underlying memory is immutable.  b_readonly is a necessary but not
     * sufficient condition for a buffer to be hashable.  Perhaps it would
     * be better to only allow hashing if the underlying object is known to
     * be immutable (e.g. PyString_Check() is true).  Another idea would
     * be to call tp_hash on the underlying object and see if it raises
     * an error. */
    //if ( !self->b_readonly )
    //{
    //   PyErr_SetString(PyExc_TypeError,
    //      "writable buffers are not hashable");
    // return -1;
    //}

    //if (!get_buf(self, &ptr, &size, ANY_BUFFER))
    //    return -1;
    p = (unsigned char *) &content()[0];
    len = (int32)content().size();
    x = *p << 7;
    while( --len >= 0 )
        x = (1000003*x) ^ *p++;
    x ^= content().size();
    if (x == -1)
        x = -2;
    mHashCache = x;
    return x;
}

size_t PyBuffer::size() const
{
    return content().size();
}

/************************************************************************/
/* PyString                                                             */
/************************************************************************/
PyString::PyString( const char* str )
: PyRep( PyRep::PyTypeString ), mValue( str ), mHashCache( -1 ) {}
PyString::PyString( const char* str, size_t len )
: PyRep( PyRep::PyTypeString ), mValue( str, len ), mHashCache( -1 ) {}
PyString::PyString( const std::string& str )
: PyRep( PyRep::PyTypeString ), mValue( str ), mHashCache( -1 ) {}

PyString::PyString( const PyBuffer& buf )
: PyRep( PyRep::PyTypeString ), mValue( (const char *) &buf.content()[0], buf.content().size() ), mHashCache( -1 )
{
    //sLog.Cyan("PyString(PyBuffer)", "Copy C'tor.");
}
PyString::PyString( const PyToken& token )
: PyRep( PyRep::PyTypeString ), mValue( token.content() ), mHashCache( -1 )
{
    //sLog.Cyan("PyString(PyToken)", "Copy C'tor.");
}
PyString::PyString( const PyString& oth )
: PyRep( PyRep::PyTypeString ), mValue( oth.mValue ), mHashCache( oth.mHashCache )
{
    //sLog.Cyan("PyString(PyString)", "Copy C'tor.");
}

PyRep* PyString::Clone() const
{
    //sLog.Magenta("PyString()", "Clone.");
    return new PyString(*this);
}

bool PyString::visit( PyVisitor& v ) const
{
    return v.VisitString( this );
}

int32 PyString::hash() const
{
    if (mHashCache != -1 )
        return mHashCache;

    if (mValue.length() > 0) {
        mHashCache = std::hash<std::string>{} (mValue);
    } else {
        mHashCache = 0;
    }

    return mHashCache;
}

/************************************************************************/
/* PyWString                                                            */
/************************************************************************/
PyWString::PyWString( const char* str, size_t len )
: PyRep( PyRep::PyTypeWString ), mValue( str, len ), mHashCache( -1 ) {}
PyWString::PyWString( const std::string& str )
: PyRep( PyRep::PyTypeWString ), mValue( str ), mHashCache( -1 ) {}

PyWString::PyWString( const PyString& str )
: PyRep( PyRep::PyTypeWString ), mValue( str.content() ), mHashCache( -1 )
{
    //sLog.Cyan("PyWString(PyString)", "Copy C'tor.");
}
PyWString::PyWString( const PyWString& oth )
: PyRep( PyRep::PyTypeWString ), mValue( oth.mValue ), mHashCache( oth.mHashCache )
{
    //sLog.Cyan("PyWString(PyWString)", "Copy C'tor.");
}

PyRep* PyWString::Clone() const
{
    //sLog.Magenta("PyWString()", "Clone.");
    return new PyWString( *this );
}

bool PyWString::visit( PyVisitor& v ) const
{
    return v.VisitWString( this );
}

size_t PyWString::size() const
{
    return utf8::distance( content().begin(), content().end() );
}

int32 PyWString::hash() const
{
    if (mHashCache != -1 )
        return mHashCache;

    if (mValue.length() > 0) {
        mHashCache = std::hash<std::string>{} (mValue);
    } else {
        mHashCache = 0;
    }

    return mHashCache;
}

/************************************************************************/
/* PyToken                                                              */
/************************************************************************/
PyToken::PyToken( const char* token ) : PyRep( PyRep::PyTypeToken ), mValue( token ) {}
PyToken::PyToken( const char* token, size_t len ) : PyRep( PyRep::PyTypeToken ), mValue( token, len ) {}
PyToken::PyToken( const std::string& token ) : PyRep( PyRep::PyTypeToken ), mValue( token ) {}

PyToken::PyToken( const PyString& token ) : PyRep( PyRep::PyTypeToken ), mValue( token.content() )
{
    //sLog.Cyan("PyToken(PyString)", "Copy C'tor.");
}
PyToken::PyToken( const PyToken& oth ) : PyRep( PyRep::PyTypeToken ), mValue( oth.content() )
{
    //sLog.Cyan("PyToken(PyToken)", "Copy C'tor.");
}

PyRep* PyToken::Clone() const
{
    //sLog.Magenta("PyToken()", "Clone.");
    return new PyToken( *this );
}

bool PyToken::visit( PyVisitor& v ) const
{
    return v.VisitToken( this );
}

/************************************************************************/
/* PyRep Tuple Class                                                    */
/************************************************************************/
PyTuple::PyTuple( size_t item_count ) : PyRep( PyRep::PyTypeTuple ), items( item_count, nullptr ) {}
PyTuple::PyTuple( const PyTuple& oth ) : PyRep( PyRep::PyTypeTuple ), items(oth.items)
{
    //sLog.Cyan("PyTuple()", "Copy C'tor.");
}

PyRep* PyTuple::Clone() const
{
    //sLog.Magenta("PyTuple()", "Clone.");
    return new PyTuple( *this );
}

bool PyTuple::visit( PyVisitor& v ) const
{
    return v.VisitTuple( this );
}

void PyTuple::clear()
{
    iterator cur = items.begin(), end = items.end();
    for (; cur != end; ++cur)
        PySafeDecRef( *cur );

    items.clear();
}

PyTuple& PyTuple::operator=( const PyTuple& oth )
{
    //sLog.Yellow("PyTuple()", "Copy assignment.");
    clear();
    items.resize( oth.size() );
    iterator cur = items.begin(), end = items.end();
    const_iterator cur_oth = oth.begin(), oth_end = oth.end();
    for (; cur != end && cur_oth != oth_end; ++cur, ++cur_oth) {
        if (*cur_oth == nullptr ) {
            *cur = nullptr;
        } else {
            *cur = (*cur_oth)->Clone();
        }
    }

    return *this;
}

int32 PyTuple::hash() const
{
    long x=0, y=0;
    int32 len = (int32)items.size();
    long index = 0;
    long mult = 1000003L;
    x = 0x345678L;
    while (--len >= 0) {
        y = items[index++]->hash();
        if (y == -1)
            return -1;
        x = (x ^ y) * mult;
        /* the cast might truncate len; that doesn't change hash stability */
        mult += (long)(82520L + len + len);
    }
    x += 97531L;
    if (x == -1)
        x = -2;
    return (uint32)x;
}

/************************************************************************/
/* PyRep List Class                                                     */
/************************************************************************/
PyList::PyList(size_t item_count) : PyRep(PyRep::PyTypeList), items(item_count, nullptr) { }
PyList::PyList(const PyList& oth) : PyRep(PyRep::PyTypeList), items(oth.items)
{
    //sLog.Cyan("PyList()", "Copy C'tor.");
}

PyList::~PyList()
{
    //for (auto cur : items)
    //    PySafeDecRef(cur);
}

PyRep* PyList::Clone() const
{
    //sLog.Magenta("PyList()", "Clone.");
    return new PyList(*this);
}

bool PyList::visit( PyVisitor& v ) const
{
    return v.VisitList( this );
}

void PyList::clear()
{
    iterator cur = items.begin(), end = items.end();
    for (; cur != end; ++cur)
        PySafeDecRef( *cur );

    items.clear();
}

// copy assignment
PyList& PyList::operator=( const PyList& oth )
{
    //sLog.Yellow("PyList()", "Copy assignment.");
    clear();
    items.resize( oth.size() );
    iterator cur = items.begin(), end = items.end();
    const_iterator cur_oth = oth.begin(), oth_end = oth.end();
    for (; cur != end && cur_oth != oth_end; ++cur, ++cur_oth) {
        if (*cur_oth == nullptr ) {
            *cur = nullptr;
        } else {
            *cur = (*cur_oth)->Clone();
        }
    }

    return *this;
}

/************************************************************************/
/* PyRep Dict Class                                                     */
/************************************************************************/
PyDict::PyDict() : PyRep( PyRep::PyTypeDict ), items() { }
PyDict::PyDict( const PyDict& oth ) : PyRep( PyRep::PyTypeDict ), items(oth.items)
{
    //sLog.Cyan("PyDict()", "Copy C'tor.");
}

PyDict::~PyDict()
{
    /*
    for (auto cur : items) {
        PyDecRef(cur.first);
        PySafeDecRef(cur.second);
    }
    */
}

PyRep* PyDict::Clone() const
{
    //sLog.Magenta("PyDict()", "Clone.");
    return new PyDict(*this);
}

bool PyDict::visit( PyVisitor& v ) const
{
    return v.VisitDict( this );
}

void PyDict::clear()
{
    iterator cur = items.begin();
    for (; cur != items.end(); ++cur) {
        PyDecRef( cur->first );
        PySafeDecRef( cur->second );
    }

    items.clear();
}

PyRep* PyDict::GetItem( PyRep* key ) const
{
    assert( key != nullptr );

    const_iterator res = items.find( key );
    if (res == items.end() )
        return nullptr;

    return res->second;
}

PyRep* PyDict::GetItemString( const char* key ) const
{
    assert( key != nullptr );

    PyString* str = new PyString( key );
    PyRep* res = GetItem( str );
    PyDecRef( str );

    return res;
}

void PyDict::SetItem( PyRep* key, PyRep* value )
{
    if ( key == nullptr )
        return;

    /* note: add check if the key object is hashable
     * if not ( it will return -1 ) return false;
     */
    if (key->hash() == -1)
        return;

    /* check if we need to replace a dictionary entry */
    iterator itr = items.find( key );
    if (itr == items.end()) {
        // Keep both key & value  (make_pair makes copy of args passed)
        items.insert( std::make_pair( key, value ) );
    } else {
        // We found 'key' in current dict, so use itr->first and decRef 'key'.
        // is this right?
        PyDecRef( key );
        // Replace itr->second with new value.
        PySafeDecRef( itr->second );
        if (value == nullptr) {
            itr->second = PyStatic.NewNone();
        } else {
            itr->second = value;
        }
    }
}

// copy assignment
PyDict& PyDict::operator=( const PyDict& oth )
{
    //sLog.Yellow("PyDict()", "Copy assignment.");
    clear();
    const_iterator cur = oth.begin(), end = oth.end();
    for (; cur != end; ++cur) {
        if (cur->second == nullptr ) {
            SetItem( cur->first->Clone(), nullptr );
        } else {
            SetItem( cur->first->Clone(), cur->second->Clone() );
        }
    }

    return *this;
}

/************************************************************************/
/* PyRep Object Class                                                   */
/************************************************************************/
PyObject::PyObject( PyString* type, PyRep* args )
: PyRep(PyRep::PyTypeObject), mType(type), mArguments(args) { }
PyObject::PyObject(const char* type, PyRep* args )
: PyRep(PyRep::PyTypeObject), mType(new PyString(type)), mArguments(args) { }
PyObject::PyObject(const PyObject& oth)
: PyRep(PyRep::PyTypeObject), mType(oth.mType), mArguments(oth.arguments())
{
    //sLog.Cyan("PyObject()", "Copy C'tor.");
}

PyObject::~PyObject()
{
    PyDecRef( mType );
    PyDecRef( mArguments );
}

PyRep* PyObject::Clone() const
{
    //sLog.Magenta("PyObject()", "Clone.");
    return new PyObject( *this );
}

bool PyObject::visit( PyVisitor& v ) const
{
    return v.VisitObject( this );
}

/************************************************************************/
/* PyObjectEx                                                           */
/************************************************************************/
PyObjectEx::PyObjectEx(bool is_type_2, PyRep* header) : PyRep(PyRep::PyTypeObjectEx),
 mHeader(header), mIsType2(is_type_2), mList(new PyList()), mDict(new PyDict()) { }
PyObjectEx::PyObjectEx( const PyObjectEx& oth ) : PyRep( PyRep::PyTypeObjectEx ),
mHeader(oth.header()->Clone()), mIsType2(oth.isType2()), mList(oth.mList), mDict(oth.mDict)
{
    //sLog.Cyan("PyObjectEx()", "Copy C'tor.");
}

PyObjectEx::~PyObjectEx()
{
    PySafeDecRef( mHeader );

    PyDecRef( mList );
    PyDecRef( mDict );
}

PyRep* PyObjectEx::Clone() const
{
    //sLog.Magenta("PyObjectEx()", "Clone.");
    return new PyObjectEx(*this);
}

bool PyObjectEx::visit( PyVisitor& v ) const
{
    return v.VisitObjectEx( this );
}

PyObjectEx& PyObjectEx::operator= (const PyObjectEx& oth)
{
    list() = oth.list();
    dict() = oth.dict();

    return *this;
}

/************************************************************************/
/* PyObjectEx_Type1                                                     */
/************************************************************************/
PyObjectEx_Type1::PyObjectEx_Type1(PyToken* type, PyTuple* args, bool enclosed )
: PyObjectEx(false, _CreateHeader(type, args, enclosed)) { }
PyObjectEx_Type1::PyObjectEx_Type1(PyObjectEx_Type1* args1, PyTuple* args2, bool enclosed )
: PyObjectEx(false, _CreateHeader(args1, args2, enclosed)) { }
PyObjectEx_Type1::PyObjectEx_Type1(PyToken* type, PyTuple* args, PyDict* keywords, bool enclosed )
: PyObjectEx(false, _CreateHeader(type, args, keywords, enclosed)) { }
PyObjectEx_Type1::PyObjectEx_Type1(PyToken* type, PyTuple* args, PyList* keywords, bool enclosed )
: PyObjectEx(false, _CreateHeader(type, args, keywords, enclosed)) { }

PyToken* PyObjectEx_Type1::GetType() const
{
    assert( mHeader != nullptr );
    return mHeader->AsTuple()->GetItem( 0 )->AsToken();
}

PyTuple* PyObjectEx_Type1::GetArgs() const
{
    assert( mHeader != nullptr );
    return mHeader->AsTuple()->GetItem( 1 )->AsTuple();
}

PyDict* PyObjectEx_Type1::GetKeywords() const
{
    // This one is slightly more complicated since keywords are optional.
    assert( mHeader != nullptr );
    PyTuple* t = mHeader->AsTuple();

    if (t->size() < 3 )
        t->items.push_back( new PyDict );

    return t->GetItem( 2 )->AsDict();
}

PyRep* PyObjectEx_Type1::FindKeyword( const char* keyword ) const
{
    PyDict* kw = GetKeywords();

    PyDict::const_iterator cur = kw->begin();
    for (; cur != kw->end(); ++cur) {
        if (cur->first->IsString() )
            if (cur->first->AsString()->content() == keyword )
                return cur->second;
    }

    return nullptr;
}

PyTuple* PyObjectEx_Type1::_CreateHeader( PyToken* type, PyTuple* args, bool enclosed /*false*/ )
{
    if (args == nullptr )
        args = new PyTuple( 0 );

    PyTuple* body = new PyTuple( 2 );
    if (enclosed) {
        PyTuple* body1 = new PyTuple( 1 );
            body1->SetItem( 0, type );

        body->SetItem( 0, body1 );
        body->SetItem( 1, args );
    } else {
        body->SetItem( 0, type );
        body->SetItem( 1, args );
    }

    return body;
}

PyTuple* PyObjectEx_Type1::_CreateHeader( PyObjectEx_Type1* args1, PyTuple* args2, bool enclosed /*false*/ )
{
    if (args2 == nullptr )
        args2 = new PyTuple( 0 );

    PyTuple* body = new PyTuple( 2 );
    if (enclosed) {
        PyTuple* body1 = new PyTuple( 1 );
            body1->SetItem( 0, args1 );

        body->SetItem( 0, body1 );
        body->SetItem( 1, args2 );
    } else {
        body->SetItem( 0, args1 );
        body->SetItem( 1, args2 );
    }

    return body;
}

PyTuple* PyObjectEx_Type1::_CreateHeader( PyToken* type, PyTuple* args, PyDict* keywords, bool enclosed /*false*/ )
{
    if (args == nullptr )
        args = new PyTuple( 0 );

    PyTuple* body = new PyTuple( keywords == nullptr ? 2 : 3 );
        body->SetItem( 0, type );
        body->SetItem( 1, args );
        if (body->size() > 2 )
            body->SetItem( 2, keywords );
    if (enclosed)
        codelog(COMMON__WARNING, "This constructor is used.  please finish code.");

    return body;
}

PyTuple* PyObjectEx_Type1::_CreateHeader( PyToken* type, PyTuple* args, PyList* keywords, bool enclosed /*false*/ )
{
    if (args == nullptr )
        args = new PyTuple( 0 );

    PyTuple* body = new PyTuple( keywords == nullptr ? 2 : 3 );
        body->SetItem( 0, type );
        body->SetItem( 1, args );
    if (body->size() > 2 )
        body->SetItem( 2, keywords );
    if (enclosed)
        codelog(COMMON__WARNING, "This constructor is used.  please finish code.");

    return body;
}

/************************************************************************/
/* PyObjectEx_Type2                                                     */
/************************************************************************/
PyObjectEx_Type2::PyObjectEx_Type2( PyTuple* args, PyDict* keywords, bool enclosed /*false*/  )
: PyObjectEx( true, _CreateHeader( args, keywords, enclosed)) { }
PyObjectEx_Type2::PyObjectEx_Type2( PyToken* args, PyDict* keywords, bool enclosed /*false*/  )
: PyObjectEx( true, _CreateHeader( args, keywords, enclosed)) { }

PyTuple* PyObjectEx_Type2::GetArgs() const
{
    assert( mHeader != nullptr );
    return mHeader->AsTuple()->GetItem( 0 )->AsTuple();
}

PyDict* PyObjectEx_Type2::GetKeywords() const
{
    assert( mHeader != nullptr );
    return mHeader->AsTuple()->GetItem( 1 )->AsDict();
}

PyRep* PyObjectEx_Type2::FindKeyword( const char* keyword ) const
{
    PyDict* kw = GetKeywords();
    PyDict::const_iterator cur = kw->begin();
    for (; cur != kw->end(); ++cur)
        if ( cur->first->IsString() )
            if ( cur->first->AsString()->content() == keyword )
                return cur->second;

    return nullptr;
}

PyTuple* PyObjectEx_Type2::_CreateHeader( PyTuple* args, PyDict* keywords, bool enclosed /*false*/  )
{
    assert( args != nullptr );
    if (keywords == nullptr )
        keywords = new PyDict();

    PyTuple* body = new PyTuple( 2 );
        body->SetItem( 0, args );
        body->SetItem( 1, keywords );
        /*
    if (enclosed) {
        PyTuple* head = new PyTuple( 1 );
            head->SetItem( 0, body );
    } */

    return body; // head;
}

PyTuple* PyObjectEx_Type2::_CreateHeader( PyToken* args, PyDict* keywords, bool enclosed /*false*/  )
{
    assert( args != nullptr );
    if (keywords == nullptr )
        keywords = new PyDict();

    PyTuple* body = new PyTuple( 2 );
        body->SetItem( 0, args );
        body->SetItem( 1, keywords );

    if (enclosed) {
        PyTuple* head = new PyTuple( 1 );
            head->SetItem( 0, body );
    }

    return body; // head;
}

/************************************************************************/
/* PyPackedRow                                                          */
/************************************************************************/
PyPackedRow::PyPackedRow(DBRowDescriptor* header)
: PyRep(PyRep::PyTypePackedRow), mHeader(header), mFields(new PyList(header->ColumnCount()) ) { }
PyPackedRow::PyPackedRow(const PyPackedRow& oth )
: PyRep(PyRep::PyTypePackedRow), mHeader(oth.header()), mFields(oth.mFields)
{
    //sLog.Cyan("PyPackedRow()", "Copy C'tor.");
}

PyPackedRow::~PyPackedRow()
{
    PyDecRef( mHeader );
    PyDecRef( mFields );
}

PyRep* PyPackedRow::Clone() const
{
    //sLog.Magenta("PyPackedRow()", "Clone.");
    return new PyPackedRow(*this);
}

bool PyPackedRow::visit( PyVisitor& v ) const
{
    return v.VisitPackedRow( this );
}

bool PyPackedRow::SetField( uint32 index, PyRep* value )
{
    if (!mHeader->VerifyValue( index, value ) )  {
        PyDecRef( value );
        return false;
    }

    mFields->SetItem( index, value );
    return true;
}

bool PyPackedRow::SetField( const char* colName, PyRep* value )
{
    return SetField( mHeader->FindColumn( colName ), value );
}

int32 PyPackedRow::hash() const
{
    assert(false);
    return PyRep::hash();
}

PyPackedRow& PyPackedRow::operator=( const PyPackedRow& oth )
{
    *mFields = *oth.mFields;

    return *this;
}

/************************************************************************/
/* PyRep SubStruct Class                                                */
/************************************************************************/
PySubStruct::PySubStruct( PyRep* t ) : PyRep( PyRep::PyTypeSubStruct ), mSub( t ) {}
PySubStruct::PySubStruct( const PySubStruct& oth ) : PyRep( PyRep::PyTypeSubStruct ), mSub( oth.sub()->Clone() )
{
    //sLog.Cyan("PySubStruct()", "Copy C'tor.");
}

PySubStruct::~PySubStruct()
{
    PyDecRef( mSub );
}

PyRep* PySubStruct::Clone() const
{
    //sLog.Magenta("PySubStruct()", "Clone.");
    return new PySubStruct(*this);
}

bool PySubStruct::visit( PyVisitor& v ) const
{
    return v.VisitSubStruct( this );
}

/************************************************************************/
/* PyRep SubStream Class                                                */
/************************************************************************/
PySubStream::PySubStream(PyRep* rep ) : PyRep( PyRep::PyTypeSubStream ), mData( nullptr ), mDecoded( rep ) {}
PySubStream::PySubStream(PyBuffer* buffer ): PyRep(PyRep::PyTypeSubStream), mData(  buffer ), mDecoded( nullptr ) {}
PySubStream::PySubStream(const PySubStream& oth )
: PyRep(PyRep::PyTypeSubStream),
  mData( oth.data() == nullptr ? nullptr : new PyBuffer( *oth.data() ) ),
  mDecoded( oth.decoded() == nullptr ? nullptr : oth.decoded()->Clone() )
{
    //sLog.Cyan("PySubStream()", "Copy C'tor.");
}

PySubStream::~PySubStream()
{
    PySafeDecRef( mData );
    PySafeDecRef( mDecoded );
}

PyRep* PySubStream::Clone() const
{
    //sLog.Magenta("PySubStream()", "Clone.");
    return new PySubStream(*this);
}

bool PySubStream::visit( PyVisitor& v ) const
{
    return v.VisitSubStream( this );
}

void PySubStream::EncodeData() const
{
    if ((mDecoded == nullptr) or (mData != nullptr))
        return;

    Buffer* buf = new Buffer();
    if (!Marshal( mDecoded, *buf ) ) {
        sLog.Error( "Marshal", "Failed to marshal rep %p.", mDecoded );
        SafeDelete( buf );
        return;
    }

    // Move ownership of Buffer to PyBuffer
    mData = new PyBuffer( &buf );
}

void PySubStream::DecodeData() const
{
    if ((mData == nullptr) or (mDecoded != nullptr))
        return;

    mDecoded = Unmarshal( mData->content() );
}

/************************************************************************/
/* PyRep ChecksumedStream Class                                         */
/************************************************************************/
PyChecksumedStream::PyChecksumedStream(PyRep* t, uint32 sum )
: PyRep(PyRep::PyTypeChecksumedStream), mStream(t), mChecksum(sum) { }
PyChecksumedStream::PyChecksumedStream(const PyChecksumedStream& oth )
: PyRep(PyRep::PyTypeChecksumedStream), mStream(oth.stream()->Clone()), mChecksum( oth.checksum())
{
    //sLog.Cyan("PyChecksumedStream()", "Copy C'tor.");
}

PyChecksumedStream::~PyChecksumedStream()
{
    PyDecRef( mStream );
}

PyRep* PyChecksumedStream::Clone() const
{
    //sLog.Magenta("PyChecksumedStream()", "Clone.");
    return new PyChecksumedStream(*this);
}

bool PyChecksumedStream::visit( PyVisitor& v ) const
{
    return v.VisitChecksumedStream( this );
}


/************************************************************************/
/* tuple large integer helper functions                                 */
/************************************************************************/
PyTuple* new_tuple(int64 arg1)
{
    PyTuple* res = new PyTuple(1);
        res->SetItem(0, new PyLong(arg1));
    return res;
}

PyTuple* new_tuple(int64 arg1, int64 arg2)
{
    PyTuple* res = new PyTuple(2);
        res->SetItem(0, new PyLong(arg1));
        res->SetItem(1, new PyLong(arg2));
    return res;
}

/************************************************************************/
/* tuple string helper functions                                        */
/************************************************************************/
PyTuple* new_tuple(const char* arg1)
{
    PyTuple* res = new PyTuple(1);
        res->SetItem(0, new PyString(arg1));
    return res;
}

PyTuple* new_tuple(const char* arg1, const char* arg2)
{
    PyTuple* res = new PyTuple(2);
        res->SetItem(0, new PyString(arg1));
        res->SetItem(1, new PyString(arg2));
    return res;
}

PyTuple* new_tuple(const char* arg1, const char* arg2, const char* arg3)
{
    PyTuple* res = new PyTuple(3);
        res->SetItem(0, new PyString(arg1));
        res->SetItem(1, new PyString(arg2));
        res->SetItem(2, new PyString(arg3));
    return res;
}

/************************************************************************/
/* mixed tuple helper functions                                         */
/************************************************************************/
PyTuple* new_tuple(const char* arg1, const char* arg2, PyTuple* arg3)
{
    PyTuple* res = new PyTuple(3);
        res->SetItem(0, new PyString(arg1));
        res->SetItem(1, new PyString(arg2));
        res->SetItem(2, arg3);
    return res;
}

PyTuple* new_tuple(const char* arg1, PyRep* arg2, PyRep* arg3)
{
    PyTuple* res = new PyTuple(3);
        res->SetItem(0, new PyString(arg1));
        res->SetItem(1, arg2);
        res->SetItem(2, arg3);
    return res;
}

PyTuple* new_tuple(PyRep* arg1, PyRep* arg2, PyRep* arg3)
{
    PyTuple* res = new PyTuple(3);
        res->SetItem(0, arg1);
        res->SetItem(1, arg2);
        res->SetItem(2, arg3);
    return res;
}

PyTuple* new_tuple( PyRep* arg1, PyRep* arg2 )
{
    PyTuple* res = new PyTuple(2);
        res->SetItem(0, arg1);
        res->SetItem(1, arg2);
    return res;
}

PyTuple* new_tuple( PyRep* arg1 )
{
    PyTuple* res = new PyTuple(1);
        res->SetItem(0, arg1);
    return res;
}
