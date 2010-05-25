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

/*************************************************************************/
/* EVEServerConfig                                                       */
/*************************************************************************/
EXPORT_SINGLETON( EVEServerConfig );

EVEServerConfig::EVEServerConfig()
{
    // register needed parsers
    AddMemberParser( "eve", &EVEServerConfig::ProcessEve );

    // Set sane defaults

    // server
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

    // files
    files.log = "log/eve-server.log";
    files.logSettings = "log/log.ini";
    files.cacheDir = "";
}

bool EVEServerConfig::ProcessEve( const TiXmlElement* ele )
{
    // entering element, extend allowed syntax
    AddMemberParser( "server",   &EVEServerConfig::ProcessServer );
    AddMemberParser( "database", &EVEServerConfig::ProcessDatabase );
    AddMemberParser( "files",    &EVEServerConfig::ProcessFiles );

    // parse the element
    const bool result = ParseElementChildren( ele );

    // leaving element, reduce allowed syntax
    RemoveParser( "server" );
    RemoveParser( "database" );
    RemoveParser( "files" );

    // return status of parsing
    return result;
}

bool EVEServerConfig::ProcessServer( const TiXmlElement* ele )
{
    // entering element, extend allowed syntax
    AddValueParser( "port",            server.port );
    AddValueParser( "startBalance",    server.startBalance );
    AddValueParser( "autoAccount",     server.autoAccount );
    AddValueParser( "autoAccountRole", server.autoAccountRole );

    // parse the element
    const bool result = ParseElementChildren( ele );

    // leaving element, reduce allowed syntax
    RemoveParser( "port" );
    RemoveParser( "startBalance" );
    RemoveParser( "autoAccount" );
    RemoveParser( "autoAccountRole" );

    // return status of parsing
    return result;
}

bool EVEServerConfig::ProcessDatabase( const TiXmlElement* ele )
{
    // entering element, extend allowed syntax
    AddValueParser( "host",     database.host );
    AddValueParser( "port",     database.port );
    AddValueParser( "username", database.username );
    AddValueParser( "password", database.password );
    AddValueParser( "db",       database.db );

    // parse the element
    const bool result = ParseElementChildren( ele );

    // leaving element, reduce allowed syntax
    RemoveParser( "host" );
    RemoveParser( "port" );
    RemoveParser( "username" );
    RemoveParser( "password" );
    RemoveParser( "db" );

    // return status of parsing
    return result;
}

bool EVEServerConfig::ProcessFiles( const TiXmlElement* ele )
{
    // entering element, extend allowed syntax
    AddValueParser( "log",         files.log );
    AddValueParser( "logSettings", files.logSettings );
    AddValueParser( "cacheDir",    files.cacheDir );

    // parse the element
    const bool result = ParseElementChildren( ele );

    // leaving element, reduce allowed syntax
    RemoveParser( "log" );
    RemoveParser( "logSettings" );
    RemoveParser( "cacheDir" );

    // return status of parsing
    return result;
}
