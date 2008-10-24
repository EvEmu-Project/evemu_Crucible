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

#include "RowsetReader.h"
#include "../common/PyRep.h"
#include "RowsetToSQL.h"
#include "misc.h"

BaseRowsetReader::base_iterator::base_iterator(const BaseRowsetReader *parent, bool is_end)
: m_end(is_end),
  m_index(0),
  m_baseReader(parent)
{
}

const BaseRowsetReader::base_iterator &RowsetReader::base_iterator::operator++() {
	m_index++;
	if(m_index >= m_baseReader->size())
		m_end = true;
	return(*this);
}

bool BaseRowsetReader::base_iterator::_baseEqual(const base_iterator &other) const {
	if(m_end && other.m_end)
		return(true);	//'end' iterator
	if(m_end || other.m_end)
		return(false);
	return(m_index == other.m_index);
}


uint32 BaseRowsetReader::base_iterator::_find(const char *fieldname) const {
	if(m_end)
		return(InvalidRow);
	std::map<std::string, uint32>::const_iterator res;
	res = m_baseReader->m_fieldMap.find(fieldname);
	if(res == m_baseReader->m_fieldMap.end())
		return(InvalidRow);
	return(res->second);
}

PyRep *BaseRowsetReader::base_iterator::_find(uint32 index) const {
	if(m_end)
		return(NULL);
	if(index == InvalidRow)
		return(NULL);
	
	PyRep *row = m_baseReader->_getRow(m_index);
	if(row == NULL || !row->CheckType(PyRep::List))
		return(NULL);
	
	PyRepList *rowl = (PyRepList *) row;
	if(rowl->items.size() <= index)
		return(NULL);
	
	return(rowl->items[index]);
}

//not using _find to allow for more verbose error messages
const char *BaseRowsetReader::base_iterator::GetString(const char *fieldname) const {
	if(m_end)
		return("INVALID ROW");
	std::map<std::string, uint32>::const_iterator res;
	res = m_baseReader->m_fieldMap.find(fieldname);
	if(res == m_baseReader->m_fieldMap.end())
		return("UNKNOWN COLUMN");
	return(GetString(res->second));
}

//not using _find to allow for more verbose error messages
const char *BaseRowsetReader::base_iterator::GetString(uint32 index) const {
	PyRep *row = m_baseReader->_getRow(m_index);
	if(row == NULL || !row->CheckType(PyRep::List))
		return("NON-LIST ROW");
	PyRepList *rowl = (PyRepList *) row;
	if(rowl->items.size() <= index)
		return("INVALID INDEX");
	PyRep *ele = rowl->items[index];
	if(!ele->CheckType(PyRep::String))
		return("NON-STRING ELEMENT");
	PyRepString *str = (PyRepString *) ele;
	return(str->value.c_str());
}

uint32 BaseRowsetReader::base_iterator::GetInt(uint32 index) const {
	PyRep *ele = _find(index);
	if(ele == NULL || !ele->CheckType(PyRep::Integer))
		return(0x7F000000LL);
	PyRepInteger *e = (PyRepInteger *) ele;
	return(e->value);
}

uint64 BaseRowsetReader::base_iterator::GetInt64(uint32 index) const {
	PyRep *ele = _find(index);
	if(ele == NULL || !ele->CheckType(PyRep::Integer))
		return(0x7F0000007F000000LL);
	PyRepInteger *e = (PyRepInteger *) ele;
	return(e->value);
}

double BaseRowsetReader::base_iterator::GetReal(uint32 index) const {
	PyRep *ele = _find(index);
	if(ele == NULL || !ele->CheckType(PyRep::Real))
		return(-9999999);
	PyRepReal *e = (PyRepReal *) ele;
	return(e->value);
}

bool BaseRowsetReader::base_iterator::IsNone(uint32 index) const {
	PyRep *ele = _find(index);
	return(ele != NULL && ele->CheckType(PyRep::None));
}

PyRep::Type BaseRowsetReader::base_iterator::GetType(uint32 index) const {
	PyRep *ele = _find(index);
	return(ele->youreallyshouldentbeusingthis_GetType());
}

/*void RowsetReader::Dump(LogType type) {
	iterator cur, end;
	cur = begin();
	end = end();
	for(; cur != end; cur++) {
		_log(type, "
	}
}*/

std::string BaseRowsetReader::base_iterator::GetAsString(const char *fieldname) const {
	if(m_end)
		return("INVALID ROW");
	std::map<std::string, uint32>::const_iterator res;
	res = m_baseReader->m_fieldMap.find(fieldname);
	if(res == m_baseReader->m_fieldMap.end())
		return("UNKNOWN COLUMN");
	return(GetAsString(res->second));
}

std::string BaseRowsetReader::base_iterator::GetAsString(uint32 index) const {
	PyRep *row = m_baseReader->_getRow(m_index);
	if(row == NULL || !row->CheckType(PyRep::List))
		return("NON-LIST ROW");
	PyRepList *rowl = (PyRepList *) row;
	if(rowl->items.size() <= index)
		return("INVALID INDEX");
	PyRep *ele = rowl->items[index];
	if(ele->CheckType(PyRep::String)) {
		PyRepString *str = (PyRepString *) ele;
		return(str->value.c_str());
	} else if(ele->CheckType(PyRep::Integer)) {
		PyRepInteger *i = (PyRepInteger *) ele;
		char buf[64];
		snprintf(buf, sizeof(buf), I64u, i->value);
		return(std::string(buf));
	} else if(ele->CheckType(PyRep::Real)) {
		PyRepReal *i = (PyRepReal *) ele;
		char buf[64];
		snprintf(buf, sizeof(buf), "%f", i->value);
		return(std::string(buf));
	} else {
		char buf[64];
		snprintf(buf, sizeof(buf), "(%s)", ele->TypeString());
		return(std::string(buf));
	}
}








RowsetReader::RowsetReader(const util_Rowset *rowset)
: BaseRowsetReader(),
  m_set(rowset)
{
	std::vector<std::string>::const_iterator cur, end;
	cur = rowset->header.begin();
	end = rowset->header.end();
	uint32 index;
	for(index = 0; cur != end; cur++, index++) {
		m_fieldMap[*cur] = index;
	}
}

RowsetReader::iterator RowsetReader::begin() {
	if(m_set->lines.items.empty())
		return(end());
	return(iterator(this, false));
}

size_t RowsetReader::size() const {
	return(m_set->lines.items.size());
}

bool RowsetReader::empty() const {
	return(m_set->lines.items.empty());
}

RowsetReader::iterator::iterator()
: base_iterator(NULL, true),
  m_parent(NULL)
{
}

RowsetReader::iterator::iterator(RowsetReader *parent, bool is_end)
: base_iterator(parent, is_end),
  m_parent(parent)
{
}

bool RowsetReader::iterator::operator==(const iterator &other) {
	if(m_parent != other.m_parent)
		return(false);
	return(_baseEqual(other));
}

bool RowsetReader::iterator::operator!=(const iterator &other) {
	if(m_parent != other.m_parent)
		return(true);
	return(!_baseEqual(other));
}

PyRep *RowsetReader::_getRow(uint32 index) const {
	const PyRepList *l = &m_set->lines;
	if(l->items.size() <= index)	//InvalidRow will be caught here!
		return(NULL);
	return(l->items[index]);
}

uint32 RowsetReader::ColumnCount() const {
	return(m_set->header.size());
}

const char *RowsetReader::ColumnName(uint32 index) const {
	if(index < ColumnCount())
		return(m_set->header[index].c_str());
	return("INVALID COLUMN INDEX");
}











TuplesetReader::TuplesetReader(const util_Tupleset *Tupleset)
: BaseRowsetReader(),
  m_set(Tupleset)
{
	std::vector<std::string>::const_iterator cur, end;
	cur = Tupleset->header.begin();
	end = Tupleset->header.end();
	uint32 index;
	for(index = 0; cur != end; cur++, index++) {
		m_fieldMap[*cur] = index;
	}
}

TuplesetReader::iterator TuplesetReader::begin() {
	if(m_set->lines.items.empty())
		return(end());
	return(iterator(this, false));
}

uint32 TuplesetReader::size() const {
	return(m_set->lines.items.size());
}

bool TuplesetReader::empty() const {
	return(m_set->lines.items.empty());
}

TuplesetReader::iterator::iterator()
: base_iterator(NULL, true),
  m_parent(NULL)
{
}

TuplesetReader::iterator::iterator(TuplesetReader *parent, bool is_end)
: base_iterator(parent, is_end),
  m_parent(parent)
{
}

bool TuplesetReader::iterator::operator==(const iterator &other) {
	if(m_parent != other.m_parent)
		return(false);
	return(_baseEqual(other));
}

bool TuplesetReader::iterator::operator!=(const iterator &other) {
	if(m_parent != other.m_parent)
		return(true);
	return(!_baseEqual(other));
}

PyRep *TuplesetReader::_getRow(uint32 index) const {
	const PyRepList *l = &m_set->lines;
	if(l->items.size() <= index)	//InvalidRow will be caught here!
		return(NULL);
	return(l->items[index]);
}

uint32 TuplesetReader::ColumnCount() const {
	return(m_set->header.size());
}

const char *TuplesetReader::ColumnName(uint32 index) const {
	if(index < ColumnCount())
		return(m_set->header[index].c_str());
	return("INVALID COLUMN INDEX");
}

SetSQLDumper::SetSQLDumper(FILE *f, const char *tablename)
: m_file(f),
  m_tablename(tablename)
{
}

void SetSQLDumper::VisitObject(const PyRepObject *rep) {
	if(rep->type != "util.Rowset") {
		//traverse the object as usual.
		PyVisitor::VisitObject(rep);
		return;
	}

	//we found a friend, decode it
	//this is annoying to have to clone it, but we cannot modify our pointer, and Decode wants to consume it.
	PyRepObject *dup = (PyRepObject *) rep->Clone();
	
	util_Rowset rowset;
	if(!rowset.Decode(&dup)) {
		codelog(COMMON__PYREP, "Unable to load a rowset from the object body!");
		delete dup;
		return;
	}
	delete dup;	//rowset took what it wanted.

	RowsetReader reader(&rowset);
	if(!ReaderToSQL<RowsetReader>(m_tablename.c_str(), NULL, m_file, reader)) {
		codelog(COMMON__PYREP, "Failed to convert rowset to SQL");
		return;
	}
}

void SetSQLDumper::VisitTuple(const PyRepTuple *rep) {

	//first we want to check to see if this could possibly even be a tupleset.

	if(		rep->items.size() != 2
	   ||	!rep->items[0]->CheckType(PyRep::List)
	   ||	!rep->items[1]->CheckType(PyRep::List) ) {
		PyVisitor::VisitTuple(rep);
		return;
	}

	const PyRepList *possible_header = (const PyRepList *) rep->items[0];
	const PyRepList *possible_items = (const PyRepList *) rep->items[1];

	//check each element of the lists to make sure they line up.
	PyRepList::const_iterator cur, end;
	cur = possible_header->begin();
	end = possible_header->end();
	for(; cur != end; cur++) {
		if(! (*cur)->CheckType(PyRep::String)) {
			//nope...
			PyVisitor::VisitTuple(rep);
			return;
		}
	}
	cur = possible_items->begin();
	end = possible_items->end();
	for(; cur != end; cur++) {
		if(! (*cur)->CheckType(PyRep::List)) {
			//nope...
			PyVisitor::VisitTuple(rep);
			return;
		}
		//it would be possible I guess to check each element of each item to make sure
		//it is a terminal type (non-container), but I dont care right now.
	}
	
	//ok, it looks like a tupleset... nothing we can do now but interpret it as one...
	
	//this is annoying to have to clone it, but we cannot modify our pointer, and Decode wants to consume it.
	PyRepTuple *dup = (PyRepTuple *) rep->Clone();
	
	util_Tupleset rowset;
	if(!rowset.Decode(&dup)) {
		codelog(COMMON__PYREP, "Unable to interpret tuple as a tupleset, it may not even be one.");
		delete dup;
		return;
	}
	delete dup;	//util_Tupleset took what it wanted.

	TuplesetReader reader(&rowset);
	if(!ReaderToSQL<TuplesetReader>(m_tablename.c_str(), NULL, m_file, reader)) {
		codelog(COMMON__PYREP, "Failed to convert tupleset to SQL");
		return;
	}
}
















