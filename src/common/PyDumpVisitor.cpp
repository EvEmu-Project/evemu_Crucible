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

void PyDumpVisitor::VisitInteger(const PyRepInteger *rep, int64 lvl ) {
    _print(std::string(std::string(lvl, ' ')+std::string("Integer field: %ll")).c_str(), rep->value);
}

void PyDumpVisitor::VisitReal(const PyRepReal *rep, int64 lvl ) {
    _print(std::string(std::string(lvl, ' ')+std::string("Real Field: %f")).c_str(), rep->value);
}

void PyDumpVisitor::VisitBoolean(const PyRepBoolean *rep, int64 lvl ) {
    _print(std::string(std::string(lvl, ' ')+std::string("Boolean field: %s")).c_str(), rep->value?"true":"false");
    _print("Boolean field: %s", rep->value?"true":"false");
}

void PyDumpVisitor::VisitNone(const PyRepNone *rep, int64 lvl ) {
    _print("(None)");
}

void PyDumpVisitor::VisitBuffer(const PyRepBuffer *rep, int64 lvl ) {

    std::string curIden(lvl, ' ');

    _print(std::string(curIden+std::string("Data buffer of length %d")), rep->GetLength());

    //kinda hackish:
    if(rep->GetLength() > 2 && *(rep->GetBuffer()) == GZipStreamHeaderByte) {
        uint32 len = rep->GetLength();
        uint8 *buf = InflatePacket(rep->GetBuffer(), &len, true);
        if(buf != NULL) {
            _print(std::string(curIden+std::string("  Data buffer contains gzipped data of length %u")).c_str(), len);

            _hexDump(buf, len, curIden.c_str());

            delete[] buf;
        }
    } else if(rep->GetLength() > 0) {
        _hexDump(rep->GetBuffer(), rep->GetLength(), curIden.c_str());
    }
}


void PyDumpVisitor::VisitString( const PyRepString *rep, int64 lvl )
{
    // string size must be at least this..
    std::string print_string;
    print_string.reserve(rep->size() + lvl + 30);
    print_string.append(lvl, ' ');

    if(ContainsNonPrintables(rep->value.c_str(), rep->value.length()))
    {
        print_string.append("String%s: '<binary, len=%d>'");
        if ( rep->is_type_1 == true )
            _print(print_string.c_str(), " (Type1)", rep->size());
        else
            _print(print_string.c_str(), "", rep->size());
    }
    else
    {
        print_string.append("String%s: '%s'");
        if ( rep->is_type_1 == true )
            _print(print_string.c_str(), " (Type1)", rep->content());
        else
            _print(print_string.c_str(), "", rep->content());
    }
}

void PyDumpVisitor::VisitObjectEx(const PyRepObjectEx *rep, int64 lvl ) {

    std::string curIden(lvl, ' ');

    _print(std::string(curIden+std::string("ObjectEx:")));
    _print(std::string(curIden+std::string("Header:")));

    rep->header->visit(this, lvl + idenAmt );

    {
        _print(std::string(curIden+std::string("ListData: %u entries")) , (uint32)rep->list_data.size());
        PyRepObjectEx::const_list_iterator cur, end;
        cur = rep->list_data.begin();
        end = rep->list_data.end();
        for(uint32 i = 0; cur != end; ++cur, ++i)
        {
            _print(std::string(curIden+std::string("  [%2d] ")) , i);
            (*cur)->visit(this, lvl + idenAmt );
        }
    }

    {
        _print(std::string(curIden+std::string("DictData: %u entries")) , (uint32)rep->dict_data.size());
        PyRepObjectEx::const_dict_iterator cur, end;
        cur = rep->dict_data.begin();
        end = rep->dict_data.end();
        for(uint32 i = 0; cur != end; ++cur, ++i)
        {
            _print(std::string(curIden+std::string("  [%2d] Key: ")) , i);
            cur->first->visit(this, lvl + idenAmt );

            _print(std::string(curIden+std::string("  [%2d] Value: ")) , i);
            cur->second->visit(this, lvl + idenAmt );
        }
    }
}

void PyDumpVisitor::VisitPackedRow(const PyRepPackedRow *rep, int64 lvl ) {

    std::string curIden(lvl, ' ');

    uint32 cc = rep->ColumnCount();

    _print(std::string(curIden+std::string("Packed Row\n")));
    _print(std::string(curIden+std::string("%s  column_count=%u header_owner=%s\n")), cc, rep->IsHeaderOwner() ? "yes" : "no" );

    for(uint32 i = 0; i < cc; i++)
    {
        PyRep *field = rep->GetField( i );

        _print(std::string(curIden+std::string("  [%u] %s: ")),  i, rep->GetColumnName( i ).c_str() );

        field->visit( this, lvl + idenAmt );
    }
}

void PyDumpVisitor::VisitObject(const PyRepObject *rep, int64 lvl ) {
    std::string curIden(lvl, ' ');
    _print(std::string(curIden+std::string("Object:")));
    _print(std::string(curIden+std::string("  Type: %s")), rep->type.c_str());
    _print(std::string(curIden+std::string("  Args: ")));

    rep->arguments->visit(this, lvl + idenAmt );
}

void PyDumpVisitor::VisitSubStruct(const PyRepSubStruct *rep, int64 lvl ) {
    _print(std::string(std::string(lvl, ' ')+std::string("SubStruct: ")));

    rep->sub->visit(this, lvl + idenAmt );
}

void PyDumpVisitor::VisitSubStream(const PyRepSubStream *rep, int64 lvl ) {
   std::string curIden(lvl, ' ');

    if(rep->decoded == NULL) {
        //we have not decoded this substream, leave it as hex:
        if(rep->data == NULL) {
            _print(std::string(curIden+std::string("INVALID Substream: no data (length %d)")), rep->length);
        } else {
            _print(std::string(curIden+std::string("Substream: length %d")), rep->length);

            _hexDump(rep->data, rep->length, curIden.c_str());
        }
    } else {
        _print(std::string(curIden+std::string("Substream: length %d %s")), rep->length, (rep->data==NULL) ? "from rep":"from data");

        rep->decoded->visit(this, lvl + idenAmt );
    }
}

void PyDumpVisitor::VisitChecksumedStream(const PyRepChecksumedStream *rep, int64 lvl ) {

}

void PyDumpVisitor::VisitDict(const PyRepDict *rep, int64 lvl ) {
    std::string curIden(lvl, ' ');
    _print(std::string(curIden+std::string("Dictionary: %d entries")), rep->size());

    PyRepDict::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(uint32 i = 0; cur != end; ++cur, ++i) {

        _print(std::string(curIden+std::string("  [%2u] Key: ")), i);
        cur->first->visit(this, lvl + idenAmt );
        _print(std::string(curIden+std::string("  [%2u] Value: ")), i);
        cur->second->visit(this, lvl + idenAmt );
    }
}

void PyDumpVisitor::VisitList(const PyRepList *rep, int64 lvl ) {
    std::string curIden(lvl, ' ');
    if(rep->items.empty()) {
        _print(std::string(curIden+std::string("List: Empty")));
    } else {
        _print(std::string(curIden+std::string("List: %d elements")), rep->size());

        PyRepList::const_iterator cur, end;
        cur = rep->begin();
        end = rep->end();
        for(uint32 i = 0; cur != end; ++cur, ++i) {

            if(!m_full_lists && i > 200) {
                _print(std::string(curIden+std::string("   ... truncated ...")));
                break;
            }

            _print(std::string(curIden+std::string("  [%2u] ")), i);
            (*cur)->visit(this, lvl + idenAmt );
        }
    }
}

void PyDumpVisitor::VisitTuple(const PyRepTuple *rep, int64 lvl ) {
    std::string curIden(lvl, ' ');
    if(rep->items.empty())
        _print(std::string(curIden+std::string("Tuple: Empty")));
    else {
        _print(std::string(curIden+std::string("Tuple: %d elements")), rep->size());

        //! visit tuple elements.
        PyRepTuple::const_iterator cur, end;
        cur = rep->begin();
        end = rep->end();
        for(uint32 i = 0; cur != end; ++cur, ++i) {

            _print(std::string(curIden+std::string("  [%2u] ")), i);
            (*cur)->visit(this, lvl + idenAmt);
        }
    }
}

void PyLogsysDump::VisitDict(const PyRepDict *rep, int64 lvl ) {
        PyDumpVisitor::VisitDict(rep, lvl);
}

void PyLogsysDump::VisitList(const PyRepList *rep, int64 lvl ) {
        PyDumpVisitor::VisitList(rep, lvl);
}

void PyLogsysDump::VisitTuple(const PyRepTuple *rep, int64 lvl ) {
        PyDumpVisitor::VisitTuple(rep, lvl);
}

void PyLogsysDump::VisitSubStream(const PyRepSubStream *rep, int64 lvl ) {
        PyDumpVisitor::VisitSubStream(rep, lvl);
}


void PyLogsysDump::_print(const char *str, ...) {
    va_list l;
    va_start(l, str);
    size_t len = strlen(str)+1;
    char *buf = new char[len];
    snprintf(buf, len, "%s", str);
    log_messageVA(m_type, buf, l);
    delete[] buf;
    va_end(l);
}

void PyLogsysDump::_print(const std::string &str, ...) {
    const char* tStr = str.c_str();
    va_list l;
    va_start(l, tStr);
    size_t len = str.size()+1;
    char *buf = new char[len];
    snprintf(buf, len, "%s", str.c_str());
    log_messageVA(m_type, buf, l);
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

void PyFileDump::_print(const std::string &str, ...) {
    va_list l;
    va_start(l, str);
    assert(vfprintf(m_into, str.c_str(), l) >= 0);
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
