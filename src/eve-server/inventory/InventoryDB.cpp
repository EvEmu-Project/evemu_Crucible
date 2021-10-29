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
    Author:     Zhur
    Updates:    Allan
*/

/** @todo  most of this needs to go into static data */

#include "eve-server.h"

#include "Client.h"
#include "PyCallable.h"
#include "character/Character.h"
#include "inventory/ItemType.h"
#include "manufacturing/Blueprint.h"
#include "ship/Ship.h"
#include "station/Station.h"
#include "system/Asteroid.h"
#include "system/SolarSystem.h"

bool InventoryDB::GetCharacterType(uint8 bloodlineID, CharacterTypeData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  bloodlineName,"
        "  raceID,"
        "  description,"
        "  maleDescription,"
        "  femaleDescription,"
        "  corporationID,"
        "  perception,"
        "  willpower,"
        "  charisma,"
        "  memory,"
        "  intelligence,"
        "  shortDescription,"
        "  shortMaleDescription,"
        "  shortFemaleDescription "
        " FROM chrBloodlines "
        " WHERE bloodlineID = %u",
        bloodlineID))
    {
        codelog(DATABASE__ERROR, "Failed to query bloodline %u: %s.", bloodlineID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data found for bloodline %u.", bloodlineID);
        return false;
    }

    into.bloodlineName = row.GetText(0);
    into.race = row.GetUInt(1);
    into.description = row.GetText(2);
    into.maleDescription = row.GetText(3);
    into.femaleDescription = row.GetText(4);
    into.corporationID = row.GetUInt(5);
    into.perception = row.GetUInt(6);
    into.willpower = row.GetUInt(7);
    into.charisma = row.GetUInt(8);
    into.memory = row.GetUInt(9);
    into.intelligence = row.GetUInt(10);
    into.shortDescription = row.GetText(11);
    into.shortMaleDescription = row.GetText(12);
    into.shortFemaleDescription = row.GetText(13);
    return true;
}

bool InventoryDB::GetCharacterTypeByBloodline(uint8 bloodlineID, uint16& characterTypeID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT typeID FROM bloodlineTypes WHERE bloodlineID = %u", bloodlineID)) {
        codelog(DATABASE__ERROR, "Failed to query bloodline %u: %s.", bloodlineID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data for bloodline %u.", bloodlineID);
        return false;
    }

    characterTypeID = row.GetUInt(0);

    return true;
}

bool InventoryDB::GetBloodlineByCharacterType(uint16 characterTypeID, uint8 &bloodlineID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT bloodlineID FROM bloodlineTypes WHERE typeID = %u", characterTypeID)) {
        codelog(DATABASE__ERROR, "Failed to query character type %u: %s.", characterTypeID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data for character type %u.", characterTypeID);
        return false;
    }

    bloodlineID = row.GetUInt(0);

    return true;
}

bool InventoryDB::GetCharacterType(uint16 characterTypeID, uint8 &bloodlineID, CharacterTypeData &into) {
    if(!GetBloodlineByCharacterType(characterTypeID, bloodlineID))
        return false;
    return GetCharacterType(bloodlineID, into);
}

bool InventoryDB::GetCharacterTypeByBloodline(uint8 bloodlineID, uint16 &characterTypeID, CharacterTypeData &into) {
    if(!GetCharacterTypeByBloodline(bloodlineID, characterTypeID))
        return false;
    return GetCharacterType(bloodlineID, into);
}

/* this is only called by Inventory::LoadContents()
 * it is optimized for specific calling objects, to avoid multiple db hits while loading,
 * and to load only things needed for this object at the time of the call.
 */
bool InventoryDB::GetItemContents(OwnerData &od, std::vector<uint32> &into) {
    std::stringstream query;
    query << "SELECT itemID FROM entity WHERE locationID = ";
    query << od.locID;

    if (sDataMgr.IsSolarSystem(od.locID)) {
        query << " AND ownerID = " << od.ownerID;
    } else if (sDataMgr.IsStation(od.locID)) {
        if (od.ownerID == 1) {
            /* this will get agents in station */
            query << " AND ownerID < " << maxNPCItem;
            query << " AND itemID < " << maxNPCItem;
        } else {
            if (IsPlayerCorp(od.corpID)) {
                // check for items owned by corp in players cargo/hangar
                query << " AND ((ownerID = " << od.ownerID << ") OR (ownerID = " << od.corpID << "))";
            } else {
                query << " AND ownerID = " << od.ownerID;
            }
        }
    } else if (IsCharacterID(od.locID)) {
        if (od.ownerID == 1) {
            // not sure what to do here....
        } else if (IsPlayerCorp(od.corpID)) {
            // check for items owned by corp in players cargo/hangar...this *should* work for cap ships' cargo, also
           query << " AND ((ownerID = " << od.ownerID << ") OR (ownerID = " << od.corpID << "))";
        } else {
            query << " AND ownerID = " << od.ownerID;
        }
    } else if (IsOfficeID(od.locID)) {
        // may not need this, as location is officeID, but items MAY be owned by players in corp hangar.
        //query << " AND ownerID = " << od.ownerID;
    }

    query << " ORDER BY itemID";

    DBQueryResult res;
    if(!sDatabase.RunQuery(res,query.str().c_str() )) {
        codelog(DATABASE__ERROR, "Error in GetItemContents query for locationID %u: %s", od.locID, res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "GetItemContents: '%s' returned %lu items", query.str().c_str(), res.GetRowCount());
    DBResultRow row;
    while( res.GetRow( row ) )
        into.push_back( row.GetUInt( 0 ) );

    return true;
}

/*  not used? */
bool InventoryDB::GetItemContents(uint32 itemID, EVEItemFlags flag, std::vector<uint32> &into)
{
    DBQueryResult res;

    if( !sDatabase.RunQuery( res,
        "SELECT "
        "  itemID"
        " FROM entity "
        " WHERE locationID=%u"
        "  AND flag=%d",
        itemID, (int)flag ) )
    {
        codelog(DATABASE__ERROR, "Error in GetItemContents query for item %u: %s", itemID, res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "GetItemContents for item %u returned %lu items", itemID, res.GetRowCount());
    DBResultRow row;
    while( res.GetRow( row ) )
        into.push_back( row.GetUInt( 0 ) );

    return true;
}

/*  not used? */
bool InventoryDB::GetItemContents(uint32 itemID, EVEItemFlags flag, uint32 ownerID, std::vector<uint32> &into)
{
    DBQueryResult res;

    if( !sDatabase.RunQuery( res,
        "SELECT "
        "  itemID"
        " FROM entity "
        " WHERE locationID=%u"
        "  AND flag=%d"
        "  AND ownerID=%u",
        itemID, (int)flag, ownerID ) )
    {
        codelog(DATABASE__ERROR, "Error in GetItemContents query for item %u with flag %i: %s", itemID, (int)flag, res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "GetItemContents for item %u with flag %i returned %lu items", itemID, (int)flag, res.GetRowCount());
    DBResultRow row;
    while( res.GetRow( row ) )
        into.push_back( row.GetUInt( 0 ) );

    return true;
}

void InventoryDB::DeleteTrackingCans()
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE customInfo LIKE '%Position Test%'");  // 90.63s on main, 0.037s on dev
    //sDatabase.RunQuery(err, "DELETE FROM entity WHERE itemName LIKE '%Bubble%'");         // 66.75s on main, 0.036s on dev
}

bool InventoryDB::GetCharacterData(uint32 characterID, CharacterData &into) {
    DBQueryResult res;

    if (IsAgent(characterID)) {
        if(!sDatabase.RunQuery(res,
            "SELECT"
            "   0 as accountID,"
            "   title,"
            "   description,"
            "   gender,"
            "   bounty,"
            "   0 as balance,"
            "   0 as aurBalance,"
            "   securityRating,"
            "   0 as logonMinutes,"
            "   stationID,"
            "   solarSystemID,"
            "   constellationID,"
            "   regionID,"
            "   ancestryID,"
            "   0 AS bloodlineID,"      /** @todo fix these */
            "   0 AS raceID,"
            "   careerID,"
            "   schoolID,"
            "   careerSpecialityID,"
            "   createDateTime,"
            "   0 as shipID,"       // update this when agents are in space
            "   0 as capsuleID,"
            "   flag,"
            "   name,"
            "   0 AS skillPoints,"
            "   typeID"
			" FROM chrNPCCharacters AS chr"
            " WHERE characterID = %u", characterID)) {
            codelog(DATABASE__ERROR, "Error in GetCharacter query: %s", res.error.c_str());
            return false;
            }
    } else {
        if(!sDatabase.RunQuery(res,
            "SELECT"
            "   accountID,"
            "   title,"
            "   description,"
            "   gender,"
            "   bounty,"
            "   balance,"
            "   aurBalance,"
            "   securityRating,"
            "   logonMinutes,"
            "   stationID,"
            "   solarSystemID,"
            "   constellationID,"
            "   regionID,"
            "   ancestryID,"
            "   bloodlineID,"
            "   raceID,"
            "   careerID,"
            "   schoolID,"
            "   careerSpecialityID,"
            "   createDateTime,"
            "   shipID,"
            "   capsuleID,"
            "   flag,"
            "   characterName,"
            "   skillPoints,"
            "   typeID"
            " FROM chrCharacters"
            " WHERE characterID = %u", characterID))
        {
            codelog(DATABASE__ERROR, "Error in GetCharacter query: %s", res.error.c_str());
            return false;
        }
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data found for character %u.", characterID);
        return false;
    }

    into.accountID = row.IsNull( 0 ) ? 0 : row.GetUInt( 0 );
    into.title = row.GetText( 1 );
    into.description = row.GetText( 2 );
    into.gender = row.GetInt( 3 ) ? true : false;
    into.bounty = row.GetDouble( 4 );
    into.balance = row.GetDouble( 5 );
    into.aurBalance = row.GetDouble( 6 );
    into.securityRating = row.GetDouble( 7 );
    into.logonMinutes = row.GetUInt( 8 );
    into.stationID = row.GetUInt( 9 );
    into.solarSystemID = row.GetUInt( 10 );
    into.locationID = (into.stationID == 0 ? into.solarSystemID : into.stationID);
    into.constellationID = row.GetUInt( 11 );
    into.regionID = row.GetUInt( 12 );
    into.ancestryID = row.GetUInt( 13 );
    into.bloodlineID =  row.GetUInt( 14 );
    into.raceID = row.GetUInt( 15 );
    into.careerID =row.GetUInt( 16 );
    into.schoolID = row.GetUInt( 17 );
    into.careerSpecialityID = row.GetUInt( 18 );
    into.createDateTime = row.GetInt64( 19 );
    into.shipID = row.GetUInt( 20 );
    into.capsuleID = row.GetUInt( 21 );
    into.flag = row.GetUInt(22);
    into.name = row.GetText(23);
    into.skillPoints = row.GetUInt(24);
    into.typeID = row.GetUInt(25);

    return true;
}

bool InventoryDB::GetCorpData(uint32 characterID, CorpData &into) {
    DBQueryResult res;
    DBResultRow row;

    if (IsAgent(characterID)) {
        into.corpAccountKey = 1001;

        if (!sDatabase.RunQuery(res,
            "SELECT corporationID, locationID"
            " FROM agtAgents"
            " WHERE agentID = %u",
            characterID))
        {
            codelog(DATABASE__ERROR, "Failed to query corp member info of character %u: %s.", characterID, res.error.c_str());
            return false;
        }

        if (!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "No corp member info found for character %u.", characterID);
            return false;
        }
        into.corporationID = row.GetInt(0);
        into.baseID = row.GetInt(1);
    } else {
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  startDateTime,"
            "  corporationID,"
            "  corpAccountKey,"
            "  corpRole,"
            "  rolesAtAll,"
            "  rolesAtBase,"
            "  rolesAtHQ,"
            "  rolesAtOther,"
            "  grantableRoles,"
            "  grantableRolesAtBase,"
            "  grantableRolesAtHQ,"
            "  grantableRolesAtOther,"
            "  baseID"
            " FROM chrCharacters"
            " WHERE characterID = %u",
            characterID))
        {
            codelog(DATABASE__ERROR, "Failed to query corp member info of character %u: %s.", characterID, res.error.c_str());
            return false;
        }
        if (!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "No corp member info found for character %u.", characterID);
            return false;
        }

        into.startDateTime = row.GetInt64(0);
        into.corporationID = row.GetInt(1);
        into.corpAccountKey = row.GetInt(2);
        into.corpRole = row.GetInt64(3);
        into.rolesAtAll = row.GetInt64(4);
        into.rolesAtBase = row.GetInt64(5);
        into.rolesAtHQ = row.GetInt64(6);
        into.rolesAtOther = row.GetInt64(7);
        into.grantableRoles = row.GetInt64(8);
        into.grantableRolesAtBase = row.GetInt64(9);
        into.grantableRolesAtHQ = row.GetInt64(10);
        into.grantableRolesAtOther = row.GetInt64(11);
        into.baseID = row.GetInt(12);
    }

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  taxRate,"
        "  stationID,"
        "  allianceID,"
        "  warFactionID,"
        "  corporationName,"
        "  tickerName"
        " FROM crpCorporation"
        " WHERE corporationID = %u", into.corporationID))
    {
        codelog(DATABASE__ERROR, "Failed to query HQ of character's %u corporation %u: %s.", characterID, into.corporationID, res.error.c_str());
        return false;
    }

    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No HQ found for character's %u corporation.", characterID);
        return false;
    }

    into.taxRate = row.GetDouble(0);
    into.corpHQ = (row.IsNull(1) ? 0 : row.GetUInt(1));
    into.allianceID = (row.IsNull(2) ? 0 : row.GetUInt(2));
    into.warFactionID = (row.IsNull(3) ? 0 : row.GetUInt(3));
    into.name = row.GetText(4);
    into.ticker = row.GetText(5);

    return true;
}

bool InventoryDB::GetCelestialObject(uint32 celestialID, CelestialObjectData &into) {
    DBQueryResult res;

    if( IsStaticMapItem(celestialID)) {
        // This Celestial object is a static celestial, so get its data from the 'mapDenormalize' table:
        if(!sDatabase.RunQuery(res,
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
        if(!res.GetRow(row)) {
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
        if(!sDatabase.RunQuery(res,
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
        if(!res.GetRow(row)) {
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

bool InventoryDB::GetSolarSystem(uint32 solarSystemID, SolarSystemData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
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
    if(!res.GetRow(row)) {
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
