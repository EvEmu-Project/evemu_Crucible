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

#include "EVECommonPCH.h"

#include "marshal/EVEMarshal.h"
#include "marshal/EVEUnmarshal.h"
#include "marshal/EVEMarshalOpcodes.h"
#include "python/classes/PyDatabase.h"
#include "python/PyDumpVisitor.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"
#include "utils/EVEUtils.h"

/************************************************************************/
/* PyRep base Class                                                     */
/************************************************************************/
const char* const PyRep::s_mTypeString[] =
{
    "Integer",          //0
    "Long",             //1
    "Real",             //2
    "Boolean",          //3
    "Buffer",           //4
    "String",           //5
    "WString",          //6
    "Token",            //7
    "Tuple",            //8
    "List",             //9
    "Dict",             //10
    "None",             //11
    "SubStruct",        //12
    "SubStream",        //13
    "ChecksumedStream", //14
    "Object",           //15
    "ObjectEx",         //16
    "PackedRow",        //17
    "UNKNOWN TYPE",     //18
};

PyRep::PyRep( PyType t ) : RefObject( 1 ), mType( t ) {}
PyRep::~PyRep() {}

const char* PyRep::TypeString() const
{
    if( mType >= 0 && mType < PyTypeMax )
        return s_mTypeString[ mType ];

    return s_mTypeString[ PyTypeError ];
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

/************************************************************************/
/* PyRep Integer Class                                                  */
/************************************************************************/
PyInt::PyInt( const int32 i ) : PyRep( PyRep::PyTypeInt ), mValue( i ) {}
PyInt::PyInt( const PyInt& oth ) : PyRep( PyRep::PyTypeInt ), mValue( oth.value() ) {}

PyRep* PyInt::Clone() const
{
    return new PyInt( *this );
}

bool PyInt::visit( PyVisitor& v ) const
{
    return v.VisitInteger( this );
}

int32 PyInt::hash() const
{
    /* XXX If this is changed, you also need to change the way
    Python's long, float and complex types are hashed. */
    int32 x = mValue;
    if( x == -1 )
        x = -2;
    return x;
}

/************************************************************************/
/* PyRep Long Class                                                     */
/************************************************************************/
PyLong::PyLong( const int64 i ) : PyRep( PyRep::PyTypeLong ), mValue( i ) {}
PyLong::PyLong( const PyLong& oth ) : PyRep( PyRep::PyTypeLong ), mValue( oth.value() ) {}

PyRep* PyLong::Clone() const
{
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

#define LONG_BIT_PyLong_SHIFT	(8*sizeof(long) - PyLong_SHIFT)

    long x;
    int i;
    int sign;

    /* This is designed so that Python ints and longs with the
    same value hash to the same value, otherwise comparisons
    of mapping keys will turn out weird */
    i = 8;
    sign = 1;
    x = 0;
    if( i < 0 ) {
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
        if( (unsigned long)x < ((uint8*)&mValue)[i] )//v->ob_digit[i])
            x++;
    }
    x = x * sign;
    if( x == -1 )
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
PyFloat::PyFloat( const double& i ) : PyRep( PyRep::PyTypeFloat ), mValue( i ) {}
PyFloat::PyFloat( const PyFloat& oth ) : PyRep( PyRep::PyTypeFloat ), mValue( oth.value() ) {}

PyRep* PyFloat::Clone() const
{
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
    double intpart, fractpart;
    int expo;
    long hipart;
    long x;		/* the final hash value */
    /* This is designed so that Python numbers of different types
    * that compare equal hash to the same value; otherwise comparisons
    * of mapping keys will turn out weird.
    */

    fractpart = modf(v, &intpart);
    if (fractpart == 0.0) {
        /* This must return the same hash as an equal int or long. */
        if (intpart > INT32_MAX || -intpart > INT32_MAX) {
            /* Convert to long and use its hash. */
            PyRep *plong;	/* converted to Python long */
            if (Py_IS_INFINITY(intpart))
                /* can't convert to long int -- arbitrary */
                v = v < 0 ? -271828.0 : 314159.0;
            //plong = PyLong_FromDouble(v);

            plong = new PyLong( (int64)v ); // this is a hack
            if (plong == NULL)
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
    v *= 2147483648.0;	/* 2**31 */
    hipart = (long)v;	/* take the top 32 bits */
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
PyBool::PyBool( bool i ) : PyRep( PyRep::PyTypeBool ), mValue( i ) {}
PyBool::PyBool( const PyBool& oth ) : PyRep( PyRep::PyTypeBool ), mValue( oth.value() ) {}

PyRep* PyBool::Clone() const
{
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
PyNone::PyNone( const PyNone& oth ) : PyRep( PyRep::PyTypeNone ) {}

PyRep* PyNone::Clone() const
{
    return new PyNone( *this );
}

bool PyNone::visit( PyVisitor& v ) const
{
	return v.VisitNone( this );
}

int32 PyNone::hash() const
{
    /* damn hack... bleh.. but its done like this... in python and PyNone is a static singleton....*/
    int32* hash = (int32*)this;
    return *((int32*)&hash);
}

/************************************************************************/
/* PyRep Buffer Class                                                   */
/************************************************************************/
PyBuffer::PyBuffer( size_t len, const uint8& value ) : PyRep( PyRep::PyTypeBuffer ), mValue( new Buffer( len, value ) ), mHashCache( -1 ) {}
PyBuffer::PyBuffer( const Buffer& buffer ) : PyRep( PyRep::PyTypeBuffer ), mValue( new Buffer( buffer ) ), mHashCache( -1 ) {}

PyBuffer::PyBuffer( Buffer** buffer ) : PyRep( PyRep::PyTypeBuffer ), mValue( *buffer ), mHashCache( -1 ) { *buffer = NULL; }
PyBuffer::PyBuffer( const PyString& str ) : PyRep( PyRep::PyTypeBuffer ), mValue( new Buffer( str.content().begin(), str.content().end() ) ), mHashCache( -1 ) {}
PyBuffer::PyBuffer( const PyBuffer& buffer ) : PyRep( PyRep::PyTypeBuffer ), mValue( new Buffer( buffer.content() ) ), mHashCache( buffer.mHashCache ) {}

PyBuffer::~PyBuffer() { delete mValue; }

PyRep* PyBuffer::Clone() const
{
	return new PyBuffer( *this );
}

bool PyBuffer::visit( PyVisitor& v ) const
{
	return v.VisitBuffer( this );
}

int32 PyBuffer::hash() const
{
    if( mHashCache != -1 )
        return mHashCache;

    register int len;
    register unsigned char *p;
    register long x;

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
    len = content().size();
    x = *p << 7;
    while( --len >= 0 )
        x = (1000003*x) ^ *p++;
    x ^= content().size();
    if( x == -1 )
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
PyString::PyString( const char* str ) : PyRep( PyRep::PyTypeString ), mValue( str ), mHashCache( -1 ) {}
PyString::PyString( const char* str, size_t len ) : PyRep( PyRep::PyTypeString ), mValue( str, len ), mHashCache( -1 ) {}
PyString::PyString( const std::string& str ) : PyRep( PyRep::PyTypeString ), mValue( str ), mHashCache( -1 ) {}

PyString::PyString( const PyBuffer& buf ) : PyRep( PyRep::PyTypeString ), mValue( (const char *) &buf.content()[0], buf.content().size() ), mHashCache( -1 ) {}
PyString::PyString( const PyToken& token ) : PyRep( PyRep::PyTypeString ), mValue( token.content() ), mHashCache( -1 ) {}
PyString::PyString( const PyString& oth ) : PyRep( PyRep::PyTypeString ), mValue( oth.mValue ), mHashCache( oth.mHashCache ) {}

PyRep* PyString::Clone() const
{
	return new PyString( *this );
}

bool PyString::visit( PyVisitor& v ) const
{
	return v.VisitString( this );
}

int32 PyString::hash() const
{
    if( mHashCache != -1 )
        return mHashCache;

    register int len;
    register unsigned char *p;
    register int32 x;

    len = mValue.length();
    p = (unsigned char *) mValue.c_str();
    x = *p << 7;
    while (--len >= 0)
        x = (1000003*x) ^ *p++;
    x ^= mValue.length();
    if (x == -1)
        x = -2;

    mHashCache = x;
    return x;
}

/************************************************************************/
/* PyWString                                                            */
/************************************************************************/
PyWString::PyWString( const char* str, size_t len ) : PyRep( PyRep::PyTypeWString ), mValue( str, len ), mHashCache( -1 ) {}
PyWString::PyWString( const std::string& str ) : PyRep( PyRep::PyTypeWString ), mValue( str ), mHashCache( -1 ) {}

PyWString::PyWString( const PyString& str ) : PyRep( PyRep::PyTypeWString ), mValue( str.content() ), mHashCache( -1 ) {}
PyWString::PyWString( const PyWString& oth ) : PyRep( PyRep::PyTypeWString ), mValue( oth.mValue ), mHashCache( oth.mHashCache ) {}

PyRep* PyWString::Clone() const
{
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
    if( mHashCache != -1 )
        return mHashCache;

    register int len;
    register unsigned char *p;
    register int32 x;

    len = mValue.length();
    p = (unsigned char *) mValue.c_str();
    x = *p << 7;
    while (--len >= 0)
        x = (1000003*x) ^ *p++;
    x ^= mValue.length();
    if (x == -1)
        x = -2;

    mHashCache = x;
    return x;
}

/************************************************************************/
/* PyToken                                                              */
/************************************************************************/
PyToken::PyToken( const char* token ) : PyRep( PyRep::PyTypeToken ), mValue( token ) {}
PyToken::PyToken( const char* token, size_t len ) : PyRep( PyRep::PyTypeToken ), mValue( token, len ) {}
PyToken::PyToken( const std::string& token ) : PyRep( PyRep::PyTypeToken ), mValue( token ) {}

PyToken::PyToken( const PyString& token ) : PyRep( PyRep::PyTypeToken ), mValue( token.content() ) {}
PyToken::PyToken( const PyToken& oth ) : PyRep( PyRep::PyTypeToken ), mValue( oth.content() ) {}

PyRep* PyToken::Clone() const
{
    return new PyToken( *this );
}

bool PyToken::visit( PyVisitor& v ) const
{
    return v.VisitToken( this );
}

/************************************************************************/
/* PyRep Tuple Class                                                    */
/************************************************************************/
PyTuple::PyTuple( size_t item_count ) : PyRep( PyRep::PyTypeTuple ), items( item_count, NULL ) {}
PyTuple::PyTuple( const PyTuple& oth ) : PyRep( PyRep::PyTypeTuple ), items()
{
	// Use assigment operator
	*this = oth;
}

PyTuple::~PyTuple()
{
    clear();
}

PyRep* PyTuple::Clone() const
{
	return new PyTuple( *this );
}

bool PyTuple::visit( PyVisitor& v ) const
{
	return v.VisitTuple( this );
}

void PyTuple::clear()
{
    iterator cur, end;
    cur = items.begin();
    end = items.end();
    for(; cur != end; cur++)
        PySafeDecRef( *cur );

    items.clear();
}

PyTuple& PyTuple::operator=( const PyTuple& oth )
{
	clear();
	items.resize( oth.size() );

    iterator cur, end;
    cur = items.begin();
    end = items.end();

	const_iterator cur_oth, end_oth;
    cur_oth = oth.begin();
    end_oth = oth.end();
    for(; cur != end && cur_oth != end_oth; cur++, cur_oth++)
	{
		if( *cur_oth == NULL )
			*cur = NULL;
		else
			*cur = (*cur_oth)->Clone();
	}

	return *this;
}

int32 PyTuple::hash() const
{
    register long x, y;
    register int len = items.size();
    register long index = 0;
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
    return x;
}

/************************************************************************/
/* PyRep List Class                                                     */
/************************************************************************/
PyList::PyList( size_t item_count ) : PyRep( PyRep::PyTypeList ), items( item_count, NULL ) {}
PyList::PyList( const PyList& oth ) : PyRep( PyRep::PyTypeList ), items()
{
	// Use assigment operator
	*this = oth;
}

PyList::~PyList()
{
    clear();
}

PyRep* PyList::Clone() const
{
	return new PyList( *this );
}

bool PyList::visit( PyVisitor& v ) const
{
	return v.VisitList( this );
}

void PyList::clear()
{
    iterator cur, end;
    cur = items.begin();
    end = items.end();
    for(; cur != end; cur++)
        PySafeDecRef( *cur );

    items.clear();
}

PyList& PyList::operator=( const PyList& oth )
{
	clear();
	items.resize( oth.size() );

	iterator cur, end;
	cur = items.begin();
	end = items.end();

	const_iterator cur_oth, end_oth;
	cur_oth = oth.begin();
	end_oth = oth.end();

	for(; cur != end && cur_oth != end_oth; cur++, cur_oth++)
	{
		if( *cur_oth == NULL )
			*cur = NULL;
		else
			*cur = (*cur_oth)->Clone();
	}

	return *this;
}

/************************************************************************/
/* PyRep Dict Class                                                     */
/************************************************************************/
PyDict::PyDict() : PyRep( PyRep::PyTypeDict ), items() {}
PyDict::PyDict( const PyDict& oth ) : PyRep( PyRep::PyTypeDict ), items()
{
	// Use assigment operator
	*this = oth;
}

PyDict::~PyDict()
{
    clear();
}

PyRep* PyDict::Clone() const
{
	return new PyDict( *this );
}

bool PyDict::visit( PyVisitor& v ) const
{
	return v.VisitDict( this );
}

void PyDict::clear()
{
    iterator cur, end;
    cur = items.begin();
    end = items.end();
    for(; cur != end; cur++)
	{
        PyDecRef( cur->first );
        PySafeDecRef( cur->second );
    }

	items.clear();
}

PyRep* PyDict::GetItem( PyRep* key ) const
{
    /* make sure we have valid arguments */
    assert( key );

    const_iterator res = items.find( key );
    if( res == items.end() )
        return NULL;

    return res->second;
}

PyRep* PyDict::GetItemString( const char* key ) const
{
    /* make sure we have valid arguments */
    assert( key );

    PyString* str = new PyString( key );
    PyRep* res = GetItem( str );
    PyDecRef( str );

    return res;
}

void PyDict::SetItem( PyRep* key, PyRep* value )
{
    /* make sure we have valid arguments */
	assert( key );

    /* note: add check if the key object is hashable
     * if not ( it will return -1 then ) return false;
     */

    /* note: needs to be enabled when object reference is working.
     */
    //PyIncRef( key );
    //PyIncRef( value );

    /* check if we need to replace a dictionary entry */
    iterator itr = items.find( key );
    if( itr == items.end() )
    {
		// Keep both key & value
        items.insert( std::make_pair( key, value ) );
    }
	else
    {
		// We don't need it anymore, we're using itr->first.
		PyDecRef( key );

		// Replace itr->second with value.
        PySafeDecRef( itr->second );
        itr->second = value;
    }
}

PyDict& PyDict::operator=( const PyDict& oth )
{
	clear();

	const_iterator cur, end;
    cur = oth.begin();
    end = oth.end();
    for(; cur != end; cur++)
	{
		if( cur->second == NULL )
			SetItem( cur->first->Clone(), NULL );
		else
			SetItem( cur->first->Clone(), cur->second->Clone() );
	}

	return *this;
}

/************************************************************************/
/* PyRep Object Class                                                   */
/************************************************************************/
PyObject::PyObject( PyString* type, PyRep* args ) : PyRep( PyRep::PyTypeObject ), mType( type ), mArguments( args ) {}
PyObject::PyObject( const PyObject& oth ) : PyRep( PyRep::PyTypeObject ), mType( new PyString( *oth.type() ) ), mArguments( oth.arguments()->Clone() ) {}
PyObject::~PyObject()
{
    PyDecRef( mType );
    PyDecRef( mArguments );
}

PyRep* PyObject::Clone() const
{
	return new PyObject( *this );
}

bool PyObject::visit( PyVisitor& v ) const
{
	return v.VisitObject( this );
}

/************************************************************************/
/* PyObjectEx                                                           */
/************************************************************************/
PyObjectEx::PyObjectEx( bool is_type_2, PyRep* header ) : PyRep( PyRep::PyTypeObjectEx ), mHeader( header ), mIsType2( is_type_2 ), mList( new PyList ), mDict( new PyDict ) {}
PyObjectEx::PyObjectEx( const PyObjectEx& oth ) : PyRep( PyRep::PyTypeObjectEx ),
  mHeader( oth.header()->Clone() ), mIsType2( oth.isType2() ), mList( new PyList ), mDict( new PyDict )
{
	// Use assigment operator
	*this = oth;
}

PyObjectEx::~PyObjectEx()
{
    PySafeDecRef( mHeader );

    PyDecRef( mList );
    PyDecRef( mDict );
}

PyRep* PyObjectEx::Clone() const
{
	return new PyObjectEx( *this );
}

bool PyObjectEx::visit( PyVisitor& v ) const
{
	return v.VisitObjectEx( this );
}

PyObjectEx& PyObjectEx::operator=( const PyObjectEx& oth )
{
	list() = oth.list();
	dict() = oth.dict();

	return *this;
}

/************************************************************************/
/* PyObjectEx_Type1                                                     */
/************************************************************************/
PyObjectEx_Type1::PyObjectEx_Type1( PyToken* type, PyTuple* args, PyDict* keywords ) : PyObjectEx( false, _CreateHeader( type, args, keywords ) ) {}

PyToken* PyObjectEx_Type1::GetType() const
{
    assert( header() );
    return header()->AsTuple()->GetItem( 0 )->AsToken();
}

PyTuple* PyObjectEx_Type1::GetArgs() const
{
    assert( header() );
    return header()->AsTuple()->GetItem( 1 )->AsTuple();
}

PyDict* PyObjectEx_Type1::GetKeywords() const
{
	// This one is slightly more complicated since
	// keywords are optional.
	assert( header() );
	PyTuple* t = header()->AsTuple();

	if( t->size() < 3 )
		t->items.push_back( new PyDict );

	return t->GetItem( 2 )->AsDict();
}

PyRep* PyObjectEx_Type1::FindKeyword( const char* keyword ) const
{
	PyDict* kw = GetKeywords();

	PyDict::const_iterator cur, end;
	cur = kw->begin();
	end = kw->end();
	for(; cur != end; cur++)
	{
		if( cur->first->IsString() )
			if( cur->first->AsString()->content() == keyword )
				return cur->second;
	}

	return NULL;
}

PyTuple* PyObjectEx_Type1::_CreateHeader( PyToken* type, PyTuple* args, PyDict* keywords )
{
	if( args == NULL )
		args = new PyTuple( 0 );

	PyTuple* head = new PyTuple( keywords == NULL ? 2 : 3 );
	head->SetItem( 0, type );
	head->SetItem( 1, args );
	if( head->size() > 2 )
		head->SetItem( 2, keywords );

	return head;
}

/************************************************************************/
/* PyObjectEx_Type2                                                     */
/************************************************************************/
PyObjectEx_Type2::PyObjectEx_Type2( PyTuple* args, PyDict* keywords ) : PyObjectEx( true, _CreateHeader( args, keywords ) ) {}

PyTuple* PyObjectEx_Type2::GetArgs() const
{
    assert( header() );
    return header()->AsTuple()->GetItem( 0 )->AsTuple();
}

PyDict* PyObjectEx_Type2::GetKeywords() const
{
    assert( header() );
    return header()->AsTuple()->GetItem( 1 )->AsDict();
}

PyRep* PyObjectEx_Type2::FindKeyword( const char* keyword ) const
{
	PyDict* kw = GetKeywords();

	PyDict::const_iterator cur, end;
	cur = kw->begin();
	end = kw->end();
	for(; cur != end; cur++)
	{
		if( cur->first->IsString() )
			if( cur->first->AsString()->content() == keyword )
				return cur->second;
	}

	return NULL;
}

PyTuple* PyObjectEx_Type2::_CreateHeader( PyTuple* args, PyDict* keywords )
{
	assert( args );
	if( keywords == NULL )
		keywords = new PyDict;

	PyTuple *head = new PyTuple( 2 );
	head->SetItem( 0, args );
	head->SetItem( 1, keywords );

	return head;
}

/************************************************************************/
/* PyPackedRow                                                          */
/************************************************************************/
PyPackedRow::PyPackedRow( DBRowDescriptor* header ) : PyRep( PyRep::PyTypePackedRow ), mHeader( header ), mFields( new PyList( header->ColumnCount() ) ) {}
PyPackedRow::PyPackedRow( const PyPackedRow& oth ) : PyRep( PyRep::PyTypePackedRow ),
  mHeader( oth.header() ), mFields( new PyList( oth.header()->ColumnCount() ) )
{
    PyIncRef( mHeader );

	*this = oth;
}

PyPackedRow::~PyPackedRow()
{
    PyDecRef( mHeader );
    PyDecRef( mFields );
}

PyRep* PyPackedRow::Clone() const
{
	return new PyPackedRow( *this );
}

bool PyPackedRow::visit( PyVisitor& v ) const
{
	return v.VisitPackedRow( this );
}

bool PyPackedRow::SetField( uint32 index, PyRep* value )
{
    if( !header()->VerifyValue( index, value ) )
    {
        PyDecRef( value );
        return false;
    }

	mFields->SetItem( index, value );
    return true;
}

bool PyPackedRow::SetField( const char* colName, PyRep* value )
{
    return SetField( header()->FindColumn( colName ), value );
}

PyPackedRow& PyPackedRow::operator=( const PyPackedRow& oth )
{
	*mFields = *oth.mFields;

	return *this;
}

int32 PyPackedRow::hash() const
{
    assert(false);
	return PyRep::hash();
}

/************************************************************************/
/* PyRep SubStruct Class                                                */
/************************************************************************/
PySubStruct::PySubStruct( PyRep* t ) : PyRep( PyRep::PyTypeSubStruct ), mSub( t ) {}
PySubStruct::PySubStruct( const PySubStruct& oth ) : PyRep( PyRep::PyTypeSubStruct ), mSub( oth.sub()->Clone() ) {}
PySubStruct::~PySubStruct()
{
    PyDecRef( mSub );
}

PyRep* PySubStruct::Clone() const
{
	return new PySubStruct( *this );
}

bool PySubStruct::visit( PyVisitor& v ) const
{
	return v.VisitSubStruct( this );
}

/************************************************************************/
/* PyRep SubStream Class                                                */
/************************************************************************/
PySubStream::PySubStream( PyRep* rep ) : PyRep( PyRep::PyTypeSubStream ), mData( NULL ), mDecoded( rep ) {}
PySubStream::PySubStream( PyBuffer* buffer ): PyRep(PyRep::PyTypeSubStream), mData(  buffer ), mDecoded( NULL ) {}
PySubStream::PySubStream( const PySubStream& oth ) : PyRep(PyRep::PyTypeSubStream),
  mData( oth.data() == NULL ? NULL : new PyBuffer( *oth.data() ) ), mDecoded( oth.decoded() == NULL ? NULL : oth.decoded()->Clone() ) {}

PySubStream::~PySubStream()
{
	PySafeDecRef( mData );
    PySafeDecRef( mDecoded );
}

PyRep* PySubStream::Clone() const
{
	return new PySubStream( *this );
}

bool PySubStream::visit( PyVisitor& v ) const
{
	return v.VisitSubStream( this );
}

void PySubStream::EncodeData() const
{
    if( decoded() == NULL || data() != NULL )
        return;

    Buffer* buf = new Buffer;
    if( !Marshal( decoded(), *buf ) )
    {
        sLog.Error( "Marshal", "Failed to marshal rep %p.", decoded() );

        SafeDelete( buf );
        return;
    }

	// Move ownership of Buffer to PyBuffer
	mData = new PyBuffer( &buf );
}

void PySubStream::DecodeData() const
{
    if( data() == NULL || decoded() != NULL )
        return;

    mDecoded = Unmarshal( data()->content() );
}

/************************************************************************/
/* PyRep ChecksumedStream Class                                         */
/************************************************************************/
PyChecksumedStream::PyChecksumedStream( PyRep* t, uint32 sum ) : PyRep( PyRep::PyTypeChecksumedStream ), mStream( t ), mChecksum( sum ) {}
PyChecksumedStream::PyChecksumedStream( const PyChecksumedStream& oth ) : PyRep( PyRep::PyTypeChecksumedStream ), mStream( oth.stream()->Clone() ), mChecksum( oth.checksum() ) {}
PyChecksumedStream::~PyChecksumedStream()
{
    PyDecRef( mStream );
}

PyRep* PyChecksumedStream::Clone() const
{
	return new PyChecksumedStream( *this );
}

bool PyChecksumedStream::visit( PyVisitor& v ) const
{
	return v.VisitChecksumedStream( this );
}


