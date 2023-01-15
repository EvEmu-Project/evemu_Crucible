/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        Zhur
*/

/** @todo  update this...  */

#include "eve-server.h"

#include "admin/CommandDB.h"

uint32_t CommandDB::GetSolarSystem(const char *name) {
    //sDatabase.ReplaceSlash(name);
    if (!sDatabase.IsSafeString(name))
        return 0;

    std::string escaped;
    sDatabase.DoEscapeString(escaped, name);

    DBQueryResult result;
    if (!sDatabase.RunQuery(result,
                            "SELECT solarSystemID "
                            "FROM mapSolarSystems "
                            "WHERE solarSystemName LIKE '%%%s%%';",
                            escaped.c_str()
                            )) {
        codelog(DATABASE__ERROR, "Error in query: %s", result.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!result.GetRow(row)) {
        codelog(COMMAND__ERROR, "Solar System query returned nothing");
        return 0;
    }

    return row.GetUInt(0);
}

uint32_t CommandDB::GetStation(const char* name)
{
    if (!sDatabase.IsSafeString(name))
        return 0;

    std::string escaped;
    sDatabase.DoEscapeString(escaped, name);

    DBQueryResult result;
    if (!sDatabase.RunQuery(result,
                "SELECT `stationID`"
                " FROM `staStations`"
                " WHERE `stationName` LIKE '%%%s%%';",
                escaped.c_str()
        )) {
        codelog(DATABASE__ERROR, "Error in query: %s", result.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!result.GetRow(row)) {
        codelog(COMMAND__ERROR, "Station query returned nothing");
        return 0;
    }

    return row.GetUInt(0);
}


bool CommandDB::ItemSearch(const char *query, std::map<uint32, std::string> &into) {

    into.clear();

    //sDatabase.ReplaceSlash(query);
    if (!sDatabase.IsSafeString(query))
        return 0;

    std::string escaped;
    sDatabase.DoEscapeString(escaped, query);


    //we need to query out the primary message here... not sure how to properly
    //grab the "main message" though... the text/plain clause is pretty hackish.
    DBQueryResult result;
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

    DBResultRow row;
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

    if ( !result.GetRow(row) )
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

int CommandDB::GetAttributeID(const char *attributeName) {

    if (!sDatabase.IsSafeString(attributeName))
        return 0;

    DBQueryResult res;
    std::string escape;
    sDatabase.DoEscapeString(escape, attributeName);

    if (!sDatabase.RunQuery(res,
        " SELECT "
        " attributeID "
        " FROM dgmAttributeTypes "
        " WHERE attributeName = '%s' ",
        escape.c_str() ) )
    {
        codelog(DATABASE__ERROR, "Error retrieving attributeID for attributeName = '%s' ", escape.c_str() );
        return 0;
    }

    DBResultRow row;
    if ( !res.GetRow(row) ){
        codelog(DATABASE__ERROR, "Null result finding attributeID for attributeName = '%s' ", escape.c_str() );
        return 0;
    }

    return row.GetUInt( 0 );

}

int CommandDB::GetAccountID(std::string name) {

    if (!sDatabase.IsSafeString(name))
        return 0;

    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT accountID FROM chrCharacters WHERE name = '%s'", name.c_str())) {
        sLog.Error("CommandDB", "Failed to retrieve accountID for %s", name.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        sLog.Error("CommandDB", "Query Returned no results");
        return 0;
    }

    return row.GetInt(0);
}

static std::array<uint32, 8> unpubSkills = {
    3755,  // Jove Frigate
    3758,  // Jove Cruiser
    9955,  // Polaris
    10264, // Concord
    11075, // Jove Industrial
    11078, // Jove Battleship
    19430, // Omnipotent
    28604, // Tournament Observation
};

bool CommandDB::FullSkillList(std::vector<uint32> &skillList) {
    skillList.clear();

    DBQueryResult result;
    if (!sDatabase.RunQuery(result,
        " SELECT typeID FROM `invTypes` WHERE "
		" ((`groupID` IN (SELECT groupID FROM invGroups WHERE categoryID = 16)) AND (published = 1)) "
        ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", result.error.c_str());
        return false;
    }

    DBResultRow row;
    while (result.GetRow(row)) {
		skillList.push_back( (row.GetInt(0)) );
    }

    PushNonPublishedSkills(skillList);

    return true;
}

/*
 * Returns a vector of pair: <skillID, currentLevel>
 * Avoids iterating over all the skills defined by building just a bit more complicated DB query so that the DB itself
 * just returns the final result which does not need to be processed another way than just putting it into the vector
 *
 * If the skill is not injected into character's skillbook it's currentLevel is reported -1
 */
bool CommandDB::NotFullyLearnedSkillList(CommandDB::charSkillStates &skillList, uint32 charID) {
    skillList.clear();

    DBQueryResult result;
    auto query = std::string();
    query.append(" WITH skillids AS (SELECT typeID "
                 "FROM `invTypes` "
                 "WHERE ((`groupID` IN ( "
                 "SELECT groupID FROM invGroups WHERE categoryID = %u)) AND (published = 1)) ");
    auto ups_begin = unpubSkills.begin(), ups_end = unpubSkills.end();
    if (ups_begin != ups_end) {
        query.append("OR typeID IN (");
        std::ostringstream types;
        types << *ups_begin;
        while (++ups_begin != ups_end) {
            types << "," << *ups_begin;
        }
        query.append(types.str());
        query.append(")");
    }
    query.append(") ");
    query.append(
        "SELECT si.typeID AS typeID, e.itemID AS itemID, ea.valueInt AS valueInt "
        "FROM skillids si "
        "LEFT OUTER JOIN entity e ON e.locationID = %u AND e.typeID = si.typeID "
        "LEFT OUTER JOIN entity_attributes ea ON e.itemID = ea.itemID AND ea.attributeID = %u "
        "WHERE (ea.valueInt < 5 OR ea.valueInt IS NULL)"
    );

    if (!sDatabase.RunQuery(result, query.c_str(),
                            EVEDB::invCategories::Skill, charID, AttrSkillLevel
    )) {
        codelog(SERVICE__ERROR, "Error in query: %s", result.error.c_str());
        return false;
    }

    DBResultRow row;
    while (result.GetRow(row)) {
        skillStateDescriptor value;
        value.first = row.GetUInt(0);
        if (row.IsNull(1)) {
            value.second = -1;
        } else {
            value.second = row.IsNull (2) ? 0 : row.GetInt(2);
        }
        skillList.push_back(value);
    }
    return true;
}

void CommandDB::PushNonPublishedSkills(std::vector<uint32> &skillList) {
    // Because we searched skills with published = 1 and some GM skills are not published but still usable,
    // we will add them manually here:
    for (auto skill: unpubSkills)
        skillList.push_back(skill);
}
