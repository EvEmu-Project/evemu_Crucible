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

#include "EVEDBUtils.h"
#include "PyRep.h"
#include "dbcore.h"
#include "logsys.h"
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
	case DBQueryResult::Integer:
		//quick hack to handle sign, im not sure it does any good at this point.
		if(row.GetText(column_index)[0] == '-')
			return(new PyRepInteger(row.GetInt64(column_index)));
		return(new PyRepInteger(row.GetUInt64(column_index)));
		break;
	case DBQueryResult::Binary:
		return(new PyRepBuffer((const byte *) row.GetText(column_index), row.GetColumnLength(column_index)));
		break;
	case DBQueryResult::DateTime:
	case DBQueryResult::String:
	default:
		return(new PyRepString(row.GetText(column_index)));
		break;
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
		sint32 k = row.GetInt(key_index);
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
		sint32 k = row.GetInt(0);
		if(k == 0)
			continue;	//likely a non-integer key
		sint32 v;
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
		sint32 v;
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

//this routine is used to order the fields in a packed row.
static const uint8 DBTypeSizeBucketCount = 5;
static int8 GetTypeSizeIndex(DBTYPE t) {
	switch(t) {
	case DBTYPE_I1:
	case DBTYPE_UI1:
	case DBTYPE_BOOL:
		return(4);
	case DBTYPE_I2:
	case DBTYPE_UI2:
		return(3);
	case DBTYPE_I4:
	case DBTYPE_UI4:
	case DBTYPE_R4:
		return(2);
	case DBTYPE_I8:
	case DBTYPE_R8:
	case DBTYPE_UI8:
	case DBTYPE_CY:
	case DBTYPE_FILETIME:
		return(1);
	//not sure where these go in the ordering.
	case DBTYPE_BYTES:
	case DBTYPE_STR:
	case DBTYPE_WSTR:
		return(0);
	}
	return(0);
}

static int8 CheckTypeMatch(DBTYPE t, DBQueryResult::ColType db_type) {
	switch(t) {
	case DBTYPE_I1:
	case DBTYPE_UI1:
	case DBTYPE_BOOL:
	case DBTYPE_I2:
	case DBTYPE_UI2:
	case DBTYPE_I4:
	case DBTYPE_UI4:
	case DBTYPE_I8:
	case DBTYPE_UI8:
	case DBTYPE_FILETIME:
		if(db_type == DBQueryResult::Integer)
			return(true);
		break;
	case DBTYPE_R4:
	case DBTYPE_R8:
	case DBTYPE_CY:
		if(db_type == DBQueryResult::Real || db_type == DBQueryResult::Integer)
			return(true);
		break;
	case DBTYPE_BYTES:
		if(db_type == DBQueryResult::String || db_type == DBQueryResult::Binary)
			return(true);
		break;
	case DBTYPE_STR:
	case DBTYPE_WSTR:
		if(db_type == DBQueryResult::String)
			return(true);
		break;
	}
	return(false);
}

//this could be done a lot better... will not work on big endian systems.
static void EncodeBytes(const void *data_, uint8 len, std::vector<byte> &into) {
	
		/*_log(DATABASE__PACKED, "Next Field (len %d):", len);
		_log(DATABASE__PACKED, "   Input:", len);
		_hex(DATABASE__PACKED, data_, len);
		_log(DATABASE__PACKED, "   Pre-Encode:");
		if(into.empty()) {
			_log(DATABASE__PACKED, "       Empty.");
		} else {
			_hex(DATABASE__PACKED, &into[0], into.size());
		}*/
		
	const byte *data = (const byte *) data_;
	while(len > 0) {
		into.push_back(*data);
		data++;
		len--;
	}
}

static void EncodePackedField(DBResultRow &row, uint32 index, DBTYPE type, std::vector<byte> &into) {
	switch(type) {
	
	case DBTYPE_I1: {
		sint32 v = row.GetInt(index);
		EncodeBytes(&v, sizeof(sint8), into);
		} break;
		
	case DBTYPE_UI1:
	case DBTYPE_BOOL: {
		uint32 v = row.GetUInt(index);
		EncodeBytes(&v, sizeof(uint8), into);
		} break;
		
	case DBTYPE_I2: {
		sint32 v = row.GetInt(index);
		EncodeBytes(&v, sizeof(sint16), into);
		} break;
		
	case DBTYPE_UI2: {
		uint32 v = row.GetUInt(index);
		EncodeBytes(&v, sizeof(uint16), into);
		} break;
		
	case DBTYPE_I4: {
		sint32 v = row.GetInt(index);
		EncodeBytes(&v, sizeof(v), into);
		} break;
		
	case DBTYPE_UI4: {
		uint32 v = row.GetUInt(index);
		EncodeBytes(&v, sizeof(v), into);
		} break;
		
	case DBTYPE_I8: {
		sint64 v = row.GetInt64(index);
//_log(DATABASE__ERROR, "sint64 = " I64d " = 0x " I64x, v, v);
		EncodeBytes(&v, sizeof(v), into);
		} break;
		
	case DBTYPE_UI8:
	case DBTYPE_FILETIME: {
		uint64 v = row.GetUInt64(index);
//_log(DATABASE__ERROR, "sint64 = " I64u " = 0x " I64x, v, v);
		EncodeBytes(&v, sizeof(v), into);
		} break;
		
	case DBTYPE_CY: {
		//encoded as an integer, multiplied by 10000 to get fractional ISK
		uint64 v = uint64(row.GetDouble(index)*10000.00);
//_log(DATABASE__ERROR, "cy = " I64u " = 0x " I64x, v, v);
		EncodeBytes(&v, sizeof(v), into);
		} break;
		
	case DBTYPE_R4: {
		float v = row.GetFloat(index);
		EncodeBytes(&v, sizeof(v), into);
		} break;
		
	case DBTYPE_R8: {
		double v = row.GetDouble(index);
		EncodeBytes(&v, sizeof(v), into);
		} break;
		//actually it looks like CY is sent over as an int64 * 1000
		
	case DBTYPE_BYTES:
	case DBTYPE_STR:
	case DBTYPE_WSTR:
		//no idea how these are coded...
		codelog(DATABASE__ERROR, "Unsupported field type %d", type);
		break;
	}
}


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
}

PyRep *DBResultToPackedRowList(
	DBQueryResult &result, 
	const DBColumnTypeMap &types, 
	const DBColumnOrdering &ordering
) {
	PyRepPackedRowHeader *rhead = new PyRepPackedRowHeader();
	PyRepPackedResultSet *rs = new PyRepPackedResultSet();
	
	_packRowList(result, types, ordering, rs->rows, rhead);
	
	rs->format = PyRepPackedResultSet::RowList;
	dbutil_RowList_header head_coder;
	head_coder.type = "dbutil.RowList";
	head_coder.packed_header = rhead;
	
	uint32 cc = result.ColumnCount();
	uint32 r;
	for(r = 0; r < cc; r++) {
		head_coder.columns.push_back(result.ColumnName(r));
	}
	
	rs->header = head_coder.FastEncode();
	
	return(rs);
}

PyRep *DBResultToPackedRowListTuple(
	DBQueryResult &result, 
	const DBColumnTypeMap &types, 
	const DBColumnOrdering &ordering
) {
	PyRepPackedRowHeader *rhead = new PyRepPackedRowHeader();
	PyRepPackedResultSet::storage_type rows;
	
	_packRowList(result, types, ordering, rows, rhead);

	PyRepTuple *res = new PyRepTuple(2);
	res->items[0] = rhead;
	PyRepList *row_list = new PyRepList();
	res->items[1] = row_list;
	
	//move the rows from the typed row vector into the generic list.
	row_list->items.reserve(rows.size());
	PyRepPackedResultSet::iterator cur, end;
	cur = rows.begin();
	end = rows.end();
	for(; cur != end; ++cur) {
		row_list->items.push_back(*cur);
	}
	rows.clear();
	
	return(res);
}








