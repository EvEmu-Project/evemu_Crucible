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

#include "EvemuPCH.h"

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
		return(new PyRepBuffer((const byte *) row.GetText(column_index), row.GetColumnLength(column_index)));
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

PyRepObject *DBResultToIndexRowset(const char *key, DBQueryResult &result) {
	uint32 cc = result.ColumnCount();
	if(cc == 0)
		return(new PyRepObject("util.IndexRowset", new PyRepDict()));
	uint32 r;

	//start building the IndexRowset
	PyRepObject *res = new PyRepObject();
	res->type = "util.IndexRowset";
	PyRepDict *args = new PyRepDict();
	res->arguments = args;

	//RowClass:
	args->add("RowClass", new PyRepString("util.Row", true));
	//idName:
	args->add("idName", new PyRepString(key));
	
	//items:
	PyRepDict *items = new PyRepDict();
	args->add("items", items);
	
	//list off the column names:
	PyRepList *header = new PyRepList();
	args->add("header", header);
	int32 index_id = 0xFFFFFFFF;
	for(r = 0; r < cc; r++) {
		if(strcmp(result.ColumnName(r), key) == 0)
			index_id = r;
		header->add(result.ColumnName(r));
	}
	if(index_id == 0xFFFFFFFF) {
		_log(DATABASE__ERROR, "Failed to find key column '%s' in result for IndexRowset", key);
		//not really the best course of action, but this is a programming error, so they can fix it.
		return(res);
	}
	
	

	//add a line entry for each result row:
	DBResultRow row;
	while(result.GetRow(row)) {
		PyRep *keyv = DBColumnToPyRep(row, index_id);
		
		PyRepList *linedata = new PyRepList();
		for(r = 0; r < cc; r++) {
			linedata->add(DBColumnToPyRep(row, r));
		}

		items->add(keyv, linedata);
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

struct DBPackedColumnInfo {
	uint32 index;
	std::string name;
	DBTYPE type;
};

typedef std::vector<DBPackedColumnInfo> DBPackedColumnList;

//this routine is used to order the fields in a packed row.
static const uint8 DBTypeSizeBucketCount = 5;
static uint8 GetTypeSizeIndex(DBTYPE t) {
	switch(t) {
	case DBTYPE_I8:
	case DBTYPE_R8:
	case DBTYPE_UI8:
	case DBTYPE_CY:
	case DBTYPE_FILETIME:
		return(0);
	case DBTYPE_I4:
	case DBTYPE_UI4:
	case DBTYPE_R4:
		return(1);
	case DBTYPE_I2:
	case DBTYPE_UI2:
		return(2);
	case DBTYPE_I1:
	case DBTYPE_UI1:
	case DBTYPE_BOOL:
		return(3);
	//these follow encoded buffer as PyRep
	case DBTYPE_BYTES:
	case DBTYPE_STR:
	case DBTYPE_WSTR:
		return(4);
	}
	return(0);
}

//returns "their" DBTYPE based on "our" column type
static DBTYPE GetPackedColumnType(DBQueryResult::ColType colType) {
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

//builds PackedColumnList from DBQueryResult
static void GetPackedColumnList(const DBQueryResult &res, DBPackedColumnList &into) {
	into.clear();

	uint32 cc = res.ColumnCount();
	for(uint32 i = 0; i < cc; i++) {
		DBPackedColumnInfo info;
		info.index = i;
		info.name = res.ColumnName(i);
		info.type = GetPackedColumnType(res.ColumnType(i));
		into.push_back(info);
	}
}

//builds PackedColumnList from DBResultRow
static void GetPackedColumnList(const DBResultRow &row, DBPackedColumnList &into) {
	into.clear();

	uint32 cc = row.ColumnCount();
	for(uint32 i = 0; i < cc; i++) {
		DBPackedColumnInfo info;
		info.index = i;
		info.name = row.ColumnName(i);
		info.type = GetPackedColumnType(row.ColumnType(i));
		into.push_back(info);
	}
}

//builds blue.DBRowDescriptor from DBPackedColumnList
static PyRepPackedObject1 *BuildRowDescriptor(const DBPackedColumnList &columns) {
	PyRepPackedObject1 *desc = new PyRepPackedObject1("blue.DBRowDescriptor");
	desc->args = new PyRepTuple(1);
	PyRepTuple *col_list = new PyRepTuple(0);
	desc->args->items[0] = col_list;

	PyRepTuple *col;
	DBPackedColumnList::const_iterator cur, end;
	cur = columns.begin();
	end = columns.end();
	for(; cur != end; cur++) {
		col = new PyRepTuple(2);
		col->items[0] = new PyRepString(cur->name);
		col->items[1] = new PyRepInteger(cur->type);
		col_list->items.push_back(col);
	}

	return(desc);
}

//orders DBPackedColumnList by DBTYPE
//could be prolly done better
static void OrderPackedColumnList(DBPackedColumnList &columns) {
	DBPackedColumnList ordered;
	DBPackedColumnList::iterator cur, end, scur;
	cur = columns.begin();
	end = columns.end();
	for(; cur != end; cur++) {
		scur = ordered.begin();
		for(; scur != ordered.end(); scur++)
			if(GetTypeSizeIndex(cur->type) < GetTypeSizeIndex(scur->type)) {
				scur = ordered.insert(scur, *cur);
				break;
			}
		if(scur == ordered.end())
			ordered.push_back(*cur);
	}
	columns = ordered;
}

//puts value into PackedRow based on DBTYPE
static void EncodePackedField(const DBResultRow &row, const DBPackedColumnInfo &col, PyRepPackedRow &into) {
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
	default: into.PushPyRep(row.IsNull(col.index) ? new PyRepBuffer(0) : new PyRepBuffer((const byte *)row.GetText(col.index), row.GetColumnLength(col.index))); break;
	}
}

//packs row, order is determined by DBPackedColumnList
static PyRepPackedRow *PackRow(const DBResultRow &row, const DBPackedColumnList &columns, bool owns_header, const PyRep *header) {
	PyRepPackedRow *res = new PyRepPackedRow(header, owns_header);
	DBPackedColumnList::const_iterator cur, end;

	cur = columns.begin();
	end = columns.end();
	for(; cur != end; cur++)
		EncodePackedField(row, *cur, *res);

	return(res);
}

/*
class fieldinfo {
public:
	fieldinfo(uint32 i, DBTYPE t) : index(i), type(t) {}
	uint32 index;
	DBTYPE type;
};

static void _packRowList(
	DBQueryResult &result, 
	const DBColumnTypeMap &types, 
	const DBColumnOrdering &ordering, 
	PyRepPackedResultSet::storage_type &rows_into, 
	PyRepPackedRowHeader *rhead
) {
	uint32 cc = result.ColumnCount();
	uint32 r;
	
	rhead->header_type = new PyRepString("blue.DBRowDescriptor", true);
	PyRepTuple *arg_tuple = new PyRepTuple(1);
	rhead->arguments = arg_tuple;
		PyRepTuple *arg_list = new PyRepTuple(cc);
		arg_tuple->items[0] = arg_list;

	_log(DATABASE__PACKED, "Building a packed dbutil.RowList result set:");

	std::vector<uint32> field_ordering_buckets[DBTypeSizeBucketCount];
	std::map<std::string, uint32> field_name_to_index;
	
	_log(DATABASE__PACKED, "   Initial columns:");
	for(r = 0; r < cc; r++) {
		//determine the field type.
		DBTYPE field_type;
		DBColumnTypeMap::const_iterator res = types.find(result.ColumnName(r));
		if(res == types.end()) {
			codelog(DATABASE__ERROR, "Unable to find column '%s' in typemap. Hacking it to a string.", result.ColumnName(r));
			field_type = DBTYPE_STR;
		} else {
			field_type = res->second;
		}
		
		//fill in the PackedRowHeader entry.
		PyRepTuple *pair_tuple = new PyRepTuple(2);
		pair_tuple->items[0] = new PyRepString(result.ColumnName(r));
		pair_tuple->items[1] = new PyRepInteger(field_type);
		arg_list->items[r] = pair_tuple;

		field_name_to_index[result.ColumnName(r)] = r;
		
		//now handle the ordering used to pack each row.
		int8 order = GetTypeSizeIndex(field_type);
		field_ordering_buckets[order].push_back(r);
		
		_log(DATABASE__PACKED, "       [%d] %s (DBTYPE %d, order %d)", r, result.ColumnName(r), field_type, order);
	}

	//we need to pull the first row in order to properly check types.
	DBResultRow row;
	if(!result.GetRow(row)) {
		//no data...
		return;
	}    
	
	//turn the ordering buckets into a strict ordering.
	std::vector<fieldinfo> field_orderings;
	field_orderings.reserve(cc);
	
	if(ordering.empty()) {
		_log(DATABASE__PACKED, "   Trying to auto-determine column ordering:");
		//try to figure out the ordering... this isnt quite right yet,
		//which is why we support the hard coded crap...
		
		uint32 k = 0;
		for(r = 0; r < DBTypeSizeBucketCount; r++) {
			std::vector<uint32>::const_iterator cur, end;
			cur = field_ordering_buckets[r].begin();
			end = field_ordering_buckets[r].end();
			for(; cur != end; cur++) {
				DBTYPE field_type;
				DBColumnTypeMap::const_iterator res = types.find(result.ColumnName(*cur));
				if(res == types.end()) {
					//should never happen...
					codelog(DATABASE__ERROR, "Unable to find column '%s' in typemap. Hacking it to a string.", result.ColumnName(*cur));
					field_type = DBTYPE_STR;
				} else {
					field_type = res->second;
				}
				
				//do a bit of sanity checking on the field types...
				if(!CheckTypeMatch(field_type, row.ColumnType(*cur))) {
					codelog(DATABASE__ERROR, "Column type mismatch: column '%s' has DBTYPE %d but mysql type %d.", result.ColumnName(*cur), field_type, row.ColumnType(*cur));
					//may as well let them go...
				}
				field_orderings.push_back(
					fieldinfo(*cur, field_type)
					);
				_log(DATABASE__PACKED, "       [%d] %s (col %d, DBTYPE %d)", k, result.ColumnName(*cur), *cur, field_type);
				k++;
			}
		}
	} else {
		_log(DATABASE__PACKED, "   Using pre-determined column ordering:");
		DBColumnOrdering::const_iterator curo, endo;
		std::map<std::string, uint32>::const_iterator ci;
		curo = ordering.begin();
		endo = ordering.end();
		for(; curo != endo; ++curo) {
			ci = field_name_to_index.find(*curo);
			if(ci == field_name_to_index.end()) {
				codelog(DATABASE__ERROR, "Unable to find column '%s' in ordering.", curo->c_str());
				return;
			}
			
			DBTYPE field_type;
			DBColumnTypeMap::const_iterator res = types.find(curo->c_str());
			if(res == types.end()) {
				//should never happen...
				codelog(DATABASE__ERROR, "Unable to find column '%s' in typemap. Hacking it to a string.", curo->c_str());
				field_type = DBTYPE_STR;
			} else {
				field_type = res->second;
			}
			
			field_orderings.push_back(
				fieldinfo(ci->second, field_type)
				);
		}
	}

	//now start packing in the data...
	std::vector<fieldinfo>::const_iterator curf, endf;

	//we got the first row above...
	std::vector<byte> encoded_data;
	encoded_data.reserve(cc * sizeof(uint32));	//something reasonable...
	do {
		encoded_data.clear();
		
		curf = field_orderings.begin();
		endf = field_orderings.end();
		for(; curf != endf; curf++) {
			EncodePackedField(row, curf->index, curf->type, encoded_data);
		}
		
		_log(DATABASE__PACKED, "Resulting Packed Row:");
		_hex(DATABASE__PACKED, &encoded_data[0], encoded_data.size());
		
		//NOTE: apparently, they chop off any trailing zeros on this stream,
		//and the client will just pad with as many zeros as needed...
		rows_into.push_back(
			new PyRepPackedRow(
				&encoded_data[0], encoded_data.size(), 
				true, rhead->Clone()
				)
			);
	} while (result.GetRow(row));
}*/

PyRepList *DBResultToPackedRowList(
	DBQueryResult &result
) {
	DBPackedColumnList columns;
	GetPackedColumnList(result, columns);

	PyRepPackedObject1 *header = BuildRowDescriptor(columns);

	OrderPackedColumnList(columns);

	PyRepList *res = new PyRepList;

	DBResultRow row;
	while(result.GetRow(row))
		//this is piece of crap due to header cloning
		res->add(PackRow(row, columns, true, header->Clone()));

	delete header;
	return(res);
}

PyRepTuple *DBResultToPackedRowListTuple(
	DBQueryResult &result
) {
	DBPackedColumnList columns;
	GetPackedColumnList(result, columns);

	PyRepPackedObject1 *header = BuildRowDescriptor(columns);

	PyRepTuple *res = new PyRepTuple(2);
	res->items[0] = header->Clone();
	PyRepList *rowlist = new PyRepList;
	res->items[1] = rowlist;

	OrderPackedColumnList(columns);

	DBResultRow row;
	while(result.GetRow(row))
		//this is piece of crap due to header cloning
		rowlist->add(PackRow(row, columns, true, header->Clone()));

	delete header;
	return(res);
}

PyRepPackedObject2 *DBResultToPackedRowset(
	DBQueryResult &result,
	const char *type
) {
	DBPackedColumnList columns;
	GetPackedColumnList(result, columns);

	PyRepPackedObject1 *header = BuildRowDescriptor(columns);

	PyRepPackedObject2 *res = new PyRepPackedObject2(type);
	PyRepDict *d = new PyRepDict;
	res->args2 = d;

	d->add("header", header->Clone());

	PyRepList *col_list = new PyRepList;
	d->add("columns", col_list);

	DBPackedColumnList::const_iterator cur, end;
	cur = columns.begin();
	end = columns.end();
	for(; cur != end; cur++)
		col_list->add(new PyRepString(cur->name));

	OrderPackedColumnList(columns);

	DBResultRow row;
	while(result.GetRow(row))
		//this is piece of crap due to header cloning
		res->list_data.push_back(PackRow(row, columns, true, header->Clone()));

	delete header;
	return(res);
}

PyRepPackedRow *DBRowToPackedRow(
	DBResultRow &row
) {
	DBPackedColumnList columns;
	GetPackedColumnList(row, columns);

	PyRepPackedObject1 *header = BuildRowDescriptor(columns);

	OrderPackedColumnList(columns);

	return(PackRow(row, columns, true, header));
}







