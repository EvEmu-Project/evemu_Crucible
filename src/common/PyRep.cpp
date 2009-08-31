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

PyRep::PyRep(PyType t) : m_type(t), mRefcnt(1) {}
PyRep::~PyRep() {}

const char *PyRep::TypeString() const
{
    if (m_type >= 0 && m_type < PyTypeMax)
        return PyRepTypeString[m_type];

    return PyRepTypeString[PyTypeError];
}

/************************************************************************/
/* PyRep Integer Class                                                  */
/************************************************************************/
void PyInt::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sInteger field: %d\n", pfx, value);
}

void PyInt::Dump(LogType type, const char *pfx) const {
    _log(type, "%sInteger field: %d", pfx, value);
}

PyInt *PyInt::TypedClone() const {
    return new PyInt(value);
}

PyInt::PyInt( const int32 i ) : PyRep(PyRep::PyTypeInt), value(i) {}
PyInt::~PyInt() {}

EVEMU_INLINE PyRep * PyInt::Clone() const
{
    return(TypedClone());
}

EVEMU_INLINE void PyInt::visit( PyVisitor *v ) const
{
    v->VisitInteger(this);
}

EVEMU_INLINE void PyInt::visit( PyVisitorLvl *v, int64 lvl ) const
{
    v->VisitInteger(this, lvl);
}

int32 PyInt::hash()
{
    /* XXX If this is changed, you also need to change the way
    Python's long, float and complex types are hashed. */
    int32 x = value;
    if (x == -1)
        x = -2;
    return x;
}

/************************************************************************/
/* PyRep Long Class                                                     */
/************************************************************************/
void PyLong::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sInteger field: "I64d"\n", pfx, value);
}

void PyLong::Dump(LogType type, const char *pfx) const {
    _log(type, "%sInteger field: "I64d, pfx, value);
}

PyLong::PyLong( const int64 i ) : PyRep(PyRep::PyTypeLong), value(i) {}
PyLong::~PyLong() {}

EVEMU_INLINE PyRep * PyLong::Clone() const
{
    return TypedClone();
}

PyLong *PyLong::TypedClone() const {
    return(new PyLong(value));
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

int32 PyLong::hash()
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
    if (i < 0) {
        sign = -1;
        i = -(i);
    }
#define LONG_BIT_PyLong_SHIFT	(8*sizeof(long) - PyLong_SHIFT)
    /* The following loop produces a C long x such that (unsigned long)x
    is congruent to the absolute value of v modulo ULONG_MAX.  The
    resulting x is nonzero if and only if v is. */
    while (--i >= 0) {
        /* Force a native long #-bits (32 or 64) circular shift */
        x = ((x << PyLong_SHIFT) & ~PyLong_MASK) | ((x >> LONG_BIT_PyLong_SHIFT) & PyLong_MASK);
        x += ((uint8*)&value)[i];// v->ob_digit[i];
        /* If the addition above overflowed (thinking of x as
        unsigned), we compensate by incrementing.  This preserves
        the value modulo ULONG_MAX. */
        if ((unsigned long)x < ((uint8*)&value)[i])//v->ob_digit[i])
            x++;
    }
#undef LONG_BIT_PyLong_SHIFT
    x = x * sign;
    if (x == -1)
        x = -2;
    return x;
}

/************************************************************************/
/* PyRep Real/float/double Class                                        */
/************************************************************************/
void PyFloat::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sReal Field: %f\n", pfx, value);
}

void PyFloat::Dump(LogType type, const char *pfx) const {
    _log(type, "%sReal Field: %f", pfx, value);
}

PyFloat *PyFloat::TypedClone() const {
    return new PyFloat(value);
}

#define LONG_MAX      2147483647L   /* maximum (signed) long value */
#define Py_IS_INFINITY(X) (!_finite(X) && !_isnan(X))

int32 PyFloat::hash()
{
    double v = value;
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
        if (intpart > LONG_MAX || -intpart > LONG_MAX) {
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
void PyBool::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sBoolean field: %s\n", pfx, value?"true":"false");
}

void PyBool::Dump(LogType type, const char *pfx) const {
    _log(type, "%sBoolean field: %s", pfx, value?"true":"false");
}

PyBool *PyBool::TypedClone() const {
    return new PyBool(value);
}

int32 PyBool::hash()
{
    sLog.Error("PyBool", "unhashable type: 'PyBool'");
    return 0;
}

/************************************************************************/
/* PyRep None Class                                                     */
/************************************************************************/
void PyNone::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%s(None)\n", pfx);
}

void PyNone::Dump(LogType type, const char *pfx) const {
    _log(type, "%s(None)", pfx);
}

PyNone *PyNone::TypedClone() const {
    return new PyNone();
}

int32 PyNone::hash()
{
    /* damn hack... bleh.. but its done like this... in python and PyNone is a static singleton....*/
    int32 * hash = (int32 *)this;
    return *((int32*)&hash);
}

/************************************************************************/
/* PyRep Buffer Class                                                   */
/************************************************************************/
PyBuffer::PyBuffer(const uint8 *buffer, uint32 length) : PyRep(PyRep::PyTypeBuffer),
  m_value(NULL), m_length(length), m_hash_cache(-1)
{
    m_value = (uint8*)malloc(length);
    assert(m_value);
    memcpy(m_value, buffer, length);
}

PyBuffer::PyBuffer(uint32 length) : PyRep(PyRep::PyTypeBuffer),
  m_value(NULL), m_length(length), m_hash_cache(-1)
{
    m_value = (uint8*)malloc(length);
    assert(m_value);
}

PyBuffer::PyBuffer(uint8 **buffer, uint32 length) : PyRep(PyRep::PyTypeBuffer),
  m_value(*buffer), m_length(length), m_hash_cache(-1)
{
    *buffer = NULL;
}

PyBuffer::~PyBuffer() {
    SafeFree(m_value);
}

void PyBuffer::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sData buffer of length %u\n", pfx, m_length);

    //kinda hackish:
    if(m_length > 2 && *m_value == GZipStreamHeaderByte) {
        uint32 len = GetLength();
        uint8 *buf = InflatePacket(GetBuffer(), &len, true);
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
    _log(type, "%sData buffer of length %d", pfx, m_length);

    //kinda hackish:
    if(m_length > 2 && *m_value == GZipStreamHeaderByte) {
        uint32 len = GetLength();
        uint8 *buf = InflatePacket(GetBuffer(), &len, true);
        if(buf != NULL) {
            string p(pfx);
            p += "  ";
            _log(type, "%sData buffer contains gzipped data of length %u", p.c_str(), len);
            pfxPreviewHexDump(p.c_str(), type, buf, len);
            SafeFree(buf);
        }
    }
}

PySubStream *PyBuffer::CreateSubStream() const {
    if(*m_value == SubStreamHeaderByte) {
        return(new PySubStream(m_value, m_length));
    } else if(m_length > 2 && *m_value == GZipStreamHeaderByte) {
        uint32 len = GetLength();
        uint8 *buf = InflatePacket(GetBuffer(), &len, true);

        PySubStream *res = NULL;
        if(buf == NULL) {
            //unable to unzip, this does not appear to be a stream, so refuse to turn into one.
        } else if(*buf != SubStreamHeaderByte) {
            //wrong header byte, this does not appear to be a stream, so refuse to turn into one.
        } else {
            res = new PySubStream(buf, len);
        }

        SafeFree(buf);
        return res;
    }
    //else, we don't think this is a substream, so don't become one.
    return NULL;
}

PyBuffer *PyBuffer::TypedClone() const {
    return new PyBuffer(m_value, m_length);
}

int32 PyBuffer::hash()
{
    void *ptr;
    size_t size;
    register size_t len;
    register unsigned char *p;
    register long x;

    if ( m_hash_cache != -1 )
        return m_hash_cache;

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
    ptr = m_value;
    p = (unsigned char *) ptr;
    len = m_length;
    x = *p << 7;
    while (--len >= 0)
        x = (1000003*x) ^ *p++;
    x ^= m_length;
    if (x == -1)
        x = -2;
    m_hash_cache = x;
    return x;
}

/************************************************************************/
/* PyRep Tuple Class                                                    */
/************************************************************************/
PyTuple::~PyTuple() {
    clear();
}

void PyTuple::clear() {
    storage_type::iterator cur, _end;
    cur = begin();
    _end = end();
    for(; cur != _end; cur++)
        PyDecRef( *cur );
    items.clear();
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

uint32 PyTuple::size() const
{
    return (uint32)items.size();
}

/************************************************************************/
/* PyRep List Class                                                     */
/************************************************************************/
PyList::~PyList()
{
    clear();
}

void PyList::clear() {
    std::vector<PyRep *>::iterator cur, _end;
    cur = items.begin();
    _end = items.end();
    for(; cur != _end; cur++)
        PyDecRef( *cur );
    items.clear();
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

/************************************************************************/
/* PyRep Dict Class                                                     */
/************************************************************************/
PyDict::~PyDict() {
    clear();
}

void PyDict::clear() {
    iterator cur, _end;
    cur = items.begin();
    _end = items.end();
    for(; cur != _end; cur++) {
        PyDecRef( cur->first );
        PyDecRef( cur->second );
    }
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

/************************************************************************/
/* PyRep Object Class                                                   */
/************************************************************************/
PyObject::~PyObject() {
    PyDecRef( arguments );
}

void PyObject::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sObject:\n", pfx);
    fprintf(into, "%s  PyType: %s\n", pfx, type.c_str());

    std::string m(pfx);
    m += "  Args: ";
    arguments->Dump(into, m.c_str());
}

void PyObject::Dump(LogType ltype, const char *pfx) const {
    _log(ltype, "%sObject:", pfx);
    _log(ltype, "%s  PyType: %s", pfx, type.c_str());

    std::string m(pfx);
    m += "  Args: ";
    arguments->Dump(ltype, m.c_str());
}

/************************************************************************/
/* PyRep SubStruct Class                                                */
/************************************************************************/
PySubStruct::~PySubStruct()
{
    PyDecRef( sub );
}

void PySubStruct::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sSubstruct:\n", pfx);
    std::string m(pfx);
    m += "    ";
    sub->Dump(into, m.c_str());
}

void PySubStruct::Dump(LogType type, const char *pfx) const {
    _log(type, "%sSubstruct:", pfx);
    std::string m(pfx);
    m += "    ";
    sub->Dump(type, m.c_str());
}

/************************************************************************/
/* PyRep SubStream Class                                                */
/************************************************************************/
PySubStream::PySubStream(const uint8 *buffer, uint32 len)
: PyRep(PyRep::PyTypeSubStream),
  length(len),
  data(NULL),
  decoded(NULL)
{
    data = (uint8*)malloc(len);
    assert(data != NULL);
    memcpy(data, buffer, length);
}

PySubStream::~PySubStream() {
    SafeFree( data );
    PySafeDecRef( decoded );
}

void PySubStream::Dump(FILE *into, const char *pfx) const {
    if(decoded == NULL) {
        //we have not decoded this substream, leave it as hex:
        if(data == NULL) {
            fprintf(into, "%sINVALID Substream: no data (length %u)\n", pfx, length);
        } else {
            fprintf(into, "%sSubstream: length %u\n", pfx, length);
            std::string m(pfx);
            m += "  ";
            pfxPreviewHexDump(m.c_str(), into, data, length);
        }
    } else {
        fprintf(into, "%sSubstream: length %u %s\n", pfx, length, (data==NULL)?"from rep":"from data");
        std::string m(pfx);
        m += "    ";
        decoded->Dump(into, m.c_str());
    }
}

void PySubStream::Dump(LogType type, const char *pfx) const {
    //extra check to avoid potentially a lot of work if we are disabled
    if(!is_log_enabled(type))
        return;

    if(decoded == NULL) {
        //we have not decoded this substream, leave it as hex:
        if(data == NULL) {
            _log(type, "%sINVALID Substream: no data (length %u)", pfx, length);
        } else {
            _log(type, "%sSubstream: length %u", pfx, length);
            std::string m(pfx);
            m += "  ";
            pfxPreviewHexDump(m.c_str(), type, data, length);
        }
    } else {
        _log(type, "%sSubstream: length %u %s", pfx, length, (data==NULL)?"from rep":"from data");
        std::string m(pfx);
        m += "    ";
        decoded->Dump(type, m.c_str());
    }
}

void PySubStream::EncodeData() {
    if(data != NULL)
        return;
    if(decoded == NULL)
        return;
    data = Marshal(this, length);
}

void PySubStream::DecodeData() const {
    if(decoded != NULL)
        return;
    decoded = InflateAndUnmarshal(data, length);
}

/************************************************************************/
/* PyRep ChecksumedStream Class                                         */
/************************************************************************/
PyChecksumedStream::~PyChecksumedStream() {
    PyDecRef(stream);
}

void PyChecksumedStream::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sStream With Checksum: 0x%08x\n", pfx, checksum);
    stream->Dump(into, pfx);
}

void PyChecksumedStream::Dump(LogType type, const char *pfx) const {
    _log(type, "%sStream With Checksum: 0x%08x", pfx, checksum);
    stream->Dump(type, pfx);
}

PyString *PyString::TypedClone() const {
    return(new PyString(value, is_type_1));
}

PyTuple *PyTuple::TypedClone() const {
    PyTuple *r = new PyTuple(0);
    r->CloneFrom(this);
    return(r);
}

void PyTuple::CloneFrom(const PyTuple *from) {
    clear();
    items.reserve(items.size());

    std::vector<PyRep *>::const_iterator cur, _end;
    cur = from->items.begin();
    _end = from->items.end();
    for(; cur != _end; cur++) {
        items.push_back((*cur)->Clone());
    }
}

void PyTuple::SetItem( uint32 index, PyRep* object )
{
    assert(index < items.size());
    items[index] = object;
}

int32 PyTuple::hash()
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

PyList *PyList::TypedClone() const {
    PyList *r = new PyList();
    r->CloneFrom(this);
    return r;
}

void PyList::CloneFrom(const PyList *from) {
    PyList::const_iterator cur, _end;
    cur = items.begin();
    _end = items.end();
    for(; cur != _end; cur++) {
        delete *cur;
    }

    items.clear();
    items.reserve(items.size());

    cur = from->items.begin();
    _end = from->items.end();
    for(; cur != _end; cur++) {
        items.push_back((*cur)->Clone());
    }
}

int32 PyList::hash()
{
    sLog.Error("PyList", "unhashable type: 'PyList'");
    return -1;
}

PyDict *PyDict::TypedClone() const {
    PyDict *r = new PyDict();
    r->CloneFrom(this);
    return r;
}

void PyDict::CloneFrom(const PyDict *from) {
    PyDict::const_iterator cur, _end;
    cur = items.begin();
    _end = items.end();
    for(; cur != _end; cur++) {
        delete cur->first;
        delete cur->second;
    }

    items.clear();

    cur = from->items.begin();
    _end = from->items.end();
    for(; cur != _end; cur++) {
        items[ cur->first->Clone() ] = cur->second->Clone();
    }
}

int32 PyDict::hash()
{
    sLog.Error("PyDict", "unhashable type: 'PyDict'");
    return -1;
}

bool PyDict::SetItem( PyRep * key, PyRep * value )
{
    /* make sure we have valid arguments */
    if ( key == NULL || value == NULL )
        return false;

    /* note: add check if the key object is hashable
     * if not ( it will return -1 then ) return false;
     */

    /* check if we need to replace a dictionary entry */
    iterator itr = items.find( key );
    if ( itr != items.end() )
    {
        PyDecRef( itr->second );
        items.erase( itr ); // this is the fast solution not the best.
    }

    /* note: needs to be enabled when object reference is working.
     */
    //PyIncRef( key );
    //PyIncRef( value );

    items.insert( std::make_pair( key, value ) );
    return true;
}

bool PyDict::SetItemString( const char *key, PyObject *item )
{
    return SetItem( new PyString( key ), item );
}

void PyDict::add(PyRep *key, PyRep *value) {
    if(key == NULL || value == NULL)
        return;
    items[key] = value;
}

void PyDict::add(const char *key, PyRep *value) {
    if(key == NULL || value == NULL)
        return;
    items[new PyString(key)] = value;
}

void PyDict::addStr(const char *key, const char *value) {
    if(key == NULL || value == NULL)
        return;
    items[new PyString(key)] = new PyString(value);
}

PyObject *PyObject::TypedClone() const {
    return new PyObject( type, arguments->Clone() );
}

int32 PyObject::hash()
{
    sLog.Error("PyObject", "hash not implemented");
    return -1;
}

PySubStruct *PySubStruct::TypedClone() const {
    return new PySubStruct( sub->Clone() );
}

int32 PySubStruct::hash()
{
    sLog.Error("PySubStruct", "unhashable type: 'PySubStruct'");
    return -1;
}

PySubStream *PySubStream::TypedClone() const {
    if(data == NULL)
        return(new PySubStream( decoded->Clone() ));
    return(new PySubStream( data, length ));
}

int32 PySubStream::hash()
{
    sLog.Error("PySubStream", "unhashable type: 'PySubStream'");
    return -1;
}

PyChecksumedStream *PyChecksumedStream::TypedClone() const {
    return new PyChecksumedStream( checksum, stream->Clone() );
}

int32 PyChecksumedStream::hash()
{
    sLog.Error("PyChecksumedStream", "unhashable type: 'PyChecksumedStream'");
    return -1;
}

PyPackedRow::PyPackedRow(DBRowDescriptor &header, bool header_owner)
: PyRep( PyRep::PyTypePackedRow ),
  mHeader( header ),
  mHeaderOwner( header_owner )
{
    mFields.resize( header.ColumnCount() );
}

PyPackedRow::~PyPackedRow()
{
    if( IsHeaderOwner() )
        PyDecRef( &mHeader );

    std::vector<PyRep *>::iterator cur, end;
    cur = mFields.begin();
    end = mFields.end();
    for(; cur != end; cur++)
        PySafeDecRef( *cur );
}

void PyPackedRow::Dump(FILE *into, const char *pfx) const
{
    fprintf( into, "%sPacked Row\n", pfx );
    fprintf( into, "%s column_count=%lu header_owner=%s\n", pfx, mFields.size(), IsHeaderOwner() ? "yes" : "no" );

    std::vector<PyRep *>::const_iterator cur, end;
    cur = mFields.begin();
    end = mFields.end();
    char buf[32];
    for(uint32 i = 0; cur != end; cur++, i++)
    {
        PyRep *v = *cur;

        std::string n( pfx );
        snprintf( buf, 32, "  [%u] %s: ", i, GetHeader().GetColumnName( i ).c_str() );
        n += buf;

        if( v == NULL )
            fprintf( into, "%sNULL\n", n.c_str() );
        else
            v->Dump( into, n.c_str() );
    }
}

void PyPackedRow::Dump(LogType ltype, const char *pfx) const
{
    _log( ltype, "%sPacked Row", pfx );
    _log( ltype, "%s column_count=%lu header_owner=%s", pfx, mFields.size(), IsHeaderOwner() ? "yes" : "no" );

    std::vector<PyRep *>::const_iterator cur, end;
    cur = mFields.begin();
    end = mFields.end();
    char buf[32];
    for(uint32 i = 0; cur != end; cur++, i++)
    {
        PyRep *v = *cur;

        std::string n( pfx );
        snprintf( buf, 32, "  [%u] %s: ", i, GetHeader().GetColumnName( i ).c_str() );
        n += buf;

        if( v == NULL ) {
            _log( ltype, "%sNULL", n.c_str() );
        }
        else
            v->Dump( ltype, n.c_str() );
    }
}

PyPackedRow *PyPackedRow::TypedClone() const
{
    PyPackedRow *res = new PyPackedRow( IsHeaderOwner() ? *GetHeader().TypedClone() : GetHeader(),
                                              IsHeaderOwner() );
    res->CloneFrom( this );
    return res;
}

void PyPackedRow::CloneFrom(const PyPackedRow *from)
{
    // clone fields
    uint32 cc = mFields.size();
    for(uint32 i = 0; i < cc; i++)
    {
        PyRep *v = from->GetField( i );
        if( v != NULL )
            v = v->Clone();

        SetField( i, v );
    }
}

bool PyPackedRow::SetField(uint32 index, PyRep *value)
{
    if( index >= mFields.size() )
        return false;

    if( value != NULL )
    {
        // verify type
        if( !DBTYPE_IsCompatible( GetHeader().GetColumnType( index ), *value ) )
        {
            //sLog.Error("PyPackedRow", "uncompatible DBTYPE");
            PyDecRef( value );
            return false;
        }
    }

    /* check if a object is already prescient and replace it if necessary */
    PyRep *& tObject = mFields.at( index );

    if ( tObject != NULL)
        PyDecRef( tObject );

    tObject = value;

    return true;
}

bool PyPackedRow::SetField(const char *colName, PyRep *value)
{
    uint32 index = GetHeader().FindColumn( colName );
    if( index >= mFields.size() )
        return false;
    return SetField( index, value );
}

int32 PyPackedRow::hash()
{
    sLog.Error("PyPackedRow", "unhashable type: 'PyPackedRow'");
    assert(false);
    return -1;
}

/************************************************************************/
/* PyObjectEx                                                        */
/************************************************************************/
PyObjectEx::PyObjectEx( bool _is_type_2, PyRep *_header /*= NULL*/ ) : PyRep(PyRep::PyTypeObjectEx), header(_header), is_type_2(_is_type_2) {}

PyObjectEx::~PyObjectEx()
{
    PyDecRef( header );
    {
        const_list_iterator cur, end;
        cur = list_data.begin();
        end = list_data.end();
        for(; cur != end; cur++)
            PyDecRef( *cur );
        list_data.clear();
    }
    {
        const_dict_iterator cur, end;
        cur = dict_data.begin();
        end = dict_data.end();
        for(; cur != end; cur++) {
            PyDecRef( cur->first );
            PyDecRef( cur->second );
        }
        dict_data.clear();
    }
}

void PyObjectEx::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sObjectEx%s\n", pfx, (is_type_2 ? " (Type2)" : ""));
    fprintf(into, "%sHeader:\n", pfx);
    if(header == NULL)
        fprintf(into, "%s  None\n", pfx);
    else {
        std::string p(pfx);
        p += "  ";

        header->Dump(into, p.c_str());
    }

    fprintf(into, "%sList data:\n", pfx);
    if(list_data.empty())
        fprintf(into, "%s  Empty\n", pfx);
    else {
        const_list_iterator cur, end;
        cur = list_data.begin();
        end = list_data.end();
        char t[16];
        for(int i = 0; cur != end; cur++, i++) {
            std::string n(pfx);
            snprintf(t, 16, "  [%2d] ", i);
            n += t;
            (*cur)->Dump(into, n.c_str());
        }
    }

    fprintf(into, "%sDict data:\n", pfx);
    if(dict_data.empty())
        fprintf(into, "%s  Empty\n", pfx);
    else {
        const_dict_iterator cur, end;
        cur = dict_data.begin();
        end = dict_data.end();
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

    {_log(ltype, "%sObjectEx%s\n", pfx, (is_type_2 ? " (Type2)" : ""));}
    {_log(ltype, "%sHeader:\n", pfx);}

    if(header == NULL)
        {_log(ltype, "%s  None\n", pfx);}
    else {
        std::string p(pfx);
        p += "  ";

        header->Dump(ltype, p.c_str());
    }

    {_log(ltype, "%sList data:", pfx);}
    if(list_data.empty())
        {_log(ltype, "%s  Empty", pfx);}
    else {
        const_list_iterator cur, end;
        cur = list_data.begin();
        end = list_data.end();
        char t[16];
        for(int i = 0; cur != end; cur++, i++) {
            std::string n(pfx);
            snprintf(t, 16, "  [%2d] ", i);
            n += t;
            (*cur)->Dump(ltype, n.c_str());
        }
    }

    {_log(ltype, "%sDict data:", pfx);}
    if(dict_data.empty())
        {_log(ltype, "%s  Empty", pfx);}
    else {
        const_dict_iterator cur, end;
        cur = dict_data.begin();
        end = dict_data.end();
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

PyObjectEx *PyObjectEx::TypedClone() const {
    PyObjectEx *clone = new PyObjectEx(is_type_2);
    clone->CloneFrom(this);
    return(clone);
}

void PyObjectEx::CloneFrom(const PyObjectEx *from) {
    if(from->header != NULL)
        header = from->header->Clone();
    else
        header = NULL;
    {
        const_list_iterator cur, end;
        cur = from->list_data.begin();
        end = from->list_data.end();
        for(; cur != end; cur++)
            list_data.push_back((*cur)->Clone());
    }
    {
        const_dict_iterator cur, end;
        cur = from->dict_data.begin();
        end = from->dict_data.end();
        for(; cur != end; cur++) {
            dict_data[cur->first->Clone()] = cur->second->Clone();
        }
    }
}

int32 PyObjectEx::hash()
{
    sLog.Error("PyObjectEx", "unhashable type: 'PyObjectEx'");
    return -1;
}

PyObjectEx_Type1::PyObjectEx_Type1(const char *type, PyTuple *args, PyDict *keywords)
: PyObjectEx( false, _CreateHeader( type, args, keywords ) )
{
}

std::string &PyObjectEx_Type1::GetType() const
{
	assert( header );
	return header->AsTuple().items.at( 0 )->AsString().value;
}

PyTuple &PyObjectEx_Type1::GetArgs() const
{
	assert( header );
	return header->AsTuple().items.at( 1 )->AsTuple();
}

PyDict &PyObjectEx_Type1::GetKeywords() const
{
	// This one is slightly more complicated since
	// keywords are optional.
	assert( header );
	PyTuple &t = header->AsTuple();

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
			if( cur->first->AsString().value == keyword )
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
	assert( header );
	return header->AsTuple().items.at( 0 )->AsTuple();
}

PyDict &PyObjectEx_Type2::GetKeywords() const
{
	assert( header );
	return header->AsTuple().items.at( 1 )->AsDict();
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
			if( cur->first->AsString().value == keyword )
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
/* PyString                                                          */
/************************************************************************/
PyString::PyString( const char *str, bool type_1 ) : PyRep(PyRep::PyTypeString), value(str), is_type_1(type_1), m_hash_cache(-1) {}

PyString::PyString( const std::string &str, bool type_1 ) : PyRep(PyRep::PyTypeString), is_type_1(type_1), m_hash_cache(-1)
{
    value.assign( str.c_str(), str.length() );
}

PyString::PyString( const uint8 *data, uint32 len, bool type_1 ) : PyRep(PyRep::PyTypeString), value((const char *) data, len), is_type_1(type_1), m_hash_cache(-1) {}

PyString::~PyString() {}

void PyString::set( const char* str, size_t len )
{
    value.assign( str, len );
}

int32 PyString::hash()
{
    register size_t len;
    register unsigned char *p;
    register int32 x;

    if (m_hash_cache != -1)
        return m_hash_cache;
    len = value.length();
    p = (unsigned char *) value.c_str();
    x = *p << 7;
    while (--len > 0)
        x = (1000003*x) ^ *p++;
    x ^= value.length();
    if (x == -1)
        x = -2;
    m_hash_cache = x;
    return x;
}

void PyString::Dump(FILE *into, const char *pfx) const {
    if(ContainsNonPrintables( value ) ) {
        fprintf(into, "%sString%s: '<binary, len=%lu>'\n", pfx, is_type_1?" (Type1)":"", value.length());
    } else {
        fprintf(into, "%sString%s: '%s'\n", pfx, is_type_1?" (Type1)":"", value.c_str());
    }
}

void PyString::Dump(LogType type, const char *pfx) const {
    if(ContainsNonPrintables( value ) ) {
        _log(type, "%sString%s: '<binary, len=%lu>'", pfx, is_type_1?" (Type1)":"", value.length());
    } else {
        _log(type, "%sString%s: '%s'", pfx, is_type_1?" (Type1)":"", value.c_str());
    }
}
