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
    Author:     Bloody.Rabbit
*/

#include "CRowSet.h"
#include "DBRowDescriptor.h"

CRowSet::CRowSet(DBRowDescriptor **rowDesc)
: PyObjectEx_Type2( _CreateArgs(), _CreateKeywords( *rowDesc ) )
{
	*rowDesc = NULL;
}

uint32 CRowSet::GetRowCount() const
{
	return list_data.size();
}

PyPackedRow &CRowSet::GetRow(uint32 index) const
{
	return list_data.GetItem( index )->AsPackedRow();
}

PyPackedRow &CRowSet::NewRow()
{
    DBRowDescriptor & rowDesc = _GetRowDesc();
    PyIncRef( &rowDesc );

    PyPackedRow *row = new PyPackedRow( rowDesc );
    
	list_data.AddItem( row );
	return *row;
}

DBRowDescriptor &CRowSet::_GetRowDesc() const
{
	PyRep *r = FindKeyword( "header" );
	assert( r );

	return ( DBRowDescriptor & )r->AsObjectEx();
}

PyList &CRowSet::_GetColumnList() const
{
	PyRep *r = FindKeyword( "columns" );
	assert( r );

	return r->AsList();
}

PyTuple *CRowSet::_CreateArgs()
{
	PyTuple *args = new PyTuple( 1 );
	args->SetItem( 0, new PyString( "dbutil.CRowset", true ) );

	return args;
}

PyDict *CRowSet::_CreateKeywords(DBRowDescriptor *rowDesc)
{
	assert( rowDesc );

	PyDict *keywords = new PyDict;
	keywords->SetItemString( "header", rowDesc );

	uint32 cc = rowDesc->ColumnCount();
	PyList *columns = new PyList( cc );
	for( uint32 i = 0; i < cc; i++ )
		columns->SetItem( i,  new PyString( rowDesc->GetColumnName( i ) ) );
	keywords->SetItemString( "columns", columns );

	return keywords;
}
