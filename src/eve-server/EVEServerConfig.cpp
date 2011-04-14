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
    Author:     Zhur, Bloody.Rabbit
*/

#include "EVEServerPCH.h"

/*************************************************************************/
/* EVEServerConfig                                                       */
/*************************************************************************/
EVEServerConfig::EVEServerConfig()
{
    // register needed parsers
    AddMemberParser( "eve-server", &EVEServerConfig::ProcessEveServer );

    // Set sane defaults

    // account
    account.autoAccountRole = 0;
    account.loginMessage =
        "<html>"
            "<head>"
            "</head>"
            "<body>"
                "Welcome to <b>EVEmu server</b>.<br>"
                "<br>"
                "You can find a lot of interesting information about this project at <a href=\"http://forum.evemu.org/\">forum.evemu.org</a>.<br>"
                "<br>"
                "You can also join our IRC channel at <b>irc.mmoforge.org:6667</b>, channel <b>#evemu</b>.<br>"
                "<br>"
                "Best wishes,<br>"
                "EVEmu development team"
           "</body>"
        "</html>";

    // character
    character.startBalance = 6666000000.0f;

    // database
    database.host = "localhost";
    database.port = 3306;
    database.username = "eve";
    database.password = "eve";
    database.db = "eve";

    // files
    files.log = "../log/eve-server.log";
    files.logSettings = "../etc/log.ini";
    files.cacheDir = "";

    // net
    net.port = 26001;
}

bool EVEServerConfig::ProcessEveServer( const TiXmlElement* ele )
{
    // entering element, extend allowed syntax
    AddMemberParser( "account",   &EVEServerConfig::ProcessAccount );
    AddMemberParser( "character", &EVEServerConfig::ProcessCharacter );
    AddMemberParser( "database",  &EVEServerConfig::ProcessDatabase );
    AddMemberParser( "files",     &EVEServerConfig::ProcessFiles );
    AddMemberParser( "net",       &EVEServerConfig::ProcessNet );

    // parse the element
    const bool result = ParseElementChildren( ele );

    // leaving element, reduce allowed syntax
    RemoveParser( "account" );
    RemoveParser( "character" );
    RemoveParser( "database" );
    RemoveParser( "files" );
    RemoveParser( "net" );

    // return status of parsing
    return result;
}

bool EVEServerConfig::ProcessAccount( const TiXmlElement* ele )
{
    AddValueParser( "autoAccountRole", account.autoAccountRole );
    AddValueParser( "loginMessage",    account.loginMessage );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "autoAccountRole" );
    RemoveParser( "loginMessage" );

    return result;
}

bool EVEServerConfig::ProcessCharacter( const TiXmlElement* ele )
{
    AddValueParser( "startBalance", character.startBalance );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "startBalance" );

    return result;
}

bool EVEServerConfig::ProcessDatabase( const TiXmlElement* ele )
{
    AddValueParser( "host",     database.host );
    AddValueParser( "port",     database.port );
    AddValueParser( "username", database.username );
    AddValueParser( "password", database.password );
    AddValueParser( "db",       database.db );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "host" );
    RemoveParser( "port" );
    RemoveParser( "username" );
    RemoveParser( "password" );
    RemoveParser( "db" );

    return result;
}

bool EVEServerConfig::ProcessFiles( const TiXmlElement* ele )
{
    AddValueParser( "log",         files.log );
    AddValueParser( "logSettings", files.logSettings );
    AddValueParser( "cacheDir",    files.cacheDir );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "log" );
    RemoveParser( "logSettings" );
    RemoveParser( "cacheDir" );

    return result;
}

bool EVEServerConfig::ProcessNet( const TiXmlElement* ele )
{
    AddValueParser( "port", net.port );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "port" );

    return result;
}
