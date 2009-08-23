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

#include "EVEDBUtils.h"
#include "PyVisitor.h"
#include "PyRep.h"
#include "LogNew.h"

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

PyRep *DBColumnToPyRep(const DBResultRow &row, uint32 column_index)
{
    /* check for valid colm */
    if(row.IsNull(column_index))
    {
        return new PyNone();
    }

    switch(row.ColumnType(column_index))
    {
        /* hack to handle mixed type fields.
         * we take the overhead for granted
         */
    case DBQueryResult::Real:
        /* check if there is a dot in the field like "100.54" */
        if(strchr(row.GetText(column_index), '.') != NULL)
        {
            return new PyFloat(row.GetDouble(column_index));
        }

        /* fall trough */
    case DBQueryResult::Int8:
    case DBQueryResult::Int16:
    case DBQueryResult::Int32:
        return new PyInt(row.GetInt(column_index));
    case DBQueryResult::Int64:
            return new PyLong(row.GetInt64(column_index));
    case DBQueryResult::Binary:
        return new PyBuffer((const uint8 *) row.GetText(column_index), row.GetColumnLength(column_index));
    case DBQueryResult::Bool:
        {
            int field_data = row.GetInt(column_index);
            // safe thingy to make sure we don't fuck things up in the db
            assert(field_data == 0 || field_data == 1);
            return new PyBool( field_data != 0 );
        }

    case DBQueryResult::DateTime:
    case DBQueryResult::String:
    default:
        return new PyString(row.GetText(column_index));
    }
    //unreachable:
    return new PyNone();
}

PyObject *DBResultToRowset(DBQueryResult &result)
{
    uint32 r;
    uint32 cc = result.ColumnCount();

    PyDict *args = new PyDict();
    PyObject *res = new PyObject("util.Rowset", args);

    /* check if we have a empty query result and return a empty RowSet */
    if( cc == 0 )
        return res;

    //list off the column names:
    PyList *header = new PyList( cc );
    args->add("header", header);
    for(r = 0; r < cc; r++) {
        header->setStr( r, result.ColumnName(r));
    }

    //RowClass:
    args->add("RowClass", new PyString("util.Row", true));

    //lines:
    PyList *rowlist = new PyList();
    args->add("lines", rowlist);

    //add a line entry for each result row:
    DBResultRow row;
    while(result.GetRow(row)) {
        PyList *linedata = new PyList( cc );
        rowlist->add(linedata);
        for(r = 0; r < cc; r++) {
            linedata->set( r, DBColumnToPyRep(row, r) );
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
        cols->setStr(r, result.ColumnName(r));
    }

    //add a line entry for each result row:
    DBResultRow row;
    while(result.GetRow(row)) {
        PyList *linedata = new PyList(cc);
        reslist->items.push_back(linedata);
        for(r = 0; r < cc; r++) {
            linedata->set(r, DBColumnToPyRep(row, r));
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
    PyObject *res = new PyObject("util.IndexRowset", args);

    if(cc == 0 || cc < key_index)
        return res;
    
    //list off the column names:
    PyList *header = new PyList(cc);
    args->add("header", header);
    for(uint32 i = 0; i < cc; i++)
        header->setStr(i, result.ColumnName(i));

    //RowClass:
    args->add("RowClass", new PyString("util.Row", true));
    //idName:
    args->addStr("idName", result.ColumnName(key_index));

    //items:
    PyDict *items = new PyDict();
    args->add("items", items);

    //add a line entry for each result row:
    DBResultRow row;
    while(result.GetRow(row)) {
        PyRep *key = DBColumnToPyRep(row, key_index);

        PyList *line = new PyList(cc);
        for(uint32 i = 0; i < cc; i++)
            line->set(i, DBColumnToPyRep(row, i));

        items->add(key, line);
    }

    return res;
}

PyObject *DBRowToKeyVal(DBResultRow &row) {

    PyDict *args = new PyDict();
    PyObject *res = new PyObject("util.KeyVal", args);
    
    uint32 cc = row.ColumnCount();
    for( uint32 r = 0; r < cc; r++ )
    {
        args->add( row.ColumnName(r), DBColumnToPyRep(row, r));
    }

    return res;
}

PyObject *DBRowToRow(DBResultRow &row, const char *type)
{

    PyDict *args = new PyDict();
    PyObject *res = new PyObject(type, args);

    //list off the column names:
    uint32 cc = row.ColumnCount();
    PyList *header = new PyList(cc);
    args->add("header", header);

    for(uint32 r = 0; r < cc; r++)
    {
        header->setStr(r, row.ColumnName(r));
    }

    //lines:
    PyList *rowlist = new PyList(cc);
    args->add("line", rowlist);

    //add a line entry for the row:
    for(uint32 r = 0; r < cc; r++)
    {
        rowlist->set(r, DBColumnToPyRep(row, r));
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
        cols->setStr(r, result.ColumnName(r));
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
        res->add(new PyInt(k), r);
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

        res->add( new PyInt(k), new PyInt(v) );
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
				SafeDelete( res->second );

			into[k] = l = new PyList();
			last_key = k;
		}

        l->add( new PyInt( row.GetInt( 1 ) ) );
    }
}

DBTYPE GetPackedColumnType( DBQueryResult::ColType colType )
{
    switch( colType )
    {
        case DBQueryResult::Int8:       return DBTYPE_I1;
        case DBQueryResult::Int16:      return DBTYPE_I2;
        case DBQueryResult::Int32:      return DBTYPE_I4;
        case DBQueryResult::Int64:      return DBTYPE_I8;
        case DBQueryResult::Real:       return DBTYPE_R8;
        case DBQueryResult::DateTime:   return DBTYPE_FILETIME;
        case DBQueryResult::String:     return DBTYPE_STR;
        case DBQueryResult::Binary:     return DBTYPE_BYTES;
        case DBQueryResult::Bool:       return DBTYPE_BOOL;
        default:                        return DBTYPE_STR; //default to string
    }
}

PyObjectEx *DBResultToRowDescriptor( const DBQueryResult &res )
{
    uint32 cc = res.ColumnCount();

    /* First we create the payload */
    PyTuple * RowPayload = new PyTuple( cc );

    for(uint32 i = 0; i < cc; i++)
    {
        PyTuple *RowField = new PyTuple( 2 );

        RowField->SetItem( 0, new PyString( res.ColumnName( i ) ));
        RowField->SetItem( 1, new PyInt( GetPackedColumnType( res.ColumnType( i ) ) ));

        RowPayload->SetItem( i , RowField );
    }

    /* store the payload in its tuple container */
    PyTuple * PayloadContainer = new PyTuple(1);
    PayloadContainer->SetItem( 0, RowPayload );

    /* we create the class instance */
    PyString * ClassTypeName = new PyString("blue.DBRowDescriptor", true);
    
    PyTuple * ClassHeader = new PyTuple(2);

    ClassHeader->SetItem( 0, ClassTypeName );
    ClassHeader->SetItem( 1, PayloadContainer );

    /* now everything is almost ready */

    /* we don't have root list or root dict stuff here */
    PyObjectEx *main_obj = new PyObjectEx( false, ClassHeader );
    return main_obj;
}

PyObjectEx *DBRowToRowDescriptor( const DBResultRow &row )
{
    uint32 cc = row.ColumnCount();

    /* First we create the payload */
    PyTuple * RowPayload = new PyTuple( cc );

    for(uint32 i = 0; i < cc; i++)
    {
        PyTuple *RowField = new PyTuple( 2 );

        RowField->SetItem( 0, new PyString( row.ColumnName( i ) ));
        RowField->SetItem( 1, new PyInt( GetPackedColumnType( row.ColumnType( i ) ) ));

        RowPayload->SetItem( i , RowField );
    }

    /* store the payload in its tuple container */
    PyTuple * PayloadContainer = new PyTuple(1);
    PayloadContainer->SetItem( 0, RowPayload );

    /* we create the class instance */
    PyString * ClassTypeName = new PyString("blue.DBRowDescriptor", true);
    
    PyTuple * ClassHeader = new PyTuple(2);

    ClassHeader->SetItem( 0, ClassTypeName );
    ClassHeader->SetItem( 1, PayloadContainer );

    /* now everything is almost ready */

    /* we don't have root list or root dict stuff here */
    PyObjectEx *main_obj = new PyObjectEx( false, ClassHeader );
    return main_obj;
}

PyPackedRow *CreatePackedRow( const DBResultRow &row, PyRep &header, bool headerOwner )
{
    PyPackedRow *res = new PyPackedRow( header, headerOwner );

    uint32 cc = row.ColumnCount();
    for(uint32 i = 0; i < cc; i++)
        res->SetField( i, DBColumnToPyRep( row, i ) );

    return res;
}

PyList *DBResultToPackedRowList( DBQueryResult &result ) {
    PyRep *header = DBResultToRowDescriptor( result );

    PyList *res = new PyList(result.GetRowCount());

    DBResultRow row;
    uint32 i = 0;
    while(result.GetRow(row))
        //this is piece of crap due to header cloning
        res->set(i++, CreatePackedRow( row, *header->Clone(), true ) );

    SafeDelete( header );
    return res;
}

/* function not used */
PyTuple *DBResultToPackedRowListTuple( DBQueryResult &result )
{
    size_t row_count = result.GetRowCount();
    PyList * list = new PyList( row_count );
    PyTuple * root = new PyTuple(2);
    PyRep * header = DBResultToRowDescriptor( result );
    root->SetItem( 0, header );
    root->SetItem( 1, list );

    DBResultRow row;
    for (size_t i = 0; i < row_count; i++)
    {
        if (!result.GetRow(row))
            break;
        list->set( i, CreatePackedRow( row, *header->Clone(), true ) );
    }

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
PyObjectEx *DBResultToCRowset( DBQueryResult &result ) {

    uint32 cc = result.GetRowCount();

    /* create CRowSet class header */
    PyTuple * ClassHeader = new PyTuple(2);

    PyString * ClassTypeName = new PyString("dbutil.CRowset", true);  // 0
    PyObjectEx * RowDescriptor = DBResultToRowDescriptor( result );      // 1

    /* object containers */
    PyDict * headerDict = new PyDict(); headerDict->add( "header", RowDescriptor );
    PyTuple * headerTuple = new PyTuple(1); headerTuple->SetItem( 0, ClassTypeName );

    ClassHeader->SetItem( 0, headerTuple );
    ClassHeader->SetItem( 1, headerDict );

    PyObjectEx *root = new PyObjectEx( true, ClassHeader );
    
    root->list_data.resize( cc );

    DBResultRow row;
    uint32 i = 0;
    //res.header->IncRef();
    while(result.GetRow(row))
    {
        //this is piece of crap due to header cloning
        //res.header->IncRef();
        root->list_data[i++] = CreatePackedRow( row, *RowDescriptor->Clone(), true );
    }

    return root;
}

PyPackedRow *DBRowToPackedRow( DBResultRow &row )
{
    PyRep *header = DBRowToRowDescriptor( row );

    return CreatePackedRow( row, *header, true );
}
