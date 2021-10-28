
/**
  * @name SovereigntyDataMgr.cpp
  *   memory object caching system for managing and saving ingame static and dynamic station data
  *
  * @Author:         James
  * @date:           21Apr13
  *
  */

#include <boost/range/iterator_range.hpp>
#include "system/sov/SovereigntyDataMgr.h"
#include "database/EVEDBUtils.h"
#include "packets/Sovereignty.h"
#include "StaticDataMgr.h"
#include "Client.h"

SovereigntyDataMgr::SovereigntyDataMgr()
{
    m_sovData.clear();
}

SovereigntyDataMgr::~SovereigntyDataMgr()
{
    //Clear();
}

int SovereigntyDataMgr::Initialize()
{
    Populate();
    sLog.Blue("       SovDataMgr", "Sovereignty Data Manager Initialized.");
    return 1;
}

void SovereigntyDataMgr::Close()
{
    /** @todo put a save method here which will save anything changed before shutdown */
    //for (auto cur : m_sovPyData)
    //    PySafeDecRef(cur.second);

    sLog.Warning("       SovDataMgr", "Sovereignty Data Manager has been closed.");
}

void SovereigntyDataMgr::Clear()
{
    //for (auto cur : m_sovPyData)
    //    PySafeDecRef(cur.second);
    m_sovData.clear();
    //m_sovPyData.clear();
}

void SovereigntyDataMgr::Populate()
{
    double start = GetTimeMSeconds();
    DBQueryResult *res = new DBQueryResult();
    DBResultRow row;

    std::map<int8, int32>::iterator itr;

    //This data is added by solar system, so we need to use that tag here
    auto &bySolar = m_sovData.get<SovDataBySolarSystem>();

    SovereigntyDB::GetSovereigntyData(*res);
    while (res->GetRow(row))
    {
        SovereigntyData sData = SovereigntyData();
        sData.solarSystemID = row.GetUInt(0);
        sData.constellationID = row.GetUInt(1);
        sData.regionID = row.GetUInt(2);
        sData.corporationID = row.GetUInt(3);
        sData.allianceID = row.GetUInt(4);
        sData.claimStructureID = row.GetUInt(5);
        sData.claimTime = row.GetInt64(6);
        sData.hubID = row.GetUInt(7);
        sData.contested = row.GetUInt(8);
        sData.stationCount = row.GetUInt(9);
        sData.militaryPoints = row.GetUInt(10);
        sData.industrialPoints = row.GetUInt(11);
        sData.claimID = row.GetUInt(12);
        sData.beaconID = row.GetUInt(13);
        sData.jammerID = row.GetUInt(14);
        bySolar.insert(sData);
    }

    sLog.Cyan("       SovDataMgr", "%lu Sovereignty data sets loaded in %.3fms.",
              bySolar.size(), (GetTimeMSeconds() - start));

    //cleanup
    SafeDelete(res);
}

void SovereigntyDataMgr::GetInfo()
{
    /* return info about loaded items? */
    /*
     * m_sovData;
     *
     */
}

uint32 SovereigntyDataMgr::GetSystemAllianceID(uint32 systemID)
{
    auto it = m_sovData.get<SovDataBySolarSystem>().find(systemID);
    if (it != m_sovData.get<SovDataBySolarSystem>().end())
    {
        SovereigntyData sData = *it;
        return sData.allianceID;
    }
    return 0;
}

SovereigntyData SovereigntyDataMgr::GetSovereigntyData(uint32 systemID)
{
    auto it = m_sovData.get<SovDataBySolarSystem>().find(systemID);
    if (it != m_sovData.get<SovDataBySolarSystem>().end())
    {
        SovereigntyData sData = *it;
        return sData;
    } else
    {
        //Return empty object in case we don't find anything
        SovereigntyData sData;
        return sData;
    }
}

PyRep *SovereigntyDataMgr::GetSystemSovereignty(uint32 systemID)
{
    SystemData sysData = SystemData();
    PyDict *args = new PyDict();

    //Figure out if this system is in empire space
    sDataMgr.GetSystemData(systemID, sysData);

    if (sysData.factionID)
    { //If we have a factionID, we can set the system's sov data to it
        args->SetItemString("contested", PyStatic.NewZero());
        args->SetItemString("corporationID", PyStatic.NewZero());
        args->SetItemString("claimTime", new PyLong(0));
        args->SetItemString("claimStructureID", PyStatic.NewZero());
        args->SetItemString("hubID", PyStatic.NewZero());
        args->SetItemString("allianceID", new PyInt(sysData.factionID));
        args->SetItemString("solarSystemID", new PyInt(systemID));
        _log(SOV__INFO, "SovereigntyDataMgr::GetSystemSovereignty(): Faction system %u found, assigning factionID as allianceID.", systemID);
    }
    else
    {
        auto it = m_sovData.get<SovDataBySolarSystem>().find(systemID);
        if (it != m_sovData.get<SovDataBySolarSystem>().end())
        {
            SovereigntyData sData = *it;
            _log(SOV__INFO, "SovereigntyDataMgr::GetSystemSovereignty(): Found sov data for solarSystemID %u.", systemID);
            _log(SOV__DEBUG, "== data (GetSystemSovereignty()) ==");
            _log(SOV__DEBUG, "claimID: %u", sData.claimID);
            _log(SOV__DEBUG, "solarSystemID: %u", sData.solarSystemID);
            _log(SOV__DEBUG, "constellationID: %u", sData.constellationID);
            _log(SOV__DEBUG, "corporationID: %u", sData.corporationID);
            _log(SOV__DEBUG, "allianceID: %u", sData.allianceID);
            _log(SOV__DEBUG, "claimStructureID: %u", sData.claimStructureID);
            _log(SOV__DEBUG, "==========");
            args->SetItemString("contested", new PyInt(sData.contested));
            args->SetItemString("corporationID", new PyInt(sData.corporationID));
            args->SetItemString("claimTime", new PyLong(sData.claimTime));
            args->SetItemString("claimStructureID", new PyInt(sData.claimStructureID));
            args->SetItemString("hubID", new PyInt(sData.hubID));
            args->SetItemString("allianceID", new PyInt(sData.allianceID));
            args->SetItemString("solarSystemID", new PyInt(sData.solarSystemID));
        }
        else
        {
            _log(SOV__INFO, "SovereigntyDataMgr::GetSystemSovereignty(): No data for solarSystemID %u. Sending blank SovereigntyData object.", systemID);
            args->SetItemString("contested", PyStatic.NewNone());
            args->SetItemString("corporationID", PyStatic.NewNone());
            args->SetItemString("claimTime", PyStatic.NewNone());
            args->SetItemString("claimStructureID", PyStatic.NewNone());
            args->SetItemString("hubID", PyStatic.NewNone());
            args->SetItemString("allianceID", PyStatic.NewNone());
            args->SetItemString("solarSystemID", new PyInt(systemID));
        }
    }
    return new PyObject("util.KeyVal", args);
}

PyRep *SovereigntyDataMgr::GetAllianceSystems() //Get all systems associated with all alliances?
{
    PyList *list = new PyList();
    for (auto &it : m_sovData.get<SovDataBySolarSystem>())
    {
        _log(SOV__DEBUG, "== data (GetAllianceSystems()) ==");
        _log(SOV__DEBUG, "claimID: %u", it.claimID);
        _log(SOV__DEBUG, "solarSystemID: %u", it.solarSystemID);
        _log(SOV__DEBUG, "allianceID: %u", it.allianceID);
        _log(SOV__DEBUG, "==========");

        PyDict *dict = new PyDict();
        dict->SetItemString("allianceID", new PyInt(it.allianceID));
        dict->SetItemString("solarSystemID", new PyInt(it.solarSystemID));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }
    return list;
}

PyRep *SovereigntyDataMgr::GetAllianceBeacons(uint32 allianceID) //Get all beacons associated with current alliance
{
    PyList* list = new PyList();
    for (SovereigntyData const &sData : boost::make_iterator_range(
                 m_sovData.get<SovDataByAlliance>().equal_range(allianceID)))
    {
        // Don't add systems where no beacon exists
        if (sData.beaconID == 0) {
            continue;
        }
        _log(SOV__DEBUG, "== data (GetAllianceBeacons()) ==");
        _log(SOV__DEBUG, "solarSystemID: %u", sData.solarSystemID);
        _log(SOV__DEBUG, "beaconID: %u", sData.beaconID);
        _log(SOV__DEBUG, "==========");

        PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0, new PyInt(sData.solarSystemID));
        tuple->SetItem(1, new PyInt(sData.beaconID));
        tuple->SetItem(2, new PyInt(EVEDB::invTypes::CynosuralGeneratorArray));
        list->AddItem(tuple);
    }
    return list;
}

PyRep *SovereigntyDataMgr::GetCurrentSovData(uint32 locationID)
{
    DBRowDescriptor *header = new DBRowDescriptor();
    header->AddColumn("locationID", DBTYPE_I4);
    header->AddColumn("allianceID", DBTYPE_I4);
    header->AddColumn("stationCount", DBTYPE_I2);
    header->AddColumn("militaryPoints", DBTYPE_I2);
    header->AddColumn("industrialPoints", DBTYPE_I2);
    header->AddColumn("claimedFor", DBTYPE_I4);
    CRowSet *rowset = new CRowSet(&header);

    if (IsConstellationID(locationID))
    {
        for (SovereigntyData const &sData : boost::make_iterator_range(
                 m_sovData.get<SovDataByConstellation>().equal_range(locationID)))
        {
            PyPackedRow *row = rowset->NewRow();
            row->SetField("locationID", new PyInt(sData.solarSystemID));
            row->SetField("allianceID", new PyInt(sData.allianceID));
            row->SetField("stationCount", new PyInt(sData.stationCount));
            row->SetField("militaryPoints", new PyInt(sData.militaryPoints));
            row->SetField("industrialPoints", new PyInt(sData.industrialPoints));
            row->SetField("claimedFor", new PyInt(sData.allianceID));
        }
    }
    //Get all unique alliances in the region who hold sovereignty
    else if (IsRegionID(locationID))
    {
        std::vector<uint32> cv;
        for (SovereigntyData const &sData : boost::make_iterator_range(
                 m_sovData.get<SovDataByRegion>().equal_range(locationID)))
        {
            if (!(std::find(cv.begin(), cv.end(), sData.constellationID) != cv.end()))
            {
                PyPackedRow *row = rowset->NewRow();
                row->SetField("locationID", new PyInt(sData.constellationID));
                row->SetField("allianceID", new PyInt(sData.allianceID));
                row->SetField("stationCount", new PyInt(sData.stationCount));
                row->SetField("militaryPoints", new PyInt(sData.militaryPoints));
                row->SetField("industrialPoints", new PyInt(sData.industrialPoints));
                row->SetField("claimedFor", new PyInt(sData.allianceID));
                cv.push_back(sData.constellationID);
            }
        }
    }
    else if (sDataMgr.IsSolarSystem(locationID))
    {
        for (SovereigntyData const &sData : boost::make_iterator_range(
                 m_sovData.get<SovDataByRegion>().equal_range(locationID)))
        {
            PyPackedRow *row = rowset->NewRow();
            row->SetField("locationID", new PyInt(sData.solarSystemID));
            row->SetField("allianceID", new PyInt(sData.allianceID));
            row->SetField("stationCount", new PyInt(sData.stationCount));
            row->SetField("militaryPoints", new PyInt(sData.militaryPoints));
            row->SetField("industrialPoints", new PyInt(sData.industrialPoints));
            row->SetField("claimedFor", new PyInt(sData.allianceID));
        }
    }

    return rowset;
}

void SovereigntyDataMgr::AddSovClaim(SovereigntyData data)
{
    _log(SOV__INFO, "AddSovClaim() - Adding claim for %u to DataMgr...", data.solarSystemID);

    //Add a new sovereignty claim to DB
    uint32 claimID(0);
    SovereigntyDB::AddSovereigntyData(data, claimID);
    _log(SOV__DEBUG, "AddSovClaim() - ClaimID %u added to DB...", claimID);

    //Define our view from container
    auto &bySolar = m_sovData.get<SovDataBySolarSystem>();

    //Delete existing records in container which match the systemID
    auto it = m_sovData.get<SovDataBySolarSystem>().find(data.solarSystemID);
    if (it != m_sovData.get<SovDataBySolarSystem>().end())
    {
        bySolar.erase(data.solarSystemID);
    }

    UpdateClaim(data.solarSystemID);
}

void SovereigntyDataMgr::MarkContested(uint32 systemID, bool contested)
{
    _log(SOV__INFO, "MarkContested() - Marking system %u as state %u", systemID, int(contested));

    //Update state in DB
    SovereigntyDB::SetContested(systemID, contested);

    //Re-sync the claim data
    UpdateClaim(systemID);
}

void SovereigntyDataMgr::RemoveSovClaim(uint32 systemID)
{
    _log(SOV__INFO, "RemoveSovClaim() - Removing claim for %u from DataMgr...", systemID);

    //Delete sovereignty claim from DB
    SovereigntyDB::RemoveSovereigntyData(systemID);
    _log(SOV__DEBUG, "RemoveSovClaim() - Claim for %u removed from DB...", systemID);

    //Remove claim from container
    auto &bySolar = m_sovData.get<SovDataBySolarSystem>();
    bySolar.erase(systemID);
}

void SovereigntyDataMgr::UpdateSystemHubID(uint32 systemID, uint32 hubID)
{
    _log(SOV__INFO, "UpdateSystemHubID() - Updating hubID for system %u with hub %u", systemID, hubID);

    //Update state in DB
    SovereigntyDB::SetHubID(systemID, hubID);

    //Re-sync the claim data
    UpdateClaim(systemID);
}

void SovereigntyDataMgr::UpdateSystemJammerID(uint32 systemID, uint32 jammerID)
{
    _log(SOV__INFO, "UpdateSystemJammerID() - Updating jammerID for system %u with jammer %u", systemID, jammerID);

    //Update state in DB
    SovereigntyDB::SetJammerID(systemID, jammerID);

    //Re-sync the claim data
    UpdateClaim(systemID);
}

void SovereigntyDataMgr::UpdateSystemBeaconID(uint32 systemID, uint32 beaconID)
{
    _log(SOV__INFO, "UpdateSystemBeaconID() - Updating beaconID for system %u with beacon %u", systemID, beaconID);

    //Update state in DB
    SovereigntyDB::SetBeaconID(systemID, beaconID);

    //Re-sync the claim data
    UpdateClaim(systemID);
}

// Update managed claim data from database, effectively a re-sync
void SovereigntyDataMgr::UpdateClaim(uint32 systemID)
{
    //Define our view from container
    auto &bySolar = m_sovData.get<SovDataBySolarSystem>();

    //Remove old from container
    bySolar.erase(systemID);

    //Get the data from the DB, this will avoid inconsistencies
    DBQueryResult *res = new DBQueryResult();
    DBResultRow row;

    SovereigntyDB::GetSovereigntyDataForSystem(*res, systemID);
    while (res->GetRow(row))
    {
        SovereigntyData sData = SovereigntyData();
        sData.solarSystemID = row.GetUInt(0);
        sData.constellationID = row.GetUInt(1);
        sData.regionID = row.GetUInt(2);
        sData.corporationID = row.GetUInt(3);
        sData.allianceID = row.GetUInt(4);
        sData.claimStructureID = row.GetUInt(5);
        sData.claimTime = row.GetInt64(6);
        sData.hubID = row.GetUInt(7);
        sData.contested = row.GetUInt(8);
        sData.stationCount = row.GetUInt(9);
        sData.militaryPoints = row.GetUInt(10);
        sData.industrialPoints = row.GetUInt(11);
        sData.claimID = row.GetUInt(12);
        sData.beaconID = row.GetUInt(13);
        sData.jammerID = row.GetUInt(14);
        bySolar.insert(sData);
    }
    SafeDelete(res);
}