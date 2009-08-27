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

CRowSet::CRowSet(blue_DBRowDescriptor **rowDesc)
: PyObjectEx( true, _CreateHeader( *rowDesc ) )
{
	*rowDesc = NULL;
}

uint32 CRowSet::GetRowCount() const
{
	return list_data.size();
}

PyPackedRow &CRowSet::GetRow(uint32 index) const
{
	return list_data.at( index )->AsPackedRow();
}

PyPackedRow &CRowSet::NewRow()
{
	PyPackedRow *row = new PyPackedRow( *_GetRowDesc().TypedClone(), true );
	list_data.push_back( row );
	return *row;
}

PyDict &CRowSet::_GetKeywords() const
{
	assert( header );

	return header->AsTuple().items.at( 1 )->AsDict();
}

PyRep *CRowSet::_FindKeyword(const char *keyword) const
{
	PyDict &kw = _GetKeywords();

	PyDict::const_iterator cur, end;
	cur = kw.begin();
	end = kw.end();
	for(; cur != end; cur++)
	{
		if( cur->first->AsString().value == keyword )
			return cur->second;
	}

	return NULL;
}

blue_DBRowDescriptor &CRowSet::_GetRowDesc() const
{
	PyRep *r = _FindKeyword( "header" );
	assert( r );

	return ( blue_DBRowDescriptor & )r->AsObjectEx();
}

PyList &CRowSet::_GetColumnList() const
{
	PyRep *r = _FindKeyword( "columns" );
	assert( r );

	return r->AsList();
}

PyTuple *CRowSet::_CreateHeader(blue_DBRowDescriptor *rowDesc)
{
	assert( rowDesc );

	PyTuple *type = new PyTuple( 1 );
	type->SetItem( 0, new PyString( "dbutil.CRowset", true ) );

	PyDict *keywords = new PyDict;
	keywords->add( "header", rowDesc );

	uint32 cc = rowDesc->ColumnCount();
	PyList *columns = new PyList( cc );
	for( uint32 i = 0; i < cc; i++ )
		columns->set( i,  new PyString( rowDesc->GetColumnName( i ) ) );
	keywords->add( "columns", columns );

	PyTuple *head = new PyTuple( 2 );
	head->SetItem( 0, type );
	head->SetItem( 1, keywords );

	return head;
}



