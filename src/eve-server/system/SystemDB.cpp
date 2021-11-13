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
    Updates:    Allan
*/

#include "eve-server.h"

#include "system/SystemDB.h"

PyObject* SystemDB::ListFactions() {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT factionID FROM facFactions")) {
        codelog(DATABASE__ERROR, "Error in ListFactions query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyObject* SystemDB::ListJumps(uint32 gateID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        "   celestialID AS toCelestialID,"
        "   solarSystemID AS locationID"
        " FROM mapJumps "
        "  LEFT JOIN mapDenormalize ON celestialID=itemID"
        " WHERE stargateID=%u", gateID))
    {
        codelog(DATABASE__ERROR, "Error in ListJumps query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyPackedRow* SystemDB::GetSolarSystemPackedRow(uint32 systemID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        " mss.solarSystemID,"
        " mss.solarSystemName,"
        " mss.x, mss.y, mss.z,"
        " mss.radius,"
        " mss.security,"
        " mss.constellationID,"
        " mss.factionID,"
        " mss.sunTypeID,"
        " mss.regionID,"
        " mlwc.wormholeClassID"
        " FROM mapSolarSystems AS mss"
        " LEFT JOIN mapLocationWormholeClasses AS mlwc ON mlwc.locationID = mss.regionID"
        " WHERE solarSystemID=%u", systemID ))
    {
        codelog(DATABASE__ERROR, "Error in GetSolarSystem query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Error in GetSolarSystem query: no solarsystem for id %u", systemID);
        return nullptr;
    }

    return DBRowToPackedRow(row);
}

GPoint SystemDB::GetSolarSystemPosition(uint32 systemID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT x,y,z"
        " FROM mapSolarSystems WHERE solarSystemID=%u",
        systemID))
    {
        codelog(DATABASE__ERROR, "Error in GetSolarSystemPosition query: %s", res.error.c_str());
        return NULL_ORIGIN;
    }

    _log(DATABASE__RESULTS, "GetSolarSystemPosition returned %lu items", res.GetRowCount());

    DBResultRow row;
    GPoint point(NULL_ORIGIN);
    if (res.GetRow(row))
        point = GPoint(row.GetDouble(0), row.GetDouble(1), row.GetDouble(2));

    return point;
}

bool SystemDB::LoadSystemStaticEntities(uint32 systemID, std::vector<DBSystemEntity>& into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT itemID,typeID,groupID,radius"
        " FROM mapDenormalize WHERE solarSystemID=%u ORDER BY itemID",
        systemID))
    {
        codelog(DATABASE__ERROR, "Error in LoadSystemStaticEntities query: %s", res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "LoadSystemStaticEntities returned %lu items", res.GetRowCount());

    DBResultRow row;
    while(res.GetRow(row)) {
        DBSystemEntity entry = DBSystemEntity();
        entry.itemID    = row.GetUInt(0);
        entry.typeID    = row.GetInt(1);
        entry.groupID   = row.GetInt(2);
        entry.radius    = row.GetDouble(3);
        into.push_back(entry);
    }

    return true;
}

/* load system dynamics owned by the EvE _System (1) and globals owned by Players or PlayerCorps  */
bool SystemDB::LoadSystemDynamicEntities(uint32 systemID, std::vector<DBSystemDynamicEntity>& into) {
    using namespace EVEDB::invCategories;
    DBQueryResult res, res2;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   e.itemID,"
        "   e.itemName,"
        "   e.ownerID,"
        "   e.typeID,"
        "   t.groupID,"
        "   g.categoryID,"
        "   e.x, e.y, e.z,"
        "   IFNULL(e.customInfo, '0')"
        " FROM entity AS e"
        "  LEFT JOIN invTypes AS t USING (typeID)"
        "  LEFT JOIN invGroups AS g USING (groupID)"
        " WHERE e.locationID = %u"
        "  AND g.categoryID NOT IN (%u,%u,%u,%u)"    // not Characters, stations, or roids
        "  AND (e.ownerID = 1"      // get dynamics owned by the system -include abandonded ships
        "  OR g.categoryID IN (%u,%u))"   // or orbitals and SOV structs owned by pc corps
        "  ORDER BY e.itemID",
        systemID,
        //exclude categories not applicable for in-space system entities or owned by player/corp :
        _System/*0*/, /*Character*/1, /*Station*/3, Asteroid/*25*/, //asteroids are owned/controlled by BeltMgr.
        Orbitals/*46*/,SovereigntyStructure/*40*/ )) {
            codelog(DATABASE__ERROR, "Error in LoadSystemDynamicEntities query: %s", res.error.c_str());
            return false;
        }

    _log(DATABASE__RESULTS, "LoadSystemDynamicEntities returned %lu items", res.GetRowCount());

    DBResultRow row, row2;
    while(res.GetRow(row)) {
        DBSystemDynamicEntity entry = DBSystemDynamicEntity();
        entry.itemID        = row.GetUInt(0);
        entry.itemName      = row.GetText(1);
        entry.ownerID       = row.GetInt(2);
        entry.typeID        = row.GetUInt(3);
        entry.groupID       = row.GetUInt(4);
        entry.categoryID    = row.GetUInt(5);
        GPoint pos(row.GetDouble(6), row.GetDouble(7), row.GetDouble(8));
        entry.position      = pos;
        entry.planetID      = atoi(row.GetText(9));

        if (IsCorp(entry.ownerID)) {
            entry.corporationID = entry.ownerID;
            if (sDatabase.RunQuery(res2, "SELECT allianceID, warFactionID FROM crpCorporation WHERE corporationID = %u", entry.corporationID))
                if (res2.GetRow(row2)) {
                    entry.allianceID    = row2.GetUInt(0);
                    entry.factionID     = row2.GetUInt(1);
                }
        } else if (IsCharacterID(entry.ownerID)) {
            if (sDatabase.RunQuery(res2,
                "SELECT c.corporationID, co.allianceID, co.warFactionID FROM chrCharacters AS c"
                " LEFT JOIN crpCorporation AS co USING (corporationID)"
                " WHERE c.characterID = %u", entry.ownerID))
            {
                if (res2.GetRow(row2)) {
                    entry.corporationID = row2.GetUInt(0);
                    entry.allianceID    = row2.GetUInt(1);
                    entry.factionID     = row2.GetUInt(2);
                }
            }
        }
        into.push_back(entry);
    }

    return true;
}

/* load system dynamics owned by players  */
bool SystemDB::LoadPlayerDynamicEntities(uint32 systemID, std::vector<DBSystemDynamicEntity>& into)
{
    using namespace EVEDB::invCategories;
    DBQueryResult res, res2;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   e.itemID,"
        "   e.itemName,"
        "   e.typeID,"
        "   e.ownerID,"
        "   t.groupID,"
        "   g.categoryID,"  //5
        "   e.x, e.y, e.z" //8
        " FROM entity AS e"
        "  LEFT JOIN invTypes AS t USING (typeID)"
        "  LEFT JOIN invGroups AS g USING (groupID)"
        " WHERE e.locationID = %u"
        "  AND g.categoryID IN (%u,%u,%u,%u,%u,%u,%u)"
        "  AND e.ownerID != 1"  // get dynamics not owned by the system
        " ORDER BY e.itemID",   // should we order by category?  or group?
        systemID, Celestial/*2*/,   // Celestial is for containers (wrecks, jetcans, lsc)
        Charge /*8*/,   // this is for probes and spheres launched from ship (abandonded)
        Deployable/*22*/,           // include deployed items owned by players or corps
        Drone/*18*/, Entity/*11*/,  // Entity also contains NPCs, sentrys, LCOs, and other destructible objects
        /*Structure*/23, StructureUpgrade/*39*/)) {     // this is for POS and POCO
            codelog(DATABASE__ERROR, "Error in LoadPlayerDynamicEntities query: %s", res.error.c_str());
            return false;
    }

    _log(DATABASE__RESULTS, "LoadPlayerDynamicEntities returned %lu items", res.GetRowCount());
    DBResultRow row, row2;
    while(res.GetRow(row)) {
        DBSystemDynamicEntity entry = DBSystemDynamicEntity();
        entry.itemID        = row.GetUInt(0);
        entry.itemName      = row.GetText(1);
        entry.typeID        = row.GetUInt(2);
        entry.ownerID       = row.GetInt(3);
        entry.groupID       = row.GetUInt(4);
        entry.categoryID    = row.GetUInt(5);
        GPoint pos(row.GetDouble(6), row.GetDouble(7), row.GetDouble(8));
        entry.position      = pos;

        if (IsCorp(entry.ownerID)) {
            entry.corporationID = entry.ownerID;
            if (sDatabase.RunQuery(res2, "SELECT allianceID, warFactionID FROM crpCorporation WHERE corporationID = %u", entry.corporationID))
                if (res2.GetRow(row2)) {
                    entry.allianceID    = row2.GetUInt(0);
                    entry.factionID     = row2.GetUInt(1);
                }
        } else if (IsCharacterID(entry.ownerID)) {
            if (sDatabase.RunQuery(res2,
                "SELECT c.corporationID, co.allianceID, co.warFactionID FROM chrCharacters AS c"
                " LEFT JOIN crpCorporation AS co USING (corporationID)"
                " WHERE c.characterID = %u", entry.ownerID))
            {
                if (res2.GetRow(row2)) {
                    entry.corporationID = row2.GetUInt(0);
                    entry.allianceID    = row2.GetUInt(1);
                    entry.factionID     = row2.GetUInt(2);
                }
            }
        }
        into.push_back(entry);
    }
    return true;
}

uint32 SystemDB::GetObjectLocationID(uint32 itemID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT locationID FROM entity WHERE itemID=%u", itemID )) {
        codelog(DATABASE__ERROR, "Error in GetObjectLocationID query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (res.GetRow(row))
        return (row.GetUInt(0));
    return 0;
}

double SystemDB::GetItemTypeRadius(uint32 typeID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT radius FROM invTypes WHERE typeID=%u", typeID )) {
        codelog(DATABASE__ERROR, "Error in GetItemTypeRadius query: %s", res.error.c_str());
        return 0.0;
    }

    DBResultRow row;
    if (res.GetRow(row))
        return (row.GetDouble(0));
    return 0.0;
}

double SystemDB::GetCelestialRadius(uint32 itemID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT radius FROM mapDenormalize WHERE itemID=%u", itemID )) {
        codelog(DATABASE__ERROR, "Error in GetItemTypeRadius query: %s", res.error.c_str());
        return 10.1;
    }

    DBResultRow row;
    if (res.GetRow(row))
        return (row.GetDouble(0));
    return 12.3;
}

bool SystemDB::GetWrecksToTypes(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT typeID, wreckTypeID FROM invTypesToWrecks")) {
        codelog(DATABASE__ERROR, "Error in GetWrecksToTypes query: %s", res.error.c_str());
        return false;
    }
    return true;
}

void SystemDB::GetLootGroups(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT npcGroupID, itemGroupID, groupDropChance FROM lootGroup")) {
        codelog(DATABASE__ERROR, "Error in GetLootGroups query: %s", res.error.c_str());
        return;
    }
}

void SystemDB::GetLootGroupTypes(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT itemGroupID, itemID, itemMetaLevel, minAmount, maxAmount FROM lootItemGroup")) {
        codelog(DATABASE__ERROR, "Error in GetLootGroupTypes query: %s", res.error.c_str());
        return;
    }
}

bool SystemDB::GetCelestialObjectData(uint32 celestialID, CelestialObjectData &into) {
    DBQueryResult res;

    if ( IsStaticMapItem(celestialID)) {
        // This Celestial object is a static celestial, so get its data from the 'mapDenormalize' table:
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  security, radius, celestialIndex, orbitIndex"
            " FROM mapDenormalize"
            " WHERE itemID = %u",
            celestialID))
        {
            codelog(DATABASE__ERROR, "Failed to query celestial object %u: %s.", celestialID, res.error.c_str());
            return false;
        }

        DBResultRow row;
        if (!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "Static Celestial object %u not found.", celestialID);
            return false;
        }

        into.security = (row.IsNull(0) ? 0 : row.GetDouble(0));
        into.radius = row.GetDouble(1);
        into.celestialIndex = (row.IsNull(2) ? 0 : row.GetUInt(2));
        into.orbitIndex = (row.IsNull(3) ? 0 : row.GetUInt(3));
    } else {
        // Quite possibly, this Celestial object is a dynamic one, so try to get its data from the 'entity' table,
        // and if it's not there either, then flag an error.
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  entity.itemID, "
            "  invTypes.radius "
            " FROM entity "
            "  LEFT JOIN invTypes USING (typeID)"
            " WHERE entity.itemID = %u",
            celestialID))
        {
            codelog(DATABASE__ERROR, "Failed to query celestial object %u: %s.", celestialID, res.error.c_str());
            return false;
        }

        DBResultRow row;
        if (!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "Dynamic Celestial object %u not found.", celestialID);
            return false;
        }

        into.security = 1.0;
        into.radius = (row.IsNull(1) ? 1 : row.GetDouble(1));
        into.celestialIndex = 0;
        into.orbitIndex = 0;
    }

    return true;
}

bool SystemDB::GetSolarSystemData(uint32 solarSystemID, SolarSystemData &into) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  xMin, yMin, zMin,"
        "  xMax, yMax, zMax,"
        "  luminosity,"
        "  border, fringe, corridor, hub, international, regional, constellation,"
        "  security, factionID, radius, sunTypeID, securityClass"
        " FROM mapSolarSystems"
        " WHERE solarSystemID=%u", solarSystemID))
    {
        codelog(DATABASE__ERROR, "Error in GetSolarSystem query for system %u: %s.", solarSystemID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data found for solar system %u.", solarSystemID);
        return false;
    }

    into.minPosition = GPoint(row.GetDouble(0), row.GetDouble(1), row.GetDouble(2));
    into.maxPosition = GPoint(row.GetDouble(3), row.GetDouble(4), row.GetDouble(5));
    into.luminosity = row.GetDouble(6);

    into.border = row.GetBool(7);
    into.fringe = row.GetBool(8);
    into.corridor = row.GetBool(9);
    into.hub = row.GetBool(10);
    into.international = row.GetBool(11);
    into.regional = row.GetBool(12);
    into.constellation = row.GetBool(13);

    into.security = row.GetDouble(14);
    into.factionID = (row.IsNull(15) ? 0 : row.GetUInt(15));
    into.radius = row.GetDouble(16);
    into.sunTypeID = row.GetUInt(17);
    into.securityClass = (row.IsNull(18) ? (std::string("")) : row.GetText(18));

    return true;
}
