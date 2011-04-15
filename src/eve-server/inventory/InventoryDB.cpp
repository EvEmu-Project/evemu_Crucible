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
    Author:     Zhur
*/

#include "EVEServerPCH.h"


bool InventoryDB::GetCategory(EVEItemCategories category, CategoryData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        " categoryName,"
        " description,"
        " published"
        " FROM invCategories"
        " WHERE categoryID=%u",
        uint32(category)))
    {
        _log(DATABASE__ERROR, "Error in query: %s.", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Category %u not found.", uint32(category));
        return false;
    }

    into.name = row.GetText(0);
    into.description = row.GetText(1);
    into.published = row.GetInt(2) ? true : false;

    return true;
}

bool InventoryDB::GetGroup(uint32 groupID, GroupData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        " categoryID,"
        " groupName,"
        " description,"
        " useBasePrice,"
        " allowManufacture,"
        " allowRecycler,"
        " anchored,"
        " anchorable,"
        " fittableNonSingleton,"
        " published"
        " FROM invGroups"
        " WHERE groupID=%u",
        groupID))
    {
        _log(DATABASE__ERROR, "Failed to query group %u: %s.", groupID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Group %u not found.", groupID);
        return false;
    }

    into.category = EVEItemCategories(row.GetUInt(0));
    into.name = row.GetText(1);
    into.description = row.GetText(2);
    into.useBasePrice = row.GetInt(3) ? true : false;
    into.allowManufacture = row.GetInt(4) ? true : false;
    into.allowRecycler = row.GetInt(5) ? true : false;
    into.anchored = row.GetInt(6) ? true : false;
    into.anchorable = row.GetInt(7) ? true : false;
    into.fittableNonSingleton = row.GetInt(8) ? true : false;
    into.published = row.GetInt(9) ? true : false;

    return true;
}

bool InventoryDB::GetType(uint32 typeID, TypeData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        " groupID,"
        " typeName,"
        " description,"
        " radius,"
        " mass,"
        " volume,"
        " capacity,"
        " portionSize,"
        " raceID,"
        " basePrice,"
        " published,"
        " marketGroupID,"
        " chanceOfDuplicating"
        " FROM invTypes"
        " WHERE typeID=%u",
        typeID))
    {
        _log(DATABASE__ERROR, "Failed to query type %u: %s.", typeID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Type %u not found.", typeID);
        return false;
    }

    into.groupID = row.GetUInt(0);
    into.name = row.GetText(1);
    into.description = row.GetText(2);
    into.radius = row.GetDouble(3);
    into.mass = row.GetDouble(4);
    into.volume = row.GetDouble(5);
    into.capacity = row.GetDouble(6);
    into.portionSize = row.GetUInt(7);
    into.race = EVERace(row.IsNull(8) ? 0 : row.GetUInt(8));
    into.basePrice = row.GetDouble(9);
    into.published = row.GetInt(10) ? true : false;
    into.marketGroupID = row.IsNull(11) ? 0 : row.GetUInt(11);
    into.chanceOfDuplicating = row.GetDouble(12);

    return true;
}

bool InventoryDB::GetBlueprintType(uint32 blueprintTypeID, BlueprintTypeData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        " parentBlueprintTypeID,"
        " productTypeID,"
        " productionTime,"
        " techLevel,"
        " researchProductivityTime,"
        " researchMaterialTime,"
        " researchCopyTime,"
        " researchTechTime,"
        " productivityModifier,"
        " materialModifier,"
        " wasteFactor / 100,"   // we have it in db as percentage ...
        " chanceOfReverseEngineering,"
        " maxProductionLimit"
        " FROM invBlueprintTypes"
        " WHERE blueprintTypeID=%u",
        blueprintTypeID))
    {
        _log(DATABASE__ERROR, "Error in query: %s.", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Blueprint type %u not found.", blueprintTypeID);
        return false;
    }

    into.parentBlueprintTypeID = row.IsNull(0) ? 0 : row.GetUInt(0);
    into.productTypeID = row.GetUInt(1);
    into.productionTime = row.GetUInt(2);
    into.techLevel = row.GetUInt(3);
    into.researchProductivityTime = row.GetUInt(4);
    into.researchMaterialTime = row.GetUInt(5);
    into.researchCopyTime = row.GetUInt(6);
    into.researchTechTime = row.GetUInt(7);
    into.productivityModifier = row.GetUInt(8);
    into.materialModifier = row.GetUInt(9);
    into.wasteFactor = row.GetDouble(10);
    into.chanceOfReverseEngineering = row.GetDouble(11);
    into.maxProductionLimit = row.GetUInt(12);

    return true;
}

bool InventoryDB::GetCharacterType(uint32 bloodlineID, CharacterTypeData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  bloodlineName,"
        "  raceID,"
        "  description,"
        "  maleDescription,"
        "  femaleDescription,"
        "  shipTypeID,"
        "  corporationID,"
        "  perception,"
        "  willpower,"
        "  charisma,"
        "  memory,"
        "  intelligence,"
        "  shortDescription,"
        "  shortMaleDescription,"
        "  shortFemaleDescription"
        " FROM chrBloodlines"
        " WHERE bloodlineID = %u",
        bloodlineID))
    {
        _log(DATABASE__ERROR, "Failed to query bloodline %u: %s.", bloodlineID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No data found for bloodline %u.", bloodlineID);
        return false;
    }

    into.bloodlineName = row.GetText(0);
    into.race = static_cast<EVERace>(row.GetUInt(1));
    into.description = row.GetText(2);
    into.maleDescription = row.GetText(3);
    into.femaleDescription = row.GetText(4);
    into.shipTypeID = row.GetUInt(5);
    into.corporationID = row.GetUInt(6);
    into.perception = row.GetUInt(7);
    into.willpower = row.GetUInt(8);
    into.charisma = row.GetUInt(9);
    into.memory = row.GetUInt(10);
    into.intelligence = row.GetUInt(11);
    into.shortDescription = row.GetText(12);
    into.shortMaleDescription = row.GetText(13);
    into.shortFemaleDescription = row.GetText(14);

    return true;
}

bool InventoryDB::GetCharacterTypeByBloodline(uint32 bloodlineID, uint32 &characterTypeID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  typeID"
        " FROM bloodlineTypes"
        " WHERE bloodlineID = %u",
        bloodlineID))
    {
        _log(DATABASE__ERROR, "Failed to query bloodline %u: %s.", bloodlineID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No data for bloodline %u.", bloodlineID);
        return false;
    }

    characterTypeID = row.GetUInt(0);

    return true;
}

bool InventoryDB::GetBloodlineByCharacterType(uint32 characterTypeID, uint32 &bloodlineID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  bloodlineID"
        " FROM bloodlineTypes"
        " WHERE typeID = %u",
        characterTypeID))
    {
        _log(DATABASE__ERROR, "Failed to query character type %u: %s.", characterTypeID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No data for character type %u.", characterTypeID);
        return false;
    }

    bloodlineID = row.GetUInt(0);

    return true;
}

bool InventoryDB::GetCharacterType(uint32 characterTypeID, uint32 &bloodlineID, CharacterTypeData &into) {
    if(!GetBloodlineByCharacterType(characterTypeID, bloodlineID))
        return false;
    return GetCharacterType(bloodlineID, into);
}

bool InventoryDB::GetCharacterTypeByBloodline(uint32 bloodlineID, uint32 &characterTypeID, CharacterTypeData &into) {
    if(!GetCharacterTypeByBloodline(bloodlineID, characterTypeID))
        return false;
    return GetCharacterType(bloodlineID, into);
}

bool InventoryDB::GetShipType(uint32 shipTypeID, ShipTypeData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        " weaponTypeID, miningTypeID, skillTypeID"
        " FROM invShipTypes"
        " WHERE shipTypeID = %u",
        shipTypeID))
    {
        _log(DATABASE__ERROR, "Failed to query ship type %u: %s.", shipTypeID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Ship type %u not found.", shipTypeID);
        return false;
    }

    into.mWeaponTypeID = row.IsNull(0) ? 0 : row.GetUInt(0);
    into.mMiningTypeID = row.IsNull(1) ? 0 : row.GetUInt(1);
    into.mSkillTypeID = row.IsNull(2) ? 0 : row.GetUInt(2);

    return true;
}

bool InventoryDB::GetStationType(uint32 stationTypeID, StationTypeData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        " dockingBayGraphicID, hangarGraphicID,"
        " dockEntryX, dockEntryY, dockEntryZ,"
        " dockOrientationX, dockOrientationY, dockOrientationZ,"
        " operationID, officeSlots, reprocessingEfficiency, conquerable"
        " FROM staStationTypes"
        " WHERE stationTypeID = %u",
        stationTypeID))
    {
        _log(DATABASE__ERROR, "Failed to query station type %u: %s.", stationTypeID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Station type %u not found.", stationTypeID);
        return false;
    }

    into.dockingBayGraphicID = (row.IsNull(0) ? 0 : row.GetUInt(0));
    into.hangarGraphicID = (row.IsNull(1) ? 0 : row.GetUInt(1));

    into.dockEntry = GPoint(row.GetDouble(2), row.GetDouble(3), row.GetDouble(4));
    into.dockOrientation = GVector(row.GetDouble(5), row.GetDouble(6), row.GetDouble(7));

    into.operationID = (row.IsNull(8) ? 0 : row.GetUInt(8));
    into.officeSlots = (row.IsNull(9) ? 0 : row.GetUInt(9));
    into.reprocessingEfficiency = (row.IsNull(10) ? 0.0 : row.GetDouble(10));
    into.conquerable = (row.GetInt(11) ? true : false);

    return true;
}

bool InventoryDB::GetItem(uint32 itemID, ItemData &into) {
    DBQueryResult res;

    // For certain ranges of itemID-s we use specialized tables:
    if(IsRegion(itemID)) {
        //region
        if(!sDatabase.RunQuery(res,
            "SELECT"
            " regionName, 3 AS typeID, factionID, 1 AS locationID, 0 AS flag, 0 AS contraband,"
            " 1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM mapRegions"
            " WHERE regionID=%u", itemID))
        {
            codelog(SERVICE__ERROR, "Error in query for region %u: %s", itemID, res.error.c_str());
            return NULL;
        }
    } else if(IsConstellation(itemID)) {
        //contellation
        if(!sDatabase.RunQuery(res,
            "SELECT"
            " constellationName, 4 AS typeID, factionID, regionID, 0 AS flag, 0 AS contraband,"
            " 1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM mapConstellations"
            " WHERE constellationID=%u", itemID))
        {
            codelog(SERVICE__ERROR, "Error in query for contellation %u: %s", itemID, res.error.c_str());
            return NULL;
        }
    } else if(IsSolarSystem(itemID)) {
        //solar system
        if(!sDatabase.RunQuery(res,
            "SELECT"
            " solarSystemName, 5 AS typeID, factionID, constellationID, 0 AS flag, 0 AS contraband,"
            " 1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM mapSolarSystems"
            " WHERE solarSystemID=%u", itemID))
        {
            codelog(SERVICE__ERROR, "Error in query for solar system %u: %s", itemID, res.error.c_str());
            return NULL;
        }
    } else if(IsUniverseCelestial(itemID)) {
        //use mapDenormalize
        if(!sDatabase.RunQuery(res,
            "SELECT"
            " itemName, typeID, 1 AS ownerID, solarSystemID, 0 AS flag, 0 AS contraband,"
            " 1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM mapDenormalize"
            " WHERE itemID=%u", itemID))
        {
            codelog(SERVICE__ERROR, "Error in query for universe celestial %u: %s", itemID, res.error.c_str());
            return NULL;
        }
    } else if(IsStargate(itemID)) {
        //use mapDenormalize LEFT-JOIN-ing mapSolarSystems to get factionID
        if(!sDatabase.RunQuery(res,
            "SELECT"
            " itemName, typeID, factionID, solarSystemID, 0 AS flag, 0 AS contraband,"
            " 1 AS singleton, 1 AS quantity, mapDenormalize.x, mapDenormalize.y, mapDenormalize.z, '' AS customInfo"
            " FROM mapDenormalize"
            " LEFT JOIN mapSolarSystems USING (solarSystemID)"
            " WHERE itemID=%u", itemID))
        {
            codelog(SERVICE__ERROR, "Error in query for stargate %u: %s", itemID, res.error.c_str());
            return NULL;
        }
    } else if(IsStation(itemID)) {
        //station
        if(!sDatabase.RunQuery(res,
            "SELECT"
            " stationName, stationTypeID, corporationID, solarSystemID, 0 AS flag, 0 AS contraband,"
            " 1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM staStations"
            " WHERE stationID=%u", itemID))
        {
            codelog(SERVICE__ERROR, "Error in query for station %u: %s", itemID, res.error.c_str());
            return NULL;
        }
    } else {
        //fallback to entity
        if(!sDatabase.RunQuery(res,
            "SELECT"
            " itemName, typeID, ownerID, locationID, flag, contraband,"
            " singleton, quantity, x, y, z, customInfo"
            " FROM entity WHERE itemID=%u", itemID))
        {
            codelog(SERVICE__ERROR, "Error in query for item %u: %s", itemID, res.error.c_str());
            return NULL;
        }
    }

    DBResultRow row;
    if(!res.GetRow(row))
    {
        codelog(SERVICE__ERROR, "Item %u not found.", itemID);
        return false;
    }

    into.name = row.GetText(0);
    into.typeID = row.GetUInt(1);
    into.ownerID = (row.IsNull(2) ? 1 : row.GetUInt(2));
    into.locationID = (row.IsNull(3) ? 1 : row.GetUInt(3));
    into.flag = (EVEItemFlags)row.GetUInt(4);
    into.contraband = (row.GetInt(5) ? true : false);
    into.singleton = (row.GetInt(6) ? true : false);
    into.quantity = row.GetUInt(7);

    into.position.x = row.GetDouble(8);
    into.position.y = row.GetDouble(9);
    into.position.z = row.GetDouble(10);

    into.customInfo = (row.IsNull(11) ? "" : row.GetText(11));

    return true;
}

uint32 InventoryDB::NewItem(const ItemData &data) {
    DBerror err;
    uint32 eid;

    std::string nameEsc, customInfoEsc;
    sDatabase.DoEscapeString(nameEsc, data.name);
    sDatabase.DoEscapeString(customInfoEsc, data.customInfo);

    if(!sDatabase.RunQueryLID(err, eid,
        "INSERT INTO entity ("
        "   itemName, typeID, ownerID, locationID, flag,"
        "   contraband, singleton, quantity, x, y, z,"
        "   customInfo"
        " ) "
        "VALUES('%s', %u, %u, %u, %u,"
        "   %u, %u, %u, %f, %f, %f,"
        "   '%s' )",
        nameEsc.c_str(), data.typeID, data.ownerID, data.locationID, data.flag,
        data.contraband?1:0, data.singleton?1:0, data.quantity, data.position.x, data.position.y, data.position.z,
        customInfoEsc.c_str()
        )
    ) {
        codelog(SERVICE__ERROR, "Failed to insert new entity: %s", err.c_str());
        return(0);
    }

    return(eid);
}

bool InventoryDB::SaveItem(uint32 itemID, const ItemData &data) {
    // First check whether they are trying to save proper item:
    if(IsStaticMapItem(itemID)) {
        _log(DATABASE__ERROR, "Refusing to modify static map object %u.", itemID);
        return false;
    }

    DBerror err;

    std::string nameEsc, customInfoEsc;
    sDatabase.DoEscapeString(nameEsc, data.name);
    sDatabase.DoEscapeString(customInfoEsc, data.customInfo);

    if(!sDatabase.RunQuery(err,
        "UPDATE entity"
        " SET"
        " itemName = '%s',"
        " typeID = %u,"
        " ownerID = %u,"
        " locationID = %u,"
        " flag = %u,"
        " contraband = %u,"
        " singleton = %u,"
        " quantity = %u,"
        " x = %f, y = %f, z = %f,"
        " customInfo = '%s'"
        " WHERE itemID = %u",
        nameEsc.c_str(),
        data.typeID,
        data.ownerID,
        data.locationID,
        uint32(data.flag),
        uint32(data.contraband),
        uint32(data.singleton),
        data.quantity,
        data.position.x, data.position.y, data.position.z,
        customInfoEsc.c_str(),
        itemID))
    {
        _log(DATABASE__ERROR, "Error in query: %s.", err.c_str());
        return false;
    }

    return true;
}

bool InventoryDB::DeleteItem(uint32 itemID) {
    // First check whether they are trying to save proper item:
    if(IsStaticMapItem(itemID)) {
        _log(DATABASE__ERROR, "Refusing to delete static map object %u.", itemID);
        return false;
    }

    DBerror err;

    //NOTE: all child entities should be deleted by the caller first.

    if(!sDatabase.RunQuery(err,
        "DELETE"
        " FROM entity"
        " WHERE itemID=%u",
        itemID
    ))
    {
        codelog(DATABASE__ERROR, "Failed to delete item %u: %s", itemID, err.c_str());
        return false;
    }
    return true;
}

//this could be optimized to load the full row of each
//item which is to be loaded (and used to be), but it made
//for some overly complex knowledge in the DB side which
// really did not belong here, so weo go to the simpler
// solution until it becomes a problem.
bool InventoryDB::GetItemContents(uint32 itemID, std::vector<uint32> &into)
{
    DBQueryResult res;

    if( !sDatabase.RunQuery( res,
        "SELECT "
        " itemID"
        " FROM entity "
        " WHERE locationID = %u",
        itemID ) )
    {
        codelog(SERVICE__ERROR, "Error in query for item %u: %s", itemID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while( res.GetRow( row ) )
        into.push_back( row.GetUInt( 0 ) );

    return true;
}
bool InventoryDB::GetItemContents(uint32 itemID, EVEItemFlags flag, std::vector<uint32> &into)
{
    DBQueryResult res;

    if( !sDatabase.RunQuery( res,
        "SELECT "
        " itemID"
        " FROM entity "
        " WHERE locationID=%u"
        "  AND flag=%d",
        itemID, (int)flag ) )
    {
        codelog(SERVICE__ERROR, "Error in query for item %u: %s", itemID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while( res.GetRow( row ) )
        into.push_back( row.GetUInt( 0 ) );

    return true;
}

bool InventoryDB::GetItemContents(uint32 itemID, EVEItemFlags flag, uint32 ownerID, std::vector<uint32> &into)
{
    DBQueryResult res;

    if( !sDatabase.RunQuery( res,
        "SELECT "
        " itemID"
        " FROM entity "
        " WHERE locationID=%u"
        "  AND flag=%d"
        "  AND ownerID=%u",
        itemID, (int)flag, ownerID ) )
    {
        codelog(SERVICE__ERROR, "Error in query for item %u: %s", itemID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while( res.GetRow( row ) )
        into.push_back( row.GetUInt( 0 ) );

    return true;
}

bool InventoryDB::LoadTypeAttributes(uint32 typeID, EVEAttributeMgr &into) {
#if 0
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        " attributeID,"
        " valueInt,"
        " valueFloat"
        " FROM dgmTypeAttributes"
        " WHERE typeID=%u",
        typeID))
    {
        _log(DATABASE__ERROR, "Failed to query type attributes for type %u: %s.", typeID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    EVEAttributeMgr::Attr attr;
    while(res.GetRow(row)) {
        if(row.IsNull(0)) {
            _log(DATABASE__ERROR, "Attribute row for type %u has attributeID NULL. Skipping.", typeID);
            continue;
        }
        attr = EVEAttributeMgr::Attr(row.GetUInt(0));
        if(row.IsNull(2)) {
            if(row.IsNull(1)) {
                _log(DATABASE__ERROR, "Attribute %u for type %u has both values NULL. Skipping.", attr, typeID);
            } else
                into.SetInt(attr, row.GetInt(1));
        } else {
            if(!row.IsNull(1)) {
                _log(DATABASE__ERROR, "Attribute %u for type %u has both values non-NULL. Using float.", attr, typeID);
            }
            into.SetReal(attr, row.GetDouble(2));
        }
    }
#else

    DgmTypeAttributeSet *attrset = sDgmTypeAttrMgr.GetDmgTypeAttributeSet(typeID);
    
    // if not found return true because there can be items without attributes I guess
    if (attrset == NULL)
        return true;

    DgmTypeAttributeSet::AttrSetItr itr = attrset->begin();
    
    for (; itr != attrset->end(); itr++) {
        if ((*itr)->number.get_type() == evil_number_int)
            into.SetInt((EVEAttributeMgr::Attr)(*itr)->attributeID, (*itr)->number.get_int());
        else
            into.SetReal((EVEAttributeMgr::Attr)(*itr)->attributeID, (*itr)->number.get_float());
    }
#endif
    return true;
}

bool InventoryDB::LoadItemAttributes(uint32 itemID, EVEAttributeMgr &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        " attributeID,"
        " valueInt,"
        " valueFloat"
        " FROM entity_attributes"
        " WHERE itemID=%u",
        itemID))
    {
        _log(DATABASE__ERROR, "Failed to query item attributes for item %u: %s.", itemID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    EVEAttributeMgr::Attr attr;
    while(res.GetRow(row)) {
        if(row.IsNull(0)) {
            _log(DATABASE__ERROR, "Attribute row for item %u has attributeID NULL. Skipping.", itemID);
            continue;
        }
        attr = EVEAttributeMgr::Attr(row.GetUInt(0));
        if(row.IsNull(2)) {
            if(row.IsNull(1)){
                _log(DATABASE__ERROR, "Attribute %u for item %u has both values NULL. Skipping.", attr, itemID);
            }
            else
                into.SetInt(attr, row.GetInt(1));
        } else {
            if(!row.IsNull(1)) {
                _log(DATABASE__ERROR, "Attribute %u for item %u has both values non-NULL. Using float.", attr, itemID);
            }
            into.SetReal(attr, row.GetDouble(2));
        }
    }
    return true;
}

bool InventoryDB::UpdateAttribute_int(uint32 itemID, uint32 attributeID, int v) {
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "REPLACE INTO entity_attributes"
        "   (itemID, attributeID, valueInt, valueFloat)"
        " VALUES"
        "   (%u, %u, %d, NULL)",
        itemID, attributeID, v)
    ) {
        codelog(SERVICE__ERROR, "Failed to store attribute %d for item %u: %s", attributeID, itemID, err.c_str());
        return false;
    }
    return true;
}

bool InventoryDB::UpdateAttribute_double(uint32 itemID, uint32 attributeID, double v) {
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "REPLACE INTO entity_attributes"
        "   (itemID, attributeID, valueInt, valueFloat)"
        " VALUES"
        "   (%u, %u, NULL, %f)",
        itemID, attributeID, v)
    ) {
        codelog(SERVICE__ERROR, "Failed to store attribute %d for item %u: %s", attributeID, itemID, err.c_str());
        return false;
    }
    return true;
}
bool InventoryDB::EraseAttribute(uint32 itemID, uint32 attributeID) {
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "DELETE FROM entity_attributes"
        " WHERE itemID=%u AND attributeID=%u",
        itemID, attributeID)
    ) {
        codelog(SERVICE__ERROR, "Failed to erase attribute %d for item %u: %s", attributeID, itemID, err.c_str());
        return false;
    }
    return true;
}

bool InventoryDB::EraseAttributes(uint32 itemID) {
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "DELETE"
        " FROM entity_attributes"
        " WHERE itemID=%u",
        itemID))
    {
        _log(DATABASE__ERROR, "Failed to erase attributes for item %u: %s", itemID, err.c_str());
        return false;
    }
    return true;
}

bool InventoryDB::GetBlueprint(uint32 blueprintID, BlueprintData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        " copy,"
        " materialLevel,"
        " productivityLevel,"
        " licensedProductionRunsRemaining"
        " FROM invBlueprints"
        " WHERE blueprintID=%u",
        blueprintID))
    {
        _log(DATABASE__ERROR, "Error in query: %s.", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Blueprint %u not found.", blueprintID);
        return false;
    }

    into.copy = row.GetInt(0) ? true : false;
    into.materialLevel = row.GetUInt(1);
    into.productivityLevel = row.GetUInt(2);
    into.licensedProductionRunsRemaining = row.GetInt(3);

    return true;
}

bool InventoryDB::NewBlueprint(uint32 blueprintID, const BlueprintData &data) {
    DBerror err;

    if(!sDatabase.RunQuery(err,
        "INSERT"
        " INTO invBlueprints"
        " (blueprintID, copy, materialLevel, productivityLevel, licensedProductionRunsRemaining)"
        " VALUES"
        " (%u, %u, %u, %u, %d)",
        blueprintID, data.copy, data.materialLevel, data.productivityLevel, data.licensedProductionRunsRemaining))
    {
        _log(DATABASE__ERROR, "Unable to create new blueprint entry for blueprint %u: %s.", blueprintID, err.c_str());
        return false;
    }

    return true;
}

bool InventoryDB::SaveBlueprint(uint32 blueprintID, const BlueprintData &data) {
    DBerror err;

    if(!sDatabase.RunQuery(err,
        "UPDATE invBlueprints"
        " SET"
        " copy = %u,"
        " materialLevel = %u,"
        " productivityLevel = %u,"
        " licensedProductionRunsRemaining = %d"
        " WHERE blueprintID = %u",
        uint32(data.copy),
        data.materialLevel,
        data.productivityLevel,
        data.licensedProductionRunsRemaining))
    {
        _log(DATABASE__ERROR, "Error in query: %s.", err.c_str());
        return false;
    }

    return true;
}

bool InventoryDB::DeleteBlueprint(uint32 blueprintID) {
    DBerror err;

    if(!sDatabase.RunQuery(err,
        "DELETE"
        " FROM invBlueprints"
        " WHERE blueprintID=%u",
        blueprintID))
    {
        codelog(DATABASE__ERROR, "Failed to delete blueprint %u: %s.", blueprintID, err.c_str());
        return false;
    }
    return true;
}

bool InventoryDB::GetCharacter(uint32 characterID, CharacterData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  chr.accountID,"
        "  chr.title,"
        "  chr.description,"
        "  chr.gender,"
        "  chr.bounty,"
        "  chr.balance,"
        "  chr.securityRating,"
        "  chr.logonMinutes,"
        "  chr.corporationID,"
        "  crp.allianceID,"
        "  chr.stationID,"
        "  chr.solarSystemID,"
        "  chr.constellationID,"
        "  chr.regionID,"
        "  chr.ancestryID,"
        "  chr.careerID,"
        "  chr.schoolID,"
        "  chr.careerSpecialityID,"
        "  chr.startDateTime,"
        "  chr.createDateTime,"
        "  chr.corporationDateTime"
        " FROM character_ AS chr"
        " LEFT JOIN corporation AS crp USING (corporationID)"
        " WHERE characterID = %u",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to query character %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No data found for character %u.", characterID);
        return false;
    }

    into.accountID = row.IsNull( 0 ) ? 0 : row.GetUInt( 0 );
    into.title = row.GetText( 1 );
    into.description = row.GetText( 2 );
    into.gender = row.GetUInt( 3 ) ? true : false;
    into.bounty = row.GetDouble( 4 );
    into.balance = row.GetDouble( 5 );
    into.securityRating = row.GetDouble( 6 );
    into.logonMinutes = row.GetUInt( 7 );
    into.corporationID = row.GetUInt( 8 );
    into.allianceID = row.IsNull( 9 ) ? 0 : row.GetUInt( 9 );
    into.stationID = row.GetUInt( 10 );
    into.solarSystemID = row.GetUInt( 11 );
    into.constellationID = row.GetUInt( 12 );
    into.regionID = row.GetUInt( 13 );
    into.ancestryID = row.GetUInt( 14 );
    into.careerID = row.GetUInt( 15 );
    into.schoolID = row.GetUInt( 16 );
    into.careerSpecialityID = row.GetUInt( 17 );
    into.startDateTime = row.GetUInt64( 18 );
    into.createDateTime = row.GetUInt64( 19 );
    into.corporationDateTime = row.GetUInt64( 20 );

    return true;
}

bool InventoryDB::GetCharacterAppearance(uint32 characterID, CharacterAppearance &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  accessoryID,"
        "  beardID,"
        "  costumeID,"
        "  decoID,"
        "  eyebrowsID,"
        "  eyesID,"
        "  hairID,"
        "  lipstickID,"
        "  makeupID,"
        "  skinID,"
        "  backgroundID,"
        "  lightID,"
        "  headRotation1, headRotation2, headRotation3,"
        "  eyeRotation1, eyeRotation2, eyeRotation3,"
        "  camPos1, camPos2, camPos3,"
        "  morph1n, morph1s, morph1w, morph1e,"
        "  morph2n, morph2s, morph2w, morph2e,"
        "  morph3n, morph3s, morph3w, morph3e,"
        "  morph4n, morph4s, morph4w, morph4e"
        " FROM character_"
        " WHERE characterID = %u",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to query character's %u appearance: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No appearance data found for character %u.", characterID);
        return false;
    }

    row.IsNull(0) ? into.Clear_accessoryID() : into.Set_accessoryID(row.GetUInt(0));
    row.IsNull(1) ? into.Clear_beardID() : into.Set_beardID(row.GetUInt(1));
    into.costumeID = row.GetUInt(2);
    row.IsNull(3) ? into.Clear_decoID():into.Set_decoID(row.GetUInt(3));
    into.eyebrowsID = row.GetUInt(4);
    into.eyesID = row.GetUInt(5);
    into.hairID = row.GetUInt(6);
    row.IsNull(7) ? into.Clear_lipstickID() : into.Set_lipstickID(row.GetUInt(7));
    row.IsNull(8) ? into.Clear_makeupID() : into.Set_makeupID(row.GetUInt(8));
    into.skinID = row.GetUInt(9);
    into.backgroundID = row.GetUInt(10);
    into.lightID = row.GetUInt(11);

    // none of these may be NULL
    into.headRotation1 = row.GetDouble(12);
    into.headRotation2 = row.GetDouble(13);
    into.headRotation3 = row.GetDouble(14);

    into.eyeRotation1 = row.GetDouble(15);
    into.eyeRotation2 = row.GetDouble(16);
    into.eyeRotation3 = row.GetDouble(17);

    into.camPos1 = row.GetDouble(18);
    into.camPos2 = row.GetDouble(19);
    into.camPos3 = row.GetDouble(20);

    // all of these may be NULL
    row.IsNull(21) ? into.Clear_morph1n() : into.Set_morph1n(row.GetDouble(21));
    row.IsNull(22) ? into.Clear_morph1s() : into.Set_morph1s(row.GetDouble(22));
    row.IsNull(23) ? into.Clear_morph1w() : into.Set_morph1w(row.GetDouble(23));
    row.IsNull(24) ? into.Clear_morph1e() : into.Set_morph1e(row.GetDouble(24));

    row.IsNull(25) ? into.Clear_morph2n() : into.Set_morph2n(row.GetDouble(25));
    row.IsNull(26) ? into.Clear_morph2s() : into.Set_morph2s(row.GetDouble(26));
    row.IsNull(27) ? into.Clear_morph2w() : into.Set_morph2w(row.GetDouble(27));
    row.IsNull(28) ? into.Clear_morph2e() : into.Set_morph2e(row.GetDouble(28));

    row.IsNull(29) ? into.Clear_morph3n() : into.Set_morph3n(row.GetDouble(29));
    row.IsNull(30) ? into.Clear_morph3s() : into.Set_morph3s(row.GetDouble(30));
    row.IsNull(31) ? into.Clear_morph3w() : into.Set_morph3w(row.GetDouble(31));
    row.IsNull(32) ? into.Clear_morph3e() : into.Set_morph3e(row.GetDouble(32));

    row.IsNull(33) ? into.Clear_morph4n() : into.Set_morph4n(row.GetDouble(33));
    row.IsNull(34) ? into.Clear_morph4s() : into.Set_morph4s(row.GetDouble(34));
    row.IsNull(35) ? into.Clear_morph4w() : into.Set_morph4w(row.GetDouble(35));
    row.IsNull(36) ? into.Clear_morph4e() : into.Set_morph4e(row.GetDouble(36));

    return true;
}

bool InventoryDB::GetCorpMemberInfo(uint32 characterID, CorpMemberInfo &into) {
    DBQueryResult res;
    DBResultRow row;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  corpRole,"
        "  rolesAtAll,"
        "  rolesAtBase,"
        "  rolesAtHQ,"
        "  rolesAtOther"
        " FROM character_"
        " WHERE characterID = %u",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to query corp member info of character %u: %s.", characterID, res.error.c_str());
        return false;
    }

    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No corp member info found for character %u.", characterID);
        return false;
    }

    into.corpRole = row.GetUInt64(0);
    into.rolesAtAll = row.GetUInt64(1);
    into.rolesAtBase = row.GetUInt64(2);
    into.rolesAtHQ = row.GetUInt64(3);
    into.rolesAtOther = row.GetUInt64(4);

    // this is hack and belongs somewhere else
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  corporation.stationID"
        " FROM character_"
        "  LEFT JOIN corporation USING (corporationID)"
        " WHERE characterID = %u",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to query HQ of character's %u corporation: %s.", characterID, res.error.c_str());
        return false;
    }

    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No HQ found for character's %u corporation.", characterID);
        return false;
    }

    into.corpHQ = (row.IsNull(0) ? 0 : row.GetUInt(0));

    return true;
}
/*
 * This macro checks given CharacterAppearance object (app) if given value (v) is NULL:
 *  if yes, macro evaulates to string "NULL"
 *  if no, macro evaulates to call to function _ToStr, which turns given value to string.
 *
 * This macro is needed when saving CharacterAppearance values into DB (NewCharacter, SaveCharacterAppearance).
 * Resulting value must be const char *.
 */
#define _VoN(app, v) \
    ((const char *)(app.IsNull_##v() ? "NULL" : _ToStr(app.Get_##v()).c_str()))

/* a 32 bits unsigned integer can be max 0xFFFFFFFF.
   this results in a text string: '4294967295' which
   is 10 long. Including the '\0' at the end of the
   string it is max 11.
 */
static std::string _ToStr(uint32 v) {
    char buf[11];
    snprintf(buf, 11, "%u", v);
    return(buf);
}

static std::string _ToStr(double v) {
    char buf[32];
    snprintf(buf, 32, "%.13f", v);
    return(buf);
}

bool InventoryDB::NewCharacter(uint32 characterID, const CharacterData &data, const CharacterAppearance &appData, const CorpMemberInfo &corpData) {
    DBerror err;

    std::string titleEsc, descriptionEsc;
    sDatabase.DoEscapeString(titleEsc, data.title);
    sDatabase.DoEscapeString(descriptionEsc, data.description);

    // Table character_ goes first
    if(!sDatabase.RunQuery(err,
        "INSERT INTO character_"
        // CharacterData:
        "  (characterID, accountID, title, description, bounty, balance, securityRating, petitionMessage,"
        "   logonMinutes, corporationID, corpRole, rolesAtAll, rolesAtBase, rolesAtHQ, rolesAtOther,"
        "   corporationDateTime, startDateTime, createDateTime,"
        "   ancestryID, careerID, schoolID, careerSpecialityID, gender,"
        "   stationID, solarSystemID, constellationID, regionID,"
        // CharacterAppearance:
        "   accessoryID, beardID, costumeID, decoID, eyebrowsID, eyesID, hairID, lipstickID, makeupID, skinID, backgroundID,"
        "   lightID,"
        "   headRotation1, headRotation2, headRotation3,"
        "   eyeRotation1, eyeRotation2, eyeRotation3,"
        "   camPos1, camPos2, camPos3,"
        "   morph1n, morph1s, morph1w, morph1e,"
        "   morph2n, morph2s, morph2w, morph2e,"
        "   morph3n, morph3s, morph3w, morph3e,"
        "   morph4n, morph4s, morph4w, morph4e)"
        " VALUES"
        // CharacterData:
        "  (%u, %u, '%s', '%s', %f, %f, %f, '%s',"
        "   %u, %u, "I64u ", "I64u ", "I64u ", "I64u ", "I64u ", "
        "   "I64u ", " I64u ", " I64u ","
        "   %u, %u, %u, %u, %u,"
        "   %u, %u, %u, %u,"
        // CharacterAppearance:
        "   %s, %s, %u, %s, %u, %u, %u, %s, %s, %u, %u,"
        "   %u,"
        "   %f, %f, %f,"
        "   %f, %f, %f,"
        "   %f, %f, %f,"
        "   %s, %s, %s, %s,"
        "   %s, %s, %s, %s,"
        "   %s, %s, %s, %s,"
        "   %s, %s, %s, %s)",
        // CharacterData:
        characterID, data.accountID, titleEsc.c_str(), descriptionEsc.c_str(), data.bounty, data.balance, data.securityRating, "No petition",
        data.logonMinutes, data.corporationID, corpData.corpRole, corpData.rolesAtAll, corpData.rolesAtBase, corpData.rolesAtHQ, corpData.rolesAtOther,
        data.corporationDateTime, data.startDateTime, data.createDateTime,
        data.ancestryID, data.careerID, data.schoolID, data.careerSpecialityID, data.gender,
        data.stationID, data.solarSystemID, data.constellationID, data.regionID,
        // CharacterAppearance:
        _VoN(appData, accessoryID), _VoN(appData, beardID), appData.costumeID, _VoN(appData, decoID), appData.eyebrowsID, appData.eyesID, appData.hairID, _VoN(appData, lipstickID), _VoN(appData, makeupID), appData.skinID, appData.backgroundID,
        appData.lightID,
        appData.headRotation1, appData.headRotation2, appData.headRotation3,
        appData.eyeRotation1, appData.eyeRotation2, appData.eyeRotation3,
        appData.camPos1, appData.camPos2, appData.camPos3,
        _VoN(appData, morph1n), _VoN(appData, morph1s), _VoN(appData, morph1w), _VoN(appData, morph1e),
        _VoN(appData, morph2n), _VoN(appData, morph2s), _VoN(appData, morph2w), _VoN(appData, morph2e),
        _VoN(appData, morph3n), _VoN(appData, morph3s), _VoN(appData, morph3w), _VoN(appData, morph3e),
        _VoN(appData, morph4n), _VoN(appData, morph4s), _VoN(appData, morph4w), _VoN(appData, morph4e)
    )) {
        _log(DATABASE__ERROR, "Failed to insert character %u: %s.", characterID, err.c_str());
        return false;
    }

    // Hack in the first employment record
    // TODO: Eventually, this should go under corp stuff...
    if(!sDatabase.RunQuery(err,
        "INSERT INTO chrEmployment"
        "  (characterID, corporationID, startDate, deleted)"
        " VALUES"
        "  (%u, %u, " I64u ", 0)",
        characterID, data.corporationID, Win32TimeNow()))
    {
        _log(DATABASE__ERROR, "Failed to insert employment info of character %u: %s.", characterID, err.c_str());
        //just let it go... its a lot easier this way
    }

    // And one more member to the corporation
    if(!sDatabase.RunQuery(err,
        "UPDATE corporation"
        "  SET memberCount = memberCount + 1"
        " WHERE corporationID = %u",
        data.corporationID))
    {
        _log(DATABASE__ERROR, "Failed to raise member count of corporation %u: %s.", characterID, err.c_str());
        //just let it go... its a lot easier this way
    }

    return true;
}

bool InventoryDB::SaveCharacter(uint32 characterID, const CharacterData &data) {
    DBerror err;

    std::string titleEsc;
    sDatabase.DoEscapeString(titleEsc, data.title);

    std::string descriptionEsc;
    sDatabase.DoEscapeString(descriptionEsc, data.description);

    if(!sDatabase.RunQuery(err,
        "UPDATE character_"
        " SET"
        "  accountID = %u,"
        "  title = '%s',"
        "  description = '%s',"
        "  gender = %u,"
        "  bounty = %f,"
        "  balance = %f,"
        "  securityRating = %f,"
        "  logonMinutes = %u,"
        "  corporationID = %u,"
        "  stationID = %u,"
        "  solarSystemID = %u,"
        "  constellationID = %u,"
        "  regionID = %u,"
        "  ancestryID = %u,"
        "  careerID = %u,"
        "  schoolID = %u,"
        "  careerSpecialityID = %u,"
        "  startDateTime = " I64u ","
        "  createDateTime = " I64u ","
        "  corporationDateTime = " I64u
        " WHERE characterID = %u",
        data.accountID,
        titleEsc.c_str(),
        descriptionEsc.c_str(),
        data.gender,
        data.bounty,
        data.balance,
        data.securityRating,
        data.logonMinutes,
        data.corporationID,
        data.stationID,
        data.solarSystemID,
        data.constellationID,
        data.regionID,
        data.ancestryID,
        data.careerID,
        data.schoolID,
        data.careerSpecialityID,
        data.startDateTime,
        data.createDateTime,
        data.corporationDateTime,
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to save character %u: %s.", characterID, err.c_str());
        return false;
    }

    return true;
}

bool InventoryDB::SaveCharacterAppearance(uint32 characterID, const CharacterAppearance &data) {
    DBerror err;

    if(!sDatabase.RunQuery(err,
        "UPDATE character_"
        " SET"
        "  accessoryID = %s,"
        "  beardID = %s,"
        "  costumeID = %u,"
        "  decoID = %s,"
        "  eyebrowsID = %u,"
        "  eyesID = %u,"
        "  hairID = %u,"
        "  lipstickID = %s,"
        "  makeupID = %s,"
        "  skinID = %u,"
        "  backgroundID = %u,"
        "  lightID = %u,"
        "  headRotation1 = %f, headRotation2 = %f, headRotation3 = %f,"
        "  eyeRotation1 = %f, eyeRotation2 = %f, eyeRotation3 = %f,"
        "  camPos1 = %f, camPos2 = %f, camPos3 = %f,"
        "  morph1n = %s, morph1s = %s, morph1w = %s, morph1e = %s,"
        "  morph2n = %s, morph2s = %s, morph2w = %s, morph2e = %s,"
        "  morph3n = %s, morph3s = %s, morph3w = %s, morph3e = %s,"
        "  morph4n = %s, morph4s = %s, morph4w = %s, morph4e = %s"
        " WHERE characterID = %u",
        _VoN(data, accessoryID),
        _VoN(data, beardID),
        data.costumeID,
        _VoN(data, decoID),
        data.eyebrowsID,
        data.eyesID,
        data.hairID,
        _VoN(data, lipstickID),
        _VoN(data, makeupID),
        data.skinID,
        data.backgroundID,
        data.lightID,
        data.headRotation1, data.headRotation2, data.headRotation3,
        data.eyeRotation1, data.eyeRotation2, data.eyeRotation3,
        data.camPos1, data.camPos2, data.camPos3,
        _VoN(data, morph1n), _VoN(data, morph1s), _VoN(data, morph1w), _VoN(data, morph1e),
        _VoN(data, morph2n), _VoN(data, morph2s), _VoN(data, morph2w), _VoN(data, morph2e),
        _VoN(data, morph3n), _VoN(data, morph3s), _VoN(data, morph3w), _VoN(data, morph3e),
        _VoN(data, morph4n), _VoN(data, morph4s), _VoN(data, morph4w), _VoN(data, morph4e),
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to save character's %u appearance: %s.", characterID, err.c_str());
        return false;
    }

    return true;
}

bool InventoryDB::SaveCorpMemberInfo(uint32 characterID, const CorpMemberInfo &data) {
    DBerror err;

    if(!sDatabase.RunQuery(err,
        "UPDATE character_"
        " SET"
        "  corpRole = " I64u ","
        "  rolesAtAll = " I64u ","
        "  rolesAtBase = " I64u ","
        "  rolesAtHQ = " I64u ","
        "  rolesAtOther = " I64u
        " WHERE characterID = %u",
        data.corpRole,
        data.rolesAtAll,
        data.rolesAtBase,
        data.rolesAtHQ,
        data.rolesAtOther,
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to update corp member info of character %u: %s.", characterID, err.c_str());
        return false;
    }

    return true;
}

// Undefine the macro used only in NewCharacter and SaveCharacterAppearance.
#undef _VoN

bool InventoryDB::DeleteCharacter(uint32 characterID) {
    DBerror err;

    // eveMailDetails
    if(!sDatabase.RunQuery(err,
        "DELETE FROM eveMailDetails"
        "  USING eveMail, eveMailDetails"
        " WHERE"
        "   eveMail.messageID = eveMailDetails.messageID"
        "  AND"
        "   (senderID = %u OR channelID = %u)",
        characterID, characterID))
    {
        _log(DATABASE__ERROR, "Failed to delete mail details of character %u: %s.", characterID, err.c_str());
        // ignore the error
        _log(DATABASE__MESSAGE, "Ignoring error.");
    }

    // eveMail
    if(!sDatabase.RunQuery(err,
        "DELETE FROM eveMail"
        " WHERE (senderID = %u OR channelID = %u)",
        characterID, characterID))
    {
        _log(DATABASE__ERROR, "Failed to delete mail of character %u: %s.", characterID, err.c_str());
        // ignore the error
        _log(DATABASE__MESSAGE, "Ignoring error.");
    }

    // crpCharShares
    if(!sDatabase.RunQuery(err,
        "DELETE FROM crpCharShares"
        " WHERE characterID = %u",
        characterID))
    {
        codelog(SERVICE__ERROR, "Failed to delete shares of character %u: %s.", characterID, err.c_str());
        // ignore the error
        _log(DATABASE__MESSAGE, "Ignoring error.");
    }

    // bookmarks
    if(!sDatabase.RunQuery(err,
        "DELETE FROM bookmarks"
        " WHERE ownerID = %u",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to delete bookmarks of character %u: %s.", characterID, err.c_str());
        // ignore the error
        _log(DATABASE__MESSAGE, "Ignoring error.");
    }

    // market_journal
    if(!sDatabase.RunQuery(err,
        "DELETE FROM market_journal"
        " WHERE characterID = %u",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to delete market journal of character %u: %s.", characterID, err.c_str());
        // ignore the error
        _log(DATABASE__MESSAGE, "Ignoring error.");
    }

    // market_orders
    if(!sDatabase.RunQuery(err,
        "DELETE FROM market_orders"
        " WHERE charID =% lu",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to delete market orders of character %u: %s.", characterID, err.c_str());
        // ignore the error
        _log(DATABASE__MESSAGE, "Ignoring error.");
    }

    // market_transactions
    if(!sDatabase.RunQuery(err,
        "DELETE FROM market_transactions"
        " WHERE clientID = %u",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to delete market transactions of character %u: %s.", characterID, err.c_str());
        // ignore the error
        _log(DATABASE__MESSAGE, "Ignoring error.");
    }

    // chrStandings
    if(!sDatabase.RunQuery(err,
        "DELETE FROM chrStandings"
        " WHERE characterID = %u",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to delete standings of character %u: %s.", characterID, err.c_str());
        // ignore the error
        _log(DATABASE__MESSAGE, "Ignoring error.");
    }

    // chrNPCStandings
    if(!sDatabase.RunQuery(err,
        "DELETE FROM chrNPCStandings"
        " WHERE characterID = %u",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to delete NPC standings of character %u: %s.", characterID, err.c_str());
        // ignore the error
        _log(DATABASE__MESSAGE, "Ignoring error.");
    }

    // chrEmployment
    if(!sDatabase.RunQuery(err,
        "DELETE FROM chrEmployment"
        " WHERE characterID = %u",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to delete employment of character %u: %s.", characterID, err.c_str());
        // ignore the error
        _log(DATABASE__MESSAGE, "Ignoring error.");
    }

    // character_
    if(!sDatabase.RunQuery(err,
        "DELETE FROM character_"
        " WHERE characterID = %u",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to delete character %u: %s.", characterID, err.c_str());
        return false;
    }

    return true;
}

bool InventoryDB::GetCelestialObject(uint32 celestialID, CelestialObjectData &into) {
    DBQueryResult res;

    if( IsUniverseCelestial( celestialID )
        || IsStaticMapItem( celestialID )
        || IsRegion( celestialID )
        || IsConstellation( celestialID )
        || IsSolarSystem( celestialID )
        || IsStargate( celestialID ) )
    {
        // This Celestial object is a static celestial, so get its data from the 'mapDenormalize' table:
        if(!sDatabase.RunQuery(res,
            "SELECT"
            " security, radius, celestialIndex, orbitIndex"
            " FROM mapDenormalize"
            " WHERE itemID = %u",
            celestialID))
        {
            _log(DATABASE__ERROR, "Failed to query celestial object %u: %s.", celestialID, res.error.c_str());
            return false;
        }

        DBResultRow row;
        if(!res.GetRow(row)) {
            _log(DATABASE__ERROR, "Celestial object %u not found.", celestialID);

            return false;
        }

        into.security = (row.IsNull(0) ? 0 : row.GetDouble(0));
        into.radius = (row.IsNull(1) ? 0 : row.GetDouble(1));
        into.celestialIndex = (row.IsNull(2) ? 0 : row.GetUInt(2));
        into.orbitIndex = (row.IsNull(3) ? 0 : row.GetUInt(3));
    }
    else
    {
        // Quite possibly, this Celestial object is a dynamic one, so try to get its data from the 'entity' table,
        // and if it's not there either, then flag an error.
        if(!sDatabase.RunQuery(res,
            "SELECT"
            " entity.itemID, "
            " invTypes.radius "
            " FROM entity "
            "  LEFT JOIN invTypes ON entity.typeID = invTypes.typeID"
            " WHERE entity.itemID = %u",
            celestialID))
        {
            _log(DATABASE__ERROR, "Failed to query celestial object %u: %s.", celestialID, res.error.c_str());
            return false;
        }

        DBResultRow row;
        if(!res.GetRow(row)) {
            _log(DATABASE__ERROR, "Celestial object %u not found.", celestialID);

            return false;
        }

        into.security = 1.0;
        into.radius = (row.IsNull(1) ? 0 : row.GetDouble(1));
        into.celestialIndex = 0;
        into.orbitIndex = 0;
    }

    return true;
}

bool InventoryDB::GetSolarSystem(uint32 solarSystemID, SolarSystemData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        " xMin, yMin, zMin,"
        " xMax, yMax, zMax,"
        " luminosity,"
        " border, fringe, corridor, hub, international, regional, constellation,"
        " security, factionID, radius, sunTypeID, securityClass"
        " FROM mapSolarSystems"
        " WHERE solarSystemID=%u", solarSystemID))
    {
        _log(DATABASE__ERROR, "Error in query for solar system %u: %s.", solarSystemID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No data found for solar system %u.", solarSystemID);
        return false;
    }

    into.minPosition = GPoint(row.GetDouble(0), row.GetDouble(1), row.GetDouble(2));
    into.maxPosition = GPoint(row.GetDouble(3), row.GetDouble(4), row.GetDouble(5));
    into.luminosity = row.GetDouble(6);

    into.border = row.GetInt(7) ? true : false;
    into.fringe = row.GetInt(8) ? true : false;
    into.corridor = row.GetInt(9) ? true : false;
    into.hub = row.GetInt(10) ? true : false;
    into.international = row.GetInt(11) ? true : false;
    into.regional = row.GetInt(12) ? true : false;
    into.constellation = row.GetInt(13) ? true : false;

    into.security = row.GetDouble(14);
    into.factionID = (row.IsNull(15) ? 0 : row.GetUInt(15));
    into.radius = row.GetDouble(16);
    into.sunTypeID = row.GetUInt(17);
    into.securityClass = row.GetText(18);

    return true;
}

bool InventoryDB::GetStation(uint32 stationID, StationData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        " security, dockingCostPerVolume, maxShipVolumeDockable, officeRentalCost, operationID,"
        " reprocessingEfficiency, reprocessingStationsTake, reprocessingHangarFlag"
        " FROM staStations"
        " WHERE stationID = %u",
        stationID))
    {
        _log(DATABASE__ERROR, "Failed to query data for station %u: %s.", stationID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Station %u not found.", stationID);
        return false;
    }

    into.security = row.GetUInt(0);
    into.dockingCostPerVolume = row.GetDouble(1);
    into.maxShipVolumeDockable = row.GetDouble(2);
    into.officeRentalCost = row.GetUInt(3);
    into.operationID = row.GetUInt(4);

    into.reprocessingEfficiency = row.GetDouble(5);
    into.reprocessingStationsTake = row.GetDouble(6);
    into.reprocessingHangarFlag = (EVEItemFlags)row.GetInt(7);

    return true;
}

bool InventoryDB::LoadSkillQueue(uint32 characterID, SkillQueue &into) {
    DBQueryResult res;

    if( !sDatabase.RunQuery( res,
        "SELECT"
        " typeID, level"
        " FROM chrSkillQueue"
        " WHERE characterID = %u"
        " ORDER BY orderIndex ASC",
        characterID ) )
    {
        _log(DATABASE__ERROR, "Failed to query skill queue of character %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while( res.GetRow( row ) )
    {
        QueuedSkill qs;
        qs.typeID = row.GetUInt( 0 );
        qs.level = row.GetUInt( 1 );

        into.push_back( qs );
    }

    return true;
}

bool InventoryDB::SaveSkillQueue(uint32 characterID, const SkillQueue &queue) {
    DBerror err;

    if( !sDatabase.RunQuery( err,
        "DELETE"
        " FROM chrSkillQueue"
        " WHERE characterID = %u",
        characterID ) )
    {
        _log(DATABASE__ERROR, "Failed to delete skill queue of character %u: %s.", characterID, err.c_str());
        return false;
    }

    if( queue.empty() )
        // nothing else to do
        return true;

    // now build insert query:
    std::string query;

    for(size_t i = 0; i < queue.size(); i++)
    {
        const QueuedSkill &qs = queue[ i ];

        char buf[ 64 ];
        snprintf( buf, 64, "(%u, %lu, %u, %u)", characterID, (unsigned long)i, qs.typeID, qs.level );

        if( i != 0 )
            query += ',';
        query += buf;
    }

    if( !sDatabase.RunQuery( err,
        "INSERT"
        " INTO chrSkillQueue (characterID, orderIndex, typeID, level)"
        " VALUES %s",
        query.c_str() ) )
    {
        _log(DATABASE__ERROR, "Failed to insert skill queue of character %u: %s.", characterID, err.c_str());
        return false;
    }

    return true;
}
bool InventoryDB::GetTypeID(uint32 itemID, uint32 &typeID)
{
	DBQueryResult res;
	DBResultRow row;
	
	if(!sDatabase.RunQuery(res,
		" SELECT "
		" typeID "
		" FROM entity "
		" WHERE itemID = %u ",itemID))
	{
		_log(DATABASE__ERROR, "Failed to query for itemID = %u", itemID);
	}

    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Item of type %u not found.", itemID);
        return false;
    }

	typeID = row.GetUInt(0);
    return true;
}

bool InventoryDB::GetModulePowerSlotByTypeID(uint32 typeID, uint32 &into)
{
	DBQueryResult res;
	DBResultRow row;

	if(!sDatabase.RunQuery(res,
		" SELECT "
		" groupID "
		" FROM invtypes "
		" WHERE typeID = '%u' ",
		typeID))
	{
		_log(DATABASE__ERROR, "Failed to get groupID for typeID = %u", typeID);
	}
	
	if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Item of type %u not found.", typeID);
        return false;
    }

	uint32 groupID = row.GetUInt(0);

	//TODO: put in invCat
	switch( groupID) {
		case EVEDB::invGroups::Rig_Armor:
		case EVEDB::invGroups::Rig_Astronautic:
		case EVEDB::invGroups::Rig_Drone:
		case EVEDB::invGroups::Rig_Electronics:
		case EVEDB::invGroups::Rig_Electronics_Superiority:
		case EVEDB::invGroups::Rig_Energy_Grid:
		case EVEDB::invGroups::Rig_Energy_Weapon:
		case EVEDB::invGroups::Rig_Hybrid_Weapon:
		case EVEDB::invGroups::Rig_Launcher:
		case EVEDB::invGroups::Rig_Mining:
		case EVEDB::invGroups::Rig_Projectile_Weapon:
		case EVEDB::invGroups::Rig_Security_Transponder:
		case EVEDB::invGroups::Rig_Shield:

			into = 0;
			return true;
	}


	if(!sDatabase.RunQuery(res,
		" SELECT "
		" effectID "
		" FROM dgmTypeEffects "
		" WHERE typeID = '%u' AND ( effectID = 11 OR effectID = 12 OR effectID = 13 ) ",
		typeID))
	{
		_log(DATABASE__ERROR, "Failed to get slot for typeID = %u", typeID);
	}

	if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Item of type %u not found.", typeID);
        return false;
    }

	uint32 slotType = row.GetUInt(0);

	//such crap...
	if( slotType == 11 ) {
		into = 1;
		return true;
	} else if( slotType == 12 ) {
		into = 3;
		return true;
	} else if( slotType == 13 ){
		into = 2;
		return true;
	} else
		throw PyException( MakeCustomError( "Item of type: %u is not fittable (could be a rig, as they haven't been implemented)", typeID ) );

}

bool InventoryDB::GetOpenPowerSlots(uint32 slotType, ShipRef ship, uint32 &into)
{
	DBQueryResult res;
	uint32 attributeID = 0;
	uint32 firstFlag;
	DBResultRow row;
	uint32 slotsOnShip;

	if( slotType == 0 )
	{
		//TODO: Implement Rigs
		attributeID = 1137;
		firstFlag = 92; //rigslot0
		//slotsOnShip = ship->rigSlots();
        slotsOnShip = ship->GetAttribute(AttrRigSlots).get_int();
	}
	else if( slotType == 1 )
	{
		attributeID = 12;
		firstFlag = 11; //lowslot0
		//slotsOnShip = ship->lowSlots();
        slotsOnShip = ship->GetAttribute(AttrLowSlots).get_int();
	}
	else if( slotType == 2 )
	{
		attributeID = 13;
		firstFlag = 19; //medslot0
		//slotsOnShip = ship->medSlots();
        slotsOnShip = ship->GetAttribute(AttrMedSlots).get_int();
	}
	else if( slotType == 3 )
	{
		attributeID = 14;
		firstFlag = 27; //hislot0
		//slotsOnShip = ship->hiSlots();
        slotsOnShip = ship->GetAttribute(AttrHiSlots).get_int();
	}	
	
	for( uint32 flag = firstFlag; flag < (firstFlag + slotsOnShip); flag++ )
	{
        // this is far from efficient as we are iterating trough all of the ships item slots.... every iteration... so this will be slow when you got loads of players
        // with a single free slot.
    	if(ship->IsEmptyByFlag((EVEItemFlags)flag))
		{
			into = flag;
			return true;
		}
	}

	//Only time it should make it this far...
	throw PyException( MakeCustomError( "There are no available slots" ));

	return false;

}



