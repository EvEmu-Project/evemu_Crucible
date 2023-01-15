
/**
 * @name MapData.cpp
 *   a group of methods and functions to get map info.
 *     this is mostly used for getting random points in system, system jumps, and misc mission destination info
 *  - added static data for StationExtraInfo (from mapservice)
 * @Author:         Allan
 * @date:   13 November 2018
 */
#include "../StaticDataMgr.h"
#include "agents/Agent.h"
#include "map/MapData.h"
#include "map/MapDB.h"
#include "station/StationDataMgr.h"
#include "system/SystemManager.h"
#include "system/SystemEntity.h"

#include "../../eve-common/EVE_Map.h"


MapData::MapData()
: m_stationExtraInfo(nullptr),
m_pseudoSecurities(nullptr)
{
    m_regionJumps.clear();
    m_constJumps.clear();
    m_systemJumps.clear();
}

void MapData::Close()
{
    PySafeDecRef(m_stationExtraInfo);
    PySafeDecRef(m_pseudoSecurities);
}

int MapData::Initialize()
{
    Populate();
    sLog.Blue("          MapData", "Map Data Manager Initialized.");
    return 1;
}

void MapData::Clear()
{
    m_regionJumps.clear();
    m_constJumps.clear();
    m_systemJumps.clear();
}

void MapData::GetInfo()
{
    // print out list of bad jumps
    // m_badJumps
}

void MapData::Populate()
{
    m_pseudoSecurities = MapDB::GetPseudoSecurities();

    double start = GetTimeMSeconds();

    m_stationExtraInfo = new PyTuple(3);
    m_stationExtraInfo->items[0] = MapDB::GetStationExtraInfo();
    m_stationExtraInfo->items[1] = MapDB::GetStationOpServices();
    m_stationExtraInfo->items[2] = MapDB::GetStationServiceInfo();
    sLog.Cyan("          MapData", "StationExtraInfo loaded in %.3fms.",(GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();
    MapDB::GetSystemJumps(*res);
    DBResultRow row;
    while (res->GetRow(row)) {
        //SELECT ctype, fromsol, tosol FROM mapConnections
        if (row.GetInt(0) == Map::Jumptype::Region) {
            m_regionJumps.emplace(row.GetInt(1), row.GetInt(2));
        } else if (row.GetInt(0) == Map::Jumptype::Constellation) {
            m_constJumps.emplace(row.GetInt(1), row.GetInt(2));
        } else {
            m_systemJumps.emplace(row.GetInt(1), row.GetInt(2));
        }
    }

    sLog.Cyan("          MapData", "%lu Region jumps, %lu Constellation jumps and %lu System jumps loaded in %.3fms.", //
              m_regionJumps.size(), m_constJumps.size(), m_systemJumps.size(), (GetTimeMSeconds() - start));

    // cleanup
    SafeDelete(res);
}



void MapData::GetMissionDestination(Agent* pAgent, uint8 misionType, MissionOffer& offer)
{
    using namespace Mission::Type;
    using namespace Agents::Range;

    uint8 destRange = offer.range;
    bool station = true, ship = false;    // will have to tweak this later for particular mission events

    // determine distance based on preset range from db or in some cases, mission type and agent level
    switch(misionType) {
        case Tutorial: {
            // always same system?
            destRange = SameSystem;
        } break;
        case Data:
        case Trade:
        case Courier:
        case Research: {
            //destRange += m_data.level *2;
        } break;
        case EpicArc:
        case Anomic:
        case Burner:
        case Cosmos: {
            station = false;
            destRange += pAgent->GetLevel();
        } break;
        case Mining:
        case Encounter:
        case Storyline: {
            station = false;
            //destRange = offer.range;
        } break;
    }

    switch(destRange) {
        case 0:
        case SameSystem: //1
        case SameOrNeighboringSystemSameConstellation:    //2
        case NeighboringSystemSameConstellation:  //4
        case SameConstellation:  {  //6
            uint32 systemID = pAgent->GetSystemID();
            if (station)
                if (sDataMgr.GetStationCount(systemID) < 2)
                    ++destRange;

            if ((destRange > 1) or (IsEven(MakeRandomInt(0, 100)))) {
                // neighboring system
                bool run = true;
                uint8 count = 0;
                std::vector<uint32> sysList;
                auto itr = m_systemJumps.equal_range(systemID);
                for (auto it = itr.first; it != itr.second; ++it)
                    sysList.push_back(it->second);
                /** @todo not sure why this is empty, but have segfaults from empty vector. */
                if (sysList.empty()) {
                    StationData data = StationData();
                    stDataMgr.GetStationData(pAgent->GetStationID(), data);
                    offer.destinationOwnerID    = data.corporationID;
                    offer.destinationSystemID   = data.systemID;
                    offer.destinationTypeID     = data.typeID;
                    return;
                }

                while (run) {
                    run = false;
                    systemID = sysList.at(MakeRandomInt(0, (sysList.size() -1)));
                    if (station and (sDataMgr.GetStationCount(systemID) < 1)) {
                        run = true;
                        ++count;
                    }
                    if (run and (count > sysList.size())) {
                        // problem....no station found within one jump
                        offer.destinationID = 0;
                        _log(AGENT__ERROR, "Agent::GetMissionDestination() - no station found within 1 jump." );
                        return;
                    }
                }
            }
            if (station) {
                std::vector<uint32> list;
                sDataMgr.GetStationList(systemID, list);
                if (list.size() < 2) {
                    offer.destinationID = list.at(0);
                } else {
                    bool run = true;
                    while (run) {
                        offer.destinationID = list.at(MakeRandomInt(0, (list.size() -1)));
                        if (offer.destinationID != pAgent->GetStationID())
                            run = false;
                    }
                }
            } else if (ship) {
                ;  // code here for agent in ship
            }
        } break;

        //may have to create data objects based on constellation to do ranges in neighboring constellation
        // could use data from mapSolarSystemJumps - fromRegionID, fromConstellationID, fromSolarSystemID, toSolarSystemID, toConstellationID, toRegionID

        /** @todo  make function to find route from origin to constellation/region jump point.  */
        case SameOrNeighboringSystem:  //3
        case NeighboringSystem: {  //5
            uint32 systemID = pAgent->GetSystemID();
            if (IsEven(MakeRandomInt(0, 100))) {
                // same constellation
            } else {
                // neighboring constellation
                systemID = 0;
            }
            if (ship) {
                ;  // code here for agent in ship
            }
        } break;
        case SameOrNeighboringConstellationSameRegion:   //7
        case NeighboringConstellationSameRegion: {  //9
            if (station)
                sDataMgr.GetStationConstellation(pAgent->GetStationID());
        } break;
        case SameOrNeighboringConstellation:  //8
        case NeighboringConstellation: {    //10
            if (station)
                sDataMgr.GetStationRegion(pAgent->GetStationID());
        } break;
        // not sure how to do these two yet....
        case NearestEnemyCombatZone: {  //11
        } break;
        case NearestCareerHub: {    //12
        } break;
    }

    if (sDataMgr.IsStation(offer.destinationID)) {
        StationData data = StationData();
        stDataMgr.GetStationData(offer.destinationID, data);
        offer.destinationOwnerID    = data.corporationID;
        offer.destinationSystemID   = data.systemID;
        offer.destinationTypeID     = data.typeID;
    } else if (ship) {
        offer.destinationSystemID   = offer.destinationID;
        offer.destinationTypeID     = sDataMgr.GetStaticType(offer.destinationID);
        offer.dungeonLocationID     = offer.destinationID;
        offer.dungeonSolarSystemID  = offer.destinationID;
    } else {
        offer.destinationSystemID   = offer.destinationID;
        offer.destinationTypeID     = sDataMgr.GetStaticType(offer.destinationID);
        offer.dungeonLocationID     = offer.destinationID;
        offer.dungeonSolarSystemID  = offer.destinationID;
    }
}

void MapData::GetPlanets(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> planetIDs;
    planetIDs.clear();
    MapDB::GetPlanets(systemID, planetIDs, total);
}

void MapData::GetMoons(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> moonIDs;
    moonIDs.clear();
    MapDB::GetMoons(systemID, moonIDs, total);
}

const GPoint MapData::GetRandPointOnPlanet(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> planetIDs;
    planetIDs.clear();
    MapDB::GetPlanets(systemID, planetIDs, total);

    if (planetIDs.empty())
        return NULL_ORIGIN;

    uint16 i = MakeRandomInt(1, total);
    return (planetIDs[i].position + planetIDs[i].radius + 50000);
}

const GPoint MapData::GetRandPointOnMoon(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> moonIDs;
    moonIDs.clear();
    MapDB::GetMoons(systemID, moonIDs, total);

    if (moonIDs.empty())
        return NULL_ORIGIN;

    uint16 i = MakeRandomInt(1, total);
    return (moonIDs[i].position + moonIDs[i].radius + 10000);
}

uint32 MapData::GetRandPlanet(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> planetIDs;
    planetIDs.clear();
    MapDB::GetPlanets(systemID, planetIDs, total);

    if (planetIDs.empty())
        return 0;

    uint16 i = MakeRandomInt(1, total);
    return planetIDs[i].itemID;
}

const GPoint MapData::Get2RandPlanets(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> planetIDs;
    planetIDs.clear();
    MapDB::GetPlanets(systemID, planetIDs, total);
    /** @todo finish this */
    return NULL_ORIGIN;
}

const GPoint MapData::Get3RandPlanets(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> planetIDs;
    planetIDs.clear();
    MapDB::GetPlanets(systemID, planetIDs, total);
    /** @todo finish this */

    return NULL_ORIGIN;
}

uint32 MapData::GetRandMoon(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> moonIDs;
    moonIDs.clear();
    MapDB::GetMoons(systemID, moonIDs, total);

    if (moonIDs.empty())
        return 0;

    uint16 i = MakeRandomInt(1, total);
    return moonIDs[i].itemID;
}

const GPoint MapData::GetRandPointInSystem(uint32 systemID, int64 distance) {
    // get system max diameter, verify distance is within system.

    return NULL_ORIGIN;
}

const GPoint MapData::GetAnomalyPoint(SystemManager* pSys)
{
    uint8 total = 0;
    std::vector<DBGPointEntity> planetIDs;
    planetIDs.clear();
    MapDB::GetPlanets(pSys->GetID(), planetIDs, total);

    SystemEntity* pSE(pSys->GetSE(planetIDs[MakeRandomInt(0, total)].itemID));

    GPoint pos(pSE->GetPosition());
    pos.MakeRandomPointOnSphereLayer(ONE_AU_IN_METERS / 3, ONE_AU_IN_METERS * 4);
    return pos;
}

const GPoint MapData::GetAnomalyPoint(uint32 systemID)
{
    uint8 total = 0;
    std::vector<DBGPointEntity> planetIDs;
    planetIDs.clear();
    MapDB::GetPlanets(systemID, planetIDs, total);
    GPoint pos(planetIDs[MakeRandomInt(0, total)].position);
    pos.MakeRandomPointOnSphereLayer(ONE_AU_IN_METERS / 3, ONE_AU_IN_METERS * 4);
    return pos;
}