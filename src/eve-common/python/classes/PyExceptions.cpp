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
	Author:		Bloody.Rabbit
*/

#include "EVECommonPCH.h"

#include "python/classes/PyExceptions.h"
#include "EVEVersion.h"

GPSTransportClosed::GPSTransportClosed( const char* reason )
: PyObjectEx_Type1( new PyToken( "exceptions.GPSTransportClosed" ), _CreateArgs( reason ), _CreateKeywords( reason ) )
{
}

void GPSTransportClosed::AddKeyword( const char* name, PyRep* value )
{
	_GetReasonArgs()->SetItemString( name, value );
}

PyDict* GPSTransportClosed::_GetReasonArgs() const
{
	PyRep* r = FindKeyword( "reasonArgs" );
	assert( r );

	return r->AsDict();
}

PyTuple* GPSTransportClosed::_CreateArgs( const char* reason )
{
	PyTuple* args = new PyTuple( 1 );
	args->SetItem( 0, new PyString( reason ) );

	return args;
}

PyDict* GPSTransportClosed::_CreateKeywords( const char* reason )
{
	PyDict* keywords = new PyDict;
	//keywords->SetItemString( "origin", new PyString( "proxy" ) );
	keywords->SetItemString( "reasonArgs", new PyDict );
	keywords->SetItemString( "clock", new PyLong( Win32TimeNow() ) );
	//keywords->SetItemString( "loggedOnUserCount", );
	keywords->SetItemString( "region", new PyString( EVEProjectRegion ) );
	keywords->SetItemString( "reason", new PyString( reason ) );
	keywords->SetItemString( "version", new PyFloat( EVEVersionNumber ) );
	keywords->SetItemString( "build", new PyInt( EVEBuildVersion ) );
	//keywords->SetItemString( "reasonCode", );
	keywords->SetItemString( "codename", new PyString( EVEProjectCodename ) );
	keywords->SetItemString( "machoVersion", new PyInt( MachoNetVersion ) );

	return keywords;
}

UserError::UserError( const char* msg )
: PyObjectEx_Type1( new PyToken( "ccp_exceptions.UserError" ), _CreateArgs( msg ), _CreateKeywords( msg ) )
{
}

void UserError::AddKeyword( const char* name, PyRep* value )
{
    // We need 2 refs ... the first one is given to us,
    // but we must create the second one ...
    PyIncRef( value );

	_GetTupleKeywords()->SetItemString( name, value );
	_GetDictKeywords()->SetItemString( name, value );
}

PyDict* UserError::_GetTupleKeywords() const
{
	return GetArgs()->GetItem( 1 )->AsDict();
}

PyDict* UserError::_GetDictKeywords() const
{
	PyRep* r = FindKeyword( "dict" );
	assert( r );

	return r->AsDict();
}

PyTuple* UserError::_CreateArgs( const char* msg )
{
	PyTuple* args = new PyTuple( 2 );
	args->SetItem( 0, new PyString( msg ) );
	args->SetItem( 1, new PyDict );

	return args;
}

PyDict* UserError::_CreateKeywords( const char* msg )
{
	PyDict* keywords = new PyDict;
	keywords->SetItemString( "msg", new PyString( msg ) );
	keywords->SetItemString( "dict", new PyDict );

	return keywords;
}

