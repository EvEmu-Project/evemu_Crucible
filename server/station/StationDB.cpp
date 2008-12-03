/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "EvemuPCH.h"

StationDB::StationDB(DBcore *db)
: ServiceDB(db)
{
}

StationDB::~StationDB() {
}

PyRep *StationDB::GetSolarSystem(uint32 solarSystemID) {
	
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" solarSystemID,"
		" solarSystemName,"
		" x, y, z,"
		" radius,"
		" security,"
		" constellationID,"
		" factionID,"
		" sunTypeID,"
		" regionID,"
		//crap
		" NULL AS allianceID,"
		" 0 AS sovereigntyLevel,"
		" 0 AS constellationSovereignty"
		" FROM mapSolarSystems"
		" WHERE solarSystemID=%lu", solarSystemID
	))
	{
		_log(SERVICE__ERROR, "Error in GetSolarSystem query: %s", res.error.c_str());
		return NULL;
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Error in GetSolarSystem query: no solarsystem for id %d", solarSystemID);
		return NULL;
	}
	
	return(DBRowToRow(row));
}

PyRep *StationDB::DoGetStation(uint32 sid) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		" SELECT "
		" staStations.stationID, staStations.security, staStations.dockingCostPerVolume, staStations.maxShipVolumeDockable, "
		" staStations.officeRentalCost, staStations.operationID, staStations.stationTypeID, staStations.corporationID AS ownerID, staStations.solarSystemID, staStations.constellationID, "
		" staStations.regionID, staStations.stationName, staStations.x, staStations.y, staStations.z, staStations.reprocessingEfficiency, staStations.reprocessingStationsTake, staStations.reprocessingHangarFlag, "
		" staOperations.description, "
		// damn mysql returns the result of the sum as string and so it is sent to the client as string and so it freaks out...
		" CAST(SUM(staOperationServices.serviceID) as UNSIGNED INTEGER) AS serviceMask "
		" FROM staStations "
		" LEFT JOIN staOperations ON staStations.operationID = staOperations.operationID "
		" LEFT JOIN staOperationServices ON staStations.operationID = staOperationServices.operationID "
		" WHERE staStations.stationID = %lu "
		" GROUP BY staStations.stationID ", sid
	))
	{
		_log(SERVICE__ERROR, "Error in DoGetStation query: %s", res.error.c_str());
		return NULL;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Error in DoGetStation query: no station for id %d", sid);
		return NULL;
	}

	//only a guess that this is row
	return(DBRowToKeyVal(row));
}

//hangarGraphicID
//station owner corp
//station ID
//serviceMask
//stationTypeID

PyRep *StationDB::GetStationItemBits(uint32 sid) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		" SELECT "
		" staStations.stationID, "
		" staStations.stationTypeID, staStations.corporationID AS ownerID, "
		" staStationTypes.hangarGraphicID, "
		// damn mysql returns the result of the sum as string and so it is sent to the client as string and so it freaks out...
		" CAST(SUM(staOperationServices.serviceID) as UNSIGNED INTEGER) AS serviceMask "
		" FROM staStations "
		" LEFT JOIN staStationTypes ON staStations.stationTypeID = staStationTypes.stationTypeID "
		" LEFT JOIN staOperationServices ON staStations.operationID = staOperationServices.operationID "
		" WHERE staStations.stationID = %lu "
		" GROUP BY staStations.stationID ", sid
	))
	{
		_log(SERVICE__ERROR, "Error in DoGetStation query: %s", res.error.c_str());
		return NULL;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Error in DoGetStation query: no station for id %d", sid);
		return NULL;
	}

	PyRepTuple * result = new PyRepTuple(5);
	result->items[0] = new PyRepInteger(row.GetUInt(3));
	result->items[1] = new PyRepInteger(row.GetUInt(2));
	result->items[2] = new PyRepInteger(row.GetUInt(0));
	result->items[3] = new PyRepInteger(row.GetUInt(4));
	result->items[4] = new PyRepInteger(row.GetUInt(1));

	return(result);
}


























