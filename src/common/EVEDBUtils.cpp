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
#include "EVEDBUtils.h"
#include "PyRep.h"
#include "dbcore.h"

#include "../packets/General.h"

//this is such crap
/*StringContentsType ClassifyStringContents(const char *str) {
	if(str == NULL || *str == '\0')
		return(StringContentsUnknown);

	if(*str == '-')
		str++;
	
	bool seen_dot = false;
	bool seen_num = false;
	for(; *str != '\0'; str++) {
		if(seen_num && *str == '.')
			seen_dot = true;
		else if(*str >= '0' && *str <= '9')
			seen_num = true;
		else
			return(StringContentsString);
	}

	if(seen_num) {
		if(seen_dot)
			return(StringContentsReal);
		return(StringContentsInteger);
	}
	return(StringContentsString);
}*/

PyRep *DBColumnToPyRep(DBResultRow &row, uint32 column_index) {
	if(row.IsNull(column_index)) {
		return(new PyRepNone());
	}
	
	switch(row.ColumnType(column_index)) {
	case DBQueryResult::Real:
		//quick hack to help out some queries which have mixed mode fields...
		//its a bit of overhead, but it makes things happier. I suspect that we will
		//need to make this more robust some day to avoid unexpected consequences with 
		//queries which for some reason rely on whole numbers being reals instead of 
		//integers in the client.
		if(strchr(row.GetText(column_index), '.') != NULL)
			return(new PyRepReal(row.GetDouble(column_index)));
        //else, FALLTHROUGH
	case DBQueryResult::Int8:
	case DBQueryResult::Int16:
	case DBQueryResult::Int32:
	case DBQueryResult::Int64:
		//quick check to handle sign, im not sure it does any good at this point.
		if(row.IsSigned(column_index))
			return(new PyRepInteger(row.GetInt64(column_index)));
		return(new PyRepInteger(row.GetUInt64(column_index)));
	case DBQueryResult::Binary:
		return(new PyRepBuffer((const uint8 *) row.GetText(column_index), row.GetColumnLength(column_index)));
	case DBQueryResult::DateTime:
	case DBQueryResult::String:
	default:
		return(new PyRepString(row.GetText(column_index)));
	}
	//unreachable:
	return(new PyRepNone());
}

PyRepObject *DBResultToRowset(DBQueryResult &result) {
	uint32 cc = result.ColumnCount();
	if(cc == 0)
		return(new PyRepObject("util.Rowset", new PyRepDict()));
	uint32 r;

	PyRepObject *res = new PyRepObject();
	res->type = "util.Rowset";
	PyRepDict *args = new PyRepDict();
	res->arguments = args;
	
	//list off the column names:
	PyRepList *header = new PyRepList();
	args->add("header", header);
	for(r = 0; r < cc; r++) {
		header->add(result.ColumnName(r));
	}
	
	//RowClass:
	args->add("RowClass", new PyRepString("util.Row", true));
	
	//lines:
	PyRepList *rowlist = new PyRepList();
	args->add("lines", rowlist);

	//add a line entry for each result row:
	DBResultRow row;
	while(result.GetRow(row)) {
		PyRepList *linedata = new PyRepList();
		rowlist->items.push_back(linedata);
		for(r = 0; r < cc; r++) {
			linedata->add(DBColumnToPyRep(row, r));
		}
	}

	return(res);
}



PyRepTuple *DBResultToTupleSet(DBQueryResult &result) {
	uint32 cc = result.ColumnCount();
	if(cc == 0)
		return(new PyRepTuple(0));
	uint32 r;

	PyRepTuple *res = new PyRepTuple(2);
	PyRepList *cols = new PyRepList();
	PyRepList *reslist = new PyRepList();
	res->items[0] = cols;
	res->items[1] = reslist;
	
	//list off the column names:
	for(r = 0; r < cc; r++) {
		cols->add(result.ColumnName(r));
	}

	//add a line entry for each result row:
	DBResultRow row;
	while(result.GetRow(row)) {
		PyRepList *linedata = new PyRepList();
		reslist->items.push_back(linedata);
		for(r = 0; r < cc; r++) {
			linedata->add(DBColumnToPyRep(row, r));
		}
	}

	return(res);
}

PyRepObject *DBResultToIndexRowset(DBQueryResult &result, const char *key) {
	uint32 cc = result.ColumnCount();
	uint32 key_index;
	for(key_index = 0; key_index < cc; key_index++)
		if(strcmp(key, result.ColumnName(key_index)) == 0)
			break;

	if(key_index == cc) {
		_log(DATABASE__ERROR, "Failed to find key column '%s' in result for IndexRowset", key);
		return(NULL);
	}

	return DBResultToIndexRowset(result, key_index);
}

PyRepObject *DBResultToIndexRowset(DBQueryResult &result, uint32 key_index) {
	uint32 cc = result.ColumnCount();
	if(cc == 0 || cc < key_index)
		return(new PyRepObject("util.IndexRowset", new PyRepDict()));

	//start building the IndexRowset
	PyRepObject *res = new PyRepObject();
	res->type = "util.IndexRowset";
	PyRepDict *args = new PyRepDict();
	res->arguments = args;

	//list off the column names:
	PyRepList *header = new PyRepList();
	args->add("header", header);
	for(uint32 i = 0; i < cc; i++)
		header->add(result.ColumnName(i));

	//RowClass:
	args->add("RowClass", new PyRepString("util.Row", true));
	//idName:
	args->add("idName", new PyRepString(result.ColumnName(key_index)));
	
	//items:
	PyRepDict *items = new PyRepDict();
	args->add("items", items);
	
	//add a line entry for each result row:
	DBResultRow row;
	while(result.GetRow(row)) {
		PyRep *key = DBColumnToPyRep(row, key_index);
		
		PyRepList *line = new PyRepList();
		for(uint32 i = 0; i < cc; i++)
			line->add(DBColumnToPyRep(row, i));

		items->add(key, line);
	}

	return(res);
}





PyRepObject *DBRowToKeyVal(DBResultRow &row) {

	PyRepObject *res = new PyRepObject();
	res->type = "util.KeyVal";
	PyRepDict *args = new PyRepDict();
	res->arguments = args;
	
	uint32 cc = row.ColumnCount();
	uint32 r;
	for(r = 0; r < cc; r++) {
		args->add(
			new PyRepString(row.ColumnName(r)),
			// =>
			DBColumnToPyRep(row, r)
			);
	}

	return(res);
}

PyRepObject *DBRowToRow(DBResultRow &row, const char *type) {
	PyRepObject *res = new PyRepObject();
	res->type = type;
	PyRepDict *args = new PyRepDict();
	res->arguments = args;
	
	//list off the column names:
	PyRepList *header = new PyRepList();
	args->add("header", header);
	uint32 cc = row.ColumnCount();
	uint32 r;
	for(r = 0; r < cc; r++) {
		header->add(row.ColumnName(r));
	}
	
	//lines:
	PyRepList *rowlist = new PyRepList();
	args->add("line", rowlist);

	//add a line entry for the row:
	for(r = 0; r < cc; r++) {
		rowlist->add(DBColumnToPyRep(row, r));
	}

	return(res);
}

PyRepTuple *DBResultToRowList(DBQueryResult &result, const char *type) {
	uint32 cc = result.ColumnCount();
	if(cc == 0)
		return(new PyRepTuple(0));
	uint32 r;

	PyRepTuple *res = new PyRepTuple(2);
	PyRepList *cols = new PyRepList();
	PyRepList *reslist = new PyRepList();
	res->items[0] = cols;
	res->items[1] = reslist;
	
	//list off the column names:
	for(r = 0; r < cc; r++) {
		cols->add(result.ColumnName(r));
	}

	//add a line entry for each result row:
	DBResultRow row;
	while(result.GetRow(row)) {
		//this could be more effecient by not building the column list each time, but cloning it instead.
		PyRepObject *o = DBRowToRow(row, type);
		reslist->items.push_back(o);
	}

	return(res);
}

PyRepDict *DBResultToIntRowDict(DBQueryResult &result, uint32 key_index, const char *type) {
	PyRepDict *res = new PyRepDict();

	//add a line entry for each result row:
	DBResultRow row;
	while(result.GetRow(row)) {
		//this could be more effecient by not building the column list each time, but cloning it instead.
		PyRepObject *r = DBRowToRow(row, type);
		int32 k = row.GetInt(key_index);
		if(k == 0)
			continue;	//likely a non-integer key
		res->items[
			new PyRepInteger(k)
		] = r;
	}

	return(res);
}

PyRepDict *DBResultToIntIntDict(DBQueryResult &result) {
	PyRepDict *res = new PyRepDict();

	//add a line entry for each result row:
	DBResultRow row;
	while(result.GetRow(row)) {
		if(row.IsNull(0))
			continue;	//no working with NULL keys...
		int32 k = row.GetInt(0);
		if(k == 0)
			continue;	//likely a non-integer key
		int32 v;
		if(row.IsNull(1))
			v = 0;		//we can deal with assuming NULL == 0
		else
			v = row.GetInt(1);
		
		res->items[
			new PyRepInteger(k)
		] = new PyRepInteger(v);
	}

	return(res);
}

void DBResultToIntIntDict(DBQueryResult &result, std::map<uint32, uint32> &into) {
	//add a line entry for each result row:
	DBResultRow row;
	while(result.GetRow(row)) {
		if(row.IsNull(0))
			continue;	//no working with NULL keys...
		uint32 k = row.GetUInt(0);
		int32 v;
		if(row.IsNull(1))
			v = 0;		//we can deal with assuming NULL == 0
		else
			v = row.GetInt(1);
		into[k] = v;
	}
}

void DBResultToIntIntlistDict(DBQueryResult &result, std::map<uint32, PyRep *> &into) {
    /* this builds a map from the int in result[0], to a list of each result[1]
     * which is has the same result[0]. This function assumes the result is
     * ORDER BY result[0]
     */
	uint32 last_key = 0xFFFFFFFF;

	PyRepList *l = NULL;
	
	DBResultRow row;
	while(result.GetRow(row)) {
		uint32 k = row.GetUInt(0);
		if(k != last_key || l == NULL) {
			if(l != NULL) {
				//watch for overwrite, no garuntee we are dealing with a key.
				std::map<uint32, PyRep *>::iterator res;
				res = into.find(k);
				if(res != into.end()) {
					//log an error or warning?
					delete res->second;
				}
					
				into[k] = l;
			}
			l = new PyRepList();
			last_key = k;
		}
		uint32 v = row.GetInt(1);
		l->add(new PyRepInteger(v));
	}
}

//returns "their" DBTYPE based on "our" column type
DBTYPE GetPackedColumnType(DBQueryResult::ColType colType) {
	switch(colType) {
		case DBQueryResult::Int8:		return(DBTYPE_UI1);
		case DBQueryResult::Int16:		return(DBTYPE_UI2);
		case DBQueryResult::Int32:		return(DBTYPE_UI4);
		case DBQueryResult::Int64:		return(DBTYPE_UI8);
		case DBQueryResult::Real:		return(DBTYPE_R8);
		case DBQueryResult::DateTime:	return(DBTYPE_FILETIME);
		case DBQueryResult::String:		return(DBTYPE_STR);
		case DBQueryResult::Binary:		return(DBTYPE_BYTES);
		default:						return(DBTYPE_STR);	//default to string
	}
}

/*
 * DBPackedColumnList
 */
DBPackedColumnList::DBPackedColumnList(
	const DBQueryResult &res
)
: m_order(orderByIndex)
{
	uint32 cc = res.ColumnCount();
	for(uint32 i = 0; i < cc; i++) {
		ColumnInfo info;
		info.index = i;
		info.name = res.ColumnName(i);
		info.type = GetPackedColumnType(res.ColumnType(i));

		m_columnList.push_back(info);
	}
}

DBPackedColumnList::DBPackedColumnList(
	const DBResultRow &row
)
: m_order(orderByIndex)
{
	uint32 cc = row.ColumnCount();
	for(uint32 i = 0; i < cc; i++) {
		ColumnInfo info;
		info.index = i;
		info.name = row.ColumnName(i);
		info.type = GetPackedColumnType(row.ColumnType(i));

		m_columnList.push_back(info);
	}
}

DBPackedColumnList::DBPackedColumnList(
	const blue_DBRowDescriptor &desc
)
: m_order(orderByIndex)
{
	PyRepTuple::iterator cur, end;
	cur = desc.columns->begin();
	end = desc.columns->end();
	for(uint32 index = 0; cur != end; cur++, index++) {
		if(!(*cur)->IsTuple())
			continue;
		PyRepTuple *col = (PyRepTuple *)(*cur);

		if(	col->items.size() != 2 ||
			!col->items[0]->IsString() ||
			!col->items[1]->IsInteger()
		)
			continue;
		PyRepString *name = (PyRepString *) col->items[0];
		PyRepInteger *type = (PyRepInteger *) col->items[1];

		ColumnInfo info;
		info.index = index;
		info.name = name->value;
		info.type = (DBTYPE)(type->value);

		m_columnList.push_back(info);
	}
}

void DBPackedColumnList::OrderBy(Order o) {
	if(m_order == o)
		return;

	if(o == orderByIndex) {
		//could be probably done better
		std::vector<ColumnInfo> ordered(m_columnList.size());

		iterator cur, end;
		cur = m_columnList.begin();
		end = m_columnList.end();
		for(; cur != end; cur++)
			ordered[cur->index] = *cur;

		m_columnList = ordered;
		m_order = orderByIndex;
	} else if(o == orderByType) {
		//could be probably done better
		std::vector<ColumnInfo> ordered;

		iterator cur, end, scur;
		cur = m_columnList.begin();
		end = m_columnList.end();
		for(; cur != end; cur++) {
			scur = ordered.begin();
			for(; scur != ordered.end(); scur++)
				if(GetTypeSize(cur->type) > GetTypeSize(scur->type)) {
					scur = ordered.insert(scur, *cur);
					break;
				}
			if(scur == ordered.end())
				ordered.push_back(*cur);
		}

		m_columnList = ordered;
		m_order = orderByType;
	}
}

void DBPackedColumnList::Encode(blue_DBRowDescriptor &into) const {
	into.columns = new PyRepTuple(m_columnList.size());

	PyRepTuple *col;
	const_iterator cur, end;
	cur = m_columnList.begin();
	end = m_columnList.end();
	for(size_t i = 0; cur != end; cur++, i++) {
		col = new PyRepTuple(2);
		col->items[0] = new PyRepString(cur->name);
		col->items[1] = new PyRepInteger(cur->type);

		into.columns->items[i] = col;
	}
}

PyRep *DBPackedColumnList::Encode() const {
	blue_DBRowDescriptor desc;
	Encode(desc);
	return(desc.FastEncode());
}

size_t DBPackedColumnList::CountBufferTypes() const {
	const_iterator cur, end;
	cur = m_columnList.begin();
	end = m_columnList.end();
	uint32 c = 0;
	for(; cur != end; cur++) {
		if(IsBufferType(cur->type))
			c++;
	}
	return(c);
}

size_t DBPackedColumnList::CountPyRepTypes() const {
	const_iterator cur, end;
	cur = m_columnList.begin();
	end = m_columnList.end();
	uint32 c = 0;
	for(; cur != end; cur++) {
		if(IsPyRepType(cur->type))
			c++;
	}
	return(c);
}

uint8 GetTypeSize(DBTYPE t) {
	switch(t) {
		case DBTYPE_I8:
		case DBTYPE_R8:
		case DBTYPE_UI8:
		case DBTYPE_CY:
		case DBTYPE_FILETIME:
			return(8);
		case DBTYPE_I4:
		case DBTYPE_UI4:
		case DBTYPE_R4:
			return(4);
		case DBTYPE_I2:
		case DBTYPE_UI2:
			return(2);
		case DBTYPE_I1:
		case DBTYPE_UI1:
		case DBTYPE_BOOL:
			return(1);
		//these follow encoded buffer as PyRep
		case DBTYPE_BYTES:
		case DBTYPE_STR:
		case DBTYPE_WSTR:
			return(0);
	}
	return(0);
}

bool IsBufferType(DBTYPE t) {
	switch(t) {
		case DBTYPE_I8:
		case DBTYPE_R8:
		case DBTYPE_UI8:
		case DBTYPE_CY:
		case DBTYPE_FILETIME:
		case DBTYPE_I4:
		case DBTYPE_UI4:
		case DBTYPE_R4:
		case DBTYPE_I2:
		case DBTYPE_UI2:
		case DBTYPE_I1:
		case DBTYPE_UI1:
		case DBTYPE_BOOL:
			return true;
		case DBTYPE_BYTES:
		case DBTYPE_STR:
		case DBTYPE_WSTR:
			return false;
		default:
			return false;
	}
}

bool IsPyRepType(DBTYPE t) {
	switch(t) {
		case DBTYPE_I8:
		case DBTYPE_R8:
		case DBTYPE_UI8:
		case DBTYPE_CY:
		case DBTYPE_FILETIME:
		case DBTYPE_I4:
		case DBTYPE_UI4:
		case DBTYPE_R4:
		case DBTYPE_I2:
		case DBTYPE_UI2:
		case DBTYPE_I1:
		case DBTYPE_UI1:
		case DBTYPE_BOOL:
			return false;
		case DBTYPE_BYTES:
		case DBTYPE_STR:
		case DBTYPE_WSTR:
			return true;
		default:
			return false;
	}
}

//puts value into PackedRow based on DBTYPE
void EncodePackedField(const DBResultRow &row, const DBPackedColumnList::ColumnInfo &col, PyRepPackedRow &into) {
	switch(col.type) {
	case DBTYPE_I1: into.PushInt8(row.IsNull(col.index) ? 0 : row.GetInt(col.index)); break;
	case DBTYPE_I2: into.PushInt16(row.IsNull(col.index) ? 0 : row.GetInt(col.index)); break;
	case DBTYPE_I4: into.PushInt32(row.IsNull(col.index) ? 0 : row.GetInt(col.index)); break;
	case DBTYPE_I8: into.PushInt64(row.IsNull(col.index) ? 0 : row.GetInt64(col.index)); break;

	case DBTYPE_UI1:
	case DBTYPE_BOOL: into.PushUInt8(row.IsNull(col.index) ? 0 : row.GetUInt(col.index)); break;
	case DBTYPE_UI2: into.PushUInt16(row.IsNull(col.index) ? 0 : row.GetUInt(col.index)); break;
	case DBTYPE_UI4: into.PushUInt32(row.IsNull(col.index) ? 0 : row.GetUInt(col.index)); break;
	case DBTYPE_UI8:
	case DBTYPE_FILETIME: into.PushUInt64(row.IsNull(col.index) ? 0 : row.GetUInt64(col.index)); break;

	//encoded as an integer, multiplied by 10000 to get fractional ISK
	case DBTYPE_CY: into.PushUInt64((row.IsNull(col.index) ? 0.0 : row.GetDouble(col.index))*10000.00); break;

	case DBTYPE_R4: into.PushFloat(row.IsNull(col.index) ? 0.0 : row.GetFloat(col.index)); break;
	case DBTYPE_R8: into.PushDouble(row.IsNull(col.index) ? 0.0 : row.GetDouble(col.index)); break;
		
	case DBTYPE_STR:
	case DBTYPE_WSTR: into.PushPyRep(new PyRepString(row.IsNull(col.index) ? "" : row.GetText(col.index))); break;

	case DBTYPE_BYTES:
	default: into.PushPyRep(row.IsNull(col.index) ? new PyRepBuffer(0) : new PyRepBuffer((const uint8 *)row.GetText(col.index), row.GetColumnLength(col.index))); break;
	}
}

//packs row, order is determined by DBPackedColumnList
PyRepPackedRow *PackRow(const DBResultRow &row, const DBPackedColumnList &columns, bool owns_header, const PyRep *header) {
	PyRepPackedRow *res = new PyRepPackedRow(header, owns_header);
	DBPackedColumnList::const_iterator cur, end;

	cur = columns.begin();
	end = columns.end();
	for(; cur != end; cur++)
		EncodePackedField(row, *cur, *res);

	return(res);
}

PyRepList *DBResultToPackedRowList(
	DBQueryResult &result
) {
	DBPackedColumnList columns(result);

	blue_DBRowDescriptor desc;
	columns.Encode(desc);

	columns.OrderBy(DBPackedColumnList::orderByType);

	PyRepList *res = new PyRepList;

	DBResultRow row;
	while(result.GetRow(row))
		//this is piece of crap due to header cloning
		res->add(PackRow(row, columns, true, desc.Encode()));

	return(res);
}

PyRepTuple *DBResultToPackedRowListTuple(
	DBQueryResult &result
) {
	DBPackedColumnList columns(result);

	dbutil_RowListTuple res;
	columns.Encode(res.header);

	columns.OrderBy(DBPackedColumnList::orderByType);

	DBResultRow row;
	while(result.GetRow(row))
		//this is piece of crap due to header cloning
		res.rows.add(PackRow(row, columns, true, res.header.Encode()));

	return(res.FastEncode());
}

PyRepNewObject *DBResultToCRowset(
	DBQueryResult &result
) {
	DBPackedColumnList columns(result);

	dbutil_CRowset res;
	columns.Encode(res.header);

	uint32 cc = result.ColumnCount();
	for(uint32 i = 0; i < cc; i++)
		res.columns.push_back(result.ColumnName(i));

	columns.OrderBy(DBPackedColumnList::orderByType);

	DBResultRow row;
	while(result.GetRow(row))
		//this is piece of crap due to header cloning
		res.root_list.push_back(PackRow(row, columns, true, res.header.Encode()));

	return(res.FastEncode());
}

PyRepPackedRow *DBRowToPackedRow(
	DBResultRow &row
) {
	DBPackedColumnList columns(row);

	blue_DBRowDescriptor desc;
	columns.Encode(desc);

	columns.OrderBy(DBPackedColumnList::orderByType);

	return(PackRow(row, columns, true, desc.FastEncode()));
}







