
 /**
  * @name SovereigntyDataMgr.cpp
  *   memory object caching system for managing and saving ingame static and dynamic station data
  *
  * @Author:         James
  * @date:           21Apr13
  *
  */

#include "system/sov/SovereigntyDataMgr.h"
#include "database/EVEDBUtils.h"


SovereigntyDataMgr::SovereigntyDataMgr()
{
    m_serviceMask.clear();
    m_stationData.clear();
    m_stationPyData.clear();
    m_stationOfficeData.clear();
}

SovereigntyDataMgr::~SovereigntyDataMgr()
{
    //Clear();
}

int SovereigntyDataMgr::Initialize()
{
    Populate();
    sLog.Blue("   SovereigntyDataMgr", "Sovereignty Data Manager Initialized.");
    return 1;
}

void SovereigntyDataMgr::Close()
{
    /** @todo put a save method here which will save anything changed before shutdown */
    for (auto cur : m_sovPyData)
        PySafeDecRef(cur.second);

    sLog.Warning("   SovereigntyDataMgr", "Sovereignty Data Manager has been closed." );
}


void SovereigntyDataMgr::Clear()
{
    for (auto cur : m_sovPyData)
        PySafeDecRef(cur.second);
    m_sovData.clear();
    m_sovPyData.clear();
}

void SovereigntyDataMgr::Populate()
{
    double start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    std::map<int8, int32>::iterator itr;

    SovereigntyDB::GetSovereigntyData(*res);
    while (res->GetRow(row)) {
        SovereigntyData sData = SovereigntyData();

        sData.solarSystemID             = row.GetUint(0)
        sData.constellationID           = row.GetUint(1)
        sData.corporationID             = row.GetUint(2)
        sData.allianceID                = row.GetUint(3)
        sData.claimStructureID          = row.GetUint(4)
        sData.claimTime                 = row.GetUint(5)
        sData.hubID                     = row.GetUInt(6)
        sData.contested                 = row.GetBool(7)
        sData.stationCount              = row.GetUInt(8)
        sData.militaryPoints            = row.GetUInt(9)
        sData.industrialPoints          = row.GetUInt(10)
        m_sovData.emplace(row.GetInt(0), sData);
    }

    LoadStationPyData();

    sLog.Cyan("   SovereigntyDataMgr", "%u Sovereignty data sets loaded in %.3fms.", \
                (m_sovData.size() + m_sovPyData.size()), (GetTimeMSeconds() - start));

    //cleanup
    SafeDelete(res);
}

/*void StationDataMgr::AddOffice(uint32 stationID, OfficeData& data)
{
    m_stationOfficeData.emplace(stationID, data);
}

void StationDataMgr::LoadOffices(uint32 stationID, std::map<uint32, OfficeData>& data)
{
    auto range = m_stationOfficeData.equal_range(stationID);
    for (auto itr = range.first; itr != range.second; ++itr)
        data.emplace(itr->second.officeID, itr->second);
}*/

void StationDataMgr::GetInfo()
{
    /* return info about loaded items? */
    /*
     * m_sovData;
     *
     */
}

uint32 StationDataMgr::GetOwnerID(uint32 stationID)
{
    /** @todo  update this for POS modules and Outposts  */
    std::map<uint32, StationData>::iterator itr = m_stationData.find(stationID);
    if (itr != m_stationData.end())
        return itr->second.corporationID;
    return 0;
}

uint32 StationDataMgr::GetOfficeIDForCorp(uint32 stationID, uint32 corpID)
{
    auto range = m_stationOfficeData.equal_range(stationID);
    for (auto itr = range.first; itr != range.second; ++itr)
        if (itr->second.corporationID == corpID)
            return itr->second.officeID;
    return 0;
}

double StationDataMgr::GetDockPosY(uint32 stationID)
{
    std::map<uint32, StationData>::iterator itr = m_stationData.find(stationID);
    if (itr != m_stationData.end())
        return itr->second.dockPosition.y;
    return 0;
}

std::string StationDataMgr::GetStationName(uint32 stationID)
{
    std::map<uint32, StationData>::iterator itr = m_stationData.find(stationID);
    if (itr != m_stationData.end()) {
        return itr->second.name;
    } else {
        _log(DATABASE__MESSAGE, "Failed to query station name for station %u: Station not found.", stationID);
    }
    return "";
}

uint32 StationDataMgr::GetStationSystemID(uint32 stationID)
{
    std::map<uint32, StationData>::iterator itr = m_stationData.find(stationID);
    if (itr != m_stationData.end()) {
        return itr->second.systemID;
    } else {
        _log(DATABASE__MESSAGE, "Failed to query station systemID for station %u: Station not found.", stationID);
    }
    return 0;
}


bool StationDataMgr::GetStationData(uint32 stationID, StationData& data)
{
    std::map<uint32, StationData>::iterator itr = m_stationData.find(stationID);
    if (itr != m_stationData.end()) {
        data = itr->second;
        return true;
    } else {
        _log(DATABASE__MESSAGE, "Failed to query data for station %u: Station not found.", stationID);
    }
    return true;
}

PyObject* StationDataMgr::GetStationPyData(uint32 stationID)
{
    std::map<uint32, PyObject*>::iterator itr = m_stationPyData.find(stationID);
    if (itr != m_stationPyData.end()) {
        PyIncRef(itr->second);
        return itr->second;
    } else {
        _log(DATABASE__MESSAGE, "Failed to query data for station %u: Station not found.", stationID);
    }
    return nullptr;
}

PyRep* StationDataMgr::GetStationItemBits(uint32 stationID)
{
    //['hangarGraphicID','ownerID','itemID','serviceMask','stationTypeID']
    std::map<uint32, StationData>::iterator itr = m_stationData.find(stationID);
    if (itr != m_stationData.end()) {
        std::map<int8, int32>::iterator itr2;
        PyTuple * result = new PyTuple(5);
            result->SetItem(0, new PyInt(itr->second.hangarGraphicID));
            result->SetItem(1, new PyInt(itr->second.corporationID));
            result->SetItem(2, new PyInt(stationID));
            if ((itr2 = m_serviceMask.find(itr->second.operationID)) != m_serviceMask.end()) {
                result->SetItem(3, new PyInt(itr2->second));
            } else {
                result->SetItem(3, new PyInt(0));
            }
            result->SetItem(4, new PyInt(itr->second.typeID));
        return result;
    }
    return nullptr;
}

void StationDataMgr::GetStationOfficeIDs(uint32 locationID, std::vector<OfficeData> &data)
{
    if (IsStation(locationID)) {
        auto range = m_stationOfficeData.equal_range(locationID);
        for (auto itr = range.first; itr != range.second; ++itr)
            data.push_back(itr->second);
    } else if (IsOfficeFolder(locationID)) {
        auto range = m_stationOfficeData.equal_range((locationID - STATION_OFFICE_OFFSET));
        for (auto itr = range.first; itr != range.second; ++itr)
            if (itr->second.folderID == locationID)
                data.push_back(itr->second);
    } else if (IsOffice(locationID)) {
        // no better way to do this one yet.....iterate thru the whole map?
        // this is full map of station data.  need to find station to cut down on loop time.
        for (auto cur : m_stationOfficeData)
            if (cur.second.officeID == locationID) {
                data.push_back(cur.second);
                return;
            }
    }
}

uint32 StationDataMgr::GetOfficeRentalFee(uint32 stationID)
{
    std::map<uint32, StationData>::iterator itr = m_stationData.find(stationID);
    if (itr != m_stationData.end())
        return itr->second.officeRentalFee;
    return 0;
}


void StationDataMgr::LoadStationPyData()
{
    for (auto cur : m_stationData) {
        PyDict* dict = new PyDict();
            dict->SetItemString("stationID", new PyInt(cur.first));
            dict->SetItemString("ownerID", new PyInt(cur.second.corporationID));
            dict->SetItemString("stationTypeID", new PyInt(cur.second.typeID));
            dict->SetItemString("stationName", new PyString(cur.second.name));
            dict->SetItemString("operationID", new PyInt(cur.second.operationID));
            dict->SetItemString("x", new PyFloat(cur.second.position.x));
            dict->SetItemString("y", new PyFloat(cur.second.position.y));
            dict->SetItemString("z", new PyFloat(cur.second.position.z));
            dict->SetItemString("dockEntryX", new PyFloat(cur.second.dockEntry.x));
            dict->SetItemString("dockEntryY", new PyFloat(cur.second.dockEntry.y));
            dict->SetItemString("dockEntryZ", new PyFloat(cur.second.dockEntry.z));
            dict->SetItemString("dockOrientationX", new PyFloat(cur.second.dockOrientation.x));
            dict->SetItemString("dockOrientationY", new PyFloat(cur.second.dockOrientation.y));
            dict->SetItemString("dockOrientationZ", new PyFloat(cur.second.dockOrientation.z));

            dict->SetItemString("serviceMask", new PyInt(cur.second.serviceMask));
            dict->SetItemString("conquerable", new PyBool(cur.second.conquerable));
            dict->SetItemString("upgradeLevel", new PyInt(0));  // outposts only. others are 0
            dict->SetItemString("standingOwnerID", new PyInt(cur.second.corporationID));
            dict->SetItemString("hangarGraphicID", new PyInt(cur.second.hangarGraphicID));
            dict->SetItemString("officeRentalCost", new PyInt(cur.second.officeRentalFee));
            dict->SetItemString("dockingBayGraphicID", PyStatic.NewNone());   // cannot find any data on this; all packets show PyNone
            dict->SetItemString("dockingCostPerVolume", new PyFloat(cur.second.dockingCostPerVolume));
            dict->SetItemString("maxShipVolumeDockable", new PyFloat(cur.second.maxShipVolumeDockable));
            dict->SetItemString("reprocessingEfficiency", new PyFloat(cur.second.reprocessingEfficiency));
            dict->SetItemString("reprocessingHangarFlag", new PyInt(cur.second.reprocessingHangarFlag));
            dict->SetItemString("reprocessingStationsTake", new PyFloat(cur.second.reprocessingStationsTake));

            dict->SetItemString("solarSystemID", new PyInt(cur.second.systemID));
            dict->SetItemString("constellationID", new PyInt(cur.second.constellationID));
            dict->SetItemString("regionID", new PyInt(cur.second.regionID));
            dict->SetItemString("security", new PyFloat(cur.second.security));

            dict->SetItemString("graphicID", new PyInt(cur.second.graphicID));  //invTypes.graphicID
            dict->SetItemString("description", new PyString(cur.second.description));    //staOperations.description
            dict->SetItemString("descriptionID", new PyInt(cur.second.descriptionID));    //staOperations.descriptionID

            dict->SetItemString("radius", new PyFloat(cur.second.radius));   //mapDenormalize.radius or invTypes.radius
            dict->SetItemString("orbitID", new PyInt(cur.second.orbitID));    //mapDenormalize.orbitID

        m_stationPyData.emplace(cur.first, new PyObject("util.KeyVal", dict));
    }
}