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
	Author:		Zhur
*/

#include "EVEServerPCH.h"

bool CommandDB::ItemSearch(const char *query, std::map<uint32, std::string> &into) {
	
	std::string escaped;
	sDatabase.DoEscapeString(escaped, query);

	DBQueryResult result;
	DBResultRow row;
	
	into.clear();

	//we need to query out the primary message here... not sure how to properly
	//grab the "main message" though... the text/plain clause is pretty hackish.
	if (!sDatabase.RunQuery(result,
		" SELECT typeID,typeName"
		" FROM invTypes"
		" WHERE"
		"  typeName rlike '%s'",
			escaped.c_str()
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", result.error.c_str());
		return (false);
	}
	
	while(result.GetRow(row)) {
		into[row.GetUInt(0)] = row.GetText(1);
	}
	return true;
}

bool CommandDB::ItemSearch(uint32 typeID, uint32 &actualTypeID,
    std::string &actualTypeName, uint32 &actualGroupID, uint32 &actualCategoryID, double &actualRadius)
{
	DBQueryResult result;
	DBResultRow row;
	
    if (!sDatabase.RunQuery(result,
        "SELECT  "
        " invTypes.typeID,"
        " invTypes.typeName,"
        " invTypes.groupID,"
        " invTypes.radius,"
        " invGroups.categoryID"
        " FROM invTypes"
        "  LEFT JOIN invGroups"
        "   ON invGroups.groupID = invTypes.groupID"
        " WHERE typeID = %u",
        typeID
		))
	{
        sLog.Error( "CommandDB::ItemSearch()", "Error in query: %s", result.error.c_str() );
		return (false);
	}

    if( !result.GetRow(row) )
    {
        sLog.Error( "CommandDB::ItemSearch()", "Query returned NO results: %s", result.error.c_str() );
		return (false);
    }

    // Extract values from the first row:
    actualTypeID = row.GetUInt( 0 );
    actualTypeName = row.GetText( 1 );
    actualGroupID = row.GetUInt( 2 );
    actualCategoryID = row.GetUInt( 4 );
    actualRadius = row.GetDouble( 3 );

    return true;
}

bool CommandDB::GetRoidDist(const char * sec, std::map<double, uint32> &roids) {
	DBQueryResult res;
	DBResultRow row;

	if (!sDatabase.RunQuery(res,
		" SELECT roidID, percent FROM roidDistribution WHERE systemSec = '%s' ", sec))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}

	double tot = 0.0;
	while (res.GetRow(row)) {
		tot += row.GetDouble(1);
		roids[tot] = row.GetUInt(0);
	}

	return !roids.empty();
}

int CommandDB::GetAttributeID(const char *attributeName) {

	DBQueryResult res;
	DBResultRow row;
	std::string escape;
	sDatabase.DoEscapeString(escape, attributeName);

	if(!sDatabase.RunQuery(res,
		" SELECT "
		" attributeID "
		" FROM dgmAttributeTypes "
		" WHERE attributeName = '%s' ",
		escape.c_str() ) )
	{
		_log(DATABASE__ERROR, "Error retrieving attributeID for attributeName = '%s' ", escape.c_str() );
		return 0;
	}

	if( !res.GetRow(row) ){
		_log(DATABASE__ERROR, "Null result finding attributeID for attributeName = '%s' ", escape.c_str() );
		return 0;
	}

	return row.GetUInt( 0 );

}

int CommandDB::GetAccountID(std::string name) {
	
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		" SELECT "
		" AccountID "
		" FROM character_ "
		" WHERE characterID = ( SELECT itemID FROM entity WHERE itemName = '%s' )", name.c_str()))
	{
		sLog.Error("CommandDB", "Failed to retrieve accountID for %s", name.c_str());
		return 0;
	}

	DBResultRow row;

	if( !res.GetRow(row) )
	{
		sLog.Error("CommandDB", "Query Returned no results");
		return 0;
	}

	return row.GetInt( 0 );
	
}
























