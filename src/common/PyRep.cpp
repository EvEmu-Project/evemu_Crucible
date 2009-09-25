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

#include "common.h"
#include "misc.h"
#include "packet_functions.h"
#include "PyVisitor.h"
#include "PyRep.h"
#include "EVEUtils.h"
#include "EVEMarshal.h"
#include "EVEUnmarshal.h"
#include "EVEMarshalOpcodes.h"
#include "DBRowDescriptor.h"
#include "LogNew.h"
#include "utils_hex.h"
#include "utils_string.h"
#include <float.h>

/************************************************************************/
/* PyRep utilities                                                      */
/************************************************************************/
/** Lookup table for PyRep type object type names.
  */
const char *PyRepTypeString[] = {
    "Integer",          //0
    "Long",             //1
    "Real",             //2
    "Boolean",          //3
    "Buffer",           //4
    "String",           //5
    "Tuple",            //6
    "List",             //7
    "Dict",             //8
    "None",             //9
    "SubStruct",        //10
    "SubStream",        //11
    "ChecksumedStream", //12
    "Object",           //13
    "ObjectEx",         //14
    "PackedRow",        //15
    "UNKNOWN TYPE",     //16
};

/************************************************************************/
/* PyRep base Class                                                     */
/************************************************************************/

PyRep::PyRep(PyType t) : m_type(t), mRefCnt(1) {}
PyRep::~PyRep() {}

const char* PyRep::TypeString() const
{
    if (m_type >= 0 && m_type < PyTypeMax)
        return PyRepTypeString[m_type];

    return PyRepTypeString[PyTypeError];
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
PyInt::PyInt( const PyInt &oth ) : PyRep( PyRep::PyTypeInt ), mValue( oth.value() ) {}
PyInt::~PyInt() {}

void PyInt::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sInteger field: %d\n", pfx, value());
}

void PyInt::Dump(LogType type, const char *pfx) const {
    _log(type, "%sInteger field: %d", pfx, value());
}

EVEMU_INLINE PyRep *PyInt::Clone() const
{
    return new PyInt( *this );
}

EVEMU_INLINE void PyInt::visit( PyVisitor *v ) const
{
    v->VisitInteger( this );
}

EVEMU_INLINE void PyInt::visit( PyVisitorLvl *v, int64 lvl ) const
{
    v->VisitInteger( this, lvl );
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
PyLong::PyLong( const PyLong &oth ) : PyRep( PyRep::PyTypeLong ), mValue( oth.value() ) {}
PyLong::~PyLong() {}

void PyLong::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sInteger field: "I64d"\n", pfx, value());
}

void PyLong::Dump(LogType type, const char *pfx) const {
    _log(type, "%sInteger field: "I64d, pfx, value());
}

EVEMU_INLINE PyRep *PyLong::Clone() const
{
    return new PyLong( *this );
}

EVEMU_INLINE void PyLong::visit( PyVisitor *v ) const
{
    v->VisitLong(this);
}

EVEMU_INLINE void PyLong::visit( PyVisitorLvl *v, int64 lvl ) const
{
    v->VisitLong(this, lvl);
}

#define PyLong_SHIFT    15
#define PyLong_BASE     (1 << PyLong_SHIFT)
#define PyLong_MASK     ((int)(PyLong_BASE - 1))

int32 PyLong::hash() const
{
    long x;
    size_t i;
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
#define LONG_BIT_PyLong_SHIFT	(8*sizeof(long) - PyLong_SHIFT)
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
#undef LONG_BIT_PyLong_SHIFT
    x = x * sign;
    if( x == -1 )
        x = -2;
    return x;
}

/************************************************************************/
/* PyRep Real/float/double Class                                        */
/************************************************************************/
PyFloat::PyFloat( const double &i ) : PyRep( PyRep::PyTypeFloat ), mValue( i ) {}
PyFloat::PyFloat( const PyFloat &oth ) : PyRep( PyRep::PyTypeFloat ), mValue( oth.value() ) {}
PyFloat::~PyFloat() {}

void PyFloat::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sReal Field: %f\n", pfx, value());
}

void PyFloat::Dump(LogType type, const char *pfx) const {
    _log(type, "%sReal Field: %f", pfx, value());
}

PyRep *PyFloat::Clone() const
{
	return new PyFloat( *this );
}

void PyFloat::visit( PyVisitor *v ) const
{
	v->VisitReal( this );
}

void PyFloat::visit( PyVisitorLvl *v, int64 lvl ) const
{
	v->VisitReal( this, lvl );
}

#define INT32_MAX 2147483647L   /* maximum int32 value */
#define Py_IS_INFINITY( X ) ( !_finite( X ) && !_isnan( X ) )

int32 PyFloat::hash() const
{
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
}

/************************************************************************/
/* PyRep Boolean Class                                                  */
/************************************************************************/
PyBool::PyBool( bool i ) : PyRep( PyRep::PyTypeBool ), mValue( i ) {}
PyBool::PyBool( const PyBool &oth ) : PyRep( PyRep::PyTypeBool ), mValue( oth.value() ) {}
PyBool::~PyBool() {}

void PyBool::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sBoolean field: %s\n", pfx, value()?"true":"false");
}

void PyBool::Dump(LogType type, const char *pfx) const {
    _log(type, "%sBoolean field: %s", pfx, value()?"true":"false");
}

PyRep *PyBool::Clone() const
{
	return new PyBool( *this );
}

void PyBool::visit( PyVisitor *v ) const
{
	v->VisitBoolean( this );
}

void PyBool::visit( PyVisitorLvl *v, int64 lvl ) const
{
	v->VisitBoolean( this, lvl );
}

/************************************************************************/
/* PyRep None Class                                                     */
/************************************************************************/
PyNone::PyNone() : PyRep( PyRep::PyTypeNone ) {}
PyNone::~PyNone() {}

void PyNone::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%s(None)\n", pfx);
}

void PyNone::Dump(LogType type, const char *pfx) const {
    _log(type, "%s(None)", pfx);
}

PyRep *PyNone::Clone() const
{
	return new PyNone( *this );
}

void PyNone::visit( PyVisitor *v ) const
{
	v->VisitNone( this );
}

void PyNone::visit( PyVisitorLvl *v, int64 lvl ) const
{
	v->VisitNone( this, lvl );
}

int32 PyNone::hash() const
{
    /* damn hack... bleh.. but its done like this... in python and PyNone is a static singleton....*/
    int32 * hash = (int32 *)this;
    return *((int32*)&hash);
}

/************************************************************************/
/* PyRep Buffer Class                                                   */
/************************************************************************/
PyBuffer::PyBuffer( const uint8* buffer, size_t size ) : PyRep( PyRep::PyTypeBuffer ),
  mValue( new uint8[ size ] ), mSize( size ), mHashCache( -1 )
{
    assert( mValue );

    memcpy( mValue, buffer, size );
}

PyBuffer::PyBuffer( uint8** buffer, size_t size ) : PyRep( PyRep::PyTypeBuffer ),
  mValue( *buffer ), mSize( size ), mHashCache( -1 )
{
    *buffer = NULL;
}

PyBuffer::PyBuffer( size_t size ) : PyRep( PyRep::PyTypeBuffer ),
  mValue( new uint8[ size ] ), mSize( size ), mHashCache( -1 )
{
    assert( mValue );
}

PyBuffer::PyBuffer( const PyString& str ) : PyRep( PyRep::PyTypeBuffer ),
  mValue( new uint8[ str.content().size() ] ), mSize( str.content().size() ), mHashCache( -1 )
{
    assert( mValue );

	memcpy( mValue, str.content().c_str(), str.content().size() );
}

PyBuffer::PyBuffer( const PyBuffer& buffer ) : PyRep( PyRep::PyTypeBuffer ),
  mValue( new uint8[ buffer.size() ] ), mSize( buffer.size() ), mHashCache( buffer.mHashCache )
{
    assert( mValue );

	memcpy( mValue, buffer.content(), buffer.size() );
}

PyBuffer::~PyBuffer()
{
    delete mValue;
}

void PyBuffer::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sData buffer of length %lu\n", pfx, size());

    //kinda hackish:
    if(size() > 2 && (*this)[0] == GZipHeaderByte) {
        uint32 len = size();
        uint8 *buf = InflatePacket(content(), &len, true);
        if(buf != NULL) {
            string p(pfx);
            p += "  ";
            fprintf(into, "%sData buffer contains gzipped data of length %u\n", p.c_str(), len);
            pfxPreviewHexDump(p.c_str(), into, buf, len);
            SafeFree(buf);
        }
    }
}

void PyBuffer::Dump(LogType type, const char *pfx) const {
    _log(type, "%sData buffer of length %d", pfx, size());

    //kinda hackish:
    if(size() > 2 && (*this)[0] == GZipHeaderByte) {
        uint32 len = size();
        uint8 *buf = InflatePacket(content(), &len, true);
        if(buf != NULL) {
            string p(pfx);
            p += "  ";
            _log(type, "%sData buffer contains gzipped data of length %u", p.c_str(), len);
            pfxPreviewHexDump(p.c_str(), type, buf, len);
            SafeFree(buf);
        }
    }
}

PyRep *PyBuffer::Clone() const
{
	return new PyBuffer( *this );
}

void PyBuffer::visit( PyVisitor *v ) const
{
	v->VisitBuffer( this );
}

void PyBuffer::visit( PyVisitorLvl *v, int64 lvl ) const
{
	v->VisitBuffer( this, lvl );
}

int32 PyBuffer::hash() const
{
    if( mHashCache != -1 )
        return mHashCache;

    void *ptr;
    register size_t len;
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
    ptr = mValue;
    p = (unsigned char *) ptr;
    len = mSize;
    x = *p << 7;
    while( --len >= 0 )
        x = (1000003*x) ^ *p++;
    x ^= mSize;
    if( x == -1 )
        x = -2;
    mHashCache = x;
    return x;
}

/************************************************************************/
/* PyString                                                          */
/************************************************************************/
PyString::PyString( const char* str, bool type_1 ) : PyRep( PyRep::PyTypeString ), mValue( str ), mIsType1( type_1 ), mHashCache( -1 ) {}
PyString::PyString( const std::string& str, bool type_1 ) : PyRep( PyRep::PyTypeString ), mValue( str ), mIsType1( type_1 ), mHashCache( -1 ) {}
PyString::PyString( const PyBuffer& buf, bool type_1 ) : PyRep( PyRep::PyTypeString ), mValue( (const char *) buf.content(), buf.size() ), mIsType1( type_1 ), mHashCache( -1 ) {}
PyString::PyString( const PyString& oth ) : PyRep( PyRep::PyTypeString ), mValue( oth.mValue ), mIsType1( oth.mIsType1 ), mHashCache( oth.mHashCache ) {}
PyString::~PyString() {}

void PyString::Dump(FILE *into, const char *pfx) const {
    if(ContainsNonPrintables( this ) ) {
        fprintf(into, "%sString%s: '<binary, len=%lu>'\n", pfx, isType1()?" (Type1)":"", content().length());
    } else {
        fprintf(into, "%sString%s: '%s'\n", pfx, isType1()?" (Type1)":"", content().c_str());
    }
}

void PyString::Dump(LogType type, const char *pfx) const {
    if(ContainsNonPrintables( this ) ) {
        _log(type, "%sString%s: '<binary, len=%lu>'", pfx, isType1()?" (Type1)":"", content().length());
    } else {
        _log(type, "%sString%s: '%s'", pfx, isType1()?" (Type1)":"", content().c_str());
    }
}

PyRep *PyString::Clone() const
{
	return new PyString( *this );
}

void PyString::visit( PyVisitor *v ) const
{
	v->VisitString( this );
}

void PyString::visit( PyVisitorLvl *v, int64 lvl ) const
{
	v->VisitString( this, lvl );
}

int32 PyString::hash() const
{
    if( mHashCache != -1 )
        return mHashCache;

    register size_t len;
    register unsigned char *p;
    register int32 x;

    len = mValue.length();
    p = (unsigned char *) mValue.c_str();
    x = *p << 7;
    while (--len > 0)
        x = (1000003*x) ^ *p++;
    x ^= mValue.length();
    if (x == -1)
        x = -2;

    mHashCache = x;
    return x;
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

void PyTuple::Dump(FILE *into, const char *pfx) const {
    if(items.empty())
        fprintf(into, "%sTuple: Empty\n", pfx);
    else {
        fprintf(into, "%sTuple: %lu elements\n", pfx, items.size());
        std::vector<PyRep *>::const_iterator cur, _end;
        cur = items.begin();
        _end = items.end();
        char t[15];
        for(int r = 0; cur != _end; cur++, r++) {
            std::string n(pfx);
            snprintf(t, 14, "  [%2d] ", r);
            n += t;
            (*cur)->Dump(into, n.c_str());
        }
    }
}

void PyTuple::Dump(LogType type, const char *pfx) const {
    //extra check to avoid potentially a lot of work if we are disabled
    if(!is_log_enabled(type))
        return;

    if(items.empty()) {
        _log(type, "%sTuple: Empty", pfx);
    }
    else {
        _log(type, "%sTuple: %lu elements", pfx, items.size());
        std::vector<PyRep *>::const_iterator cur, _end;
        cur = items.begin();
        _end = items.end();
        char t[15];
        for(int r = 0; cur != _end; cur++, r++) {
            std::string n(pfx);
            snprintf(t, 14, "  [%2d] ", r);
            n += t;
            (*cur)->Dump(type, n.c_str());
        }
    }
}

PyRep *PyTuple::Clone() const
{
	return new PyTuple( *this );
}

void PyTuple::visit( PyVisitor *v ) const
{
	v->VisitTuple( this );
}

void PyTuple::visit( PyVisitorLvl *v, int64 lvl ) const
{
	v->VisitTuple( this, lvl );
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

PyTuple& PyTuple::operator=(const PyTuple& oth)
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
    register size_t len = items.size();
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

void PyList::Dump(FILE *into, const char *pfx) const {
    if(items.empty())
        fprintf(into, "%sList: Empty\n", pfx);
    else {
        fprintf(into, "%sList: %lu elements\n", pfx, items.size());
        std::vector<PyRep *>::const_iterator cur, _end;
        cur = items.begin();
        _end = items.end();
        char t[15];
        for(int r = 0; cur != _end; cur++, r++) {
            std::string n(pfx);
            snprintf(t, 14, "  [%2d] ", r);
            n += t;
            (*cur)->Dump(into, n.c_str());
            if(r > 200) {
                fprintf(into, "%s   ... truncated ...\n", pfx);
                break;
            }
        }
    }
}

void PyList::Dump(LogType type, const char *pfx) const {
    //extra check to avoid potentially a lot of work if we are disabled
    if(!is_log_enabled(type))
        return;

    if(items.empty()) {
        _log(type, "%sList: Empty", pfx);
    }
    else {
        _log(type, "%sList: %lu elements", pfx, items.size());
        std::vector<PyRep *>::const_iterator cur, _end;
        cur = items.begin();
        _end = items.end();
        char t[15];
        for(int r = 0; cur != _end; cur++, r++) {
            std::string n(pfx);
            snprintf(t, 14, "  [%2d] ", r);
            n += t;
            (*cur)->Dump(type, n.c_str());
            if(r > 200) {
                _log(type, "%s   ... truncated ...", pfx);
                break;
            }
        }
    }
}

PyRep *PyList::Clone() const
{
	return new PyList( *this );
}

void PyList::visit( PyVisitor *v ) const
{
	v->VisitList( this );
}

void PyList::visit( PyVisitorLvl *v, int64 lvl ) const
{
	v->VisitList( this, lvl );
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

PyList& PyList::operator=(const PyList &oth)
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

void PyDict::Dump(FILE *into, const char *pfx) const {
    const_iterator cur, _end;
    cur = items.begin();
    _end = items.end();
    char t[17];
    fprintf(into, "%sDictionary: %lu entries\n", pfx, items.size());
    for(int r = 0; cur != _end; cur++, r++) {
        std::string n(pfx);
        snprintf(t, 16, "  [%2d] Key: ", r);
        n += t;
        cur->first->Dump(into, n.c_str());

        std::string m(pfx);
        snprintf(t, 16, "  [%2d] Value: ", r);
        m += t;
        cur->second->Dump(into, m.c_str());
    }
}

void PyDict::Dump(LogType type, const char *pfx) const {
    //extra check to avoid potentially a lot of work if we are disabled
    if(!is_log_enabled(type))
        return;

    const_iterator cur, _end;
    cur = items.begin();
    _end = items.end();
    char t[17];
    int r;
    _log(type, "%sDictionary: %lu entries", pfx, items.size());
    for(r = 0; cur != _end; cur++, r++) {
        std::string n(pfx);
        snprintf(t, 16, "  [%2d] Key: ", r);
        n += t;
        cur->first->Dump(type, n.c_str());

        std::string m(pfx);
        snprintf(t, 16, "  [%2d] Value: ", r);
        m += t;
        cur->second->Dump(type, m.c_str());
    }
}

PyRep* PyDict::Clone() const
{
	return new PyDict( *this );
}

void PyDict::visit( PyVisitor *v ) const
{
	v->VisitDict( this );
}

void PyDict::visit( PyVisitorLvl *v, int64 lvl ) const
{
	v->VisitDict( this, lvl );
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
    if( itr != items.end() )
    {
		// We don't need it anymore, we're using itr->first.
		PyDecRef( key );

		// Replace itr->second with value.
        PySafeDecRef( itr->second );
        itr->second = value;
    }
	else
		// Keep both key & value
        items.insert( std::make_pair( key, value ) );
}

PyDict& PyDict::operator=(const PyDict& oth)
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
PyObject::PyObject( const PyObject& oth ) : PyRep( PyRep::PyTypeObject ),
  mType( new PyString( *oth.type() ) ), mArguments( oth.arguments()->Clone() )
{
}

PyObject::~PyObject()
{
    PyDecRef( mType );
    PyDecRef( mArguments );
}

void PyObject::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sObject:\n", pfx);
    fprintf(into, "%s  PyType: %s\n", pfx, type()->content().c_str());

    std::string m(pfx);
    m += "  Args: ";
    arguments()->Dump(into, m.c_str());
}

void PyObject::Dump(LogType ltype, const char *pfx) const {
    _log(ltype, "%sObject:", pfx);
    _log(ltype, "%s  PyType: %s", pfx, type()->content().c_str());

    std::string m(pfx);
    m += "  Args: ";
    arguments()->Dump(ltype, m.c_str());
}

PyRep* PyObject::Clone() const
{
	return new PyObject( *this );
}

void PyObject::visit( PyVisitor* v ) const
{
	v->VisitObject( this );
}

void PyObject::visit( PyVisitorLvl* v, int64 lvl ) const
{
	v->VisitObject( this, lvl );
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

void PyObjectEx::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sObjectEx%s\n", pfx, (isType2() ? " (Type2)" : ""));
    fprintf(into, "%sHeader:\n", pfx);
    if(header() == NULL)
        fprintf(into, "%s  None\n", pfx);
    else {
        std::string p(pfx);
        p += "  ";

        header()->Dump(into, p.c_str());
    }

    fprintf(into, "%sList data:\n", pfx);
    if(list().empty())
        fprintf(into, "%s  Empty\n", pfx);
    else {
        const_list_iterator cur, end;
        cur = list().begin();
        end = list().end();
        char t[16];
        for(int i = 0; cur != end; cur++, i++) {
            std::string n(pfx);
            snprintf(t, 16, "  [%2d] ", i);
            n += t;
            (*cur)->Dump(into, n.c_str());
        }
    }

    fprintf(into, "%sDict data:\n", pfx);
    if(dict().empty())
        fprintf(into, "%s  Empty\n", pfx);
    else {
        const_dict_iterator cur, end;
        cur = dict().begin();
        end = dict().end();
        char t[16];
        for(int i = 0; cur != end; cur++, i++) {
            std::string k(pfx);
            snprintf(t, 16, "  [%2d] Key: ", i);
            k += t;
            cur->first->Dump(into, k.c_str());

            std::string v(pfx);
            snprintf(t, 16, "  [%2d] Value: ", i);
            v += t;
            cur->second->Dump(into, v.c_str());
        }
    }
}

void PyObjectEx::Dump(LogType ltype, const char *pfx) const {
    if(!is_log_enabled(ltype))
        return;

    {_log(ltype, "%sObjectEx%s\n", pfx, (isType2() ? " (Type2)" : ""));}
    {_log(ltype, "%sHeader:\n", pfx);}

    if(header() == NULL)
        {_log(ltype, "%s  None\n", pfx);}
    else {
        std::string p(pfx);
        p += "  ";

        header()->Dump(ltype, p.c_str());
    }

    {_log(ltype, "%sList data:", pfx);}
    if(list().empty())
        {_log(ltype, "%s  Empty", pfx);}
    else {
        const_list_iterator cur, end;
        cur = list().begin();
        end = list().end();
        char t[16];
        for(int i = 0; cur != end; cur++, i++) {
            std::string n(pfx);
            snprintf(t, 16, "  [%2d] ", i);
            n += t;
            (*cur)->Dump(ltype, n.c_str());
        }
    }

    {_log(ltype, "%sDict data:", pfx);}
    if(dict().empty())
        {_log(ltype, "%s  Empty", pfx);}
    else {
        const_dict_iterator cur, end;
        cur = dict().begin();
        end = dict().end();
        char t[16];
        for(int i = 0; cur != end; cur++, i++) {
            std::string k(pfx);
            snprintf(t, 16, "  [%2d] Key: ", i);
            k += t;
            cur->first->Dump(ltype, k.c_str());

            std::string v(pfx);
            snprintf(t, 16, "  [%2d] Value: ", i);
            v += t;
            cur->second->Dump(ltype, v.c_str());
        }
    }
}

PyRep* PyObjectEx::Clone() const
{
	return new PyObjectEx( *this );
}

void PyObjectEx::visit( PyVisitor* v ) const
{
	v->VisitObjectEx( this );
}

void PyObjectEx::visit( PyVisitorLvl* v, int64 lvl ) const
{
	v->VisitObjectEx( this, lvl );
}

PyObjectEx& PyObjectEx::operator=(const PyObjectEx& oth)
{
	list() = oth.list();
	dict() = oth.dict();

	return *this;
}

PyObjectEx_Type1::PyObjectEx_Type1(const char *type, PyTuple *args, PyDict *keywords)
: PyObjectEx( false, _CreateHeader( type, args, keywords ) )
{
}

PyString &PyObjectEx_Type1::GetType() const
{
	assert( header() );
	return header()->AsTuple().items.at( 0 )->AsString();
}

PyTuple &PyObjectEx_Type1::GetArgs() const
{
	assert( header() );
	return header()->AsTuple().items.at( 1 )->AsTuple();
}

PyDict &PyObjectEx_Type1::GetKeywords() const
{
	// This one is slightly more complicated since
	// keywords are optional.
	assert( header() );
	PyTuple &t = header()->AsTuple();

	if( t.size() < 3 )
		t.items.push_back( new PyDict );

	return t.items.at( 2 )->AsDict();
}

PyRep *PyObjectEx_Type1::FindKeyword(const char *keyword) const
{
	PyDict &kw = GetKeywords();

	PyDict::const_iterator cur, end;
	cur = kw.begin();
	end = kw.end();
	for(; cur != end; cur++)
	{
		if( cur->first->IsString() )
			if( cur->first->AsString().content() == keyword )
				return cur->second;
	}

	return NULL;
}

PyTuple *PyObjectEx_Type1::_CreateHeader(const char *type, PyTuple *args, PyDict *keywords)
{
	if( args == NULL )
		args = new PyTuple( 0 );

	PyTuple *head = new PyTuple( keywords == NULL ? 2 : 3 );
	head->SetItem( 0, new PyString( type, true ) );
	head->SetItem( 1, args );
	if( head->size() > 2 )
		head->SetItem( 2, keywords );

	return head;
}

PyObjectEx_Type2::PyObjectEx_Type2(PyTuple *args, PyDict *keywords)
: PyObjectEx( true, _CreateHeader( args, keywords ) )
{
}

PyTuple &PyObjectEx_Type2::GetArgs() const
{
	assert( header() );
	return header()->AsTuple().items.at( 0 )->AsTuple();
}

PyDict &PyObjectEx_Type2::GetKeywords() const
{
	assert( header() );
	return header()->AsTuple().items.at( 1 )->AsDict();
}

PyRep *PyObjectEx_Type2::FindKeyword(const char *keyword) const
{
	PyDict &kw = GetKeywords();

	PyDict::const_iterator cur, end;
	cur = kw.begin();
	end = kw.end();
	for(; cur != end; cur++)
	{
		if( cur->first->IsString() )
			if( cur->first->AsString().content() == keyword )
				return cur->second;
	}

	return NULL;
}

PyTuple *PyObjectEx_Type2::_CreateHeader(PyTuple *args, PyDict *keywords)
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
PyPackedRow::PyPackedRow( const PyPackedRow &oth ) : PyRep( PyRep::PyTypePackedRow ), mHeader( &oth.header() ), mFields( new PyList( oth.header().ColumnCount() ) )
{
    PyIncRef( mHeader );

	*this = oth;
}

PyPackedRow::~PyPackedRow()
{
    PyDecRef( mHeader );
    PyDecRef( mFields );
}

void PyPackedRow::Dump(FILE *into, const char *pfx) const
{
    fprintf( into, "%sPacked Row\n", pfx );
    fprintf( into, "%s column_count=%lu\n", pfx, mFields->size() );

    const_iterator cur, end;
    cur = this->begin();
    end = this->end();
    char buf[32];
    for(uint32 i = 0; cur != end; cur++, i++)
    {
        std::string n( pfx );
        snprintf( buf, 32, "  [%u] %s: ", i, header().GetColumnName( i ).content().c_str() );
        n += buf;

        if( (*cur) == NULL )
            fprintf( into, "%sNULL\n", n.c_str() );
        else
            (*cur)->Dump( into, n.c_str() );
    }
}

void PyPackedRow::Dump(LogType ltype, const char *pfx) const
{
    _log( ltype, "%sPacked Row", pfx );
    _log( ltype, "%s column_count=%lu", pfx, mFields->size() );

    const_iterator cur, end;
    cur = this->begin();
    end = this->end();
    char buf[32];
    for(uint32 i = 0; cur != end; cur++, i++)
    {
        std::string n( pfx );
        snprintf( buf, 32, "  [%u] %s: ", i, header().GetColumnName( i ).content().c_str() );
        n += buf;

        if( (*cur) == NULL ) {
            _log( ltype, "%sNULL", n.c_str() );
        }
        else
            (*cur)->Dump( ltype, n.c_str() );
    }
}

PyRep* PyPackedRow::Clone() const
{
	return new PyPackedRow( *this );
}

void PyPackedRow::visit( PyVisitor* v ) const
{
	v->VisitPackedRow( this );
}

void PyPackedRow::visit( PyVisitorLvl* v, int64 lvl ) const
{
	v->VisitPackedRow( this, lvl );
}

bool PyPackedRow::SetField(uint32 index, PyRep* value)
{
    if( index >= mFields->size() )
        return false;

    if( value != NULL )
    {
        // verify type
        if( !DBTYPE_IsCompatible( header().GetColumnType( index ), *value ) )
        {
            //sLog.Error("PyPackedRow", "uncompatible DBTYPE");
            PyDecRef( value );
            return false;
        }
    }

	mFields->SetItem( index, value );
    return true;
}

bool PyPackedRow::SetField(const char* colName, PyRep* value)
{
    return SetField( header().FindColumn( colName ), value );
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

void PySubStruct::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sSubstruct:\n", pfx);
    std::string m(pfx);
    m += "    ";
    sub()->Dump(into, m.c_str());
}

void PySubStruct::Dump(LogType type, const char *pfx) const {
    _log(type, "%sSubstruct:", pfx);
    std::string m(pfx);
    m += "    ";
    sub()->Dump(type, m.c_str());
}

PyRep* PySubStruct::Clone() const
{
	return new PySubStruct( *this );
}

void PySubStruct::visit( PyVisitor* v ) const
{
	v->VisitSubStruct( this );
}

void PySubStruct::visit( PyVisitorLvl* v, int64 lvl ) const
{
	v->VisitSubStruct( this, lvl );
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

void PySubStream::Dump(FILE *into, const char *pfx) const {
    if( decoded() == NULL ) {
        //we have not decoded this substream, leave it as hex:
        if( data() == NULL ) {
            fprintf(into, "%sINVALID Substream: no data\n", pfx);
        } else {
            fprintf(into, "%sSubstream:\n", pfx);

            std::string m(pfx);
            m += "  ";
			data()->Dump( into, m.c_str() );
        }
    } else {
        fprintf(into, "%sSubstream: %s\n", pfx, ( data() == NULL ) ? "from rep" : "from data");

        std::string m(pfx);
        m += "    ";
        decoded()->Dump(into, m.c_str());
    }
}

void PySubStream::Dump(LogType type, const char *pfx) const {
    //extra check to avoid potentially a lot of work if we are disabled
    if(!is_log_enabled(type))
        return;

    if( decoded() == NULL ) {
        //we have not decoded this substream, leave it as hex:
        if( data() == NULL ) {
            _log(type, "%sINVALID Substream: no data\n", pfx);
        } else {
            _log(type, "%sSubstream:\n", pfx);

            std::string m(pfx);
            m += "  ";
			data()->Dump( type, m.c_str() );
        }
    } else {
        _log(type, "%sSubstream: %s\n", pfx, ( data() == NULL ) ? "from rep" : "from data");

        std::string m(pfx);
        m += "    ";
        decoded()->Dump(type, m.c_str());
    }
}

PyRep* PySubStream::Clone() const
{
	return new PySubStream( *this );
}

void PySubStream::visit( PyVisitor* v ) const
{
	v->VisitSubStream( this );
}

void PySubStream::visit( PyVisitorLvl* v, int64 lvl ) const
{
	v->VisitSubStream( this, lvl );
}

void PySubStream::EncodeData() const
{
    if( decoded() == NULL )
        return;

	PySafeDecRef( mData );

	uint32 len;
    uint8* buf = Marshal( decoded(), len );

	// Move ownership of buffer to PyBuffer
	mData = new PyBuffer( &buf, len );
}

void PySubStream::DecodeData() const
{
    if( data() == NULL )
        return;

    PySafeDecRef( mDecoded );

    mDecoded = Unmarshal( data()->content(), data()->size() );
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

void PyChecksumedStream::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sStream With Checksum: 0x%08x\n", pfx, checksum());
    stream()->Dump(into, pfx);
}

void PyChecksumedStream::Dump(LogType type, const char *pfx) const {
    _log(type, "%sStream With Checksum: 0x%08x", pfx, checksum());
    stream()->Dump(type, pfx);
}

PyRep* PyChecksumedStream::Clone() const
{
	return new PyChecksumedStream( *this );
}

void PyChecksumedStream::visit( PyVisitor* v ) const
{
	v->VisitChecksumedStream( this );
}

void PyChecksumedStream::visit( PyVisitorLvl* v, int64 lvl ) const
{
	v->VisitChecksumedStream( this, lvl );
}


