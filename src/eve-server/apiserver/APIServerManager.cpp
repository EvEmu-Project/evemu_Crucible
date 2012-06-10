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
    Author:        Aknor Jaden
*/

#include "eve-server.h"

#include "EntityList.h"
#include "apiserver/APIServerManager.h"

APIServerManager::APIServerManager(const PyServiceMgr &services)
: APIServiceManager(services)
{
}

std::tr1::shared_ptr<std::string> APIServerManager::ProcessCall(const APICommandCall * pAPICommandCall)
{
    sLog.Debug("APIServerManager::ProcessCall()", "EVEmu API - Server Service Manager");

    if( pAPICommandCall->find( "servicehandler" ) == pAPICommandCall->end() )
    {
        sLog.Error( "APIServerManager::ProcessCall()", "Cannot find 'servicehandler' specifier in pAPICommandCall packet" );
        return std::tr1::shared_ptr<std::string>(new std::string(""));
    }

    if( pAPICommandCall->find( "servicehandler" )->second == "ServerStatus.xml.aspx" )
        return _ServerStatus(pAPICommandCall);
    //else if( pAPICommandCall->find( "servicehandler" )->second == "TODO.xml.aspx" )
    //    return _TODO(pAPICommandCall);
    else
    {
        sLog.Error("APIServerManager::ProcessCall()", "EVEmu API - Server Service Manager - ERROR: Cannot resolve '%s' as a valid service query for Server Service Manager",
            pAPICommandCall->find("servicehandler")->second.c_str() );
        return std::tr1::shared_ptr<std::string>(new std::string(""));
    }
}

std::tr1::shared_ptr<std::string> APIServerManager::_ServerStatus(const APICommandCall * pAPICommandCall)
{
    uint32 playersOnline = services().entity_list.GetClientCount();
    std::string playersOnlineStr( itoa( playersOnline ) );

    _BuildXMLHeader();
    {
        _BuildXMLTag( "result" );
        {
            _BuildSingleXMLTag( "serverOpen", "True" );
            _BuildSingleXMLTag( "onlinePlayers", playersOnlineStr );
        }
        _CloseXMLTag(); // close tag "result"
    }
    _CloseXMLHeader( EVEAPI::CacheStyles::Modified );

    return _GetXMLDocumentString();
}
