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
    Author:      Allan, bb2k
*/

#include "eve-server.h"

#include "search/SearchDB.h"

/**   search runs thru these types...even inventory (type 10)
searchResultAgent = 1
searchResultCharacter = 2
searchResultCorporation = 3
searchResultAlliance = 4
searchResultFaction = 5
searchResultConstellation = 6
searchResultSolarSystem = 7
searchResultRegion = 8
searchResultStation = 9
searchResultInventoryType = 10
searchResultAllOwners = [1, 2, 3, 4, 5]
searchResultAllLocations = [6, 7, 8, 9]
searchMaxResults = 500
searchMinWildcardLength = 3
*/

PyRep *SearchDB::Query(std::string string, std::vector<int> *searchID, uint32 charID) {
    std::string id = "";

    PyDict *dict = new PyDict();
    DBQueryResult res;

    for (uint8 i=0; i < searchID->size(); i++) {
        switch(searchID->at(i)) {
            case 1: //searchResultAgent = 1
                sDatabase.RunQuery(res,
                    "SELECT"
                    "   characterID AS agentID"
                    " FROM chrNPCCharacters"
                    " WHERE characterName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                id = "agentID";
                break;
            case 2: //searchResultCharacter = 2
                sDatabase.RunQuery(res,
                    "SELECT"
                    "   characterID AS ownerID"
                    " FROM chrCharacters"
                    " WHERE characterName LIKE '%s' ", string.c_str() );
                id = "ownerID";
                break;
            case 3: //searchResultCorporation = 3
                sDatabase.RunQuery(res,
                    "SELECT"
                    "   corporationID AS ownerID"
                    " FROM crpCorporation"
                    " WHERE corporationName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                id = "ownerID";
                break;
            case 4: //searchResultAlliance = 4
                sDatabase.RunQuery(res,
                    "SELECT allianceID AS ownerID"
                    " FROM alnAlliance"
                    " WHERE shortName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                id = "ownerID";
                break;
            case 5: //searchResultFaction = 5
                sDatabase.RunQuery(res,
                    "SELECT factionID AS ownerID"
                    " FROM facFactions"
                    " WHERE factionName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                id = "ownerID";
                break;
            case 6: //searchResultConstellation = 6
                sDatabase.RunQuery(res,
                    "SELECT"
                    "   constellationID AS itemID"
                    " FROM mapConstellations"
                    " WHERE constellationName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                id = "itemID";
                break;
            case 7: //searchResultSolarSystem = 7
                sDatabase.RunQuery(res,
                    "SELECT "
                    "   solarSystemID AS itemID"
                    " FROM mapSolarSystems "
                    " WHERE solarSystemName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                id = "itemID";
                break;
            case 8: //searchResultRegion = 8
                sDatabase.RunQuery(res,
                    "SELECT "
                    "   regionID AS itemID"
                    " FROM mapRegions"
                    " WHERE regionName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                id = "itemID";
                break;
            case 9: //searchResultStation = 9
                sDatabase.RunQuery(res,
                    "SELECT "
                    "   stationID AS itemID"
                    " FROM staStations "
                    " WHERE stationName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                id = "itemID";
                break;
            case 10:    //searchResultInventoryType = 10
                sDatabase.RunQuery(res,
                    "SELECT"
                    "   typeID"
                    " FROM entity"
                    " WHERE itemName LIKE '%s'"
                    " AND ownerID = %u", string.c_str(), charID );
                id = "typeID";
                break;
        }
        if (res.GetRowCount())
            dict->SetItem(new PyInt(searchID->at(i)),DBResultToIntIntDict(res));
    }

    return dict;
}

PyRep *SearchDB::QuickQuery(std::string string, std::vector<int> *searchID, uint32 charID, bool hideNPC, bool onlyAltName) {
    uint8 size(searchID->size());

    if (((size == 1) and (searchID->at(0) == 2))
    or  (hideNPC)) {
        /** @todo i dont remember what this was for, but need to finish it anyway */
    }

    PyList *result = new PyList();
    DBQueryResult res;
    DBResultRow row;

    for (uint8 i=0; i < searchID->size(); i++) {
        switch(searchID->at(i)) {
            case 1: //searchResultAgent = 1
                sDatabase.RunQuery(res,
                    "SELECT characterID"
                    " FROM chrNPCCharacters"
                    " WHERE characterName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                break;
            case 2: //searchResultCharacter = 2
                sDatabase.RunQuery(res,
                    "SELECT characterID"
                    " FROM chrCharacters"
                    " WHERE characterName LIKE '%s' ", string.c_str() );
                break;
            case 3: //searchResultCorporation = 3
                sDatabase.RunQuery(res,
                    "SELECT corporationID"
                    " FROM crpCorporation"
                    " WHERE corporationName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                break;
            case 4: //searchResultAlliance = 4
                sDatabase.RunQuery(res,
                    "SELECT allianceID"
                    " FROM alnAlliance"
                    " WHERE shortName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                break;
            case 5: //searchResultFaction = 5
                sDatabase.RunQuery(res,
                    "SELECT factionID"
                    " FROM facFactions"
                    " WHERE factionName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                break;
            case 6: //searchResultConstellation = 6
                sDatabase.RunQuery(res,
                    "SELECT constellationID"
                    " FROM mapConstellations"
                    " WHERE constellationName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                break;
            case 7: //searchResultSolarSystem = 7
                sDatabase.RunQuery(res,
                    "SELECT solarSystemID"
                    " FROM mapSolarSystems "
                    " WHERE solarSystemName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                break;
            case 8: //searchResultRegion = 8
                sDatabase.RunQuery(res,
                    "SELECT regionID"
                    " FROM mapRegions"
                    " WHERE regionName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                break;
            case 9: //searchResultStation = 9
                sDatabase.RunQuery(res,
                    "SELECT stationID"
                    " FROM staStations "
                    " WHERE stationName LIKE '%s' "
                    " LIMIT 0, 10", string.c_str() );
                break;
            case 10:    //searchResultInventoryType = 10
                sDatabase.RunQuery(res,
                    "SELECT typeID"
                    " FROM entity"
                    " WHERE itemName LIKE '%s'"
                    " AND ownerID = %u", string.c_str(), charID );
                break;
        }
        while (res.GetRow(row)) {
            result->AddItem( new PyInt(row.GetUInt(0) ));
        }
    }

    return result;
}
