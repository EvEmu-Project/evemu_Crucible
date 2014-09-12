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
    Author:        BB2k,Allan
*/

#include "eve-server.h"

#include "search/SearchDB.h"
#include "search/SearchMgrService.h"

PyRep *SearchDB::QuickQuery(std::string match, std::vector<int> *SearchID) {
    DBQueryResult     res; 
    DBResultRow       row;
    uint32            i, size;
    std::stringstream st;
    std::stringstream supplement;
    std::string       query      = "";
    std::string       equal      = "";
    std::string       matchEsc   = "";

    //SearchIDs are :
    // 1 : agent
    // 2 : character
    // 3 : corporation
    // 4 : alliance
    // 5 : faction
    // 6 : constellation
    // 7 : solar system
    // 8 : region
    // 9 : station

    // Find out if search is exact or not and remove the trailing '*'
    std::size_t found=match.find('*');
    if (found!=std::string::npos) {
      match.erase (std::remove(match.begin(), match.end(), '*'), match.end());
      equal = " RLIKE ";
    } else {
      equal = " = ";
    }


    // If the SearchID is for character we must filter out agents
    size = SearchID->size();
    if ((size == 1) && (SearchID->at(0)) == 2){
        supplement << "AND itemId >= ";
        supplement << EVEMU_MINIMUM_ID;
    }

    // Transform SearchId in groupID to search the rights typeIDs
    int transform[9] = {1,1,2,32,19,4,5,3,15};
    for(i=0; i<size; i++)
    {
        st << transform[SearchID->at(i)-1];
        if (i<(size-1))
            st << ", ";
    }
    
    // Escape the match string
    sDatabase.DoEscapeString(matchEsc, match.c_str());

    //Form the query and execute it
    query = "SELECT itemID,itemName FROM entity"
            " WHERE itemName RLIKE '%s' %s"
            " AND typeID in (SELECT typeID FROM invTypes LEFT JOIN invGroups ON invTypes.groupid = invGroups.groupID"
            " WHERE invGroups.groupID IN (%s))"
            " ORDER BY itemName";


    _log(SERVICE__MESSAGE, query.c_str(), matchEsc.c_str(), supplement.str().c_str() ,st.str().c_str());

    if(!sDatabase.RunQuery(res,query.c_str(), matchEsc.c_str(), supplement.str().c_str() , st.str().c_str() ))
    {
        _log(SERVICE__ERROR, "Error in LookupChars query: %s", res.error.c_str());
        return NULL;
    }

    // The cliant wants a List if Ids in return
    PyList *result = new PyList();
    while( res.GetRow( row ) ){
        result->AddItem( new PyInt(row.GetUInt(0) ));
    }

    return result;


}


PyRep *SearchDB::Query(std::string match,  std::vector<int> *searchID) {

    DBQueryResult res;
    DBResultRow   row;
    std::string   id;
    uint32        i,size;
    std::string   equal    = "";
    std::string   matchEsc = "";

    // Find out if search is exact or not and remove the trailing '*'
    std::size_t found=match.find('*');
    if (found!=std::string::npos) {
      match.erase (std::remove(match.begin(), match.end(), '*'), match.end());
      equal = " RLIKE ";
    } else {
      equal = " = ";
    }

    // Escape the searchString
    sDatabase.DoEscapeString(matchEsc, match.c_str());


    // The client wants a dict in return
    // [searchID][Results]

    PyDict *dict = new PyDict();
    size = searchID->size();

    for(i=0; i<size; i++){

	switch(searchID->at(i)) {
	  case 1:	//searchResultAgent = 1
            sDatabase.RunQuery(res,
	    "SELECT"
            " itemID AS agentID"
            " FROM entity"
            " WHERE itemName %s '%s' "
            " AND itemID BETWEEN 2999999 AND 4000000 "
            " LIMIT 0, 10", equal.c_str(), matchEsc.c_str() );
            break;
	  case 2:	//searchResultCharacter = 2
            sDatabase.RunQuery(res,
	    "SELECT"
            " itemID"
            " FROM entity"
            " WHERE itemName %s '%s' "
            " AND itemId >= %u"
            " AND ownerID = 1",equal.c_str(), matchEsc.c_str(),EVEMU_MINIMUM_ID );
            break;
	  case 3:	//searchResultCorporation = 3
            sDatabase.RunQuery(res,
	    "SELECT"
            " corporationID"
            " FROM corporation"
            " WHERE corporationName %s '%s' "
            " LIMIT 0, 10", equal.c_str(), matchEsc.c_str() );
            break;
	  case 4:	//searchResultAlliance = 4
            sDatabase.RunQuery(res,
	    "SELECT allianceID"
            " FROM alliance_ShortNames"
            " WHERE shortName %s '%s'"
            " LIMIT 0, 10", equal.c_str(), matchEsc.c_str()  );
            break;
	  case 5:	//searchResultFaction = 5
            sDatabase.RunQuery(res,
	    "SELECT factionID"
            " FROM chrFactions"
            " WHERE factionName %S '%s' "
            " LIMIT 0, 10", equal.c_str(), matchEsc.c_str() );
            break;
	  case 6:	//searchResultConstellation = 6
            sDatabase.RunQuery(res,
	    "SELECT"
            " constellationID"
            " FROM mapConstellations"
            " WHERE constellationName %s '%s' "
            " LIMIT 0, 10", equal.c_str(), matchEsc.c_str() );
            break;
          
	  case 7:	//searchResultSolarSystem = 7
            sDatabase.RunQuery(res,
	    "SELECT "
            " solarSystemID"
            " FROM mapSolarSystems"
            " WHERE solarSystemName %s '%s' "
            " LIMIT 0, 10", equal.c_str(), matchEsc.c_str() );
            break;
	  case 8:	//searchResultRegion = 8
            sDatabase.RunQuery(res,
      	    "SELECT "
            " regionID"
            " FROM mapRegions"
            " WHERE regionName %s '%s' "
            " LIMIT 0, 10", equal.c_str(), matchEsc.c_str() );
            break;
	  case 9:	//searchResultStation = 9
            sDatabase.RunQuery(res,
	    "SELECT "
            " stationID"
            " FROM staStations "
            " WHERE stationName %s '%s' "
            " LIMIT 0, 10", equal.c_str(), matchEsc.c_str() );
            break;
	  case 10:	//searchResultInventoryType = 10
	    sDatabase.RunQuery(res,
	    "SELECT"
            "   typeID"
            " FROM entity"
            " WHERE itemName %s '%s'", equal.c_str(), matchEsc.c_str() );
            break;
	}

	dict->SetItem(new PyInt(searchID->at(i)),DBResultToIntIntDict(res));
        res.Reset();
    }

    return dict;
}

