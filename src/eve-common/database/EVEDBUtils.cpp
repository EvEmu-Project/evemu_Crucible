/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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

#include "EVECommonPCH.h"

#include "database/EVEDBUtils.h"
#include "packets/General.h"
#include "python/classes/PyDatabase.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"

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

PyRep* DBColumnToPyRep( const DBResultRow& row, uint32 index )
{
    /* check for valid column */
    if( row.IsNull( index ) )
        return new PyNone;

    const DBTYPE type = row.ColumnType( index );
    switch( type )
    {
        case DBTYPE_I1:
        case DBTYPE_UI1:
        case DBTYPE_I2:
        case DBTYPE_UI2:
        case DBTYPE_I4:
        case DBTYPE_UI4:
            return new PyInt( row.GetInt( index ) );

        case DBTYPE_I8:
        case DBTYPE_UI8:
            return new PyLong( row.GetInt64( index ) );

        case DBTYPE_R8:
        case DBTYPE_R4:
            return new PyFloat( row.GetDouble( index ) );

        case DBTYPE_BOOL:
        {
            const int32 field_data = row.GetInt( index );
            // safe thingy to make sure we don't fuck things up in the db
            assert( field_data == 0 || field_data == 1 );

            return new PyBool( field_data != 0 );
        }

        case DBTYPE_STR:
            return new PyString( row.GetText( index ), row.ColumnLength( index ) );

        case DBTYPE_WSTR:
            return new PyWString( row.GetText( index ), row.ColumnLength( index ) );

        default:
            sLog.Error( "DBColumnToPyRep", "invalid column type: %u", type );
            /* hack... MAJOR... */

        case DBTYPE_BYTES:
        {
            const uint8* data = (const uint8*)row.GetText( index );
            const uint32 len = row.ColumnLength( index );

            return new PyBuffer( data, data + len );
        }
    }
}

PyObject *DBResultToRowset(DBQueryResult &result)
{
    uint32 r;
    uint32 cc = result.ColumnCount();

    PyDict *args = new PyDict();
    PyObject *res = new PyObject(
        new PyString( "util.Rowset" ), args
    );

    /* check if we have a empty query result and return a empty RowSet */
    if( cc == 0 )
        return res;

    //list off the column names:
    PyList *header = new PyList( cc );
    args->SetItemString("header", header);
    for(r = 0; r < cc; r++) {
        header->SetItemString( r, result.ColumnName(r));
    }

    //RowClass:
    args->SetItemString("RowClass", new PyToken("util.Row"));

    //lines:
    PyList *rowlist = new PyList();
    args->SetItemString("lines", rowlist);

    //add a line entry for each result row:
    DBResultRow row;
    while(result.GetRow(row)) {
        PyList *linedata = new PyList( cc );
        rowlist->AddItem(linedata);

        for(r = 0; r < cc; r++) {
            linedata->SetItem( r, DBColumnToPyRep(row, r) );
        }
    }

    return res;
}

PyTuple *DBResultToTupleSet(DBQueryResult &result) {
    uint32 cc = result.ColumnCount();
    if(cc == 0)
        return new PyTuple(0);
    
    uint32 r;

    PyTuple *res = new PyTuple(2);
    PyList *cols = new PyList(cc);
    PyList *reslist = new PyList();
    res->items[0] = cols;
    res->items[1] = reslist;

    //list off the column names:
    for(r = 0; r < cc; r++) {
        cols->SetItemString(r, result.ColumnName(r));
    }

    //add a line entry for each result row:
    DBResultRow row;
    while(result.GetRow(row)) {
        PyList *linedata = new PyList(cc);
        reslist->items.push_back(linedata);
        for(r = 0; r < cc; r++) {
            linedata->SetItem(r, DBColumnToPyRep(row, r));
        }
    }

    return res;
}

PyObject *DBResultToIndexRowset(DBQueryResult &result, const char *key) {
    uint32 cc = result.ColumnCount();
    uint32 key_index;

    for(key_index = 0; key_index < cc; key_index++)
        if(strcmp(key, result.ColumnName(key_index)) == 0)
            break;

    if(key_index == cc)
    {
        sLog.Error("EVEDBUtils", "DBResultToIndexRowset | Failed to find key column '%s' in result for IndexRowset", key);
        return NULL;
    }

    return DBResultToIndexRowset(result, key_index);
}

PyObject *DBResultToIndexRowset(DBQueryResult &result, uint32 key_index) {
    uint32 cc = result.ColumnCount();

    //start building the IndexRowset
    PyDict *args = new PyDict();
    PyObject *res = new PyObject(
        new PyString( "util.IndexRowset" ), args
    );

    if(cc == 0 || cc < key_index)
        return res;
    
    //list off the column names:
    PyList *header = new PyList(cc);
    args->SetItemString("header", header);
    for(uint32 i = 0; i < cc; i++)
        header->SetItemString(i, result.ColumnName(i));

    //RowClass:
    args->SetItemString("RowClass", new PyToken("util.Row"));
    //idName:
    args->SetItemString("idName", new PyString( result.ColumnName(key_index) ));

    //items:
    PyDict *items = new PyDict();
    args->SetItemString("items", items);

    //add a line entry for each result row:
    DBResultRow row;
    while(result.GetRow(row)) {
        PyRep *key = DBColumnToPyRep(row, key_index);

        PyList *line = new PyList(cc);
        for(uint32 i = 0; i < cc; i++)
            line->SetItem(i, DBColumnToPyRep(row, i));

        items->SetItem(key, line);
    }

    return res;
}

PyObject *DBRowToKeyVal(DBResultRow &row) {

    PyDict *args = new PyDict();
    PyObject *res = new PyObject(
        new PyString( "util.KeyVal" ), args
    );
    
    uint32 cc = row.ColumnCount();
    for( uint32 r = 0; r < cc; r++ )
        args->SetItemString( row.ColumnName(r), DBColumnToPyRep(row, r));

    return res;
}

PyObject *DBRowToRow(DBResultRow &row, const char *type)
{

    PyDict *args = new PyDict();
    PyObject *res = new PyObject(
        new PyString( type ), args
    );

    //list off the column names:
    uint32 cc = row.ColumnCount();
    PyList *header = new PyList(cc);
    args->SetItemString("header", header);

    for(uint32 r = 0; r < cc; r++)
    {
        header->SetItemString(r, row.ColumnName(r));
    }

    //lines:
    PyList *rowlist = new PyList(cc);
    args->SetItemString("line", rowlist);

    //add a line entry for the row:
    for(uint32 r = 0; r < cc; r++)
    {
        rowlist->SetItem(r, DBColumnToPyRep(row, r));
    }

    return res;
}

PyTuple *DBResultToRowList(DBQueryResult &result, const char *type) {
    uint32 cc = result.ColumnCount();
    if(cc == 0)
        return(new PyTuple(0));
    uint32 r;

    PyTuple *res = new PyTuple(2);
    PyList *cols = new PyList(cc);
    PyList *reslist = new PyList();
    res->SetItem( 0, cols );
    res->SetItem( 1, reslist );

    //list off the column names:
    for(r = 0; r < cc; r++) {
        cols->SetItemString(r, result.ColumnName(r));
    }

    //add a line entry for each result row:
    DBResultRow row;
    while(result.GetRow(row)) {
        //this could be more efficient by not building the column list each time, but cloning it instead.
        PyObject *o = DBRowToRow(row, type);
        reslist->items.push_back(o);
    }

    return res;
}

PyDict *DBResultToIntRowDict(DBQueryResult &result, uint32 key_index, const char *type) {
    PyDict *res = new PyDict();

    //add a line entry for each result row:
    DBResultRow row;
    while(result.GetRow(row)) {
        //this could be more efficient by not building the column list each time, but cloning it instead.
        PyObject *r = DBRowToRow(row, type);
        int32 k = row.GetInt(key_index);
        if(k == 0)
            continue;   //likely a non-integer key
        res->SetItem(new PyInt(k), r);
    }

    return res;
}

PyDict *DBResultToIntIntDict(DBQueryResult &result) {
    PyDict *res = new PyDict();

    //add a line entry for each result row:
    DBResultRow row;
    while(result.GetRow(row)) {
        if(row.IsNull(0))
            continue;   //no working with NULL keys...
        int32 k = row.GetInt(0);
        if(k == 0)
            continue;   //likely a non-integer key
        int32 v;
        if(row.IsNull(1))
            v = 0;      //we can deal with assuming NULL == 0
        else
            v = row.GetInt(1);

        res->SetItem( new PyInt(k), new PyInt(v) );
    }

    return res;
}

void DBResultToIntIntDict(DBQueryResult &result, std::map<int32, int32> &into) {
    //add a line entry for each result row:
    DBResultRow row;
    while(result.GetRow(row)) {
        if(row.IsNull(0))
            continue;   //no working with NULL keys...
        uint32 k = row.GetUInt(0);
        int32 v;
        if(row.IsNull(1))
            v = 0;      //we can deal with assuming NULL == 0
        else
            v = row.GetInt(1);
        into[k] = v;
    }
}

void DBResultToUIntUIntDict(DBQueryResult &result, std::map<uint32, uint32> &into) {
    //add a line entry for each result row:
    DBResultRow row;
    while(result.GetRow(row)) {
        if(row.IsNull(0))
            continue;   //no working with NULL keys...
        uint32 k = row.GetUInt(0);
        int32 v;
        if(row.IsNull(1))
            v = 0;      //we can deal with assuming NULL == 0
        else
            v = row.GetInt(1);
        into[k] = v;
    }
}

/**
 * this function isn't used.
 */
void DBResultToIntIntlistDict( DBQueryResult &result, std::map<int32, PyRep *> &into ) {
    /* this builds a map from the int in result[0], to a list of each result[1]
     * which is has the same result[0]. This function assumes the result is
     * ORDER BY result[0]
     */
    uint32 last_key = 0xFFFFFFFF;

    PyList *l = NULL;

    DBResultRow row;
    while( result.GetRow( row ) )
    {
        uint32 k = row.GetUInt(0);
        if( k != last_key )
		{
			//watch for overwrite, no guarantee we are dealing with a key.
			std::map<int32, PyRep *>::iterator res = into.find(k);
			if( res != into.end() )
				//log an error or warning?
				PyDecRef( res->second );

			into[k] = l = new PyList();
			last_key = k;
		}

        l->AddItemInt( row.GetInt( 1 ) );
    }
}

void FillPackedRow( const DBResultRow& row, PyPackedRow* into )
{
    uint32 cc = row.ColumnCount();
    for( uint32 i = 0; i < cc; i++ )
        into->SetField( i, DBColumnToPyRep( row, i ) );
}

PyPackedRow* CreatePackedRow( const DBResultRow& row, DBRowDescriptor* header )
{
    PyPackedRow* res = new PyPackedRow( header );
	FillPackedRow( row, res );
    return res;
}

PyList *DBResultToPackedRowList( DBQueryResult &result )
{
	DBRowDescriptor *header = new DBRowDescriptor( result );

    PyList *res = new PyList( result.GetRowCount() );

    DBResultRow row;
    for( uint32 i = 0; result.GetRow( row ); i++ )
    {
        res->SetItem( i, CreatePackedRow( row, header ) );
        PyIncRef( header );
    }

    PyDecRef( header );
    return res;
}

/* function not used */
PyTuple *DBResultToPackedRowListTuple( DBQueryResult &result )
{
    DBRowDescriptor * header = new DBRowDescriptor( result );

    size_t row_count = result.GetRowCount();
    PyList * list = new PyList( row_count );

	DBResultRow row;
	uint32 i = 0;
    while( result.GetRow(row) )
    {
        list->SetItem( i++, CreatePackedRow( row, header ) );
        PyIncRef( header );
    }

    PyTuple * root = new PyTuple(2);
    root->SetItem( 0, header );
    root->SetItem( 1, list );

    return root;
}

/* Class structure
 * PyClass
 *   PyTuple:2
 *     itr[0]:PyTuple:1
 *       itr[0]:PyClass
 *         PyString:"dbutil.CRowset"
 *     itr[1]:PyDict:1
 *       dict["header"]=PyClass
 *         PyTuple:2
 *           itr[0]:PyClass
 *             PyString:"blue.DBRowDescriptor"
 *           itr[1]:PyTuple:1
 *             itr[0]:PyTuple:N<Amount of cols>
 *               itr[0]:PyTuple:2
 *                 itr[0]:PyString<FieldName>
 *                 itr[1]:PyInt<FieldType> DBTYPE
 *               itr[N]:PyTuple:2
 *                 itr[0]:PyString<FieldName>
 *                 itr[1]:PyInt<FieldType> DBTYPE
 *            PyDict:0
 *            PyList:0
 * PyDict:0
 * PyList:N<Amount of rows>
 *   itr[N]:PyPackedRow
 */

/* this is a very monstrous implementation of a Python Class/Function call
 */
PyObjectEx *DBResultToCRowset( DBQueryResult &result )
{
    DBRowDescriptor *header = new DBRowDescriptor( result );
	CRowSet *rowset = new CRowSet( &header );

	DBResultRow row;
	while( result.GetRow( row ) )
	{
		PyPackedRow* into = rowset->NewRow();
		FillPackedRow( row, into );
	}

	return rowset;
}

PyPackedRow *DBRowToPackedRow( DBResultRow &row )
{
    DBRowDescriptor *header = new DBRowDescriptor( row );

    return CreatePackedRow( row, header );
}
