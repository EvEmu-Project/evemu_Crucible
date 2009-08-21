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

/************************************************************************/
/* PyRep utilities                                                      */
/************************************************************************/
/** Lookup table for PyRep type object type names.
*/
const char *PyRepTypeString[] = {
    "Integer",          //0
    "Real",             //1
    "Boolean",          //2
    "Buffer",           //3
    "String",           //4
    "Tuple",            //5
    "List",             //6
    "Dict",             //7
    "None",             //8
    "SubStruct",        //9
    "SubStream",        //10
    "ChecksumedStream", //11
    "Object",           //12
    "ObjectEx",         //13
    "PackedRow",        //14
    "UNKNOWN TYPE",     //15
};

static void pfxHexDump(const char *pfx, FILE *into, const uint8 *data, uint32 length) {
    char buffer[80];
    uint32 offset;
    for(offset=0;offset<length;offset+=16) {
        build_hex_line((const char *)data,length,offset,buffer,4);
        fprintf(into, "%s%s\n", pfx, buffer);
    }
}

static void pfxPreviewHexDump(const char *pfx, FILE *into, const uint8 *data, uint32 length) {
    char buffer[80];

    if(length > 1024) {
        pfxHexDump(pfx, into, data, 1024-32);
        fprintf(into, "%s ... truncated ...\n", pfx);
        build_hex_line((const char *)data,length,length-16,buffer,4);
        fprintf(into, "%s%s\n", pfx, buffer);
    } else {
        pfxHexDump(pfx, into, data, length);
    }
}

static void pfxHexDump(const char *pfx, LogType type, const uint8 *data, uint32 length) {
    char buffer[80];
    uint32 offset;
    for(offset=0;offset<length;offset+=16) {
        build_hex_line((const char *)data,length,offset,buffer,4);
        _log(type, "%s%s\n", pfx, buffer);
    }
}

static void pfxPreviewHexDump(const char *pfx, LogType type, const uint8 *data, uint32 length) {
    char buffer[80];

    if(length > 1024) {
        pfxHexDump(pfx, type, data, 1024-32);
        _log(type, "%s ... truncated ...", pfx);
        build_hex_line((const char *)data,length,length-16,buffer,4);
        _log(type, "%s%s", pfx, buffer);
    } else {
        pfxHexDump(pfx, type, data, length);
    }
}

/************************************************************************/
/* PyRep base Class                                                     */
/************************************************************************/

PyRep::PyRep(Type t) : m_type(t), mRefcnt(1) {}
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
    fprintf(into, "%sInteger field: "I64u"\n", pfx, value);
}

void PyInt::Dump(LogType type, const char *pfx) const {
    _log(type, "%sInteger field: "I64u, pfx, value);
}

/************************************************************************/
/* PyRep Long Class                                                     */
/************************************************************************/
void PyLong::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sInteger field: "I64u"\n", pfx, value);
}

void PyLong::Dump(LogType type, const char *pfx) const {
    _log(type, "%sInteger field: "I64u, pfx, value);
}

PyLong::PyLong( const int64 i ) : PyRep(PyRep::PyTypeInt), value(i) {}
PyLong::~PyLong() {}

EVEMU_INLINE PyRep * PyLong::Clone() const
{
    return TypedClone();
}

EVEMU_INLINE void PyLong::visit( PyVisitor *v ) const
{
    v->VisitLong(this);
}

EVEMU_INLINE void PyLong::visit( PyVisitorLvl *v, int64 lvl ) const
{
    v->VisitLong(this, lvl);
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

/************************************************************************/
/* PyRep Boolean Class                                                  */
/************************************************************************/
void PyBool::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sBoolean field: %s\n", pfx, value?"true":"false");
}

void PyBool::Dump(LogType type, const char *pfx) const {
    _log(type, "%sBoolean field: %s", pfx, value?"true":"false");
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

/************************************************************************/
/* PyRep Buffer Class                                                   */
/************************************************************************/
PyBuffer::PyBuffer(const uint8 *buffer, uint32 length) : PyRep(PyRep::PyTypeBuffer),
  m_value(NULL), m_length(length)
{
    m_value = (uint8*)malloc(length);
    assert(m_value);
    memcpy(m_value, buffer, length);
}

PyBuffer::PyBuffer(uint32 length) : PyRep(PyRep::PyTypeBuffer),
  m_value(NULL), m_length(length)
{
    m_value = (uint8*)malloc(length);
    assert(m_value);
}

PyBuffer::PyBuffer(uint8 **buffer, uint32 length) : PyRep(PyRep::PyTypeBuffer),
  m_value(*buffer), m_length(length)
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

/************************************************************************/
/* PyRep String Class                                                   */
/************************************************************************/
void PyString::Dump(FILE *into, const char *pfx) const {
    if(ContainsNonPrintables(value.c_str(), (uint32)value.length())) {
        fprintf(into, "%sString%s: '<binary, len=%lu>'\n", pfx, is_type_1?" (Type1)":"", value.length());
    } else {
        fprintf(into, "%sString%s: '%s'\n", pfx, is_type_1?" (Type1)":"", value.c_str());
    }
}

void PyString::Dump(LogType type, const char *pfx) const {
    if(ContainsNonPrintables(value.c_str(), (uint32)value.length())) {
        _log(type, "%sString%s: '<binary, len=%lu>'", pfx, is_type_1?" (Type1)":"", value.length());
    } else {
        _log(type, "%sString%s: '%s'", pfx, is_type_1?" (Type1)":"", value.c_str());
    }
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

/************************************************************************/
/* PyRep Object Class                                                   */
/************************************************************************/
PyObject::~PyObject() {
    PyDecRef( arguments );
}

void PyObject::Dump(FILE *into, const char *pfx) const {
    fprintf(into, "%sObject:\n", pfx);
    fprintf(into, "%s  Type: %s\n", pfx, type.c_str());

    std::string m(pfx);
    m += "  Args: ";
    arguments->Dump(into, m.c_str());
}

void PyObject::Dump(LogType ltype, const char *pfx) const {
    _log(ltype, "%sObject:", pfx);
    _log(ltype, "%s  Type: %s", pfx, type.c_str());

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
    PyDecRef( decoded );
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

PyInt *PyInt::TypedClone() const {
    return(new PyInt(value));
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

PyFloat *PyFloat::TypedClone() const {
    return(new PyFloat(value));
}

PyBool *PyBool::TypedClone() const {
    return(new PyBool(value));
}

PyNone *PyNone::TypedClone() const {
    return(new PyNone());
}

PyBuffer *PyBuffer::TypedClone() const {
    return(new PyBuffer(m_value, m_length));
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

PyList *PyList::TypedClone() const {
    PyList *r = new PyList();
    r->CloneFrom(this);
    return(r);
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

PyDict *PyDict::TypedClone() const {
    PyDict *r = new PyDict();
    r->CloneFrom(this);
    return(r);
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

PyObject *PyObject::TypedClone() const {
    return(new PyObject( type, arguments->Clone() ));
}

PySubStruct *PySubStruct::TypedClone() const {
    return(new PySubStruct( sub->Clone() ));
}

PySubStream *PySubStream::TypedClone() const {
    if(data == NULL)
        return(new PySubStream( decoded->Clone() ));
    return(new PySubStream( data, length ));
}

PyChecksumedStream *PyChecksumedStream::TypedClone() const {
    return(new PyChecksumedStream( checksum, stream->Clone() ));
}


PyPackedRow::PyPackedRow(PyRep &header, bool header_owner)
: PyRep( PyRep::PyTypePackedRow ),
  mHeader( header ),
  mHeaderOwner( header_owner ),
  mColumnInfo( NULL )
{
    assert( GetHeader().IsObjectEx() );
    PyRep *r = GetHeader().AsObjectEx().header;

    assert( r != NULL && r->IsTuple() );
    PyTuple *t = &r->AsTuple();

    assert( t->size() == 2 );

    r = t->items[ 0 ];
    assert( r->IsString() );
    assert( r->AsString().value == "blue.DBRowDescriptor" );

    r = t->items[ 1 ];
    assert( r->IsTuple() );
    t = &r->AsTuple();

    assert( t->items.size() == 1 );
    r = t->items[ 0 ];

    assert( r->IsTuple() );
    t = &r->AsTuple();

    mColumnInfo = t;

    mFields.resize( mColumnInfo->size() );
}

PyPackedRow::~PyPackedRow()
{
    if( IsHeaderOwner() )
        PyDecRef(&mHeader);

    std::vector<PyRep *>::iterator cur, end;
    cur = mFields.begin();
    end = mFields.end();
    for(; cur != end; cur++)
        PyDecRef( *cur );
}


void PyPackedRow::Dump(FILE *into, const char *pfx) const
{
    fprintf( into, "%sPacked Row\n", pfx );
    fprintf( into, "%s column_count=%u header_owner=%s\n", pfx, ColumnCount(), IsHeaderOwner() ? "yes" : "no" );

    std::vector<PyRep *>::const_iterator cur, end;
    cur = mFields.begin();
    end = mFields.end();
    char buf[32];
    for(uint32 i = 0; cur != end; cur++, i++)
    {
        PyRep *v = *cur;

        std::string n( pfx );
        snprintf( buf, 32, "  [%u] %s: ", i, GetColumnName( i ).c_str() );
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
    _log( ltype, "%s column_count=%u header_owner=%s", pfx, ColumnCount(), IsHeaderOwner() ? "yes" : "no" );

    std::vector<PyRep *>::const_iterator cur, end;
    cur = mFields.begin();
    end = mFields.end();
    char buf[32];
    for(uint32 i = 0; cur != end; cur++, i++)
    {
        PyRep *v = *cur;

        std::string n( pfx );
        snprintf( buf, 32, "  [%u] %s: ", i, GetColumnName( i ).c_str() );
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
    PyPackedRow *res = new PyPackedRow( IsHeaderOwner() ? *GetHeader().Clone() : GetHeader(),
                                              IsHeaderOwner() );
    res->CloneFrom( this );
    return res;
}

void PyPackedRow::CloneFrom(const PyPackedRow *from)
{
    // clone fields
    uint32 cc = ColumnCount();
    for(uint32 i = 0; i < cc; i++)
    {
        PyRep *v = from->GetField( i );
        if( v != NULL )
            v = v->Clone();

        SetField( i, v );
    }
}

const std::string &PyPackedRow::GetColumnName(uint32 index) const
{
    assert( mColumnInfo->items.size() > index );
    PyRep *r = mColumnInfo->items[ index ];

    assert( r->IsTuple() );
    PyTuple &t = r->AsTuple();

    assert( t.items.size() == 2 );
    r = t.items[ 0 ];

    assert( r->IsString() );
    return r->AsString().value;
}

uint32 PyPackedRow::GetColumnIndex(const char *name) const
{
    uint32 cc = ColumnCount();
    for(uint32 i = 0; i < cc; i++)
    {
        const std::string &colName = GetColumnName( i );

        if( colName == name )
            return i;
    }

    return cc;
}

DBTYPE PyPackedRow::GetColumnType(uint32 index) const
{
    assert( mColumnInfo->items.size() > index );
    PyRep *r = mColumnInfo->items[ index ];

    assert( r->IsTuple() );
    PyTuple *t = &r->AsTuple();

    assert( t->items.size() == 2 );
    r = t->items[ 1 ];

    assert( r->IsInt() );
    return static_cast<DBTYPE>( r->AsInt().GetValue() );
}

bool PyPackedRow::SetField(uint32 index, PyRep *value)
{
    if( index >= ColumnCount() )
        return false;

    if( value != NULL )
    {
        // verify type
        if( !DBTYPE_IsCompatible( GetColumnType( index ), *value ) )
        {
            PyDecRef( value );
            return false;
        }
    }

    PyRep *&v = mFields.at( index );
    PyDecRef( v );
    v = value;

    return true;
}

bool PyPackedRow::SetField(const char *colName, PyRep *value)
{
    uint32 index = GetColumnIndex( colName );
    if( index >= ColumnCount() )
        return false;
    return SetField( index, value );
}

/************************************************************************/
/* PyObjectEx                                                        */
/************************************************************************/
PyObjectEx::PyObjectEx( bool _is_type_1, PyRep *_header /*= NULL*/ ) : PyRep(PyRep::PyTypeObjectEx), header(_header), is_type_1(_is_type_1) {}

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
    fprintf(into, "%sObjectEx%s\n", pfx, (is_type_1 ? " (Type1)" : ""));
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

    {_log(ltype, "%sObjectEx%s\n", pfx, (is_type_1 ? " (Type1)" : ""));}
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
    PyObjectEx *clone = new PyObjectEx(is_type_1);
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



/************************************************************************/
/* string table code                                                    */
/************************************************************************/
/*EVEStringTable *PyString::s_stringTable = NULL;
bool PyString::LoadStringFile(const char *file)
{
    if(s_stringTable != NULL)
        SafeDelete(s_stringTable);

    s_stringTable = new EVEStringTable;
    return s_stringTable->LoadFile(file);
}

EVEStringTable *PyString::GetStringTable()
{
    if(s_stringTable == NULL)
        s_stringTable = new EVEStringTable(); //make an empty one.

    return s_stringTable;
}*/

/************************************************************************/
/* PyString                                                          */
/************************************************************************/
PyString::PyString( const char *str, bool type_1 ) : PyRep(PyRep::PyTypeString), value(str), is_type_1(type_1) {}

PyString::PyString( const std::string &str, bool type_1 ) : PyRep(PyRep::PyTypeString), is_type_1(type_1)
{
    value.assign(str.c_str(), str.length());
}

PyString::PyString( const uint8 *data, uint32 len, bool type_1 ) : PyRep(PyRep::PyTypeString), value((const char *) data, len), is_type_1(type_1) {}

PyString::~PyString() {}

void PyString::set( const char* str, size_t len )
{
     value.assign(str, len);
     return;

    /* don't allow invalid strings to be set */
    if (str == NULL && len != 0)
        return;

    value.resize(len);

    /* if we set a empty string */
    if (str == NULL || len == 0)
        return;

    memcpy(&value[0], str, len);
    //value[len] = '\0';
}

