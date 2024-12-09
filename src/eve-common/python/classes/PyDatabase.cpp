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
    Author:     Bloody.Rabbit
    Updates:    Allan
*/

#include "eve-common.h"
#include "python/classes/PyDatabase.h"
#include "utils/EVEUtils.h"

/************************************************************************/
/* DBRowDescriptor                                                      */
/************************************************************************/
DBRowDescriptor::DBRowDescriptor()
: PyObjectEx_Type1( new PyToken( "blue.DBRowDescriptor" ), _CreateArgs() )
{
}

DBRowDescriptor::DBRowDescriptor(PyList* keywords)
: PyObjectEx_Type1( new PyToken( "blue.DBRowDescriptor" ), _CreateArgs(), keywords )
{
}

DBRowDescriptor::DBRowDescriptor( const DBQueryResult& res )
: PyObjectEx_Type1( new PyToken( "blue.DBRowDescriptor" ), _CreateArgs() )
{
    uint32 cc(res.ColumnCount());
    for (uint32 i(0); i < cc; ++i)
        AddColumn( res.ColumnName( i ), res.ColumnType( i ) );
}

DBRowDescriptor::DBRowDescriptor( const DBResultRow& row )
: PyObjectEx_Type1( new PyToken( "blue.DBRowDescriptor" ), _CreateArgs() )
{
    uint32 cc(row.ColumnCount());
    for (uint32 i(0); i < cc; ++i)
        AddColumn( row.ColumnName( i ), row.ColumnType( i ) );
}

uint32 DBRowDescriptor::ColumnCount() const
{
    size_t count = _GetColumnList()->size();
    if (count > static_cast<size_t>(UINT32_MAX)) {
        sLog.Error("DBRowDescriptor", "Column count %zu exceeds maximum allowed value", count);
        return 0;
    }
    return static_cast<uint32>(count);
}

PyString* DBRowDescriptor::GetColumnName( uint32 index ) const
{
    size_t columnCount = _GetColumnList()->size();
    if (index >= columnCount) {
        sLog.Error("DBRowDescriptor", "Column index %u out of range (max: %zu)", 
                   index, columnCount - 1);
        return nullptr;
    }
    return _GetColumn( index )->GetItem( 0 )->AsString();
}

DBTYPE DBRowDescriptor::GetColumnType( uint32 index ) const
{
    size_t columnCount = _GetColumnList()->size();
    if (index >= columnCount) {
        sLog.Error("DBRowDescriptor", "Column index %u out of range (max: %zu)", 
                   index, columnCount - 1);
        return DBTYPE_ERROR;
    }
    return (DBTYPE)_GetColumn( index )->GetItem( 1 )->AsInt()->value();
}

uint32 DBRowDescriptor::FindColumn( const char* name ) const
{
    uint32 cc(ColumnCount());
    PyString* stringName = new PyString( name );

    for( uint32 i(0); i < cc; ++i ) {
        if( stringName->hash() == GetColumnName( i )->hash() ) {
            PyDecRef( stringName );
            return i;
        }
    }

    PyDecRef( stringName );
    return cc;
}

bool DBRowDescriptor::VerifyValue( uint32 index, PyRep* value )
{
    return DBTYPE_IsCompatible( GetColumnType( index ), value );
}

void DBRowDescriptor::AddColumn( const char* name, DBTYPE type )
{
    PyTuple* col = new PyTuple( 2 );
        col->SetItem( 0, new PyString( name ) );
        col->SetItem( 1, new PyInt( type ) );
    _GetColumnList()->items.push_back( col );
}

PyTuple* DBRowDescriptor::_GetColumnList() const
{
    return GetArgs()->GetItem( 0 )->AsTuple();
}

PyTuple* DBRowDescriptor::_GetColumn( size_t index ) const
{
    return _GetColumnList()->GetItem( index )->AsTuple();
}

PyTuple* DBRowDescriptor::_CreateArgs()
{
    PyTuple* columnList = new PyTuple( 0 );
    PyTuple* args = new PyTuple( 1 );
        args->SetItem( 0, columnList );

    return args;
}

/************************************************************************/
/* CRowSet                                                              */
/************************************************************************/
CRowSet::CRowSet( DBRowDescriptor** rowDesc )
: PyObjectEx_Type2( _CreateArgs(), _CreateKeywords( *rowDesc ) )
{
}

PyPackedRow* CRowSet::NewRow()
{
    //DBRowDescriptor* rowDesc = _GetRowDesc();
    PyPackedRow* row = new PyPackedRow( _GetRowDesc() );

    list().AddItem( row );
    return row;
}

DBRowDescriptor* CRowSet::_GetRowDesc() const
{
    PyRep* r(FindKeyword( "header" ));
    if (r != nullptr)
        return (DBRowDescriptor*)r->AsObjectEx();
    return nullptr;
}

PyTuple* CRowSet::_CreateArgs()
{
    PyTuple* args = new PyTuple( 1 );
        args->SetItem( 0, new PyToken( "dbutil.CRowset" ) );
    return args;
}

PyDict* CRowSet::_CreateKeywords(DBRowDescriptor* rowDesc)
{
    assert( rowDesc );

    PyDict* keywords = new PyDict();
        keywords->SetItemString( "header", rowDesc );

    //The Type_2 i had no longer used this
    //uint32 cc = rowDesc->ColumnCount();
    //PyList* columns = new PyList( cc );
    //for( uint32 i(0); i < cc; ++i )
    //    columns->SetItem( i,  new PyString( *rowDesc->GetColumnName( i ) ) );
    //keywords->SetItemString( "columns", columns );

    return keywords;
}

/************************************************************************/
/* CIndexedRowSet                                                              */
/************************************************************************/
CIndexedRowSet::CIndexedRowSet( DBRowDescriptor** rowDesc )
: PyObjectEx_Type2( _CreateArgs(), _CreateKeywords( *rowDesc ) )
{
}

PyPackedRow* CIndexedRowSet::NewRow( PyRep* key )
{
    //DBRowDescriptor* rowDesc = _GetRowDesc();
    PyPackedRow* row = new PyPackedRow( _GetRowDesc() );

    dict().SetItem( key , row );
    return row;
}

DBRowDescriptor* CIndexedRowSet::_GetRowDesc() const
{
    PyRep* r(FindKeyword( "header" ));
    if (r != nullptr)
        return (DBRowDescriptor*)r->AsObjectEx();
    return nullptr;
}

PyTuple* CIndexedRowSet::_CreateArgs()
{
    PyTuple* args = new PyTuple( 1 );
        args->SetItem( 0, new PyToken( "dbutil.CIndexedRowset" ) );
    return args;
}

PyDict* CIndexedRowSet::_CreateKeywords(DBRowDescriptor* rowDesc)
{
    assert( rowDesc );

    PyDict* keywords = new PyDict();
    keywords->SetItemString( "header", rowDesc );
    keywords->SetItemString( "columnName", rowDesc->GetColumnName(0) );

    return keywords;
}


/************************************************************************/
/* CFilterRowSet                                                              */
/************************************************************************/
CFilterRowSet::CFilterRowSet( DBRowDescriptor** rowDesc )
: PyObjectEx_Type2( _CreateArgs(), _CreateKeywords( *rowDesc ) )
{
}

CRowSet* CFilterRowSet::NewRowset( PyRep* key )
{
    DBRowDescriptor* rowDesc = _GetRowDesc();
    CRowSet* row = new CRowSet( &rowDesc );

    dict().SetItem( key , row );
    return row;
}

DBRowDescriptor* CFilterRowSet::_GetRowDesc() const
{
    PyRep* r(FindKeyword( "header" ));
    if (r != nullptr)
        return (DBRowDescriptor*)r->AsObjectEx();
    return nullptr;
}

PyTuple* CFilterRowSet::_CreateArgs()
{
    PyTuple* args = new PyTuple( 1 );
        args->SetItem( 0, new PyToken( "dbutil.CFilterRowset" ) );
    return args;
}

PyDict* CFilterRowSet::_CreateKeywords(DBRowDescriptor* rowDesc)
{
    assert( rowDesc );

    PyDict* keywords = new PyDict();
    keywords->SetItemString( "header", rowDesc );
    keywords->SetItemString( "columnName", rowDesc->GetColumnName(0) );

    return keywords;
}
