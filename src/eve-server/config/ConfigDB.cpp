/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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
    Author:     Zhur
    Updates:    Allan
*/

#include "eve-server.h"

#include "config/ConfigDB.h"

PyRep *ConfigDB::GetMultiOwnersEx(const std::vector<int32> &entityIDs) {
    // separate list of ids into respective groups
    std::vector<int32> player, corp, ally;
    player.clear();
    corp.clear();
    ally.clear();

    for (auto cur : entityIDs) {
        if (IsCorp(cur))
            corp.push_back(cur);
        else if (IsAlliance(cur))
            ally.push_back(cur);
        else
            player.push_back(cur);
    }

    DBQueryResult res;
    std::string ids = "";

    if (corp.size()) {
        ListToINString(corp, ids, "0");

        if (!sDatabase.RunQuery(res,
            "SELECT "
            "  corporationID as ownerID,"
            "  corporationName as ownerName,"
            "  2 AS typeID,"                    // corp typeID
            "  NULL AS gender,"
            "  NULL AS ownerNameID"
            " FROM corporation"
            " WHERE corporationID IN (%s)", ids.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
            // DONT return on error...may be more in list.
        }
        ids = "";
    }
    /* wait on this one, as this table doesnt exist yet in this version...but neither do alliances.
    if (ally.size()) {
        ListToINString(ally, ids, "0");

        if (!sDatabase.RunQuery(res,
            "SELECT "
            "  allianceID as ownerID,"
            "  allianceShortName as ownerName,"
            "  16159 AS typeID,"                 // alliance typeID.
            "  NULL AS gender,"
            "  NULL AS ownerNameID"
            " FROM crpAlliance"
            " WHERE allianceID IN (%s)", ids.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        }
        ids = "";
    } */

    if (player.size()) {
        ListToINString(player, ids, "0");

        if (!sDatabase.RunQuery(res,
            "SELECT "
            "  c.characterID as ownerID,"
            "  e.itemName as ownerName,"
            "  e.typeID,"
            "  c.gender,"
            "  NULL AS ownerNameID"
            " FROM character_ AS c"
            "  LEFT JOIN entity AS e ON e.itemID = c.characterID"
            " WHERE characterID IN (%s)", ids.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        }
    }

    return DBResultToTupleSet(res);
}

PyRep *ConfigDB::GetMultiAllianceShortNamesEx(const std::vector<int32> &entityIDs) {

    //im not sure how this query is supposed to work, as far as what table
    //we use to get the fields from.

    std::string ids;
    ListToINString(entityIDs, ids, "-1");

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        " entity.itemID as allianceID,"
        " entity.itemName as shortName" //we likely need to use customInfo or something for this.
        " FROM entity "
        " WHERE entity.typeID = %d"
        "   AND itemID in (%s)",
            AllianceTypeID,
            ids.c_str()
        ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return(DBResultToTupleSet(res));
}


PyRep *ConfigDB::GetMultiLocationsEx(const std::vector<int32> &entityIDs) {

    //im not sure how this query is supposed to work, as far as what table
    //we use to get the fields from.

    bool use_map = false;
    if(!entityIDs.empty()) {
        //this is a big of a hack at this point... basically
        //we are assuming that they only query locations for map items
        // and non-map items disjointly....
        use_map = IsStaticMapItem(entityIDs[0]);
    }

    std::string ids;
    ListToINString(entityIDs, ids, "-1");

    DBQueryResult res;

    if(use_map) {
        if(!sDatabase.RunQuery(res,
            "SELECT "
            " mapDenormalize.itemID AS locationID,"
            " mapDenormalize.itemName AS locationName,"
            " mapDenormalize.x AS x,"
            " mapDenormalize.y AS y,"
            " mapDenormalize.z AS z,"
            " NULL AS locationNameID"
            " FROM mapDenormalize "
            " WHERE itemID in (%s)", ids.c_str()))
        {
            codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
            return NULL;
        }
    } else {
        if(!sDatabase.RunQuery(res,
            "SELECT "
            " entity.itemID AS locationID,"
            " entity.itemName AS locationName,"
            " entity.x AS x,"
            " entity.y AS y,"
            " entity.z AS z,"
            " NULL AS locationNameID"
            " FROM entity "
            " WHERE itemID in (%s)", ids.c_str()))
        {
            codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
            return NULL;
        }
    }

    //return(DBResultToRowset(res));
    return(DBResultToTupleSet(res));
}


PyRep *ConfigDB::GetMultiCorpTickerNamesEx(const std::vector<int32> &entityIDs) {

    std::string ids;
    ListToINString(entityIDs, ids, "-1");

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   corporationID, tickerName, "
        "   shape1, shape2, shape3,"
        "   color1, color2, color3 "
        " FROM corporation "
        " WHERE corporationID in (%s)", ids.c_str()))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return(DBResultToRowList(res));
}


PyRep *ConfigDB::GetMultiGraphicsEx(const std::vector<int32> &entityIDs) {

    std::string ids;
    ListToINString(entityIDs, ids, "-1");

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "   graphicID,url3D,urlWeb,icon,urlSound,explosionID"
        " FROM eveGraphics "
        " WHERE graphicID in (%s)", ids.c_str()))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return(DBResultToRowList(res));
}

PyObject *ConfigDB::GetUnits() {

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        " unitID,unitName,displayName"
        " FROM eveUnits "))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return(DBResultToIndexRowset(res, "unitID"));
}

PyObjectEx *ConfigDB::GetMapObjects(uint32 entityID, bool wantRegions,
    bool wantConstellations, bool wantSystems, bool wantStations)
{
    const char *key = "solarSystemID";
    if(wantRegions) {
        entityID = 3;   /* a little hackish... */
        key = "typeID";
    } else if(wantConstellations) {
        key = "regionID";
    } else if(wantSystems) {
        key = "constellationID";
    } else if(wantStations) {
        key = "solarSystemID";
    }

    DBQueryResult res;

    //TODO: hacked 'connector' field in GetMapObjects

    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   groupID,typeID,itemID,itemName,solarSystemID AS locationID,"
        "   orbitID,"
        "   0 as connector,"
        "   x,y,z"
        " FROM mapDenormalize"
        " WHERE %s=%u", key, entityID
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToCRowset(res);
//    return DBResultToRowset(res);
}

PyObject *ConfigDB::GetMap(uint32 solarSystemID) {
    DBQueryResult res;

    //TODO: a lot of missing data in GetMap

    //how in the world do they get a list in the freakin rowset for destinations???
    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   s.solarSystemID AS locationID,"
        "   s.xMin AS xMin,"
        "   s.xMax AS xMax,"
        "   s.yMin AS yMin,"
        "   s.yMax AS yMax,"
        "   s.zMin AS zMin,"
        "   s.zMax AS zMax,"
        "   s.luminosity AS luminosity,"
        "   d.x AS x, d.y AS y, d.z AS z,"
        "   d.itemID,"
        "   d.itemName,"
        "   d.typeID,"
        "   d.groupID,"
        "   d.orbitID AS orbitID,"
        "   j.celestialID AS destinations"
        " FROM mapSolarSystems AS s"
        "  LEFT JOIN mapDenormalize AS d USING (solarSystemID)"
        "  LEFT JOIN mapJumps AS j ON j.stargateID = d.itemID"
		" WHERE solarSystemID=%u", solarSystemID
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *ConfigDB::ListLanguages() {
    DBQueryResult res;

    //how in the world do they get a list in the freakin rowset for destinations???
    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   languageID,languageName,translatedLanguageName"
        " FROM languages"
    ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}


PyRep *ConfigDB::GetMultiInvTypesEx(const std::vector<int32> &entityIDs) {

    std::string ids;
    ListToINString(entityIDs, ids, "-1");

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "   typeID,groupID,typeName,description,graphicID,radius,"
        "   mass,volume,capacity,portionSize,raceID,basePrice,"
        "   published,marketGroupID,chanceOfDuplicating "
        " FROM invTypes "
        " WHERE typeID in (%s)", ids.c_str()))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return(DBResultToRowList(res));
}

PyRep *ConfigDB::GetStationSolarSystemsByOwner(uint32 ownerID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        " corporationID, solarSystemID "
        " FROM staStations "
        " WHERE corporationID = %u ", ownerID
        ))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyRep *ConfigDB::GetCelestialStatistic(uint32 celestialID) {
    DBQueryResult res;
    DBResultRow row;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        " groupID "
        " FROM mapDenormalize "
        " WHERE itemID = %u ", celestialID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    if (!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Unable to find celestial object %u", celestialID);
        return NULL;
    }
    uint32 groupID = row.GetUInt(0);

    std::string query = "";

    switch (groupID) {
    case EVEDB::invGroups::Sun:
            query = " SELECT "
                    "    temperature, "
                    "    spectralClass, "
                    "    luminosity, "
                    "    age, "
                    "    radius "
                    " FROM mapCelestialStatistics "
                    " WHERE celestialID = %u ";
            break;
    case EVEDB::invGroups::Planet:
            query = " SELECT "
                    "     temperature, "
                    "    orbitRadius, "
                    "    eccentricity, "
                    "    massDust, "
                    "    density, "
                    "    surfaceGravity, "
                    "    escapeVelocity, "
                    "    orbitPeriod, "
                    "    pressure, "
                    "    radius "
                    " FROM mapCelestialStatistics "
                    " WHERE celestialID = %u ";
            break;
    case EVEDB::invGroups::Moon:
            query = " SELECT "
                    "    temperature, "
                    "    orbitRadius, "
                    "    massDust, "
                    "    density, "
                    "    surfaceGravity, "
                    "    escapeVelocity, "
                    "    orbitPeriod, "
                    "    pressure, "
                    "    radius "
                    " FROM mapCelestialStatistics "
                    " WHERE celestialID = %u ";
            break;
    case EVEDB::invGroups::Asteroid_Belt:
            query = " SELECT "
                    "    orbitRadius, "
                    "    eccentricity, "
                    "    massDust, "
                    "    density, "
                    "    orbitPeriod "
                    " FROM mapCelestialStatistics "
                    " WHERE celestialID = %u ";
            break;

    default:
            codelog(SERVICE__ERROR, "Invalid object groupID (%u) for %u", groupID, celestialID);
            return (NULL);
    }

    if (!sDatabase.RunQuery(res, query.c_str(), celestialID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToCRowset(res);
}

PyRep *ConfigDB::GetDynamicCelestials(uint32 solarSystemID) {

    sLog.Error("ConfigDB::GetDynamicCelestials", "This query is intentionally made to yield an empty result.  It must be re-worked to provide actual data!");

    const std::string query = " SELECT "
                              "     `itemID`, "
                              "     `entity`.`typeID`, "
                              "     `invTypes`.`groupID`, "
                              "     `itemName`, "
                              "     0, " // This field refers to the orbitID of the dynamic celestial and needs to be implemented
                              "     0, " // This field refers to the boolean value of isConnector. It may signify some sort of mission jump bridge
                              "     `x`, "
                              "     `y`, "
                              "     `z` "
                              " FROM `entity` JOIN `invTypes` ON `entity`.`typeID` = `invTypes`.`typeID`"
                              " WHERE "
                              "     `locationID` = %u AND " // In the future, the locationID field needs to be constrained to being a solarSystemID
                              "     `groupID` = -1"; // This is set to -1 because we do not know what the ID(s) of dynamic celestials is/are.

    DBQueryResult result;
    DBResultRow currentRow;

    if (!sDatabase.RunQuery(result, query.c_str(), solarSystemID))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", result.error.c_str());
        return NULL;
    }

    return DBResultToRowset(result);
}

PyRep *ConfigDB::GetTextsForGroup(const std::string & langID, uint32 textgroup) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT textLabel, `text` FROM intro WHERE langID = '%s' AND textgroup = %u", langID.c_str(), textgroup))
    {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}
