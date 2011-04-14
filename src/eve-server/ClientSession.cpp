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
	Author:		Zhur
*/

#include "EVEServerPCH.h"

/* Things todo or missing
	[missing]
		* atribute dep's
		* GetDefaultValueOfAttribute
		* SESSIONCHANGEDELAY = (30 * 10000000L)
		* create a SID system (session ID system)
*/

ClientSession::ClientSession()
: mSession( new PyDict ),
  mDirty( false )
{
}

ClientSession::~ClientSession()
{
    PyDecRef( mSession );
}

int32 ClientSession::GetLastInt( const char* name ) const
{
    PyRep* v = _GetLast( name );
    if( v == NULL )
        return 0;

    if( !v->IsInt() )
        return 0;

    return v->AsInt()->value();
}

int32 ClientSession::GetCurrentInt( const char* name ) const
{
    PyRep* v = _GetCurrent( name );
    if( v == NULL )
        return 0;

    if( !v->IsInt() )
        return 0;

    return v->AsInt()->value();
}

void ClientSession::SetInt( const char* name, int32 value )
{
    _Set( name, new PyInt( value ) );
}

int64 ClientSession::GetLastLong( const char* name ) const
{
    PyRep* v = _GetLast( name );
    if( v == NULL )
        return 0;

    if( !v->IsLong() )
        return 0;

    return v->AsLong()->value();
}

int64 ClientSession::GetCurrentLong( const char* name ) const
{
    PyRep* v = _GetCurrent( name );
    if( v == NULL )
        return 0;

    if( !v->IsLong() )
        return 0;

    return v->AsLong()->value();
}

void ClientSession::SetLong( const char* name, int64 value )
{
    _Set( name, new PyLong( value ) );
}

std::string ClientSession::GetLastString( const char* name ) const
{
    PyRep* v = _GetLast( name );
    if( v == NULL )
        return std::string();

    if( !v->IsString() )
        return std::string();

    return v->AsString()->content();
}

std::string ClientSession::GetCurrentString( const char* name ) const
{
    PyRep* v = _GetCurrent( name );
    if( v == NULL )
        return std::string();

    if( !v->IsString() )
        return std::string();

    return v->AsString()->content();
}

void ClientSession::SetString( const char* name, const char* value )
{
    _Set( name, new PyString( value ) );
}

void ClientSession::Clear( const char* name )
{
    _Set( name, new PyNone );
}

void ClientSession::EncodeChanges( PyDict* into )
{
    PyDict::const_iterator cur, end;
    cur = mSession->begin();
    end = mSession->end();
    for(; cur != end; cur++)
    {
        PyString* str = cur->first->AsString();
        PyTuple* value = cur->second->AsTuple();

        PyRep* last = value->GetItem( 0 );
        PyRep* current = value->GetItem( 1 );

        if( last->hash() != current->hash() )
        {
            // Duplicate tuple
            PyTuple* t = new PyTuple( 2 );
            t->SetItem( 0, last ); PyIncRef( last );
            t->SetItem( 1, current ); PyIncRef( current );
            into->SetItem( str, t ); PyIncRef( str );

            // Update our tuple
            value->SetItem( 0, current ); PyIncRef( current );
        }
    }

	mDirty = false;
}

PyTuple* ClientSession::_GetValueTuple( const char* name ) const
{
    PyRep* v = mSession->GetItemString( name );
    if( v == NULL )
        return NULL;
    return v->AsTuple();
}

PyRep* ClientSession::_GetLast( const char* name ) const
{
    PyTuple* v = _GetValueTuple( name );
    if( v == NULL )
        return NULL;
    return v->GetItem( 0 );
}

PyRep* ClientSession::_GetCurrent( const char* name ) const
{
    PyTuple* v = _GetValueTuple( name );
    if( v == NULL )
        return NULL;
    return v->GetItem( 1 );
}

void ClientSession::_Set( const char* name, PyRep* value )
{
    PyTuple* v = _GetValueTuple( name );
    if( v == NULL )
    {
        v = new PyTuple( 2 );
        v->SetItem( 0, new PyNone );
        v->SetItem( 1, new PyNone );
        mSession->SetItemString( name, v );
    }

    PyRep* current = v->GetItem( 1 );
    if( value->hash() != current->hash() )
    {
        v->SetItem( 1, value );

        mDirty = true;
    }
    else
    {
        PyDecRef( value );
    }
}

