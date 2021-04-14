
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
        sData.claimTime = row.GetUInt(5);
        sData.hubID = row.GetUInt(6);
        sData.contested = row.GetBool(7);
        sData.stationCount = row.GetUInt(8);
        sData.militaryPoints = row.GetUInt(9);
        sData.industrialPoints = row.GetUInt(10);
        bySolar.insert(&sData); //Insert into the container
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
        codelog(SOV__INFO, "SovereigntyDataMgr::GetSystemSovereignty()", "Faction system %u found, assigning factionID as allianceID.", systemID);
    }
    else
    {
        //Start claim time at 0
        uint32 cTime = 0;
        uint32 claimID = 0;

        //Get what we want from the container
        //We may have multiple sov change records for a solar system, so it is important we load them all

        auto &bySolar = m_sovData.get<SovDataBySolarSystem>();
        for (auto it = bySolar.begin(); it != bySolar.end();)
        {
            SovereigntyData *found = *it;
            //Now we have an individual record, we need to find out which one has the highest claimTime
            if (found->claimTime > cTime)
            {
                cTime = found->claimTime;
                claimID = found->claimID;
            }
            it = bySolar.upper_bound(found->solarSystemID);
        }

        //Now we have the latest effective claimID
        if (claimID != 0)
        {
            auto &byClaim = m_sovData.get<SovDataByClaim>();
            auto itFound = byClaim.find(claimID);
            if (itFound != byClaim.end())
            {
                SovereigntyData *sData = *itFound;
                args->SetItemString("contested", new PyInt(sData->contested));
                args->SetItemString("corporationID", new PyInt(sData->corporationID));
                args->SetItemString("claimTime", new PyLong(sData->claimTime));
                args->SetItemString("claimStructureID", new PyInt(sData->claimStructureID));
                args->SetItemString("hubID", new PyInt(sData->hubID));
                args->SetItemString("allianceID", new PyInt(sData->allianceID));
                args->SetItemString("solarSystemID", new PyInt(sData->solarSystemID));
            }
        }
        else
        {
            codelog(SOV__INFO, "SovereigntyDataMgr::GetSystemSovereignty()", "No results found for %u in sov container.", systemID);
        }
    }
    return new PyObject("util.KeyVal", args);
}

/*
PyRep *SovereigntyDataMgr::GetAllianceSystems() //Get all systems associated with all alliances?
{
        //Get what we want from the container
        auto &bySolar = m_sovData.get<SovDataBySolarSystem>();
        auto found = m_sovData.find(systemID);

        //We may have multiple sov change records for a solar system, so it is important we load them all
        for (auto it = bySolar.begin(); it != bySolar.end();)
        {
            //Now we have an individual record, we need to find out which one has the highest claimTime
            if (it->claimTime > cTime)
            {
                cTime = it->claimTime;
                claimID = it->claimID;
            }
            it = bySolar.upper_bound(it->solarSystemID);
        }

    DBQueryResult res;
    DBResultRow row;

    PyList *list = new PyList();

    if (!sDatabase.RunQuery(res,
                            "SELECT"
                            "   allianceID,"
                            "   solarSystemID "
                            " FROM mapSystemSovInfo"))
    {
        codelog(SOV__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    //Clunky way to do it
    while (res.GetRow(row)) {
        PyDict* dict = new PyDict();
        dict->SetItemString("allianceID", new PyInt(row.GetInt(0)));
        dict->SetItemString("solarSystemID", new PyInt(row.GetInt(1)));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }
    return list;
    return DBResultToPackedRowList(res);
}
/*
PyRep *SovereigntyDB::GetCurrentSovData(uint32 locationID)
{
    //We get an arbitrary locationID, and need to get a list of solar system IDs along with allianceID
    //TODO: For now, we return 0 as stationCount since such system is not implemented
    //TODO: Handle militaryPoints and industrialPoints, but for now they are 5 by default since we don't have a mechanism for determining them

    DBQueryResult res;
    DBResultRow row;

    if (IsConstellation(locationID))
    { //Convert constellation into list of solar system IDs
        if (!sDatabase.RunQuery(res,
                                "SELECT mapSystemSovInfo.solarSystemID as locationID, mapSystemSovInfo.allianceID, 0 as stationCount, "
                                " 5 as militaryPoints, 5 as industrialPoints, mapSystemSovInfo.allianceID as claimedFor "
                                " FROM mapSystemSovInfo "
                                " INNER JOIN mapSolarSystems ON mapSolarSystems.solarSystemID=mapSystemSovInfo.solarSystemID "
                                " WHERE mapSolarSystems.constellationID=%u",
                                locationID))
        {
            codelog(SOV__ERROR, "Error in query: %s", res.error.c_str());
            return NULL;
        }
    }
    else if (IsSolarSystem(locationID))
    {
        if (!sDatabase.RunQuery(res,
                                "SELECT solarSystemID as locationID, allianceID, 0 as stationCount, "
                                " 0 as militaryPoints, 0 as industrialPoints, mapSystemSovInfo.allianceID as claimedFor "
                                " FROM mapSystemSovInfo "
                                " WHERE solarSystemID=%u",
                                locationID))
        {
            codelog(SOV__ERROR, "Error in query: %s", res.error.c_str());
            return NULL;
        }
    }

    return DBResultToCRowset(res);
}

*/