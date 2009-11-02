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
	Author:		Zhur
*/

#include "EVEServerPCH.h"

EVEmuServerConfig::EVEmuServerConfig()
{
	// register needed parsers
    RegisterParser( "server",   &EVEmuServerConfig::_ParseServer );
    RegisterParser( "database", &EVEmuServerConfig::_ParseDatabase );
    RegisterParser( "files",    &EVEmuServerConfig::_ParseFiles );

	// Set sane defaults

	//server
	server.port = 26001;
	server.startBalance = 6666000000.0f;
	server.autoAccount = false;
	server.autoAccountRole = 2;

	// database
	database.host = "localhost";
	database.port = 3306;
	database.username = "eve";
	database.password = "eve";
	database.db = "eve";

	// Files
	files.log = "logs/eveserver.log";
	files.logSettings = "log.ini";
	files.cacheDir = "";
}

bool EVEmuServerConfig::_ParseServer( const TiXmlElement* ele )
{
	const char* text;

	text = ParseTextBlock( ele, "port", true );
	if( text )
		server.port = atoi( text );

	text = ParseTextBlock( ele, "startBalance", true );
	if( text )
		server.startBalance = atof( text );

	text = ParseTextBlock( ele, "autoAccount", true );
	if( text )
		server.autoAccount = atobool( text );
	
	text = ParseTextBlock( ele, "autoAccountRole", true );
	if( text )
		server.autoAccountRole = atof( text );

    return true;
}

bool EVEmuServerConfig::_ParseDatabase( const TiXmlElement* ele )
{
	const char* text;

	text = ParseTextBlock( ele, "host", true );
	if( text )
		database.host = text;

	text = ParseTextBlock( ele, "port", true );
	if( text )
		database.port = atoi( text );

	text = ParseTextBlock( ele, "username", true );
	if( text )
		database.username = text;

	text = ParseTextBlock( ele, "password", true );
	if( text )
		database.password = text;

	text = ParseTextBlock( ele, "db", true );
	if( text )
		database.db = text;

    return true;
}

bool EVEmuServerConfig::_ParseFiles( const TiXmlElement* ele )
{
	const char* text;

	text = ParseTextBlock( ele, "logfile", true );
	if( text )
		files.log = text;

	text = ParseTextBlock( ele, "logsettings", true );
	if( text )
		files.logSettings = text;

	text = ParseTextBlock( ele, "cache", true );
	if( text )
		files.cacheDir = text;

    return true;
}

