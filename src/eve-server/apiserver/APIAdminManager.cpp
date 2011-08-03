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


APIAdminManager::APIAdminManager()
{
}

std::tr1::shared_ptr<std::string> APIAdminManager::ProcessCall(const APICommandCall * pAPICommandCall)
{
    sLog.Debug("APIServiceManager::ProcessCall()", "EVEmu API - Admin Service Manager");

    if( pAPICommandCall->find( "servicehandler" ) == pAPICommandCall->end() )
    {
        sLog.Error( "APIAdminManager::ProcessCall()", "Cannot find 'servicehandler' specifier in pAPICommandCall packet" );
        return NULL;
    }

    if( pAPICommandCall->find( "servicehandler" )->second == "APIKeyRequest.xml.aspx" )
        return _APIKeyRequest(pAPICommandCall);
    //else if( pAPICommandCall->find( "servicehandler" )->second == "TODO.xml.aspx" )
    //    return _TODO(pAPICommandCall);
    else
    {
        sLog.Error("APIServiceManager::ProcessCall()", "EVEmu API - Admin Service Manager - ERROR: Cannot resolve '%s' as a valid service query for Admin Service Manager", pAPICommandCall->find("admin")->second.c_str() );
        return std::tr1::shared_ptr<std::string>(new std::string(""));
    }
}

std::tr1::shared_ptr<std::string> APIAdminManager::_APIKeyRequest(const APICommandCall * pAPICommandCall)
{
    bool status = false;
    uint32 userID, apiRole;
    std::string accountID;
    std::string apiLimitedKey;
    std::string apiFullKey;
    sLog.Debug("APIServiceManager::_APIKeyRequest()", "EVEmu API - Admin Service Manager - CALL: API Key Request");

    // 1: Determine if this account's userID exists:
    status = m_db.GetApiAccountInfoUsingAccountID( accountID, &userID, &apiFullKey, &apiLimitedKey, &apiRole );

    // 2: If userID already exists, generate new 
/*   
    std::string username = "mdrayman";
    std::string accountID;
    std::string apiFullKey;
    std::string apiLimitedKey;
    uint32 userID;
    uint32 apiRole;
    uint32 accountIDint;
    std::string userIDstr = "1000001";
    bool status = false;
    status = m_db.GetAccountIdFromUsername(username, &accountID);
    status = m_db.GetApiAccountInfoUsingAccountID(accountID, &userID, &apiFullKey, &apiLimitedKey, &apiRole);
    accountIDint = 3;
    userID = 1000001;
    status = m_db.InsertNewUserIdApiKeyInfoToDatabase(accountIDint, apiFullKey, apiLimitedKey, apiRole);
    status = m_db.UpdateUserIdApiKeyDatabaseRow(userID, apiFullKey, apiLimitedKey);
    apiRole = 2000;
    status = m_db.UpdateUserIdApiRole(userID, apiRole);

    status = m_db.GetApiAccountInfoUsingUserID(userIDstr, &apiFullKey, &apiLimitedKey, &apiRole);

    return BuildErrorXMLResponse( "203", "Authentication failure." );
*/
}

std::string APIAdminManager::_GenerateAPIKey()
{
    //
}
