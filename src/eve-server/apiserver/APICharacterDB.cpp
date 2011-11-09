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


APICharacterDB::APICharacterDB()
{
}

bool APICharacterDB::GetCharacterSkillsTrained(uint32 characterID, std::vector<std::string> & skillTypeIDList, std::vector<std::string> & skillPointsList,
    std::vector<std::string> & skillLevelList, std::vector<std::string> & skillPublishedList)
{
	DBQueryResult res;

	// Get list of characters and their corporation info from the accountID:
	if( !sDatabase.RunQuery(res,
        " SELECT "
        "   entity.itemID, "
        "   entity.typeID, "
        "   entity_attributes.attributeID, "
        "   entity_attributes.valueInt, "
        "   invTypes.groupID, "
        "   invTypes.published, "
        "   invGroups.categoryID "
        " FROM `entity` "
        "   LEFT JOIN entity_attributes ON entity_attributes.itemID = entity.itemID "
        "   LEFT JOIN invTypes ON invTypes.typeID = entity.typeID "
        "   LEFT JOIN invGroups ON invGroups.groupID = invTypes.groupID "
        " WHERE `ownerID` = %u AND invGroups.categoryID = 16 ", characterID ))
	{
		sLog.Error( "APIAccountDB::GetCharacterSkillsTrained()", "Cannot find characterID %u", characterID );
		return false;
	}

    uint32 prevTypeID = 0;
	DBResultRow row;
    std::map<std::string, std::string> charInfo;
    while( res.GetRow( row ) )
    {
        if( prevTypeID != row.GetUInt(1) )
        {
            skillTypeIDList.push_back( std::string(row.GetText(1)) );
            skillPublishedList.push_back( std::string(row.GetText(5)) );
        }

        prevTypeID = row.GetUInt(1);
        
        if( row.GetUInt(2) == AttrSkillPoints )
            skillPointsList.push_back( std::string((row.GetText(3) == NULL ? "" : row.GetText(3))) );

        if( row.GetUInt(2) == AttrSkillLevel )
            skillLevelList.push_back( std::string((row.GetText(3) == NULL ? "" : row.GetText(3))) );
    }

	return true;
}

bool APICharacterDB::GetCharacterInfo(uint32 characterID, std::vector<std::string> & charInfoList)
{
	DBQueryResult res;

	// Get list of characters and their corporation info from the accountID:
	if( !sDatabase.RunQuery(res,
        " SELECT "
        "  character_.*, "
        "  chrAncestries.ancestryName, "
        "  chrBloodlines.bloodlineName, "
        "  chrRaces.raceName, "
        "  entity.itemName, "
        "  corporation.corporationName "
        " FROM character_ "
        "  LEFT JOIN chrAncestries ON character_.ancestryID = chrAncestries.ancestryID "
        "  LEFT JOIN chrBloodlines ON chrAncestries.bloodlineID = chrBloodlines.bloodlineID "
        "  LEFT JOIN chrRaces ON chrBloodlines.raceID = chrRaces.raceID "
        "  LEFT JOIN entity ON entity.itemID = character_.characterID "
        "  LEFT JOIN corporation ON corporation.corporationID = character_.corporationID "
        " WHERE character_.characterID = %u ", characterID ))
	{
		sLog.Error( "APIAccountDB::GetCharacterSkillsTrained()", "Cannot find characterID %u", characterID );
		return false;
	}

	DBResultRow row;
	if( !res.GetRow(row) )
	{
		sLog.Error( "APIServiceDB::GetAccountIdFromUsername()", "res.GetRow(row) failed for unknown reason." );
		return false;
	}

    charInfoList.push_back( std::string(row.GetText(5)) );      // 0. Balance
    charInfoList.push_back( std::string(row.GetText(9)) );      // 1. Skill Points
    charInfoList.push_back( std::string(row.GetText(10)) );     // 2. corporationID
    charInfoList.push_back( std::string(row.GetText(11)) );     // 3. corp Role
    charInfoList.push_back( std::string(row.GetText(12)) );     // 4. roles At All
    charInfoList.push_back( std::string(row.GetText(13)) );     // 5. roles At Base
    charInfoList.push_back( std::string(row.GetText(14)) );     // 6. roles At HQ
    charInfoList.push_back( std::string(row.GetText(15)) );     // 7. roles At Other
    charInfoList.push_back( std::string(row.GetText(17)) );     // 8. birthday
    charInfoList.push_back( std::string(row.GetText(32)) );     // 9. ancestry Name
    charInfoList.push_back( std::string(row.GetText(33)) );     // 10. bloodline Name
    charInfoList.push_back( std::string(row.GetText(34)) );     // 11. race Name
    charInfoList.push_back( std::string(row.GetText(35)) );     // 12. char Name
    charInfoList.push_back( std::string(row.GetText(36)) );     // 13. corp Name
    charInfoList.push_back( std::string(row.GetText(23)) );     // 14. gender (0 = female, 1 = male)

	return true;
}

bool APICharacterDB::GetCharacterImplants(uint32 characterID, std::map<std::string, std::string> * implantList)
{
    return false;
}

bool APICharacterDB::GetCharacterAttributes(uint32 characterID, std::map<std::string, std::string> * attribList)
{
    return false;
}

bool APICharacterDB::GetCharacterCertificates(uint32 characterID, std::vector<std::string> * certList)
{
    return false;
}

bool APICharacterDB::GetCharacterCorporationRoles(uint32 characterID, std::string roleType, std::map<std::string, std::string> * roleList)
{
    return false;
}



/*
bool APIServiceDB::GetAccountIdFromUsername(std::string username, std::string * accountID)
{
	DBQueryResult res;

	// Find accountID in 'account' table using accountName:
	if( !sDatabase.RunQuery(res,
		"SELECT"
		"	accountID "
		" FROM account "
        " WHERE accountName='%s'" , username.c_str() ))
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
        " WHERE accountID='%s'" , accountID.c_str() ))
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
        " WHERE userID='%s'" , userID.c_str() ))
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

	*apiFullKey = row.GetText(0);		// Grab Full API Key from retrieved row from the 'accountApi' table
	*apiLimitedKey = row.GetText(1);	// Grab Limited API Key from retrieved row from the 'accountApi' table
	*apiRole = row.GetUInt(2);			// Grab API Role from retrieved row from the 'accountApi' table
	return true;
}

bool APIServiceDB::UpdateUserIdApiKeyDatabaseRow(uint32 userID, std::string apiFullKey, std::string apiLimitedKey)
{
	// Check key lengths and report error and return if either are incorrect:
	if( apiLimitedKey.length() != 64 )
	{
		sLog.Error( "APIServiceDB::UpdateUserIdApiKeyDatabaseRow()", "limitedApiKey length != 64, but rather %u", apiLimitedKey.length() );
		return false;
	}

	if( apiFullKey.length() != 64 )
	{
		sLog.Error( "APIServiceDB::UpdateUserIdApiKeyDatabaseRow()", "fullApiKey length != 64, but rather %u", apiFullKey.length() );
		return false;
	}

	// Update fullKey and limitedKey in the 'accountApi' table using userID:
	DBerror err;

	if( !sDatabase.RunQuery(err,
		"UPDATE"
		" accountApi"
		" SET fullKey = '%s', limitedKey = '%s'"
		" WHERE userID = %u",
		apiFullKey.c_str(), apiLimitedKey.c_str(), userID ))
	{
		sLog.Error( "", "Error in query: %s.", err.c_str());
		return false;
	}
	else
		return true;
}

bool APIServiceDB::InsertNewUserIdApiKeyInfoToDatabase(uint32 accountID, std::string apiFullKey, std::string apiLimitedKey, uint32 apiRole)
{
	// Check key lengths and report error and return if either are incorrect:
	if( apiLimitedKey.length() != 64 )
	{
		sLog.Error( "APIServiceDB::UpdateUserIdApiKeyDatabaseRow()", "limitedApiKey length != 64, but rather %u", apiLimitedKey.length() );
		return false;
	}
	
	if( apiFullKey.length() != 64 )
	{
		sLog.Error( "APIServiceDB::UpdateUserIdApiKeyDatabaseRow()", "fullApiKey length != 64, but rather %u", apiFullKey.length() );
		return false;
	}

	DBerror err;

	if( !sDatabase.RunQuery(err,
		"INSERT INTO"
		" accountApi ("
		"	accountID, fullKey, limitedKey, apiRole"
		" ) VALUES ("
		"	%u, '%s', '%s', %u"
		" )", 
			accountID, apiFullKey.c_str(), apiLimitedKey.c_str(), apiRole
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
		" WHERE userID = %u",
		apiRole, userID ))
	{
		sLog.Error( "", "Error in query: %s.", err.c_str());
		return false;
	}
	else
		return true;
}
*/
