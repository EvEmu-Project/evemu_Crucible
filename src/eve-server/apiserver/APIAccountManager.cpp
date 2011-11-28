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


std::string APIAccountManager::m_hexCharMap("0123456789ABCDEF");


APIAccountManager::APIAccountManager(const PyServiceMgr &services)
: APIServiceManager(services)
{
}

std::tr1::shared_ptr<std::string> APIAccountManager::ProcessCall(const APICommandCall * pAPICommandCall)
{
    sLog.Debug("APIAccountManager::ProcessCall()", "EVEmu API - Account Service Manager");

    if( pAPICommandCall->find( "servicehandler" ) == pAPICommandCall->end() )
    {
        sLog.Error( "APIAccountManager::ProcessCall()", "Cannot find 'servicehandler' specifier in pAPICommandCall packet" );
        return std::tr1::shared_ptr<std::string>(new std::string(""));
    }

    if( pAPICommandCall->find( "servicehandler" )->second == "APIKeyRequest.xml.aspx" )
        return _APIKeyRequest(pAPICommandCall);
    else if( pAPICommandCall->find( "servicehandler" )->second == "Characters.xml.aspx" )
        return _Characters(pAPICommandCall);
    else if( pAPICommandCall->find( "servicehandler" )->second == "AccountStatus.xml.aspx" )
        return _AccountStatus(pAPICommandCall);
    //else if( pAPICommandCall->find( "servicehandler" )->second == "TODO.xml.aspx" )
    //    return _TODO(pAPICommandCall);
    else
    {
        sLog.Error("APIAccountManager::ProcessCall()", "EVEmu API - Account Service Manager - ERROR: Cannot resolve '%s' as a valid service query for Admin Service Manager",
            pAPICommandCall->find("servicehandler")->second.c_str() );
        return std::tr1::shared_ptr<std::string>(new std::string(""));
    }

    return BuildErrorXMLResponse( "9999", "EVEmu API Server: Account Manager - Unknown call." );
}

std::tr1::shared_ptr<std::string> APIAccountManager::_APIKeyRequest(const APICommandCall * pAPICommandCall)
{
    bool status = false;
    uint32 userID, apiRole;
    std::string username;
    std::string password;
    std::string keyType;
    std::string action;
    std::string apiLimitedKey;
    std::string apiFullKey;
    std::string accountID;
    std::string keyTag;

    sLog.Debug("APIAccountManager::_APIKeyRequest()", "EVEmu API - Account Service Manager - CALL: APIKeyRequest.xml.aspx");

    // 1: Decode arguments:
    if( pAPICommandCall->find( "username" ) != pAPICommandCall->end() )
        username = pAPICommandCall->find( "username" )->second;
    else
    {
        sLog.Error( "APIAccountManager::_APIKeyRequest()", "ERROR: No 'username' parameter found in call argument list - exiting with error" );
        return BuildErrorXMLResponse( "203", "Authentication failure." );
    }

    if( pAPICommandCall->find( "password" ) != pAPICommandCall->end() )
        password = pAPICommandCall->find( "password" )->second;
    else
    {
        sLog.Error( "APIAccountManager::_APIKeyRequest()", "ERROR: No 'password' parameter found in call argument list - exiting with error" );
        return BuildErrorXMLResponse( "203", "Authentication failure." );
    }

    if( pAPICommandCall->find( "keytype" ) != pAPICommandCall->end() )
        keyType = pAPICommandCall->find( "keytype" )->second;
    else
    {
        sLog.Error( "APIAccountManager::_APIKeyRequest()", "ERROR: No 'keytype' parameter found in call argument list - exiting with error" );
        return BuildErrorXMLResponse( "203", "Authentication failure." );
    }

    if( keyType != "full" )
        if( keyType != "limited" )
        {
            sLog.Error( "APIAccountManager::_APIKeyRequest()", "ERROR: 'keytype' parameter has invalid value '%s' - exiting with error", keyType.c_str() );
            return BuildErrorXMLResponse( "203", "Authentication failure." );
        }

    if( pAPICommandCall->find( "action" ) != pAPICommandCall->end() )
        action = pAPICommandCall->find( "action" )->second;
    else
    {
        sLog.Error( "APIAccountManager::_APIKeyRequest()", "ERROR: No 'action' parameter found in call argument list - exiting with error" );
        return BuildErrorXMLResponse( "203", "Authentication failure." );
    }

    if( action != "new" )
        if( action != "get" )
        {
            sLog.Error( "APIAccountManager::_APIKeyRequest()", "ERROR: 'action' parameter has invalid value '%s' - exiting with error", action.c_str() );
            return BuildErrorXMLResponse( "203", "Authentication failure." );
        }

    // 2: Authenticate the username and password against the account table:
    status = _AuthenticateUserNamePassword( username, password );
    if( !status )
    {
        sLog.Error( "APIAccountManager::_APIKeyRequest()", "ERROR: username='%s' password='%s' does not authenticate.", username.c_str(), password.c_str() );
        return BuildErrorXMLResponse( "203", "Authentication failure." );
    }
    // 2a: Get accountID using username, now that it's been validated:
    status = m_db.GetAccountIdFromUsername( username, &accountID );
    if( !status )
    {
        sLog.Error( "APIAccountManager::_APIKeyRequest()", "ERROR: username='%s' cannot be found in 'account' table.", username.c_str() );
        return BuildErrorXMLResponse( "203", "Authentication failure." );
    }

    // 3: Determine if this account's userID exists:
    status = m_db.GetApiAccountInfoUsingAccountID( accountID, &userID, &apiFullKey, &apiLimitedKey, &apiRole );

    // 4: Generate new random 64-character hexadecimal API Keys:
    // Write new API Key to database if key request 'action' is 'new':
    if( !status )
    {
        // 4a: If userID does not exist for this accountID, then insert a new row into the 'accountApi' table regardless of 'get' or 'new':
        apiLimitedKey = _GenerateAPIKey();
        apiFullKey = _GenerateAPIKey();
        status = m_db.InsertNewUserIdApiKeyInfoToDatabase( atol(accountID.c_str()), apiFullKey, apiLimitedKey, EVEAPI::Roles::Player );
    }

    if( action == "new" )
        if( keyType == "limited" )
            apiLimitedKey = _GenerateAPIKey();
        else //if( keyType == "full" )
            apiFullKey = _GenerateAPIKey();

    if( action == "new" )
    {
        if( status )
            // 4b: If userID already exists, generate new API keys and write them back to the database under that userID:
            status = m_db.UpdateUserIdApiKeyDatabaseRow( userID, apiFullKey, apiLimitedKey );
    }

    // 5: Build XML document to return to API client:
    userID = 0;
    apiFullKey = "";
    apiLimitedKey = "";
    apiRole = 0;
    //status = m_db.GetAccountIdFromUsername( username, &accountID );

    status = m_db.GetApiAccountInfoUsingAccountID( accountID, &userID, &apiFullKey, &apiLimitedKey, &apiRole );
    if( !status )
    {
        sLog.Error( "APIAccountManager::_APIKeyRequest()", "ERROR: username='%s' cannot be found in 'account' table.", username.c_str() );
        return BuildErrorXMLResponse( "203", "Authentication failure." );
    }

    // 6: Return the userID and the API key requested in the xml structure:
    _BuildXMLHeader();
    {
        _BuildSingleXMLTag( "userID", std::string(itoa(userID)) );
        if( keyType == "full" )
        {
            keyTag = "apiFullKey";
            _BuildSingleXMLTag( keyTag, apiFullKey );
        }
        else
        {
            keyTag = "apiLimitedKey";
            _BuildSingleXMLTag( keyTag, apiLimitedKey );
        }
        _BuildSingleXMLTag( "expiresOn", Win32TimeToString(Win32TimeNow() + 180*Win32Time_Day) );
    }
    _CloseXMLHeader( EVEAPI::CacheStyles::Long );

    return _GetXMLDocumentString();
}

std::tr1::shared_ptr<std::string> APIAccountManager::_Characters(const APICommandCall * pAPICommandCall)
{

    sLog.Error( "APIAccountManager::_Characters()", "TODO: Insert code to validate userID and apiKey" );

    sLog.Debug("APIAccountManager::_Characters()", "EVEmu API - Account Service Manager - CALL: Characters.xml.aspx");

    if( pAPICommandCall->find( "userid" ) == pAPICommandCall->end() )
    {
        sLog.Error( "APIAccountManager::_Characters()", "ERROR: No 'userID' parameter found in call argument list - exiting with error" );
		return BuildErrorXMLResponse( "106", "Must provide userID parameter for authentication." );
	}

    uint32 status = 0;
    uint32 accountID = 0;
    std::vector<std::string> charIDList;
    std::vector<std::string> charNameList;
    std::vector<std::string> charCorpIDList;
    std::vector<std::string> charCorpNameList;

    std::string userID = pAPICommandCall->find( "userid" )->second;

    status = m_db.GetAccountIdFromUserID( userID, &accountID );
    if( !status )
    {
        sLog.Error( "APIAccountManager::_Characters()", "ERROR: userID='%s' cannot be found in 'accountApi' table.", userID.c_str() );
        return BuildErrorXMLResponse( "203", "Authentication failure." );
    }

    if( !( m_accountDB.GetCharactersList(accountID, charIDList, charNameList, charCorpIDList, charCorpNameList) ) )
    {
        sLog.Error( "APIAccountManager::_Characters()", "ERROR: m_accountDB.GetCharactersList() call failed for unknown reason - exiting with error" );
        BuildErrorXMLResponse( "9999", "EVEmu API Server: Account Manager - Characters.xml.aspx STUB" );
    }

    std::vector<std::string> rowset;
    _BuildXMLHeader();
    {
        _BuildXMLTag( "result" );
        {
            rowset.push_back("name");
            rowset.push_back("characterID");
            rowset.push_back("corporationName");
            rowset.push_back("corporationID");
            _BuildXMLRowSet( "characters", "characterID", &rowset );
            {
                for(uint32 i=0; i<charIDList.size(); i++)
                {
                    rowset.clear();
                    rowset.push_back(charNameList.at(i));
                    rowset.push_back(charIDList.at(i));
                    rowset.push_back(charCorpNameList.at(i));
                    rowset.push_back(charCorpIDList.at(i));
                    _BuildXMLRow( &rowset );
                }
            }
            _CloseXMLRowSet();  // close rowset "characters"
        }
        _CloseXMLTag(); // close tag "result"
    }
    _CloseXMLHeader( EVEAPI::CacheStyles::Long );

    return _GetXMLDocumentString();
}

std::tr1::shared_ptr<std::string> APIAccountManager::_AccountStatus(const APICommandCall * pAPICommandCall)
{
    sLog.Error( "APIAccountManager::_AccountStatus()", "TODO: Insert code to validate userID and apiKey" );

    if( pAPICommandCall->find( "userid" ) == pAPICommandCall->end() )
    {
        sLog.Error( "APIAccountManager::_AccountStatus()", "ERROR: No 'userID' parameter found in call argument list - exiting with error" );
        return BuildErrorXMLResponse( "106", "Must provide userID parameter for authentication." );
    }

    uint32 accountID = 0;
    std::vector<std::string> accountInfoList;
    
    if( !(m_db.GetAccountIdFromUserID( pAPICommandCall->find( "userid" )->second, &accountID )) )
    {
        sLog.Error( "APIAccountManager::_AccountStatus()", "ERROR: Could not find 'accountID' in 'accountApi' table - exiting with error" );
        return BuildErrorXMLResponse( "106", "Must provide userID parameter for authentication." );
    }

    if( !(m_accountDB.GetAccountInfo( accountID, accountInfoList )) )
    {
        sLog.Error( "APIAccountManager::_AccountStatus()", "ERROR: Could not find 'accountID' in 'account' table; there is an invalid 'accountID' referenced by api account 'userID' = %s - exiting with error", pAPICommandCall->find( "userid" )->second.c_str() );
        return BuildErrorXMLResponse( "106", "Must provide userID parameter for authentication." );
    }

    _BuildXMLHeader();
    {
        _BuildXMLTag( "result" );
        {
            _BuildSingleXMLTag( "paidUntil", "2100-12-31 12:00:00" );
            _BuildSingleXMLTag( "createDate", "2000-01-01 12:00:00" );
            _BuildSingleXMLTag( "logonCount", accountInfoList.at(2) );
            _BuildSingleXMLTag( "logonMinutes", "1000" );
        }
        _CloseXMLTag(); // close tag "result"
    }
    _CloseXMLHeader( EVEAPI::CacheStyles::Short );

    return _GetXMLDocumentString();
}

std::string APIAccountManager::_GenerateAPIKey()
{
    std::string key = "";

    for( int i=0; i<64; i++ )
        key += m_hexCharMap.substr(static_cast<uint16>(MakeRandomInt(0, 15)), 1);

    return key;
}
