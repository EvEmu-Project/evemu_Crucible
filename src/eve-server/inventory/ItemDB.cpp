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
    Author:        Allan
*/


#include "eve-server.h"

#include "inventory/ItemDB.h"


bool ItemDB::GetItem(uint32 itemID, ItemData &into) {
    /* called by RefPtr<_Ty> _Load() at InventoryItem.h:189 */
    DBQueryResult res;

    // For ranges of itemIDs we use specialized tables:
    if (IsRegionID(itemID)) {
        //region
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  regionName, 3 AS typeID, factionID, 1 AS locationID, 0 AS flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM mapRegions"
            " WHERE regionID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for region %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (IsConstellationID(itemID)) {
        //contellation
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  constellationName, 4 AS typeID, factionID, regionID, 0 AS flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM mapConstellations"
            " WHERE constellationID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for contellation %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (sDataMgr.IsSolarSystem(itemID)) {
        //solar system
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  solarSystemName, 5 AS typeID, factionID, constellationID, 0 AS flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM mapSolarSystems"
            " WHERE solarSystemID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for solar system %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (IsStargateID(itemID)) {
        //use mapDenormalize LEFT-JOIN-ing mapSolarSystems to get factionID
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  itemName, typeID, factionID, solarSystemID, 0 AS flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, mapDenormalize.x, mapDenormalize.y, mapDenormalize.z, '' AS customInfo"
            " FROM mapDenormalize"
            " LEFT JOIN mapSolarSystems USING (solarSystemID)"
            " WHERE itemID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for stargate %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (sDataMgr.IsStation(itemID)) {
        //station
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  stationName, stationTypeID, corporationID, solarSystemID, 0 AS flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM staStations"
            " WHERE stationID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for station %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (IsCelestialID(itemID)) {
        //use mapDenormalize
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  itemName, typeID, 1 AS ownerID, solarSystemID, 0 AS flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM mapDenormalize"
            " WHERE itemID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for universe celestial %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (IsAsteroidID(itemID)) {
        //use sysAsteroids
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  itemName, typeID, 1 AS ownerID, systemID, 0 AS flag, 0 AS contraband,"
            "  1 AS singleton, quantity, x, y, z, '' AS customInfo"
            " FROM sysAsteroids"
            " WHERE itemID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for asteroid %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (IsCharacterID(itemID)) {
        //use chrCharacters
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  characterName, typeID, 1 AS ownerID, solarSystemID, flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, 0 AS x, 0 AS y, 0 AS z, '' AS customInfo"
            " FROM chrCharacters"
            " WHERE characterID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for character %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (IsOfficeID(itemID)) {
        //use staOffices
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  name, typeID, corporationID, solarSystemID, flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, 0 AS x, 0 AS y, 0 AS z, '' AS customInfo"
            " FROM staOffices"
            " WHERE itemID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for character %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else {
        //fallback to entity
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  itemName, typeID, ownerID, locationID, flag, contraband,"
            "  singleton, quantity, x, y, z, customInfo"
            " FROM entity WHERE itemID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for item %u: %s", itemID, res.error.c_str());
            return false;
        }
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "ItemDB::GetItem() - Item %u not found.", itemID);
        return false;
    }

    into.name = row.GetText(0);
    into.typeID = row.GetUInt(1);
    into.ownerID = (row.IsNull(2) ? 1 : row.GetUInt(2));
    into.locationID = (row.IsNull(3) ? 0 : row.GetUInt(3));
    into.flag = (EVEItemFlags)row.GetUInt(4);
    into.contraband = row.GetInt(5) ? true : false;
    into.singleton = row.GetInt(6) ? true : false;
    into.quantity = row.GetUInt(7);

    into.position.x = row.GetDouble(8);
    into.position.y = row.GetDouble(9);
    into.position.z = row.GetDouble(10);

    into.customInfo = (row.IsNull(11) ? "" : row.GetText(11));

    return true;
}

uint32 ItemDB::NewItem(const ItemData &data) {
    // check for common errors ('common' is relative.)
    if (data.position.isNaN() or data.position.isInf())
        return 0;  // make error here?

    DBerror err;
    uint32 uid = 0;

    std::string nameEsc, customInfoEsc;
    sDatabase.DoEscapeString(nameEsc, data.name);
    sDatabase.DoEscapeString(customInfoEsc, data.customInfo);

    if(!sDatabase.RunQueryLID(err, uid,
        "INSERT INTO entity ("
        "   itemName, typeID, ownerID, locationID, flag,"
        "   contraband, singleton, quantity, x, y, z,"
        "   customInfo) "
        "VALUES('%s', %u, %u, %u, %u,%u,"
        "        %u, %u, %f, %f, %f, '%s' )",
        nameEsc.c_str(), data.typeID, data.ownerID, data.locationID, data.flag, data.contraband?1:0,
        data.singleton?1:0, data.quantity, data.position.x, data.position.y, data.position.z, customInfoEsc.c_str()
    )) {
        codelog(DATABASE__ERROR, "Failed to insert new entity: %s", err.c_str());
        return 0;
    }

    return uid;
}

void ItemDB::UpdateLocation(uint32 itemID, uint32 locationID, EVEItemFlags flag)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE entity SET locationID = %u, flag = %u WHERE itemID = %u", \
    locationID, (uint16)flag, itemID);
}

bool ItemDB::SaveItem(uint32 itemID, const ItemData &data) {
    // First check whether they are trying to save proper item:
    if (IsStaticMapItem(itemID)) {
        _log(ITEM__ERROR, "Refusing to modify static map object %u.", itemID);
        return false;
    }

    // check for common error ('common' is relative.)
    if (data.position.isNaN())
        return false;  // make error here?

    std::string nameEsc, customInfoEsc;
    sDatabase.DoEscapeString(nameEsc, data.name);
    sDatabase.DoEscapeString(customInfoEsc, data.customInfo);

    DBerror err;
    if(!sDatabase.RunQuery(err,
        "UPDATE entity"
        " SET"
        "  itemName = '%s',"
        "  ownerID = %u,"
        "  locationID = %u,"
        "  flag = %u,"
        "  singleton = %u,"
        "  quantity = %u,"
        "  x = %f, y = %f, z = %f,"
        "  customInfo = '%s'"
        " WHERE itemID = %u",
        nameEsc.c_str(),
        data.ownerID,
        data.locationID,
        (uint16)data.flag,
        (data.singleton?1:0),
        data.quantity,
        data.position.x, data.position.y, data.position.z,
        customInfoEsc.c_str(),
        itemID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s.", err.c_str());
        return false;
    }

    return true;
}

void ItemDB::SaveItems(std::vector<Inv::SaveData>& data)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO entity";
    Inserts << " (itemID, typeID, ownerID, locationID, flag, contraband, singleton, quantity, x, y, z, customInfo)";
    bool first = true;
    for (auto cur : data) {
        if (cur.position.isNaN() or cur.position.isInf()) {
            _log(DATABASE__ERROR, "ItemDB::SaveItems() - %u has invalid position", cur.itemID);
            continue;
        }
        if (cur.locationID == 0) {
            _log(DATABASE__ERROR, "ItemDB::SaveItems() - %u has invalid location", cur.itemID);
            continue;
        }
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else {
            Inserts << ", ";
        }
        Inserts << "(" << cur.itemID << ", " << cur.typeID << ", " << cur.ownerID << ", " << cur.locationID << ", ";
        Inserts << cur.flag << ", " << cur.contraband << ", " << (cur.singleton ? 1 : 0) << ", ";
        Inserts << cur.quantity << ", " << std::to_string(cur.position.x) << ", " << std::to_string(cur.position.y) << ", " << std::to_string(cur.position.z);
        Inserts << ", '" << cur.customInfo << "')";
    }

    if (!first) {
        Inserts << "ON DUPLICATE KEY UPDATE ";
        Inserts << "quantity=VALUES(quantity), ";
        Inserts << "ownerID=VALUES(ownerID), ";
        Inserts << "locationID=VALUES(locationID), ";
        Inserts << "flag=VALUES(flag), ";
        Inserts << "singleton=VALUES(singleton), ";
        Inserts << "quantity=VALUES(quantity), ";
        Inserts << "x=VALUES(x), ";
        Inserts << "y=VALUES(y), ";
        Inserts << "z=VALUES(z), ";
        Inserts << "customInfo=VALUES(customInfo) ";
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveItems - unable to save data - %s", err.c_str());
    }
}

void ItemDB::SaveAttributes(bool isChar, std::vector<Inv::AttrData>& data)
{
    std::ostringstream Inserts;
    // start the insert into command.
    if (isChar) {
        DBerror err;
        sDatabase.RunQuery(err, "DELETE FROM chrCharacterAttributes WHERE charID = %u", data[0].itemID);
        Inserts << "INSERT INTO chrCharacterAttributes";
        Inserts << " (charID, attributeID, valueInt, valueFloat)";
    } else {
        Inserts << "INSERT INTO entity_attributes";
        Inserts << " (itemID, attributeID, valueInt, valueFloat)";
    }
    bool first(true);
    for (auto cur : data) {
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else
            Inserts << ", ";
        Inserts << "(" << cur.itemID << ", " << cur.attrID << ", ";
        if (cur.type) {
            Inserts << "NULL, " << cur.valueFloat << ")";
        } else {
            Inserts << cur.valueInt << ", NULL)";
        }
    }

    if (!first) {
        Inserts << "ON DUPLICATE KEY UPDATE ";
        Inserts << "valueInt=VALUES(valueInt), ";
        Inserts << "valueFloat=VALUES(valueFloat)";
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveItems - unable to save data - %s", err.c_str());
    }
}

bool ItemDB::DeleteItem(uint32 itemID) {
    if (IsStaticMapItem(itemID)) {
        _log(ITEM__ERROR, "Refusing to delete static map object %u.", itemID);
        return false;
    }

    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM entity WHERE itemID = %u", itemID)) {
        codelog(DATABASE__ERROR, "Failed to delete item %u: %s", itemID, err.c_str());
        return false;
    }

    if (!sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID=%u", itemID)) {
        codelog(DATABASE__ERROR, "Failed to delete item %u: %s", itemID, err.c_str());
        return false;
    }
    return true;
}

void ItemDB::GetItems(uint16 catID, std::map< uint16, std::string >& typeIDs) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  typeID,"
        "  typeName"
        " FROM invTypes "
        " WHERE groupID IN"
        "   (SELECT groupID"
        "    FROM invGroups"
        "    WHERE categoryID = %u)",
        catID))
    {
        codelog(DATABASE__ERROR, "Failed to query types for catID %u: %s.", catID, res.error.c_str());
        return;
    }

    DBResultRow row;
    while(res.GetRow(row))
        typeIDs.emplace(row.GetUInt(0), row.GetText(1));
}
