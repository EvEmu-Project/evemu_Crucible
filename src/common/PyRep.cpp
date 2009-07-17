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
	Author:		Zhur
*/

#include "common.h"
#include "misc.h"
#include "packet_functions.h"
#include "PyRep.h"
#include "PyVisitor.h"
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
	"Integer",			//0
	"Real",				//1
	"Boolean",			//2
	"Buffer",			//3
	"String",			//4
	"Tuple",			//5
	"List",				//6
	"Dict",				//7
	"None",				//8
	"SubStruct",		//9
	"SubStream",		//10
	"ChecksumedStream",	//11
	"Object",			//12
	"ObjectEx",			//13
	"PackedRow",		//14
	"UNKNOWN TYPE",		//15
};

//#undef _log
//#define _log //

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

PyRep::PyRep(Type t) : m_type(t) {}
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
void PyRepInteger::Dump(FILE *into, const char *pfx) const {
	fprintf(into, "%sInteger field: "I64u"\n", pfx, value);
}

void PyRepInteger::Dump(LogType type, const char *pfx) const {
	_log(type, "%sInteger field: "I64u, pfx, value);
}

/************************************************************************/
/* PyRep Real/float/double Class                                        */
/************************************************************************/

void PyRepReal::Dump(FILE *into, const char *pfx) const {
	fprintf(into, "%sReal Field: %f\n", pfx, value);
}

void PyRepReal::Dump(LogType type, const char *pfx) const {
	_log(type, "%sReal Field: %f", pfx, value);
}

/************************************************************************/
/* PyRep Boolean Class                                                  */
/************************************************************************/
void PyRepBoolean::Dump(FILE *into, const char *pfx) const {
	fprintf(into, "%sBoolean field: %s\n", pfx, value?"true":"false");
}

void PyRepBoolean::Dump(LogType type, const char *pfx) const {
	_log(type, "%sBoolean field: %s", pfx, value?"true":"false");
}

/************************************************************************/
/* PyRep None Class                                                     */
/************************************************************************/
void PyRepNone::Dump(FILE *into, const char *pfx) const {
	fprintf(into, "%s(None)\n", pfx);
}

void PyRepNone::Dump(LogType type, const char *pfx) const {
	_log(type, "%s(None)", pfx);
}

/************************************************************************/
/* PyRep Buffer Class                                                   */
/************************************************************************/
PyRepBuffer::PyRepBuffer(const uint8 *buffer, uint32 length) : PyRep(PyRep::PyTypeBuffer),
  m_value(new uint8[length]),
  m_length(length)
{
	memcpy(m_value, buffer, length);
}

PyRepBuffer::PyRepBuffer(uint32 length)
: PyRep(PyRep::PyTypeBuffer),
  m_value(new uint8[length]),
  m_length(length)
{
}

PyRepBuffer::PyRepBuffer(uint8 **buffer, uint32 length)
: PyRep(PyRep::PyTypeBuffer),
  m_value(*buffer),
  m_length(length)
{
	*buffer = NULL;
}

PyRepBuffer::~PyRepBuffer() {
	SafeDeleteArray(m_value);
}

void PyRepBuffer::Dump(FILE *into, const char *pfx) const {
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
			SafeDeleteArray(buf);
		}
	}
}

void PyRepBuffer::Dump(LogType type, const char *pfx) const {
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
			SafeDeleteArray(buf);
		}
	}
}

PyRepSubStream *PyRepBuffer::CreateSubStream() const {
	if(*m_value == SubStreamHeaderByte) {
		return(new PyRepSubStream(m_value, m_length));
	} else if(m_length > 2 && *m_value == GZipStreamHeaderByte) {
		uint32 len = GetLength();
		uint8 *buf = InflatePacket(GetBuffer(), &len, true);

		PyRepSubStream *res = NULL;
		if(buf == NULL) {
			//unable to unzip, this does not appear to be a stream, so refuse to turn into one.
		} else if(*buf != SubStreamHeaderByte) {
			//wrong header byte, this does not appear to be a stream, so refuse to turn into one.
		} else {
			res = new PyRepSubStream(buf, len);
		}

		SafeDeleteArray(buf);
		return res;
	}
	//else, we don't think this is a substream, so don't become one.
	return NULL;
}

/************************************************************************/
/* PyRep String Class                                                   */
/************************************************************************/
void PyRepString::Dump(FILE *into, const char *pfx) const {
	if(ContainsNonPrintables(value.c_str(), (uint32)value.length())) {
		fprintf(into, "%sString%s: '<binary, len=%lu>'\n", pfx, is_type_1?" (Type1)":"", value.length());
	} else {
		fprintf(into, "%sString%s: '%s'\n", pfx, is_type_1?" (Type1)":"", value.c_str());
	}
}

void PyRepString::Dump(LogType type, const char *pfx) const {
	if(ContainsNonPrintables(value.c_str(), (uint32)value.length())) {
		_log(type, "%sString%s: '<binary, len=%lu>'", pfx, is_type_1?" (Type1)":"", value.length());
	} else {
		_log(type, "%sString%s: '%s'", pfx, is_type_1?" (Type1)":"", value.c_str());
	}
}

/************************************************************************/
/* PyRep Tuple Class                                                    */
/************************************************************************/
PyRepTuple::~PyRepTuple() {
	clear();
}

void PyRepTuple::clear() {
	storage_type::iterator cur, _end;
	cur = begin();
	_end = end();
	for(; cur != _end; cur++)
		delete *cur;
	items.clear();
}

void PyRepTuple::Dump(FILE *into, const char *pfx) const {
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

void PyRepTuple::Dump(LogType type, const char *pfx) const {
	//extra check to avoid potentially a lot of work if we are disabled
	if(!is_log_enabled(type))
		return;
	
	if(items.empty())
		_log(type, "%sTuple: Empty", pfx);
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

uint32 PyRepTuple::size() const
{
    return (uint32)items.size();
}

/************************************************************************/
/* PyRep List Class                                                     */
/************************************************************************/
PyRepList::~PyRepList()
{
    clear();
}

void PyRepList::clear() {
	std::vector<PyRep *>::iterator cur, _end;
	cur = items.begin();
	_end = items.end();
	for(; cur != _end; cur++)
		delete *cur;
	items.clear();
}

uint32 PyRepList::size() const
{
    return (uint32)items.size();
}

void PyRepList::Dump(FILE *into, const char *pfx) const {
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

void PyRepList::Dump(LogType type, const char *pfx) const {
	//extra check to avoid potentially a lot of work if we are disabled
	if(!is_log_enabled(type))
		return;
	
	if(items.empty())
		_log(type, "%sList: Empty", pfx);
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

void PyRepList::addInt(uint64 intval) {
	items.push_back(new PyRepInteger(intval));
}

void PyRepList::addReal(double realval) {
	items.push_back(new PyRepReal(realval));
}

void PyRepList::addStr(const char *str) {
	items.push_back(new PyRepString(str));
}

void PyRepList::add(const char *str) {
	items.push_back(new PyRepString(str));
}

void PyRepList::add(PyRep *i) {
	items.push_back(i);
}

/************************************************************************/
/* PyRep Dict Class                                                     */
/************************************************************************/
PyRepDict::~PyRepDict() {
	clear();
}

void PyRepDict::clear() {
	iterator cur, _end;
	cur = items.begin();
	_end = items.end();
	for(; cur != _end; cur++) {
		delete cur->first;
		delete cur->second;
	}
}

void PyRepDict::Dump(FILE *into, const char *pfx) const {
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

void PyRepDict::Dump(LogType type, const char *pfx) const {
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

void PyRepDict::add(PyRep *key, PyRep *value) {
	if(key == NULL || value == NULL)
		return;
	items[key] = value;
}

void PyRepDict::add(const char *key, PyRep *value) {
	if(key == NULL || value == NULL)
		return;
	items[new PyRepString(key)] = value;
}

void PyRepDict::add(const char *key, const char *value) {
	if(key == NULL || value == NULL)
		return;
	items[new PyRepString(key)] = new PyRepString(value);
}

/************************************************************************/
/* PyRep Object Class                                                   */
/************************************************************************/
PyRepObject::~PyRepObject() {
	SafeDelete(arguments);
}

void PyRepObject::Dump(FILE *into, const char *pfx) const {
	fprintf(into, "%sObject:\n", pfx);
	fprintf(into, "%s  Type: %s\n", pfx, type.c_str());
	
	std::string m(pfx);
	m += "  Args: ";
	arguments->Dump(into, m.c_str());
}

void PyRepObject::Dump(LogType ltype, const char *pfx) const {
	_log(ltype, "%sObject:", pfx);
	_log(ltype, "%s  Type: %s", pfx, type.c_str());
	
	std::string m(pfx);
	m += "  Args: ";
	arguments->Dump(ltype, m.c_str());
}

/************************************************************************/
/* PyRep SubStruct Class                                                */
/************************************************************************/
PyRepSubStruct::~PyRepSubStruct()
{
	SafeDelete(sub);
}

void PyRepSubStruct::Dump(FILE *into, const char *pfx) const {
	fprintf(into, "%sSubstruct:\n", pfx);
	std::string m(pfx);
	m += "    ";
	sub->Dump(into, m.c_str());
}

void PyRepSubStruct::Dump(LogType type, const char *pfx) const {
	_log(type, "%sSubstruct:", pfx);
	std::string m(pfx);
	m += "    ";
	sub->Dump(type, m.c_str());
}

/************************************************************************/
/* PyRep SubStream Class                                                */
/************************************************************************/
PyRepSubStream::PyRepSubStream(const uint8 *buffer, uint32 len)
: PyRep(PyRep::PyTypeSubStream),
  length(len),
  data(new uint8[len]),
  decoded(NULL)
{
	memcpy(data, buffer, length);
}

PyRepSubStream::~PyRepSubStream() {
	SafeDeleteArray(data);
	SafeDelete(decoded);
}

void PyRepSubStream::Dump(FILE *into, const char *pfx) const {
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

void PyRepSubStream::Dump(LogType type, const char *pfx) const {
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

void PyRepSubStream::EncodeData() {
	if(data != NULL)
		return;
	if(decoded == NULL)
		return;
	data = Marshal(this, length);
}

void PyRepSubStream::DecodeData() const {
	if(decoded != NULL)
		return;
	decoded = InflateAndUnmarshal(data, length);
}

/************************************************************************/
/* PyRep ChecksumedStream Class                                         */
/************************************************************************/
PyRepChecksumedStream::~PyRepChecksumedStream() {
	SafeDelete(stream);
}

void PyRepChecksumedStream::Dump(FILE *into, const char *pfx) const {
	fprintf(into, "%sStream With Checksum: 0x%08x\n", pfx, checksum);
	stream->Dump(into, pfx);
}

void PyRepChecksumedStream::Dump(LogType type, const char *pfx) const {
	_log(type, "%sStream With Checksum: 0x%08x", pfx, checksum);
	stream->Dump(type, pfx);
}

PyRepInteger *PyRepInteger::TypedClone() const {
	return(new PyRepInteger(value));
}

PyRepReal *PyRepReal::TypedClone() const {
	return(new PyRepReal(value));
}

PyRepBoolean *PyRepBoolean::TypedClone() const {
	return(new PyRepBoolean(value));
}

PyRepNone *PyRepNone::TypedClone() const {
	return(new PyRepNone());
}

PyRepBuffer *PyRepBuffer::TypedClone() const {
	return(new PyRepBuffer(m_value, m_length));
}

PyRepString *PyRepString::TypedClone() const {
	return(new PyRepString(value, is_type_1));
}

PyRepTuple *PyRepTuple::TypedClone() const {
	PyRepTuple *r = new PyRepTuple(0);
	r->CloneFrom(this);
	return(r);
}

void PyRepTuple::CloneFrom(const PyRepTuple *from) {
	clear();
	items.reserve(items.size());
	
	std::vector<PyRep *>::const_iterator cur, _end;
	cur = from->items.begin();
	_end = from->items.end();
	for(; cur != _end; cur++) {
		items.push_back((*cur)->Clone());
	}
}

PyRepList *PyRepList::TypedClone() const {
	PyRepList *r = new PyRepList();
	r->CloneFrom(this);
	return(r);
}

void PyRepList::CloneFrom(const PyRepList *from) {
	PyRepList::const_iterator cur, _end;
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

PyRepDict *PyRepDict::TypedClone() const {
	PyRepDict *r = new PyRepDict();
	r->CloneFrom(this);
	return(r);
}

void PyRepDict::CloneFrom(const PyRepDict *from) {
	PyRepDict::const_iterator cur, _end;
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

PyRepObject *PyRepObject::TypedClone() const {
	return(new PyRepObject( type, arguments->Clone() ));
}

PyRepSubStruct *PyRepSubStruct::TypedClone() const {
	return(new PyRepSubStruct( sub->Clone() ));
}

PyRepSubStream *PyRepSubStream::TypedClone() const {
	if(data == NULL)
		return(new PyRepSubStream( decoded->Clone() ));
	return(new PyRepSubStream( data, length ));
}

PyRepChecksumedStream *PyRepChecksumedStream::TypedClone() const {
	return(new PyRepChecksumedStream( checksum, stream->Clone() ));
}

void PyRepInteger::visit(PyVisitor *v) const {
	v->VisitInteger(this);
}

void PyRepReal::visit(PyVisitor *v) const {
	v->VisitReal(this);
}

void PyRepBoolean::visit(PyVisitor *v) const {
	v->VisitBoolean(this);
}

void PyRepNone::visit(PyVisitor *v) const {
	v->VisitNone(this);
}

void PyRepBuffer::visit(PyVisitor *v) const {
	v->VisitBuffer(this);
}

void PyRepString::visit(PyVisitor *v) const {
	v->VisitString(this);
}

void PyRepTuple::visit(PyVisitor *v) const {
	v->VisitTuple(this);
}

void PyRepList::visit(PyVisitor *v) const {
	v->VisitList(this);
}

void PyRepDict::visit(PyVisitor *v) const {
	v->VisitDict(this);
}

void PyRepObject::visit(PyVisitor *v) const {
	v->VisitObject(this);
}

void PyRepPackedRow::visit(PyVisitor *v) const {
	v->VisitPackedRow(this);
}

void PyRepSubStruct::visit(PyVisitor *v) const {
	v->VisitSubStruct(this);
}

void PyRepSubStream::visit(PyVisitor *v) const {
	v->VisitSubStream(this);
}

void PyRepChecksumedStream::visit(PyVisitor *v) const {
	v->VisitChecksumedStream(this);
}





PyRepPackedRow::PyRepPackedRow(const PyRep &header, bool header_owner)
: PyRep( PyRep::PyTypePackedRow ),
  mHeader( header ),
  mHeaderOwner( header_owner ),
  mColumnInfo( NULL )
{
	assert( GetHeader().IsObjectEx() );
	PyRep *r = GetHeader().AsObjectEx().header;

	assert( r != NULL && r->IsTuple() );
	PyRepTuple *t = &r->AsTuple();

	assert( t->items.size() == 2 );

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

	mFields.resize( mColumnInfo->items.size() );
}

PyRepPackedRow::~PyRepPackedRow()
{
	if( IsHeaderOwner() )
		delete &mHeader;

	std::vector<PyRep *>::iterator cur, end;
	cur = mFields.begin();
	end = mFields.end();
	for(; cur != end; cur++)
		SafeDelete( *cur );
}


void PyRepPackedRow::Dump(FILE *into, const char *pfx) const
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

void PyRepPackedRow::Dump(LogType ltype, const char *pfx) const
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

		if( v == NULL )
			_log( ltype, "%sNULL", n.c_str() );
		else
			v->Dump( ltype, n.c_str() );
	}
}

PyRepPackedRow *PyRepPackedRow::TypedClone() const
{
	PyRepPackedRow *res = new PyRepPackedRow( IsHeaderOwner() ? *GetHeader().Clone() : GetHeader(),
	                                          IsHeaderOwner() );
	res->CloneFrom( this );
	return res;
}

void PyRepPackedRow::CloneFrom(const PyRepPackedRow *from)
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

const std::string &PyRepPackedRow::GetColumnName(uint32 index) const
{
	assert( mColumnInfo->items.size() > index );
	PyRep *r = mColumnInfo->items[ index ];

	assert( r->IsTuple() );
	PyRepTuple &t = r->AsTuple();

	assert( t.items.size() == 2 );
	r = t.items[ 0 ];

	assert( r->IsString() );
	return r->AsString().value;
}

uint32 PyRepPackedRow::GetColumnIndex(const char *name) const
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

DBTYPE PyRepPackedRow::GetColumnType(uint32 index) const
{
	assert( mColumnInfo->items.size() > index );
	PyRep *r = mColumnInfo->items[ index ];

	assert( r->IsTuple() );
	PyRepTuple *t = &r->AsTuple();

	assert( t->items.size() == 2 );
	r = t->items[ 1 ];

	assert( r->IsInteger() );
	return static_cast<DBTYPE>( r->AsInteger().value );
}

bool PyRepPackedRow::SetField(uint32 index, PyRep *value)
{
	if( index >= ColumnCount() )
		return false;

	if( value != NULL )
	{
		// verify type
		if( !DBTYPE_IsCompatible( GetColumnType( index ), *value ) )
		{
			SafeDelete( value );
			return false;
		}
	}

	PyRep *&v = mFields.at( index );
	SafeDelete( v );
	v = value;

	return true;
}

bool PyRepPackedRow::SetField(const char *colName, PyRep *value)
{
	uint32 index = GetColumnIndex( colName );
	if( index >= ColumnCount() )
		return false;
	return SetField( index, value );
}



PyRepObjectEx::~PyRepObjectEx() {
	{
		const_list_iterator cur, end;
		cur = list_data.begin();
		end = list_data.end();
		for(; cur != end; cur++)
			delete *cur;
		list_data.clear();
	}
	{
		const_dict_iterator cur, end;
		cur = dict_data.begin();
		end = dict_data.end();
		for(; cur != end; cur++) {
			delete cur->first;
			delete cur->second;
		}
		dict_data.clear();
	}
}

void PyRepObjectEx::Dump(FILE *into, const char *pfx) const {
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

void PyRepObjectEx::Dump(LogType ltype, const char *pfx) const {
	if(!is_log_enabled(ltype))
		return;

	_log(ltype, "%sObjectEx%s\n", pfx, (is_type_1 ? " (Type1)" : ""));
	_log(ltype, "%sHeader:\n", pfx);
	if(header == NULL)
		_log(ltype, "%s  None\n", pfx);
	else {
		std::string p(pfx);
		p += "  ";

		header->Dump(ltype, p.c_str());
	}

	_log(ltype, "%sList data:", pfx);
	if(list_data.empty())
		_log(ltype, "%s  Empty", pfx);
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

	_log(ltype, "%sDict data:", pfx);
	if(dict_data.empty())
		_log(ltype, "%s  Empty", pfx);
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

void PyRepObjectEx::visit(PyVisitor *v) const {
	v->VisitObjectEx(this);
}

PyRepObjectEx *PyRepObjectEx::TypedClone() const {
	PyRepObjectEx *clone = new PyRepObjectEx(is_type_1);
	clone->CloneFrom(this);
	return(clone);
}

void PyRepObjectEx::CloneFrom(const PyRepObjectEx *from) {
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
EVEStringTable *PyRepString::s_stringTable = NULL;
bool PyRepString::LoadStringFile(const char *file)
{
	if(s_stringTable != NULL)
		SafeDelete(s_stringTable);

	s_stringTable = new EVEStringTable;
	return s_stringTable->LoadFile(file);
}

EVEStringTable *PyRepString::GetStringTable()
{
	if(s_stringTable == NULL)
		s_stringTable = new EVEStringTable(); //make an empty one.

	return s_stringTable;
}

EVEStringTable::EVEStringTable() : m_size(0) { }
EVEStringTable::~EVEStringTable()
{
	char *entry = NULL;
	for (uint32 i = 0; i != m_size; i++)
	{
		entry = m_forwardLookup[i];
		if (entry != NULL)
			delete []entry;
	}
	m_forwardLookup.clear();
	m_reverseLookup.clear();
}

bool EVEStringTable::LoadFile(const char *file)
{
	m_forwardLookup.clear();
	m_reverseLookup.clear();

	//first line of the file is item 1
	m_forwardLookup.push_back(NULL);	//should never be used.

	FILE *f = fopen(file, "r");
	if(f == NULL)
		return false;

	uint8 indexNr = 1;
	
	char buf[256];
	while(fgets(buf, 255, f) != NULL)
	{
		char *e = strchr(buf, '\r');
		 
		if(e == NULL)
			e = strchr(buf, '\n');
		if(e != NULL)
			*e = '\0';

		uint8 lookupStrLen = strlen(buf);
		char* lookupStr = new char[lookupStrLen+1];
		memcpy(lookupStr,buf,lookupStrLen+1);
		lookupStr[lookupStrLen] = '\0';

		m_forwardLookup.push_back(lookupStr);
		m_reverseLookup.insert(LookupMap::value_type(lookupStr, indexNr)); 
		indexNr++;
	}
	m_size = indexNr;
	fclose(f);
	return true;
}

//takes a string as an argument since its going to be converted to
//one anyhow when we give it to the map.
uint8 EVEStringTable::LookupString(const std::string &str) const
{
	LookupMapConstItr res = m_reverseLookup.find(str);
	if(res == m_reverseLookup.end())
		return None;

	return res->second;
}

const char *EVEStringTable::LookupIndex(uint8 index) const
{
	if(index == None || index >= m_size)
		return NULL;

	return m_forwardLookup[index];
}
