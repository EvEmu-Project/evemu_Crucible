/**
 * @name StationDB.cpp
 *      database methods for station data
 *
 * @author Allan
 * @date 14 December 2017
 *
 */


#include "eve-server.h"

#include "station/StationDB.h"
#include "station/StationDataMgr.h"


uint32 StationDB::CreateOffice(ItemData& idata, OfficeData& odata)
{
    uint32 uid = 0;
    DBerror err;
    if (!sDatabase.RunQueryLID(err, uid,
        "INSERT INTO staOffices (name, corporationID, stationID, officeFolderID, flag, solarSystemID, typeID, stationTypeID, lockDown, rentalFee, expiryDateTime)"
        " VALUES"
        " ('%s',%u,%u,%u,%u,%u, 27, %u,%u,%u,%li)",
        odata.name.c_str(), odata.corporationID, odata.stationID, odata.folderID, idata.flag, stDataMgr.GetStationSystemID(odata.stationID), odata.typeID, odata.lockDown, odata.rentalFee, odata.expiryTime)
    )
        codelog(DATABASE__ERROR, "Error in CreateOffice query: %s", err.c_str());

    return (odata.officeID = uid);
}

void StationDB::UpdateOfficeData(OfficeData& data)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE staOffices SET lockDown = %u, rentalFee = %u, expiryDateTime = %li)", data.lockDown, data.rentalFee, data.expiryTime);
}

int32 StationDB::GetOfficeCount(uint32 corpID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT COUNT(itemID) FROM staOffices WHERE corporationID = %u ", corpID)) {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "Unable to find corporation's office data (%u)", corpID);
        return 0;
    }
    return row.GetInt(0);
}

PyRep* StationDB::GetOffices(uint32 stationID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        " corporationID, itemID, officeFolderID"
        " FROM staOffices"
        " WHERE stationID = %u", stationID)
    )
        codelog(DATABASE__ERROR, "Error in GetOffices query: %s", res.error.c_str());

    return DBResultToCRowset(res);
}

bool StationDB::GetOfficeData(uint32 officeID, OfficeData& odata)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        " corporationID, stationID, typeID, lockDown, rentalFee, expiryDateTime, officeFolderID"
        " FROM staOffices"
        " WHERE itemID = %u", officeID)
    ) {
        codelog(DATABASE__ERROR, "Error in GetOfficeData query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return false;

    odata.officeID      = officeID;
    odata.corporationID = row.GetInt(0);
    odata.stationID     = row.GetInt(1);
    odata.typeID        = row.GetInt(2);
    odata.lockDown      = row.GetBool(3);
    odata.rentalFee     = row.GetInt64(4);
    odata.expiryTime    = row.GetInt64(5);
    odata.folderID      = row.GetInt(6);
    return true;
}


void StationDB::GetStationOfficeData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT"
        " itemID, corporationID, stationID, typeID, lockDown, rentalFee, expiryDateTime, officeFolderID"
        " FROM staOffices")
    )
        codelog(DATABASE__ERROR, "Error in GetStationOfficeData query: %s", res.error.c_str());
}

PyRep* StationDB::GetStationOfficeIDs(uint32 locationID, uint32 corpID, const char* key)
{
    DBQueryResult res;
    if (sDataMgr.IsStation(locationID)) {
        sDatabase.RunQuery(res, "SELECT itemID AS officeID, stationID, officeFolderID"
        " FROM staOffices WHERE stationID = %u", locationID);
    } else if (IsOfficeFolder(locationID)) {
        sDatabase.RunQuery(res, "SELECT itemID AS officeID, stationID, officeFolderID"
        " FROM staOffices WHERE officeFolderID = %u",
            locationID - STATION_OFFICE_OFFSET);
    } else if (IsOfficeID(locationID)) {
        sDatabase.RunQuery(res, "SELECT itemID AS officeID, stationID, officeFolderID"
        " FROM staOffices WHERE itemID = %u", locationID);
    } else {
        _log(CORP__DB_ERROR, "StationDB::GetStationOfficeIDs got invalid locationID %u", locationID);
    }

    DBResultRow row;
    if (res.GetRow(row))
        return DBRowToRow(row);

    return nullptr;

    //return DBResultToCIndexedRowset(res, key);
    //return DBResultToIndexRowset(res, key);
    //return DBResultToCRowset(res);
    //return DBResultToRowList(res);
    //return DBResultToRowset(res);
    //return DBResultToPackedRowDict(res, key);
}

void StationDB::GetStationData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  s.stationID, s.x, s.y, s.z, st.dockOrientationX, st.dockOrientationY, st.dockOrientationZ,"
        "  s.dockingCostPerVolume, s.maxShipVolumeDockable, s.officeSlots, s.officeRentalCost,"
        "  s.operationID, s.stationTypeID, s.corporationID, s.stationName,"
        "  s.reprocessingStationsTake, s.reprocessingEfficiency, s.reprocessingHangarFlag, st.conquerable, st.hangarGraphicID,"
        "  m.orbitID, m.radius, m.security, o.description, o.descriptionID, t.graphicID, s.solarSystemID, s.constellationID, s.regionID,"
        "  st.dockEntryX, st.dockEntryY, st.dockEntryZ"
        " FROM staStations AS s"
        "  LEFT JOIN staStationTypes AS st USING (stationTypeID)"
        "  LEFT JOIN invTypes AS t ON t.typeID = s.stationTypeID"
        "  LEFT JOIN staOperations AS o ON o.operationID = s.operationID"
        "  LEFT JOIN mapDenormalize AS m ON m.itemID = s.stationID")
        )
        codelog(DATABASE__ERROR, "Error in GetStationData query: %s", res.error.c_str());
}

void StationDB::GetStationBaseData(DBQueryResult& res, uint32 typeID)
{
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  st.dockOrientationX, st.dockOrientationY, st.dockOrientationZ,"
        "  st.conquerable, st.hangarGraphicID,"
        "  o.description, o.descriptionID, t.graphicID,"
        "  st.dockEntryX, st.dockEntryY, st.dockEntryZ, o.operationID, o.operationName"
        " FROM staStationTypes AS st"
        "  LEFT JOIN invTypes AS t ON t.typeID = st.stationTypeID"
        "  LEFT JOIN staOperations AS o ON o.operationName = t.typeName"
        " COLLATE utf8_general_ci"
        " WHERE st.stationTypeID=%u", typeID)
        )
        codelog(DATABASE__ERROR, "Error in GetStationData query: %s", res.error.c_str());
}

void StationDB::GetStationSystem(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT stationID, solarSystemID FROM staStations"))
        codelog(DATABASE__ERROR, "Error in GetStationSystem query: %s", res.error.c_str());
}

void StationDB::GetStationRegion(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT stationID, regionID FROM staStations"))
        codelog(DATABASE__ERROR, "Error in GetStationRegion query: %s", res.error.c_str());
}

void StationDB::GetStationConstellation(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT stationID, constellationID FROM staStations"))
        codelog(DATABASE__ERROR, "Error in GetStationConstellation query: %s", res.error.c_str());
}

void StationDB::GetOperationServiceIDs(DBQueryResult& res)
{
    sDatabase.RunQuery(res, "SELECT operationID, serviceID FROM staOperationServices");
}

void StationDB::LoadOffices(OwnerData& od, std::vector< uint32 >& into)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT itemID FROM staOffices WHERE stationID = %u", od.locID)) {
        codelog(DATABASE__ERROR, "Error in LoadOffices query: %s", res.error.c_str());
        return;
    }

    DBResultRow row;
    while (res.GetRow(row))
        into.push_back(row.GetInt(0));
}

void StationDB::CreateOutpost(StationData &data)
{
    // Add data to mapDenormalize table
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO mapDenormalize (itemID,typeID,groupID,solarSystemID,constellationID,regionID,orbitID,x,y,z,radius,itemName,itemNameID,security)"
        " VALUES"
        " (%u,%u,%u,%u,%u,%u,%u,%f,%f,%f,%f,'%s',0,%f)",
        data.stationID,            //itemID
        data.typeID,               //typeID
        EVEDB::invGroups::Station, //groupID
        data.systemID,             //solarSystemID
        data.constellationID,      //constellationID
        data.regionID,             //regionID
        data.orbitID,              //orbitID
        data.position.x,           //x
        data.position.y,           //y
        data.position.z,           //z
        data.radius,               //radius
        data.name.c_str(),         //itemName
        data.security              //security
    )) {
        codelog(DATABASE__ERROR, "Error in CreateStation query: %s", err.c_str());
    }

    if (!sDatabase.RunQuery(err,
        "INSERT INTO staStations (stationID,security,dockingCostPerVolume,maxShipVolumeDockable,officeSlots,officeRentalCost,operationID,stationTypeID,corporationID,solarSystemID,constellationID,regionID,stationName,x,y,z,reprocessingEfficiency,reprocessingStationsTake,reprocessingHangarFlag)"
        " VALUES"
        " (%u,%f,%f,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s',%f,%f,%f,%f,%f,%u)",
        data.stationID,                //itemID
        data.security,                 //security
        data.dockingCostPerVolume,     //dockingCostPerVolume
        data.maxShipVolumeDockable,    //maxShipVolumeDockable
        data.officeSlots,              //officeSlots
        data.officeRentalFee,          //officeRentalCost
        data.operationID,              //operationID
        data.typeID,                   //stationTypeID
        data.corporationID,            //corporationID
        data.systemID,                 //solarSystemID
        data.constellationID,          //constellationID
        data.regionID,                 //regionID
        data.name.c_str(),             //stationName
        data.position.x,               //x
        data.position.y,               //y
        data.position.z,               //z
        data.reprocessingEfficiency,   //reprocessingEfficiency
        data.reprocessingStationsTake, //reprocessingStationsTake
        flagHangar                     //reprocessingHangarFlag
    )) {
        codelog(DATABASE__ERROR, "Error in CreateStation query: %s", err.c_str());
    }
}

// This function gets the next available outpostID from the staStation table for use during outpost construction
uint32 StationDB::GetNewOutpostID()
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT stationID FROM staStations WHERE stationID>61000000 ORDER BY stationID DESC LIMIT 1")) {
        codelog(DATABASE__ERROR, "Error in GetNewOutpostID query: %s", res.error.c_str());
        return 0;
    }

    uint32 newID(0);
    DBResultRow row;
    while (res.GetRow(row)) {
        newID = row.GetUInt(0) + 1;
    }

    // If there is no existing outpost in the db, we will use the base value
    if (newID == 0) {
        return 61000000;
    }

    return newID;
}