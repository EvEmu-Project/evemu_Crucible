
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
#include "packets/Sovereignty.h"
#include "StaticDataMgr.h"
#include "Client.h"
#include <boost/range/iterator_range.hpp>

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
    sLog.Blue("   SovereigntyDataMgr", "Sovereignty Data Manager Initialized.");
    return 1;
}

void SovereigntyDataMgr::Close()
{
    /** @todo put a save method here which will save anything changed before shutdown */
    //for (auto cur : m_sovPyData)
    //    PySafeDecRef(cur.second);

    sLog.Warning("   SovereigntyDataMgr", "Sovereignty Data Manager has been closed.");
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
        sData.corporationID = row.GetUInt(2);
        sData.allianceID = row.GetUInt(3);
        sData.claimStructureID = row.GetUInt(4);
        sData.claimTime = row.GetInt64(5);
        sData.hubID = row.GetUInt(6);
        sData.contested = row.GetUInt(7);
        sData.stationCount = row.GetUInt(8);
        sData.militaryPoints = row.GetUInt(9);
        sData.industrialPoints = row.GetUInt(10);
        sData.claimID = row.GetUInt(11);
        bySolar.insert(sData);
    }

    //Just for testing, lets see what we are putting into our container
    for (auto &it : m_sovData.get<SovDataBySolarSystem>())
    {
        _log(SOV__DEBUG, "== data (initial population) ==");
        _log(SOV__DEBUG, "claimID: %u", it.claimID);
        _log(SOV__DEBUG, "solarSystemID: %u", it.solarSystemID);
        _log(SOV__DEBUG, "constellationID: %u", it.constellationID);
        _log(SOV__DEBUG, "corporationID: %u", it.corporationID);
        _log(SOV__DEBUG, "allianceID: %u", it.allianceID);
        _log(SOV__DEBUG, "claimStructureID: %u", it.claimStructureID);
        _log(SOV__DEBUG, "==========");
    }

    sLog.Cyan("   SovereigntyDataMgr", "%u Sovereignty data sets loaded in %.3fms.",
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

PyRep *SovereigntyDataMgr::GetSystemSovereignty(uint32 systemID)
{
    SystemData sysData;
    PyDict *args = new PyDict;

    //Figure out if this system is in empire space
    sDataMgr.GetSystemData(systemID, sysData);

    if (sysData.factionID)
    { //If we have a factionID, we can set the system's sov data to it
        args->SetItemString("contested", new PyInt(0));
        args->SetItemString("corporationID", new PyInt(0));
        args->SetItemString("claimTime", new PyLong(0));
        args->SetItemString("claimStructureID", new PyInt(0));
        args->SetItemString("hubID", new PyInt(0));
        args->SetItemString("allianceID", new PyInt(sysData.factionID));
        args->SetItemString("solarSystemID", new PyInt(systemID));
        _log(SOV__INFO, "SovereigntyDataMgr::GetSystemSovereignty()", "Faction system %u found, assigning factionID as allianceID.", systemID);
    }
    else
    {
        auto it = m_sovData.get<SovDataBySolarSystem>().find(systemID);
        if (it != m_sovData.get<SovDataBySolarSystem>().end())
        {
            SovereigntyData sData = *it;
            _log(SOV__INFO, "SovereigntyDataMgr::GetSystemSovereignty()", "Found sov data for solarSystemID %u.", systemID);
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
            _log(SOV__ERROR, "SovereigntyDataMgr::GetSystemSovereignty()", "Unable to find data for solarSystemID %u.", systemID);
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

PyRep *SovereigntyDataMgr::GetCurrentSovData(uint32 locationID)
{
    PyList *list = new PyList();

    DBRowDescriptor* header = new DBRowDescriptor();
    header->AddColumn( "quantity",          DBTYPE_I4 );
    header->AddColumn( "requiredTypeID",    DBTYPE_I4 );
    header->AddColumn( "damagePerJob",      DBTYPE_R4 );

    if (IsConstellation(locationID))
    {
        for (SovereigntyData const &sData : boost::make_iterator_range(m_sovData.get<SovDataByConstellation>().equal_range(locationID)))
        {
            PyDict *dict = new PyDict();
            _log(SOV__INFO, "SovereigntyDataMgr::GetCurrentSovData()", "Found current sov data for solarSystemID %u.", sData.solarSystemID);
            _log(SOV__DEBUG, "== data (GetCurrentSovData()) ==");
            _log(SOV__DEBUG, "locationID: %u", sData.solarSystemID);
            _log(SOV__DEBUG, "allianceID: %u", sData.allianceID);
            _log(SOV__DEBUG, "stationCount: %u", sData.stationCount);
            _log(SOV__DEBUG, "militaryPoints: %u", sData.militaryPoints);
            _log(SOV__DEBUG, "industrialPoints: %u", sData.industrialPoints);
            _log(SOV__DEBUG, "claimedFor: %u", sData.allianceID);
            _log(SOV__DEBUG, "==========");

            dict->SetItemString("locationID", new PyInt(sData.solarSystemID));
            dict->SetItemString("allianceID", new PyInt(sData.allianceID));
            dict->SetItemString("stationCount", new PyInt(sData.stationCount));
            dict->SetItemString("militaryPoints", new PyInt(sData.militaryPoints));
            dict->SetItemString("industrialPoints", new PyInt(sData.industrialPoints));
            dict->SetItemString("claimedFor", new PyInt(sData.allianceID));
            list->AddItem(new PyObject("util.KeyVal", dict));
        }
    }
    else if (IsSolarSystem(locationID))
    {
        auto it = m_sovData.get<SovDataBySolarSystem>().find(locationID);
        if (it != m_sovData.get<SovDataBySolarSystem>().end())
        {
            PyDict *dict = new PyDict();
            SovereigntyData sData = *it;
            _log(SOV__INFO, "SovereigntyDataMgr::GetCurrentSovData()", "Found current sov data for solarSystemID %u.", sData.solarSystemID);
            _log(SOV__DEBUG, "== data (GetCurrentSovData()) ==");
            _log(SOV__DEBUG, "locationID: %u", sData.solarSystemID);
            _log(SOV__DEBUG, "allianceID: %u", sData.allianceID);
            _log(SOV__DEBUG, "stationCount: %u", sData.stationCount);
            _log(SOV__DEBUG, "militaryPoints", sData.militaryPoints);
            _log(SOV__DEBUG, "industrialPoints", sData.industrialPoints);
            _log(SOV__DEBUG, "claimedFor", sData.allianceID);
            _log(SOV__DEBUG, "==========");

            dict->SetItemString("locationID", new PyInt(sData.solarSystemID));
            dict->SetItemString("allianceID", new PyInt(sData.allianceID));
            dict->SetItemString("stationCount", new PyInt(sData.stationCount));
            dict->SetItemString("militaryPoints", new PyInt(sData.militaryPoints));
            dict->SetItemString("industrialPoints", new PyInt(sData.industrialPoints));
            dict->SetItemString("claimedFor", new PyInt(sData.allianceID));
            list->AddItem(new PyObject("util.KeyVal", dict));
        }
    }

    return list;
}