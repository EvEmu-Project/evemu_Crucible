/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "common.h"
#include "PyRep.h"
#include "PyVisitor.h"
#include "packet_functions.h"
#include "misc.h"
#include "EVEUnmarshal.h"
#include "EVEMarshal.h"
#include "EVEMarshalOpcodes.h"

static void pfxHexDump(const char *pfx, FILE *into, const byte *data, uint32 length) {
	char buffer[80];
	uint32 offset;
	for(offset=0;offset<length;offset+=16) {
		build_hex_line((const char *)data,length,offset,buffer,4);
		fprintf(into, "%s%s\n", pfx, buffer);
	}
}

static void pfxPreviewHexDump(const char *pfx, FILE *into, const byte *data, uint32 length) {
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

static void pfxHexDump(const char *pfx, LogType type, const byte *data, uint32 length) {
	char buffer[80];
	uint32 offset;
	for(offset=0;offset<length;offset+=16) {
		build_hex_line((const char *)data,length,offset,buffer,4);
		_log(type, "%s%s\n", pfx, buffer);
	}
}

static void pfxPreviewHexDump(const char *pfx, LogType type, const byte *data, uint32 length) {
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

const char *PyRep::TypeString() const {
	switch(m_type) {
	case Integer: return("Integer");
	case Real: return("Real");
	case Boolean: return("Boolean");
	case Buffer: return("Buffer");
	case String: return("String");
	case Tuple: return("Tuple");
	case List: return("List");
	case Dict: return("Dict");
	case None: return("None");
	case SubStruct: return("SubStruct");
	case SubStream: return("SubStream");
	case ChecksumedStream: return("ChecksumedStream");
	case Object: return("Object");
	case PackedHeader: return("PackedHeader");
	case PackedRow: return("PackedRow");
	case PackedResultSet: return("PackedResultSet");
	//no default on purpose
	}
	return("UNKNOWN TYPE");
}


void PyRepInteger::Dump(FILE *into, const char *pfx) const {
	fprintf(into, "%sInteger field: %lld\n", pfx, value);
}

void PyRepInteger::Dump(LogType type, const char *pfx) const {
	_log(type, "%sInteger field: %lld", pfx, value);
}

void PyRepReal::Dump(FILE *into, const char *pfx) const {
	fprintf(into, "%sReal Field: %f\n", pfx, value);
}

void PyRepReal::Dump(LogType type, const char *pfx) const {
	_log(type, "%sReal Field: %f", pfx, value);
}

void PyRepBoolean::Dump(FILE *into, const char *pfx) const {
	fprintf(into, "%sBoolean field: %s\n", pfx, value?"true":"false");
}

void PyRepBoolean::Dump(LogType type, const char *pfx) const {
	_log(type, "%sBoolean field: %s", pfx, value?"true":"false");
}

void PyRepNone::Dump(FILE *into, const char *pfx) const {
	fprintf(into, "%s(None)\n", pfx);
}

void PyRepNone::Dump(LogType type, const char *pfx) const {
	_log(type, "%s(None)", pfx);
}


PyRepBuffer::PyRepBuffer(const byte *buffer, uint32 length)
: PyRep(PyRep::Buffer),
  m_value(new byte[length]),
  m_length(length)
{
	memcpy(m_value, buffer, length);
}

PyRepBuffer::PyRepBuffer(uint32 length)
: PyRep(PyRep::Buffer),
  m_value(new byte[length]),
  m_length(length)
{
}

PyRepBuffer::PyRepBuffer(byte **buffer, uint32 length)
: PyRep(PyRep::Buffer),
  m_value(*buffer),
  m_length(length)
{
	*buffer = NULL;
}

PyRepBuffer::~PyRepBuffer() {
	delete[] m_value;
}

void PyRepBuffer::Dump(FILE *into, const char *pfx) const {
	fprintf(into, "%sData buffer of length %ld\n", pfx, m_length);

	//kinda hackish:
	if(m_length > 2 && *m_value == GZipStreamHeaderByte) {
		byte *ucbuf = new byte[m_length*10];	//bullshit length
		uint32 outlen = InflatePacket(m_value, m_length, ucbuf, m_length*10, true);
		if(outlen > 0) {
			string p(pfx);
			p += "  ";
			fprintf(into, "%sData buffer contains gzipped data of length %lu\n", p.c_str(), outlen);
			pfxPreviewHexDump(p.c_str(), into, ucbuf, outlen);
		}
		delete[] ucbuf;
	}
}

void PyRepBuffer::Dump(LogType type, const char *pfx) const {
	_log(type, "%sData buffer of length %d", pfx, m_length);

	//kinda hackish:
	if(m_length > 2 && *m_value == GZipStreamHeaderByte) {
		byte *ucbuf = new byte[m_length*10];	//bullshit length
		uint32 outlen = InflatePacket(m_value, m_length, ucbuf, m_length*10, true);
		if(outlen > 0) {
			string p(pfx);
			p += "  ";
			_log(type, "%sData buffer contains gzipped data of length %lu", p.c_str(), outlen);
			pfxPreviewHexDump(p.c_str(), type, ucbuf, outlen);
		}
		delete[] ucbuf;
	}
}

PyRepSubStream *PyRepBuffer::CreateSubStream() const {
	if(*m_value == SubStreamHeaderByte) {
		return(new PyRepSubStream(m_value, m_length));
	} else if(m_length > 2 && *m_value == GZipStreamHeaderByte) {
		byte *ucbuf = new byte[m_length*10];	//bullshit length
		uint32 outlen = InflatePacket(m_value, m_length, ucbuf, m_length*10, true);
		
		PyRepSubStream *res = NULL;
		if(outlen == 0) {
			//unable to unzip, this does not appear to be a stream, so refuse to turn into one.
		} else if(*ucbuf != SubStreamHeaderByte) {
			//wrong header byte, this does not appear to be a stream, so refuse to turn into one.
		} else {
			res = new PyRepSubStream(ucbuf, outlen);
		}
		
		delete[] ucbuf;
		return(res);
	}
	//else, we dont think this is a substream, so dont become one.
	return(NULL);
}

void PyRepString::Dump(FILE *into, const char *pfx) const {
	if(ContainsNonPrintables(value.c_str(), value.length())) {
		fprintf(into, "%sString%s: '<binary, len=%d>'\n", pfx, is_type_1?" (Type1)":"", value.length());
	} else {
		fprintf(into, "%sString%s: '%s'\n", pfx, is_type_1?" (Type1)":"", value.c_str());
	}
}

void PyRepString::Dump(LogType type, const char *pfx) const {
	if(ContainsNonPrintables(value.c_str(), value.length())) {
		_log(type, "%sString%s: '<binary, len=%d>'", pfx, is_type_1?" (Type1)":"", value.length());
	} else {
		_log(type, "%sString%s: '%s'", pfx, is_type_1?" (Type1)":"", value.c_str());
	}
}

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
		fprintf(into, "%sTuple: %d elements\n", pfx, items.size());
		std::vector<PyRep *>::const_iterator cur, _end;
		cur = items.begin();
		_end = items.end();
		char t[15];
		int r;
		for(r = 0; cur != _end; cur++, r++) {
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
		_log(type, "%sTuple: %d elements", pfx, items.size());
		std::vector<PyRep *>::const_iterator cur, _end;
		cur = items.begin();
		_end = items.end();
		char t[15];
		int r;
		for(r = 0; cur != _end; cur++, r++) {
			std::string n(pfx);
			snprintf(t, 14, "  [%2d] ", r);
			n += t;
			(*cur)->Dump(type, n.c_str());
		}
	}
}

PyRepList::~PyRepList() {
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

void PyRepList::Dump(FILE *into, const char *pfx) const {
	if(items.empty())
		fprintf(into, "%sList: Empty\n", pfx);
	else {
		fprintf(into, "%sList: %d elements\n", pfx, items.size());
		std::vector<PyRep *>::const_iterator cur, _end;
		cur = items.begin();
		_end = items.end();
		char t[15];
		int r;
		for(r = 0; cur != _end; cur++, r++) {
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
		_log(type, "%sList: %d elements", pfx, items.size());
		std::vector<PyRep *>::const_iterator cur, _end;
		cur = items.begin();
		_end = items.end();
		char t[15];
		int r;
		for(r = 0; cur != _end; cur++, r++) {
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
	int r;
	fprintf(into, "%sDictionary: %d entries\n", pfx, items.size());
	for(r = 0; cur != _end; cur++, r++) {
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
	_log(type, "%sDictionary: %d entries", pfx, items.size());
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

PyRepObject::~PyRepObject() {
	delete arguments;
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

PyRepSubStruct::~PyRepSubStruct() {
	delete sub;
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

PyRepSubStream::PyRepSubStream(const byte *buffer, uint32 len)
: PyRep(PyRep::SubStream),
  length(len),
  data(new byte[len]),
  decoded(NULL)
{
	memcpy(data, buffer, length);
}

PyRepSubStream::~PyRepSubStream() {
	delete[] data;
	delete decoded;
}

void PyRepSubStream::Dump(FILE *into, const char *pfx) const {
	if(decoded == NULL) {
		//we have not decoded this substream, leave it as hex:
		if(data == NULL) {
			fprintf(into, "%sINVALID Substream: no data (length %ld)\n", pfx, length);
		} else {
			fprintf(into, "%sSubstream: length %ld\n", pfx, length);
			std::string m(pfx);
			m += "  ";
			pfxPreviewHexDump(m.c_str(), into, data, length);
		}
	} else {
		fprintf(into, "%sSubstream: length %ld %s\n", pfx, length, (data==NULL)?"from rep":"from data");
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
			_log(type, "%sINVALID Substream: no data (length %d)", pfx, length);
		} else {
			_log(type, "%sSubstream: length %d", pfx, length);
			std::string m(pfx);
			m += "  ";
			pfxPreviewHexDump(m.c_str(), type, data, length);
		}
	} else {
		_log(type, "%sSubstream: length %d %s", pfx, length, (data==NULL)?"from rep":"from data");
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
	data = MarshalOnly(this, length);
}

void PyRepSubStream::DecodeData() const {
	if(decoded != NULL)
		return;
	decoded = InflateAndUnmarshal(data, length);
}


PyRepChecksumedStream::~PyRepChecksumedStream() {
	delete stream;
}

void PyRepChecksumedStream::Dump(FILE *into, const char *pfx) const {
	fprintf(into, "%sStream With Checksum: 0x%08lx\n", pfx, checksum);
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

void PyRepPackedRowHeader::visit(PyVisitor *v) const {
	v->VisitPackedRowHeader(this);
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





PyRepPackedRow::PyRepPackedRow(const byte *buffer, uint32 length, bool own_header, const PyRep *header)
: PyRep(PyRep::PackedRow),
  m_value(new byte[length]),
  m_length(length),
  m_ownsHeader(own_header),
  m_header(header)
{
	memcpy(m_value, buffer, length);
}

PyRepPackedRow::~PyRepPackedRow() {
	delete[] m_value;
	if(m_ownsHeader)
		delete m_header;
}


void PyRepPackedRow::Dump(FILE *into, const char *pfx) const {
	fprintf(into, "%sPacked Row of length %ld (owned header? %s)\n", pfx, m_length, m_ownsHeader?"yes":"no");
	string p(pfx);
	p += "  ";
	pfxPreviewHexDump(p.c_str(), into, m_value, m_length);
}

void PyRepPackedRow::Dump(LogType ltype, const char *pfx) const {
	_log(ltype, "%sPacked Row of length %d (owned header? %s)", pfx, m_length, m_ownsHeader?"yes":"no");
	string p(pfx);
	p += "  ";
	pfxPreviewHexDump(p.c_str(), ltype, m_value, m_length);
}

PyRepPackedRow *PyRepPackedRow::TypedClone() const {
	return(new PyRepPackedRow(m_value, m_length, m_ownsHeader, 
		m_ownsHeader?m_header->Clone():m_header));
}

void PyRepPackedRow::visit(PyVisitor *v) const {
	v->VisitPacked(this);
}





PyRepPackedRowHeader::~PyRepPackedRowHeader() {
	delete header_type;
	delete arguments;
	clear();
}

PyRepPackedRowHeader *PyRepPackedRowHeader::TypedClone() const {
	PyRepPackedRowHeader *rh = new PyRepPackedRowHeader( format, header_type->Clone(), arguments->Clone() );

	storage_type::const_iterator cur, _end;
	
	cur = rows.begin();
	_end = rows.end();
	for(; cur != _end; cur++) {
		rh->rows.push_back((*cur)->TypedClone());
	}
	
	return(rh);
}

void PyRepPackedRowHeader::clear() {
	storage_type::iterator cur, _end;
	
	cur = rows.begin();
	_end = rows.end();
	for(; cur != _end; cur++)
		delete *cur;
	rows.clear();
}

void PyRepPackedRowHeader::Dump(FILE *into, const char *pfx) const {
	const char *type_string = "UNKNOWN TYPE";
	switch(format) {
	case RowList: type_string = "dbutil.RowList"; break;
	case RowDict: type_string = "dbutil.RowDict"; break;
	}
	
	fprintf(into, "%sPacked Row Header: (data format=%s)\n", pfx, type_string);
	std::string n(pfx);
	n += "  Type: ";
	header_type->Dump(into, n.c_str());
	
	std::string m(pfx);
	m += "  Args: ";
	arguments->Dump(into, m.c_str());
	
	if(rows.empty())
		fprintf(into, "%sRows: Empty\n", pfx);
	else {
		fprintf(into, "%sRows: %d elements\n", pfx, rows.size());
		storage_type::const_iterator cur, _end;
		cur = rows.begin();
		_end = rows.end();
		char t[15];
		int r;
		for(r = 0; cur != _end; cur++, r++) {
			n = pfx;
			snprintf(t, 14, "  [%2d] ", r);
			n += t;
			(*cur)->Dump(into, n.c_str());
		}
	}
}

void PyRepPackedRowHeader::Dump(LogType ltype, const char *pfx) const {
	//extra check to avoid potentially a lot of work if we are disabled
	if(!is_log_enabled(ltype))
		return;
	
	const char *format_string = "UNKNOWN TYPE";
	switch(format) {
	case RowList: format_string = "dbutil.RowList"; break;
	case RowDict: format_string = "dbutil.RowDict"; break;
	}
	
	_log(ltype, "%sPacked Row Header: (data format=%s)", pfx, format_string);
	std::string n(pfx);
	n += "  Type: ";
	header_type->Dump(ltype, n.c_str());
	
	std::string m(pfx);
	m += "  Args: ";
	arguments->Dump(ltype, m.c_str());
	
	if(rows.empty()) {
		_log(ltype, "%sRows: Empty", pfx);
	} else {
		_log(ltype, "%sRows: %d elements", pfx, rows.size());
		storage_type::const_iterator cur, _end;
		cur = rows.begin();
		_end = rows.end();
		char t[15];
		int r;
		for(r = 0; cur != _end; cur++, r++) {
			n = pfx;
			snprintf(t, 14, "  [%2d] ", r);
			n += t;
			(*cur)->Dump(ltype, n.c_str());
		}
	}
}







PyRepPackedResultSet::~PyRepPackedResultSet() {
	delete header;
	clear();
}

void PyRepPackedResultSet::clear() {
	storage_type::iterator cur, _end;
	cur = begin();
	_end = end();
	for(; cur != _end; cur++)
		delete *cur;
	rows.clear();
}

void PyRepPackedResultSet::Dump(FILE *into, const char *pfx) const {
	
	const char *format_string = "UNKNOWN TYPE";
	switch(format) {
	case RowList: format_string = "dbutil.RowList"; break;
	case RowDict: format_string = "dbutil.RowDict"; break;
	}
	
	fprintf(into, "%sPacked Result Set: (data format=%s)\n", pfx, format_string);
	
	std::string n(pfx);
	n += " Header: ";
	header->Dump(into, n.c_str());
	
	if(rows.empty())
		fprintf(into, "%s  Rows: Empty\n", pfx);
	else {
		fprintf(into, "%s  Rows: %d elements\n", pfx, rows.size());
		storage_type::const_iterator cur, _end;
		cur = begin();
		_end = end();
		char t[15];
		int r;
		for(r = 0; cur != _end; cur++, r++) {
			n = pfx;
			snprintf(t, 14, "    [%2d] ", r);
			n += t;
			(*cur)->Dump(into, n.c_str());
		}
	}
}

void PyRepPackedResultSet::Dump(LogType type, const char *pfx) const {
	//extra check to avoid potentially a lot of work if we are disabled
	if(!is_log_enabled(type))
		return;
	
	const char *format_string = "UNKNOWN TYPE";
	switch(format) {
	case RowList: format_string = "dbutil.RowList"; break;
	case RowDict: format_string = "dbutil.RowDict"; break;
	}
	
	_log(type, "%sPacked Result Set: (data format=%s)", pfx, format_string);
	
	std::string n(pfx);
	n += " Header: ";
	header->Dump(type, n.c_str());
	
	if(rows.empty()) {
		_log(type, "%s  Rows: Empty", pfx);
	} else {
		_log(type, "%s  Rows: %d elements", pfx, rows.size());
		storage_type::const_iterator cur, _end;
		cur = begin();
		_end = end();
		char t[15];
		int r;
		for(r = 0; cur != _end; cur++, r++) {
			n = pfx;
			snprintf(t, 14, "    [%2d] ", r);
			n += t;
			(*cur)->Dump(type, n.c_str());
		}
	}
}

PyRepPackedResultSet *PyRepPackedResultSet::TypedClone() const {
	PyRepPackedResultSet *rs = new PyRepPackedResultSet(format, header->Clone());
	storage_type::const_iterator cur, _end;
	cur = begin();
	_end = end();
	for(; cur != _end; cur++) {
		rs->rows.push_back((*cur)->TypedClone());
	}
	return(rs);
}

void PyRepPackedResultSet::visit(PyVisitor *v) const {
	v->VisitPackedResultSet(this);
}









EVEStringTable *PyRepString::s_stringTable = NULL;
bool PyRepString::LoadStringFile(const char *file) {
	if(s_stringTable != NULL)
		delete s_stringTable;
	s_stringTable = new EVEStringTable;
	return(s_stringTable->LoadFile(file));
}

EVEStringTable *PyRepString::GetStringTable() {
	if(s_stringTable == NULL)
		s_stringTable = new EVEStringTable(); //make an empty one.
	return(s_stringTable);
}


bool EVEStringTable::LoadFile(const char *file) {
	m_forwardLookup.clear();
	m_reverseLookup.clear();

	//first line of the file is item 1
	m_forwardLookup.push_back("");	//should never be used.

	FILE *f = fopen(file, "r");
	if(f == NULL)
		return(false);

	uint8 index = 1;
	
	char buf[256];
	while(fgets(buf, 255, f) != NULL) {
		char *e;
		e = strchr(buf, '\r');
		if(e == NULL)
			e = strchr(buf, '\n');
		if(e != NULL)
			*e = '\0';
		m_forwardLookup.push_back(buf);
		m_reverseLookup[buf] = index;
		index++;
	}
	fclose(f);
	return(true);
}

//takes a string as an argument since its going to be converted to
//one anyhow when we give it to the map.
uint8 EVEStringTable::LookupString(const std::string &str) const {
	std::map<std::string, uint8>::const_iterator res;
	res = m_reverseLookup.find(str);
	if(res == m_reverseLookup.end())
		return(None);
	return(res->second);
}

const char *EVEStringTable::LookupIndex(uint8 index) const {
	if(index == None || index >= m_forwardLookup.size())
		return(NULL);
	return(m_forwardLookup[index].c_str());
}


















