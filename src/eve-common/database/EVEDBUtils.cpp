/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Updates:    Allan
*/

#include "eve-common.h"

#include "database/EVEDBUtils.h"
#include "packets/General.h"
#include "python/classes/PyDatabase.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"


PyRep* DBColumnToPyRep(const DBResultRow& row, uint32 index)
{
    /* check for valid column */
    if (row.IsNull(index))
        return PyStatic.NewNone();      // this doesnt work right.  client still sees this as 0 instead of None

    switch(row.ColumnType(index)) {
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
            return new PyBool( row.GetBool(index) );

        case DBTYPE_STR:
            return new PyString( row.GetText( index ), row.ColumnLength( index ) );

        case DBTYPE_WSTR:
            return new PyWString( row.GetText( index ), row.ColumnLength( index ) );

        case DBTYPE_BYTES: {
            const uint8* data = (const uint8*)row.GetText( index );
            const uint32 len = row.ColumnLength( index );
            return new PyBuffer( data, data + len );
        }

        default: {
            sLog.Error( "DBColumnToPyRep", "invalid column type: %u", row.ColumnType(index) );
            return PyStatic.NewNone();
        }
    }
}

PyObject *DBResultToRowset(DBQueryResult &result)
{
    uint32 cc = result.ColumnCount();

    PyDict *args = new PyDict();

    /* check if we have a empty query result and return a empty RowSet */
    if (cc == 0 )
        return new PyObject( "util.Rowset" , args );

    //list off the column names:
    PyList *header = new PyList( cc );
    for(uint32 r(0); r < cc; ++r)
        header->SetItemString( r, result.ColumnName(r));
    args->SetItemString("header", header);

    //RowClass:
    args->SetItemString("RowClass", new PyToken("util.Row"));

    //lines:
    PyList *rowlist = new PyList();
    //add a line entry for each result row:
    uint32 r(0);
    DBResultRow row;
    while(result.GetRow(row)) {
        PyList *linedata = new PyList( cc );
        for (r = 0; r < cc; ++r)
            linedata->SetItem( r, DBColumnToPyRep(row, r) );
        rowlist->AddItem(linedata);
    }
    args->SetItemString("lines", rowlist);

    return new PyObject( "util.Rowset" , args );
}

PyTuple *DBResultToTupleSet(DBQueryResult &result) {
    uint32 cc = result.ColumnCount();
    if(cc == 0)
        return new PyTuple(0);

    PyTuple *res = new PyTuple(2);

    //list off the column names:
    PyList *cols = new PyList(cc);
    for(uint32 r(0); r < cc; ++r)
        cols->SetItemString(r, result.ColumnName(r));
    res->items[0] = cols;

    //add a line entry for each result row:
    uint32 r(0);
    DBResultRow row;
    PyList *reslist = new PyList();
    while(result.GetRow(row)) {
        PyList *linedata = new PyList(cc);
        for(r = 0; r < cc; ++r)
            linedata->SetItem(r, DBColumnToPyRep(row, r));
        reslist->items.push_back(linedata);
    }
    res->items[1] = reslist;

    return res;
}

/**
 * Service function - for now used particularly as substitution of DBResultToTupleSet in case if we have multiple DB queries
 * and we need to get the values from them all.
 * This function extracts the values and pushes them in provided PyList. Just values, not including columns header.
 * @param result - DBQueryResult object
 * @param into - PyList to dump values into
 */
void populateResListWithValues(DBQueryResult &result, PyList *into) {
    uint32 cc = result.ColumnCount();
    if (cc == 0) {
        return;
    }

    DBResultRow row;
    uint32 r(0);
    while(result.GetRow(row)) {
        PyList *linedata = new PyList(cc);
        for(auto index = 0; index < cc; index++)
            linedata->SetItem(index, DBColumnToPyRep(row, index));
        into->items.push_back(linedata);
    }
}

PyObject *DBResultToIndexRowset(DBQueryResult &result, const char *key) {
    uint32 cc = result.ColumnCount();
    uint32 key_index(0);

    for(key_index = 0; key_index < cc; ++key_index)
        if(strcmp(key, result.ColumnName(key_index)) == 0)
            break;

    if(key_index == cc)
    {
        sLog.Error("EVEDBUtils", "DBResultToIndexRowset | Failed to find key column '%s' in result for IndexRowset", key);
        return nullptr;
    }

    return DBResultToIndexRowset(result, key_index);
}

PyObject *DBResultToIndexRowset(DBQueryResult &result, uint32 key_index) {
    uint32 cc = result.ColumnCount();

    //start building the IndexRowset
    PyDict *args = new PyDict();

    if(cc == 0 || cc < key_index)
        return new PyObject( "util.IndexRowset", args );

    //list off the column names:
    PyList *header = new PyList(cc);
    args->SetItemString("header", header);
    for(uint32 i(0); i < cc; ++i)
        header->SetItemString(i, result.ColumnName(i));

    //RowClass:
    args->SetItemString("RowClass", new PyToken("util.Row"));
    //idName:
    args->SetItemString("idName", new PyString( result.ColumnName(key_index) ));

    //items:
    PyDict *items = new PyDict();
    //add a line entry for each result row:
    uint32 i(0);
    DBResultRow row;
    while(result.GetRow(row)) {
        PyRep *key = DBColumnToPyRep(row, key_index);
        PyList *line = new PyList(cc);
        for(i = 0; i < cc; ++i)
            line->SetItem(i, DBColumnToPyRep(row, i));

        items->SetItem(key, line);
    }

    args->SetItemString("items", items);
    return new PyObject( "util.IndexRowset", args );
}

PyObject *DBRowToKeyVal(DBResultRow &row) {
    PyDict *args = new PyDict();
    uint32 cc = row.ColumnCount();
    for( uint32 r(0); r < cc; ++r )
        args->SetItemString( row.ColumnName(r), DBColumnToPyRep(row, r));

    return new PyObject( "util.KeyVal", args );
}

PyObject *DBRowToRow(DBResultRow &row, const char *type)
{
    PyDict *args = new PyDict();

    //list off the column names:
    uint32 cc = row.ColumnCount();
    PyList *header = new PyList(cc);
    for(uint32 r(0); r < cc; ++r)
        header->SetItemString(r, row.ColumnName(r));

    args->SetItemString("header", header);

    //lines:
    PyList *rowlist = new PyList(cc);
    //add a line entry for the row:
    for(uint32 r(0); r < cc; ++r)
        rowlist->SetItem(r, DBColumnToPyRep(row, r));

    args->SetItemString("line", rowlist);

    return new PyObject( type, args );
}

PyTuple *DBResultToRowList(DBQueryResult &result, const char *type) {
    uint32 cc = result.ColumnCount();
    if(cc == 0)
        return(new PyTuple(0));

    PyList *cols = new PyList(cc);
    //list off the column names:
    for(uint32 r(0); r < cc; ++r)
        cols->SetItemString(r, result.ColumnName(r));

    PyTuple *res = new PyTuple(2);
    res->SetItem( 0, cols );

    //add a line entry for each result row:
    DBResultRow row;
    PyList *reslist = new PyList();
    while(result.GetRow(row)) {
        //this could be more efficient by not building the column list each time, but cloning it instead.
        PyObject *o = DBRowToRow(row, type);
        reslist->items.push_back(o);
    }
    res->SetItem( 1, reslist );

    return res;
}

PyDict *DBResultToIntRowDict(DBQueryResult &result, uint32 key_index, const char *type) {
    PyDict *res = new PyDict();
    //add a line entry for each result row:
    DBResultRow row;
    while (result.GetRow(row)) {
        //this could be more efficient by not building the column list each time, but cloning it instead.
        PyObject *r = DBRowToRow(row, type);
        int32 k = row.GetInt(key_index);
        if (k == 0)
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
        if (row.IsNull(0))
            continue;   //no working with NULL keys...
        int32 k = row.GetInt(0);
        if (k == 0)
            continue;   //likely a non-integer key
        if (row.IsNull(1))
            res->SetItem( new PyInt(k), PyStatic.NewNone() );
        else
            res->SetItem( new PyInt(k), new PyInt(row.GetInt(1)) );
    }

    return res;
}

void FillPackedRow( const DBResultRow& row, PyPackedRow* into )
{
    uint32 cc = row.ColumnCount();
    for( uint32 i(0); i < cc; ++i )
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
    PyList * list = new PyList( result.GetRowCount() );

    uint32 i(0);
    DBResultRow row;
    while( result.GetRow(row) ) {
        list->SetItem( i++, CreatePackedRow( row, header ) );
        PyIncRef( header );
    }

    PyDecRef( header );
    return list;
}

PyTuple *DBResultToPackedRowListTuple( DBQueryResult &result )
{
    DBRowDescriptor * header = new DBRowDescriptor( result );
    PyList * list = new PyList( result.GetRowCount() );

    DBResultRow row;
    uint32 i(0);
    while( result.GetRow(row) ) {
        list->SetItem( i++, CreatePackedRow( row, header ) );
        PyIncRef( header );
    }

    PyTuple* res = new PyTuple(2);
        res->SetItem( 0, header );
        res->SetItem( 1, list );
    return res;
}

PyDict *DBResultToPackedRowDict( DBQueryResult &result, const char *key )
{
    uint32 cc = result.ColumnCount();
    uint32 key_index(0);

    for (key_index = 0; key_index < cc; ++key_index)
        if (strcmp(key, result.ColumnName(key_index)) == 0)
            break;

    if(key_index == cc)
    {
        sLog.Error("EVEDBUtils", "DBResultToPackedRowDict | Failed to find key column '%s' in result for CIndexRowset", key);
        return NULL;
    }

    return DBResultToPackedRowDict(result, key_index);
}

PyDict *DBResultToPackedRowDict( DBQueryResult &result, uint32 key_index )
{
    DBRowDescriptor *header = new DBRowDescriptor( result );

    PyDict *res = new PyDict();

    DBResultRow row;
    while( result.GetRow(row) ) {
        res->SetItem( DBColumnToPyRep(row, key_index), CreatePackedRow( row, header ) );
        PyIncRef( header );
    }

    PyDecRef( header );
    return res;
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
    /** @todo Mem leak.  `header` never freed */
    DBRowDescriptor *header = new DBRowDescriptor( result );
    CRowSet *rowset = new CRowSet( &header );

    DBResultRow row;
    while( result.GetRow( row ) )
    {
        PyPackedRow* into = rowset->NewRow();
        FillPackedRow( row, into );
    }

    //PyDecRef( header );
    return rowset;
}

PyObjectEx *DBResultToCIndexedRowset( DBQueryResult &result, const char *key )
{
    uint32 cc = result.ColumnCount();
    uint32 key_index(0);

    for(key_index = 0; key_index < cc; key_index++)
        if(strcmp(key, result.ColumnName(key_index)) == 0)
            break;

    if(key_index == cc)
    {
        sLog.Error("EVEDBUtils", "DBResultToCIndexedRowset | Failed to find key column '%s' in result for CIndexRowset", key);
        return NULL;
    }

    return DBResultToCIndexedRowset(result, key_index);
}

PyObjectEx *DBResultToCIndexedRowset(DBQueryResult &result, uint32 key_index) {
    /** @todo Mem leak.  `header` never freed */
    DBRowDescriptor *header = new DBRowDescriptor( result );
    CIndexedRowSet *rowset = new CIndexedRowSet( &header );

    DBResultRow row;
    while( result.GetRow( row ) )
    {
        PyPackedRow* into = rowset->NewRow( DBColumnToPyRep(row, key_index) );
        FillPackedRow( row, into );
    }

    //PyDecRef( header );
    return rowset;
}

PyPackedRow *DBRowToPackedRow( DBResultRow &row )
{
    DBRowDescriptor *header = new DBRowDescriptor( row );

    return CreatePackedRow( row, header );
}

/**
 * these functions aren't used.
 */
void DBResultToIntIntDict(DBQueryResult &result, std::map<int32, int32> &into) {
    //add a line entry for each result row:
    DBResultRow row;
    while(result.GetRow(row)) {
        if (row.IsNull(0))
            continue;   //no working with NULL keys...
            int32 k = row.GetInt(0);
        int32 v(0);
        if (row.IsNull(1))
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
        uint32 v(0);
        if(row.IsNull(1))
            v = 0;      //we can deal with assuming NULL == 0
            else
                v = row.GetUInt(1);
            into[k] = v;
    }
}

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
        if (k != last_key )
        {
            //watch for overwrite, no guarantee we are dealing with a key.
            std::map<int32, PyRep *>::iterator res = into.find(k);
            if (res != into.end() )
                //log an error or warning?
                PyDecRef( res->second );

            into[k] = l = new PyList();
            last_key = k;
        }

        l->AddItemInt( row.GetInt( 1 ) );
    }
}
