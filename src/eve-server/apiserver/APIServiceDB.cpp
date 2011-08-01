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


APIServiceDB::APIServiceDB()
{
}

bool APIServiceDB::GetAccountIdFromUsername(std::string username, std::string * accountID)
{
	DBQueryResult res;

	// Find accountID in 'account' table using accountName:
	if( !sDatabase.RunQuery(res,
		"SELECT"
		"	accountID "
		" FROM account "
        " WHERE accountName=%s" , username.c_str() ))
	{
		sLog.Error( "APIServiceDB::GetAccountIdFromUsername()", "Cannot find accountID for username %s", username.c_str() );
		return false;
	}

	DBResultRow row;
	if( !res.GetRow(row) )
	{
		sLog.Error( "APIServiceDB::GetAccountIdFromUsername()", "res.GetRow(row) failed for unknown reason." );
		return false;
	}

	*accountID = row.GetText(0);		// Grab accountID from the retrieved row from the 'account' table
	return true;
}

bool APIServiceDB::GetApiAccountInfoUsingAccountID(std::string accountID, uint32 * userID, std::string * apiFullKey,
    std::string * apiLimitedKey, uint32 * apiRole)
{
	DBQueryResult res;

	// Find userID, fullKey, limitedKey, and apiRole from 'accountApi' table using accountID obtained from 'account' table:
	if( !sDatabase.RunQuery(res,
		"SELECT"
		"	userID, fullKey, limitedKey, apiRole "
		" FROM accountApi "
        " WHERE accountID=%s" , accountID.c_str() ))
	{
		sLog.Error( "APIServiceDB::GetApiAccountInfoUsingAccountID()", "Cannot find accountID '%s' in 'accountApi' table", accountID.c_str() );
		return false;
	}

	DBResultRow row;
	if( !res.GetRow(row) )
	{
		sLog.Error( "APIServiceDB::GetApiAccountInfoUsingAccountID()", "res.GetRow(row) failed for unknown reason." );
		return false;
	}

	*userID = row.GetUInt(0);			// Grab userID from retrieved row from the 'accountApi' table
	*apiFullKey = row.GetText(1);		// Grab Full API Key from retrieved row from the 'accountApi' table
	*apiLimitedKey = row.GetText(2);	// Grab Limited API Key from retrieved row from the 'accountApi' table
	*apiRole = row.GetUInt(3);			// Grab API Role from retrieved row from the 'accountApi' table
	return true;
}

bool APIServiceDB::GetApiAccountInfoUsingUserID(std::string userID, std::string * apiFullKey, std::string * apiLimitedKey, uint32 * apiRole)
{
	DBQueryResult res;

	// Find fullKey, limitedKey, and apiRole from 'accountApi' table using userID supplied from an API query string:
	if( !sDatabase.RunQuery(res,
		"SELECT"
		"	fullKey, limitedKey, apiRole "
		" FROM accountApi "
        " WHERE userID=%s" , userID.c_str() ))
	{
		sLog.Error( "APIServiceDB::GetApiAccountInfoUsingUserID()", "Cannot find userID '%s' in 'accountApi' table", userID.c_str() );
		return false;
	}

	DBResultRow row;
	if( !res.GetRow(row) )
	{
		sLog.Error( "APIServiceDB::GetApiAccountInfoUsingUserID()", "res.GetRow(row) failed for unknown reason." );
		return false;
	}

	*apiFullKey = row.GetText(1);		// Grab Full API Key from retrieved row from the 'accountApi' table
	*apiLimitedKey = row.GetText(2);	// Grab Limited API Key from retrieved row from the 'accountApi' table
	*apiRole = row.GetUInt(3);			// Grab API Role from retrieved row from the 'accountApi' table
	return true;
}

bool APIServiceDB::UpdateUserIdApiKeyDatabaseRow(uint32 userID, std::string limitedApiKey, std::string fullApiKey)
{
	// Check key lengths and report error and return if either are incorrect:
	if( limitedApiKey.length() != 64 )
	{
		sLog.Error( "APIServiceDB::UpdateUserIdApiKeyDatabaseRow()", "limitedApiKey length != 64, but rather %u", limitedApiKey.length() );
		return false;
	}

	if( fullApiKey.length() != 64 )
	{
		sLog.Error( "APIServiceDB::UpdateUserIdApiKeyDatabaseRow()", "fullApiKey length != 64, but rather %u", fullApiKey.length() );
		return false;
	}

	// Update fullKey and limitedKey in the 'accountApi' table using userID:
	DBerror err;

	if( !sDatabase.RunQuery(err,
		"UPDATE"
		" accountApi"
		" SET fullKey = %s, limitedKey = %s"
		" WHERE userID = %u",
		fullApiKey.c_str(), limitedApiKey.c_str(), userID ))
	{
		sLog.Error( "", "Error in query: %s.", err.c_str());
		return false;
	}
	else
		return true;
}

bool APIServiceDB::InsertNewUserIdApiKeyInfoToDatabase(uint32 userID, std::string limitedApiKey, std::string fullApiKey, uint32 apiRole)
{
	// Check key lengths and report error and return if either are incorrect:
	if( limitedApiKey.length() != 64 )
	{
		sLog.Error( "APIServiceDB::UpdateUserIdApiKeyDatabaseRow()", "limitedApiKey length != 64, but rather %u", limitedApiKey.length() );
		return false;
	}
	
	if( fullApiKey.length() != 64 )
	{
		sLog.Error( "APIServiceDB::UpdateUserIdApiKeyDatabaseRow()", "fullApiKey length != 64, but rather %u", fullApiKey.length() );
		return false;
	}

	if( (userID > 9999999) || (userID < 1000000) )
	{
		sLog.Error( "APIServiceDB::UpdateUserIdApiKeyDatabaseRow()", "userID of %u has length that is not 7 digits", fullApiKey.length() );
		return false;
	}

	DBerror err;

	if( !sDatabase.RunQuery(err,
		"INSERT INTO"
		" accountApi ("
		"	userID, fullKey, limitedKey, apiRole"
		" ) VALUES ("
		"	%u, %s, %s, %u"
		" )", 
			userID, fullApiKey.c_str(), limitedApiKey.c_str(), apiRole
			))
	{
		sLog.Error( "", "Error in query: %s.", err.c_str());
		return false;
	}
	else
		return true;
}

bool APIServiceDB::UpdateUserIdApiRole(uint32 userID, uint32 apiRole)
{
	// Update fullKey and limitedKey in the 'accountApi' table using userID:
	DBerror err;

	if( !sDatabase.RunQuery(err,
		"UPDATE"
		" accountApi"
		" SET apiRole = %u"
		" WHERE userID = %s",
		apiRole, userID ))
	{
		sLog.Error( "", "Error in query: %s.", err.c_str());
		return false;
	}
	else
		return true;
}

