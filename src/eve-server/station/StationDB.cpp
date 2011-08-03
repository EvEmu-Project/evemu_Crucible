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

storage StationDB::g_station_db_storage;

StationDB::StationDB()
{
	g_station_db_storage.load();
}

PyRep *StationDB::GetSolarSystem(uint32 solarSystemID) {
	
	return g_station_db_storage.find(solarSystemID)->Clone();

	// old code for reference.
	/*DBQueryResult res;
	
	if(!sDatabase.RunQuery(res,
		"SELECT "
		" solarSystemID,"			// nr
		" solarSystemName,"			// string
		" x, y, z,"					// double
		" radius,"					// double
		" security,"				// double
		" constellationID,"			// nr
		" factionID,"				// nr
		" sunTypeID,"				// nr
		" regionID,"
		//crap
		" NULL AS allianceID,"		// nr
		" 0 AS sovereigntyLevel,"	// nr
		" 0 AS constellationSovereignty"	// nr
		" FROM mapSolarSystems"		
		" WHERE solarSystemID=%u", solarSystemID
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
	
	return(DBRowToRow(row));*/
}

/*
mysql> SELECT staStations.stationID, staStations.security, staStations.dockingCostPerVolume, staStations.maxShipVolumeDockable, staStations.officeRentalCost, staStations.operationID, staStations.stationTypeID, staStations.corporationID AS ownerID, staStations.solarSystemID, staStations.constellationID, staStations.regionID, staStations.stationName, staStations.x, staStations.y, staStations.z, staStations.reprocessingEfficiency, staStations.reprocessingStationsTake, staStations.reprocessingHangarFlag, staOperations.description, CAST(SUM(staOperationServices.serviceID) as UNSIGNED INTEGER) AS serviceMask FROM staStations LEFT JOIN staOperations ON staStations.operationID = staOperations.operationID LEFT JOIN staOperationServices ON staStations.operationID = staOperationServices.operationID WHERE staStations.stationID = 60014599 GROUP BY staStations.stationID;
+-----------+----------+----------------------+-----------------------+------------------+-------------+---------------+---------+---------------+-----------------+----------+------------------------------------------------+--------------+-------------+----------------+------------------------+--------------------------+------------------------+----------------------------------------------+-------------+
| stationID | security | dockingCostPerVolume | maxShipVolumeDockable | officeRentalCost | operationID | stationTypeID | ownerID | solarSystemID | constellationID | regionID | stationName                                    | x            | y           | z              | reprocessingEfficiency | reprocessingStationsTake | reprocessingHangarFlag | description                                  | serviceMask |
+-----------+----------+----------------------+-----------------------+------------------+-------------+---------------+---------+---------------+-----------------+----------+------------------------------------------------+--------------+-------------+----------------+------------------------+--------------------------+------------------------+----------------------------------------------+-------------+
|  60014599 |      500 |                    0 |              50000000 |            10000 |          37 |          1931 | 1000165 |      30005042 |        20000737 | 10000065 | Sehmy VIII - Moon 2 - Hedion University School | 502645432320 | 45862379520 | -1989488025600 |                    0.5 |                     0.05 |                      4 | Produces personnel and sells skill packages. |    58012245 |
+-----------+----------+----------------------+-----------------------+------------------+-------------+---------------+---------+---------------+-----------------+----------+------------------------------------------------+--------------+-------------+----------------+------------------------+--------------------------+------------------------+----------------------------------------------+-------------+
1 row in set


stationID = 60014599
security = 500
dockingCostPerVolume = 0
maxShipVolumeDockable = 50000000
officeRentalCost = 10000
operationID =37
stationTypeID =1931
ownerID =1000165
solarSystemID=30005042
constellationID=20000737
regionID = 10000065
stationName = "Sehmy VIII - Moon 2 - Hedion University School"
x =502645432320           
y = 45862379520         
z =-1989488025600             
reprocessingEfficiency = 0.5
reprocessingStationsTake = 0.05
reprocessingHangarFlag = 4
description = Produces personnel and sells skill packages.                                
serviceMask = 58012245|

// missing
orbitID
hangarGraphicID
upgradeLevel
graphicID
dockingBayGraphicID x
dockOrientationX x
dockOrientationY x
dockOrientationZ x
standingOwnerID
dockEntryX x
dockEntryY x
dockEntryZ x
conquerable x
radius





// SQL QUERY
SELECT 
staStations.stationID, 
staStations.security, 
staStations.dockingCostPerVolume, 
staStations.maxShipVolumeDockable, 
staStations.officeRentalCost,
staStations.operationID, 
staStations.stationTypeID, 
staStations.corporationID AS ownerID,
staStations.solarSystemID, 
staStations.constellationID, 
staStations.regionID, 
staStations.stationName, 
staStations.x, 
staStations.y, 
staStations.z, 
staStations.reprocessingEfficiency, 
staStations.reprocessingStationsTake, 
staStations.reprocessingHangarFlag,
staStationTypes_old.hangarGraphicID,
staStationTypes.dockEntryX,
staStationTypes.dockEntryY,
staStationTypes.dockEntryZ,
staStationTypes.dockOrientationX,
staStationTypes.dockOrientationY,
staStationTypes.dockOrientationZ,
staStationTypes.conquerable,


staOperations.description, 
CAST(SUM(staOperationServices.serviceID) as UNSIGNED INTEGER) AS serviceMask 

FROM staStations
LEFT JOIN staOperations ON 
  staStations.operationID = staOperations.operationID 
LEFT JOIN staOperationServices ON
  staStations.operationID = staOperationServices.operationID 
  
  WHERE staStations.stationID = 60014599 GROUP BY staStations.stationID;

// SQL form
SELECT staStations.stationID, staStations.security, staStations.dockingCostPerVolume, staStations.maxShipVolumeDockable, staStations.officeRentalCost,staStations.operationID, staStations.stationTypeID, staStations.corporationID AS ownerID, staStations.solarSystemID, staStations.constellationID, staStations.regionID, staStations.stationName, staStations.x, staStations.y, staStations.z, staStations.reprocessingEfficiency, staStations.reprocessingStationsTake, staStations.reprocessingHangarFlag,staStationTypes_old.hangarGraphicID,staStationTypes.dockEntryX,staStationTypes.dockEntryY,staStationTypes.dockEntryZ,staStationTypes.dockOrientationX,staStationTypes.dockOrientationY,staStationTypes.dockOrientationZ,staStationTypes.conquerable,staOperations.description, CAST(SUM(staOperationServices.serviceID) as UNSIGNED INTEGER) AS serviceMask FROM staStations LEFT JOIN staOperations ON staStations.operationID = staOperations.operationID LEFT JOIN staOperationServices ON staStations.operationID = staOperationServices.operationID WHERE staStations.stationID = 60014599 GROUP BY staStations.stationID;


// new sql blah
// todo: upgradelevel, standingOwnerID, serviceMask

SELECT
staStations.x,
staStations.y,
staStations.z,
mapDenormalize.orbitID,
staStationTypes_old.hangarGraphicID,
staStations.stationID,
0 AS upgradelevel,
invtypes.graphicID,
staStations.regionID,
staStations.security,
staStations.stationTypeID,
staStationTypes_old.dockingBayGraphicID,
staStations.officeRentalCost,
staStations.stationName,
staOperations.description,
staStations.constellationID,
staStations.operationID,
staStations.solarSystemID,
staStationTypes.dockOrientationX,
staStationTypes.dockOrientationY,
staStationTypes.dockOrientationZ,
staStations.corporationID AS standingOwnerID,
58012245 AS serviceMask,
staStations.dockingCostPerVolume,
staStations.reprocessingHangarFlag,
staStations.reprocessingEfficiency,
staStationTypes.dockEntryX,
staStationTypes.dockEntryY,
staStationTypes.dockEntryZ,
staStations.maxShipVolumeDockable,
staStations.corporationID AS ownerID,
staStationTypes.conquerable,
mapDenormalize.radius

from staStations
LEFT JOIN staStationTypes_old ON
  staStations.stationTypeID=staStationTypes_old.stationTypeID
LEFT JOIN mapDenormalize ON
  staStations.stationID=mapDenormalize.itemID
LEFT JOIN invtypes ON
  staStations.stationTypeID=invtypes.typeID
LEFT JOIN staOperations ON
  staStations.operationID=staOperations.operationID
LEFT JOIN staStationTypes ON
  staStations.stationTypeID=staStationTypes_old.stationTypeID
  
WHERE staStations.stationID = 60014599 GROUP BY staStations.stationID;

** information we need in this call result **

FIELDNAMe                           TABLENAME                               INDEXFIELD
x                                   staStations                             stationID
y                                   staStations                             stationID
z                                   staStations                             stationID
orbitID                             mapDenormalize                          itemID=stationID
hangarGraphicID                     staStationTypes_old
reprocessingStationsTake
stationID
upgradeLevel
graphicID                           invtypes                                typeID=stationTypeID
regionID
security
stationTypeID
dockingBayGraphicID
officeRentalCost
stationName
description                         staOperations
constellationID
operationID
solarSystemID
dockOrientationX
dockOrientationY
dockOrientationZ
standingOwnerID
serviceMask
dockingCostPerVolume
reprocessingHangarFlag
reprocessingEfficiency
dockEntryX
dockEntryY
dockEntryZ
maxShipVolumeDockable
ownerID
conquerable
radius                              mapDenormalize                          itemID=stationID

  


****************************

dict["x"]=5.02645e+011                                      X
dict["y"]=4.58624e+010                                      X
dict["z"]=1.98949e+012
dict["orbitID"]=0x26739F6
dict["hangarGraphicID"]=0x433
dict["reprocessingStationsTake"]=0.050000
dict["stationID"]=0x393C007
dict["upgradeLevel"]=0x0
dict["graphicID"]=0x431
dict["regionID"]=0x9896C1
dict["security"]=0x1F4
dict["stationTypeID"]=0x78B
dict["dockingBayGraphicID"]=None
dict["officeRentalCost"]=0x796BCC
dict["stationName"]="Sehmy VIII - Moon 2 - Hedion University School"
dict["description"]="Produces personnel and sells skill packages."
dict["constellationID"]=0x1312FE1
dict["operationID"]=0x25
dict["solarSystemID"]=0x1C9D732
dict["dockOrientationX"]=0
dict["dockOrientationY"]=-1.000000
dict["dockOrientationZ"]=0
dict["standingOwnerID"]=0xF42E5
dict["serviceMask"]=0x3753255
dict["dockingCostPerVolume"]=0
dict["reprocessingHangarFlag"]=0x4
dict["reprocessingEfficiency"]=0.500000
dict["dockEntryX"]=-13
dict["dockEntryY"]=-5689.268066
dict["dockEntryZ"]=0
dict["maxShipVolumeDockable"]=5e+007
dict["ownerID"]=0xF42E5
dict["conquerable"]=false
dict["radius"]=29162



*/

// this one needs some love
//int henk = "bla";
PyRep *StationDB::DoGetStation(uint32 sid)
{
	DBQueryResult res;

//" SELECT staStations.stationID, staStations.security, staStations.dockingCostPerVolume, staStations.maxShipVolumeDockable, staStations.officeRentalCost, staStations.operationID, staStations.stationTypeID, staStations.corporationID AS ownerID, staStations.solarSystemID, staStations.constellationID, staStations.regionID, staStations.stationName, staStations.x, staStations.y, staStations.z, staStations.reprocessingEfficiency, staStations.reprocessingStationsTake, staStations.reprocessingHangarFlag, staOperations.description, CAST(SUM(staOperationServices.serviceID) as UNSIGNED INTEGER) AS serviceMask FROM staStations LEFT JOIN staOperations ON staStations.operationID = staOperations.operationID LEFT JOIN staOperationServices ON staStations.operationID = staOperationServices.operationID WHERE staStations.stationID = %u GROUP BY staStations.stationID 

    if(!sDatabase.RunQuery(res, "SELECT"
    "staStations.x,"
    "staStations.y,"
    "staStations.z,"
    "mapDenormalize.orbitID,"
    "staStationTypes.hangarGraphicID,"
    "staStations.stationID,"
    "0 AS upgradelevel,"
    "invtypes.graphicID,"
    "staStations.regionID,"
    "staStations.security,"
    "staStations.stationTypeID,"
    "staStationTypes.dockingBayGraphicID,"
    "staStations.officeRentalCost,"
    "staStations.stationName,"
    "staOperations.description,"
    "staStations.constellationID,"
    "staStations.operationID,"
    "staStations.solarSystemID,"
    "staStationTypes.dockOrientationX,"
    "staStationTypes.dockOrientationY,"
    "staStationTypes.dockOrientationZ,"
    "staStations.corporationID AS standingOwnerID,"
    //"58012245 AS serviceMask,"
    "CAST(SUM(staOperationServices.serviceID) as UNSIGNED INTEGER) AS serviceMask "
    "staStations.dockingCostPerVolume,"
    "staStations.reprocessingHangarFlag,"
    "staStations.reprocessingEfficiency,"
    "staStationTypes.dockEntryX,"
    "staStationTypes.dockEntryY,"
    "staStationTypes.dockEntryZ,"
    "staStations.maxShipVolumeDockable,"
    "staStations.corporationID AS ownerID,"
    "staStationTypes.conquerable,"
    "mapDenormalize.radius"
    "from staStations "
    "LEFT JOIN staStationTypes ON "
    "staStations.stationTypeID=staStationTypes.stationTypeID "
    "LEFT JOIN mapDenormalize ON "
    "staStations.stationID=mapDenormalize.itemID "
    "LEFT JOIN invtypes ON "
    "staStations.stationTypeID=invtypes.typeID "
    "LEFT JOIN staOperations ON "
    "staStations.operationID=staOperations.operationID "
    "LEFT JOIN staOperationServices ON "
    "staStations.operationID=staOperationServices.operationID "
    "WHERE staStations.stationID = %u GROUP BY staStations.stationID", sid ))
    {
        //_log(SERVICE__ERROR, "Error in DoGetStation query: %s", res.error.c_str());
        sLog.Error("StationDB", "Error in DoGetStation query: %s", res.error.c_str());
        return NULL;
    }

	/*if(!sDatabase.RunQuery(res,
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
		" WHERE staStations.stationID = %u "
		" GROUP BY staStations.stationID ", sid
	))
    
	{
		_log(SERVICE__ERROR, "Error in DoGetStation query: %s", res.error.c_str());
		return NULL;
	}*/

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Error in DoGetStation query: no station for id %d", sid);
		return NULL;
	}

	//only a guess that this is row
	return DBRowToKeyVal(row);
}

//hangarGraphicID
//station owner corp
//station ID
//serviceMask
//stationTypeID

// TODO: hangarGraphicID went missing. maybe no longer in the dump?
PyRep *StationDB::GetStationItemBits(uint32 sid) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		" SELECT "
		" staStations.stationID, "
		" staStations.stationTypeID, staStations.corporationID AS ownerID, "
		" staStationTypes.hangarGraphicID, "
		// damn mysql returns the result of the sum as string and so it is sent to the client as string and so it freaks out...
		" CAST(SUM(staOperationServices.serviceID) as UNSIGNED INTEGER) AS serviceMask "
		" FROM staStations "
		" LEFT JOIN staStationTypes ON staStations.stationTypeID = staStationTypes.stationTypeID "
		" LEFT JOIN staOperationServices ON staStations.operationID = staOperationServices.operationID "
		" WHERE staStations.stationID = %u "
		" GROUP BY staStations.stationID ", sid
	))
	{
		_log(SERVICE__ERROR, "Error in GetStationItemBits query: %s", res.error.c_str());
		return NULL;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(SERVICE__ERROR, "Error in GetStationItemBits query: no station for id %d", sid);
		return NULL;
	}

	PyTuple * result = new PyTuple(5);
	result->SetItem(0, new PyInt(row.GetUInt(3)));
	result->SetItem(1, new PyInt(row.GetUInt(2)));
	result->SetItem(2, new PyInt(row.GetUInt(0)));
	result->SetItem(3, new PyInt(row.GetUInt(4)));
	result->SetItem(4, new PyInt(row.GetUInt(1)));

	return result;
}
