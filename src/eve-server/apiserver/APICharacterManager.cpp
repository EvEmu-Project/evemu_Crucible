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
	Author:		Aknor Jaden
*/


#include "EVEServerPCH.h"


APICharacterManager::APICharacterManager(const PyServiceMgr &services)
: APIServiceManager(services)
{
}

std::tr1::shared_ptr<std::string> APICharacterManager::ProcessCall(const APICommandCall * pAPICommandCall)
{
    sLog.Debug("APIAdminManager::ProcessCall()", "EVEmu API - Character Service Manager");

    if( pAPICommandCall->find( "servicehandler" ) == pAPICommandCall->end() )
    {
        sLog.Error( "APICharacterManager::ProcessCall()", "Cannot find 'servicehandler' specifier in pAPICommandCall packet" );
        return std::tr1::shared_ptr<std::string>(new std::string(""));
    }

    if( pAPICommandCall->find( "servicehandler" )->second == "CharacterSheet.xml.aspx" )
        return _CharacterSheet(pAPICommandCall);
    //else if( pAPICommandCall->find( "servicehandler" )->second == "TODO.xml.aspx" )
    //    return _TODO(pAPICommandCall);
    else
    {
        sLog.Error("APIAdminManager::ProcessCall()", "EVEmu API - Admin Service Manager - ERROR: Cannot resolve '%s' as a valid service query for Admin Service Manager",
            pAPICommandCall->find("servicehandler")->second.c_str() );
        return std::tr1::shared_ptr<std::string>(new std::string(""));
    }
    sLog.Debug("APICharacterManager::ProcessCall()", "EVEmu API - Character Service Manager");

    return std::tr1::shared_ptr<std::string>(new std::string(""));
}

std::tr1::shared_ptr<std::string> APICharacterManager::_CharacterSheet(const APICommandCall * pAPICommandCall)
{
    if( pAPICommandCall->find( "userID" ) == pAPICommandCall->end() )
    {
        sLog.Error( "APICharacterManager::_CharacterSheet()", "ERROR: No 'userID' parameter found in call argument list - exiting with error" );
        return BuildErrorXMLResponse( "106", "Must provide userID parameter for authentication." );
    }

    // TODO
    return BuildErrorXMLResponse( "9999", "EVEmu API Server: Character Manager - CharacterSheet.xml.aspx STUB" );
}
