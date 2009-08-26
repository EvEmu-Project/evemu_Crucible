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

#include "DBRowDescriptor.h"
#include "EVEDBUtils.h"

blue_DBRowDescriptor::blue_DBRowDescriptor()
: PyObjectEx( false, _CreateEmptyHeader() )
{
}

blue_DBRowDescriptor::blue_DBRowDescriptor(const DBQueryResult &res)
: PyObjectEx( false, _CreateEmptyHeader() )
{
	uint32 cc = res.ColumnCount();

	for( uint32 i = 0; i < cc; i++ )
		AddColumn( res.ColumnName( i ), GetPackedColumnType( res.ColumnType( i ) ) );
}

blue_DBRowDescriptor::blue_DBRowDescriptor(const DBResultRow &row)
: PyObjectEx( false, _CreateEmptyHeader() )
{
	uint32 cc = row.ColumnCount();

	for( uint32 i = 0; i < cc; i++ )
		AddColumn( row.ColumnName( i ), GetPackedColumnType( row.ColumnType( i ) ) );
}

uint32 blue_DBRowDescriptor::ColumnCount() const
{
	return _GetColumnList().size();
}

const std::string &blue_DBRowDescriptor::GetColumnName(uint32 index) const
{
	PyTuple &col = _GetColumn( index );

	return col.items.at( 0 )->AsString().value;
}

DBTYPE blue_DBRowDescriptor::GetColumnType(uint32 index) const
{
	PyTuple &col = _GetColumn( index );

	return static_cast<DBTYPE>( col.items.at( 1 )->AsInt().GetValue() );
}

uint32 blue_DBRowDescriptor::FindColumn(const char *name) const
{
	uint32 cc = ColumnCount();

	for( uint32 i = 0; i < cc; i++ )
		if( GetColumnName( i ) == name )
			return i;

	return cc;
}

void blue_DBRowDescriptor::AddColumn(const char *name, DBTYPE type)
{
	PyTuple *col = new PyTuple( 2 );

	col->SetItem( 0, new PyString( name ) );
	col->SetItem( 1, new PyInt( type ) );

	_GetColumnList().items.push_back( col );
}

blue_DBRowDescriptor *blue_DBRowDescriptor::TypedClone() const
{
	return static_cast<blue_DBRowDescriptor *>( PyObjectEx::TypedClone() );
}

PyTuple &blue_DBRowDescriptor::_GetColumnList() const
{
	assert( header );

	// using at() guarantees bound check
	PyTuple *t = &header->AsTuple().items.at( 1 )->AsTuple();
	return t->items.at( 0 )->AsTuple();
}

PyTuple &blue_DBRowDescriptor::_GetColumn(size_t index) const
{
	PyTuple &cl = _GetColumnList();
	assert( index < cl.size() );

	return cl.items.at( index )->AsTuple();
}

PyTuple *blue_DBRowDescriptor::_CreateEmptyHeader()
{
	PyTuple *columnList = new PyTuple( 0 );

	PyTuple *columnListWrapper = new PyTuple( 1 );
	columnListWrapper->SetItem( 0, columnList );

	PyTuple *head = new PyTuple( 2 );

	head->SetItem( 0, new PyString( "blue.DBRowDescriptor", true ) );
	head->SetItem( 1, columnListWrapper );

	return head;
}


