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
#include <assert.h>

#include "PyVisitor.h"
#include "PyRep.h"
#include "PyDumpVisitor.h"
#include "misc.h"
#include "packet_functions.h"

//! the identation amount
uint32 idenAmt = 4;

PyDumpVisitor::PyDumpVisitor(bool full_lists) : PyVisitorLvl(), m_full_lists(full_lists) {}
PyDumpVisitor::~PyDumpVisitor() {}

PyLogsysDump::PyLogsysDump(LogType type, bool full_hex, bool full_lists)
: PyDumpVisitor(full_lists),
  m_type(type),
  m_hex_type(type),
  m_full_hex(full_hex)
{
}

PyLogsysDump::PyLogsysDump(LogType type, LogType hex_type, bool full_hex, bool full_lists)
: PyDumpVisitor(full_lists),
  m_type(type),
  m_hex_type(hex_type),
  m_full_hex(full_hex)
{
}

// --- Visitors implementation ---

void PyDumpVisitor::VisitInteger(const PyInt *rep, int64 lvl ) {
    _print(lvl, "Integer field: "I64d, rep->value);
}

void PyDumpVisitor::VisitReal(const PyFloat *rep, int64 lvl ) {
    _print(lvl, "Real field: %f", rep->value);
}

void PyDumpVisitor::VisitBoolean(const PyBool *rep, int64 lvl ) {
    _print(lvl, "Boolean field: %s", rep->value ? "true" : "false");
}

void PyDumpVisitor::VisitNone(const PyNone *rep, int64 lvl ) {
    _print(lvl, "(None)");
}

void PyDumpVisitor::VisitBuffer(const PyBuffer *rep, int64 lvl ) {

    std::string curIden(lvl, ' '); // please clean this one...

    _print(lvl, "Data buffer of length %d", rep->GetLength());

    //kinda hackish:
    if(rep->GetLength() > 2 && *(rep->GetBuffer()) == GZipStreamHeaderByte) {
        uint32 len = rep->GetLength();
        uint8 *buf = InflatePacket(rep->GetBuffer(), &len, true);
        if(buf != NULL) {
            _print(lvl, "  Data buffer contains gzipped data of length %u", len);

            _hexDump(buf, len, curIden.c_str());

            free(buf);
        }
    }
    else if(rep->GetLength() > 0)
    {
        _hexDump(rep->GetBuffer(), rep->GetLength(), curIden.c_str());
    }
}


void PyDumpVisitor::VisitString( const PyString *rep, int64 lvl )
{
    if(ContainsNonPrintables(rep->value.c_str(), rep->value.length()))
    {
        if ( rep->is_type_1 == true )
            _print(lvl, "String%s: '<binary, len=%d>'", " (Type1)", rep->size());
        else
            _print(lvl, "String%s: '<binary, len=%d>'", "", rep->size());
    }
    else
    {
        //print_string.append("String%s: '%s'");
        if ( rep->is_type_1 == true )
            _print(lvl, "String%s: '%s'", " (Type1)", rep->content());
        else
            _print(lvl, "String%s: '%s'", "", rep->content());
    }
}

void PyDumpVisitor::VisitObjectEx(const PyObjectEx *rep, int64 lvl )
{
	_print( lvl, "ObjectEx:" );
    _print( lvl, "Header:" );

    rep->header->visit(this, lvl + idenAmt );

    {
		_print( lvl, "ListData: %u entries", (uint32)rep->list_data.size() );
        PyObjectEx::const_list_iterator cur, end;
        cur = rep->list_data.begin();
        end = rep->list_data.end();
        for(uint32 i = 0; cur != end; ++cur, ++i)
        {
			_print( lvl, "  [%2d] ", i );
            (*cur)->visit(this, lvl + idenAmt );
        }
    }

    {
		_print( lvl, "DictData: %u entries", (uint32)rep->dict_data.size() );
        PyObjectEx::const_dict_iterator cur, end;
        cur = rep->dict_data.begin();
        end = rep->dict_data.end();
        for(uint32 i = 0; cur != end; ++cur, ++i)
        {
			_print( lvl, "  [%2d] Key: ", i );
            cur->first->visit(this, lvl + idenAmt );

			_print( lvl, "  [%2d] Value: ", i );
            cur->second->visit(this, lvl + idenAmt );
        }
    }
}

void PyDumpVisitor::VisitPackedRow(const PyPackedRow *rep, int64 lvl )
{
    uint32 cc = rep->ColumnCount();

	_print( lvl, "Packed Row" );
	_print( lvl, "  column_count=%u header_owner=%s", cc, rep->IsHeaderOwner() ? "yes" : "no" ); 

    for(uint32 i = 0; i < cc; i++)
    {
        PyRep *field = rep->GetField( i );

		_print( lvl, "  [%u] %s: ", i, rep->GetColumnName( i ).c_str() );

		if( field == NULL )
			_print( lvl + idenAmt, "NULL" );
		else
			field->visit( this, lvl + idenAmt );
    }
}

void PyDumpVisitor::VisitObject(const PyObject *rep, int64 lvl )
{
	_print( lvl, "Object:" );
	_print( lvl, "  Type: %s", rep->type.c_str() );
	_print( lvl, "  Args: " );

    rep->arguments->visit(this, lvl + idenAmt );
}

void PyDumpVisitor::VisitSubStruct(const PySubStruct *rep, int64 lvl ) {
	_print( lvl, "SubStruct: " );

    rep->sub->visit(this, lvl + idenAmt );
}

void PyDumpVisitor::VisitSubStream(const PySubStream *rep, int64 lvl ) {
    if(rep->decoded == NULL) {
        //we have not decoded this substream, leave it as hex:
        if(rep->data == NULL) {
            _print(lvl, "INVALID Substream: no data (length %u)", rep->length);
        } else {
            _print(lvl, "Substream: length %u", rep->length);
			_hexDump( rep->data, rep->length, std::string( lvl, ' ' ).c_str() );
        }
    } else {
        _print(lvl, "Substream: length %u %s", rep->length, (rep->data==NULL) ? "from rep":"from data");
        rep->decoded->visit(this, lvl + idenAmt );
    }
}

void PyDumpVisitor::VisitChecksumedStream(const PyChecksumedStream *rep, int64 lvl ) {

}

void PyDumpVisitor::VisitDict(const PyDict *rep, int64 lvl ) {
	_print( lvl, "Dictionary: %u entries", rep->size() );

    PyDict::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(uint32 i = 0; cur != end; ++cur, ++i) {

		_print( lvl, "  [%2u] Key: ", i );
        cur->first->visit(this, lvl + idenAmt );
		_print( lvl, "  [%2u] Value: ", i );
        cur->second->visit(this, lvl + idenAmt );
    }
}

void PyDumpVisitor::VisitList(const PyList *rep, int64 lvl )
{
    if(rep->items.empty())
		_print( lvl, "List: Empty" );
    else {
		_print( lvl, "List: %u elements", rep->size() );

        PyList::const_iterator cur, end;
        cur = rep->begin();
        end = rep->end();
        for(uint32 i = 0; cur != end; ++cur, ++i) {

            if(!m_full_lists && i > 200) {
				_print( lvl, "   ... truncated ..." );
                break;
            }

			_print( lvl, "  [%2u] ", i );
            (*cur)->visit(this, lvl + idenAmt );
        }
    }
}

void PyDumpVisitor::VisitTuple(const PyTuple *rep, int64 lvl )
{
    if(rep->items.empty())
		_print( lvl, "Tuple: Empty" );
    else {
		_print( lvl, "Tuple: %u elements", rep->size() );

        //! visit tuple elements.
        PyTuple::const_iterator cur, end;
        cur = rep->begin();
        end = rep->end();
        for(uint32 i = 0; cur != end; ++cur, ++i) {

			_print( lvl, "  [%2u] ", i ); 
            (*cur)->visit(this, lvl + idenAmt);
        }
    }
}

void PyLogsysDump::VisitDict(const PyDict *rep, int64 lvl ) {
        PyDumpVisitor::VisitDict(rep, lvl);
}

void PyLogsysDump::VisitList(const PyList *rep, int64 lvl ) {
        PyDumpVisitor::VisitList(rep, lvl);
}

void PyLogsysDump::VisitTuple(const PyTuple *rep, int64 lvl ) {
        PyDumpVisitor::VisitTuple(rep, lvl);
}

void PyLogsysDump::VisitSubStream(const PySubStream *rep, int64 lvl ) {
        PyDumpVisitor::VisitSubStream(rep, lvl);
}


void PyLogsysDump::_print(const char *str, ...)
{
    if( !is_log_enabled( m_type ) )
        return;
    va_list l;
    va_start(l, str);
    size_t len = strlen(str)+1;
    char *buf = new char[len];
    snprintf(buf, len, "%s", str);
    log_messageVA(m_type, buf, l);
    delete[] buf;
    va_end(l);
}

void PyLogsysDump::_print( uint32 iden, const char *str, ... )
{
    if( !is_log_enabled( m_type ) )
        return;
    va_list l;
    va_start(l, str);
    size_t len = strlen(str)+1;
    char *buf = new char[len];
    snprintf(buf, len, "%s", str);
    log_messageVA(m_type, iden, buf, l);
    delete[] buf;
    va_end(l);
}
void PyLogsysDump::_hexDump(const uint8 *bytes, uint32 len, const char * ident) {
    if(m_full_hex)
    {
        _hex(m_hex_type, bytes, len);
    }
    else
    {
        phex(m_hex_type, bytes, len);
    }
}


PyFileDump::PyFileDump(FILE *into, bool full_hex)
: PyDumpVisitor(false),
  m_into(into),
  m_full_hex(full_hex)
{
}

void PyFileDump::_print(const char *str, ...) {
    va_list l;
    va_start(l, str);
    size_t len = strlen(str)+1;
    char *buf = new char[len];
    snprintf(buf, len, "%s", str);
    assert(vfprintf(m_into, buf, l) >= 0);
    delete[] buf;
    va_end(l);
}

void PyFileDump::_hexDump(const uint8 *bytes, uint32 len, const char * ident) {
    if(!m_full_hex && len > 1024) {
        char buffer[80];
        _pfxHexDump(bytes, 1024-32, ident);
        fprintf(m_into, "%s ... truncated ...\n", ident);
        build_hex_line((const char *)bytes, len, len-16, buffer, 4);
        fprintf(m_into, "%s%s\n", ident, buffer);
    } else {
        _pfxHexDump(bytes, len, ident);
    }
}

void PyFileDump::_pfxHexDump(const uint8 *data, uint32 length, const char * ident) {
    char buffer[80];
    uint32 offset;
    for(offset=0;offset<length;offset+=16) {
        build_hex_line((const char *)data,length,offset,buffer,4);
        fprintf(m_into, "%s%s\n", ident, buffer);
    }
}
