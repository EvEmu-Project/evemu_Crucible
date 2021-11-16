
/**
 * @name MapData.h
 *   a group of methods and functions to get map info.
 *     this is mostly used for getting random points in system, system jumps, and misc mission destination info
 * @Author:         Allan
 * @date:   13 November 2018
 */


#ifndef _EVE_MAP_MAPDATA_H_
#define _EVE_MAP_MAPDATA_H_

#include "../eve-server.h"

#include "../../eve-common/EVE_Missions.h"


class Agent;
class SystemManager;

class MapData
: public Singleton< MapData >
{
public:
    MapData();
    ~MapData()                                          { /* do nothing here */ }

    int                 Initialize();

    void                Clear();
    void                Close();

    void                GetInfo();

    PyTuple*            GetStationExtraInfo()           { PyIncRef(m_stationExtraInfo); return m_stationExtraInfo; }
    PyObject*           GetPseudoSecurities()           { PyIncRef(m_pseudoSecurities); return m_pseudoSecurities; }

    void                GetMissionDestination(Agent* pAgent, uint8 misionType, MissionOffer& offer);


    void                GetMoons(uint32 systemID);   // incomplete
    void                GetPlanets(uint32 systemID); // incomplete
    uint32              GetRandMoon(uint32 systemID);
    uint32              GetRandPlanet(uint32 systemID);
    const GPoint        GetAnomalyPoint(SystemManager* pSys);
    const GPoint        GetAnomalyPoint(uint32 systemID);
    const GPoint        Get2RandPlanets(uint32 systemID);// incomplete
    const GPoint        Get3RandPlanets(uint32 systemID);// incomplete
    const GPoint        GetRandPointOnMoon(uint32 systemID);
    const GPoint        GetRandPointOnPlanet(uint32 systemID);
    const GPoint        GetRandPointInSystem(uint32 systemID, int64 distance);// incomplete

protected:
    void                Populate();

private:
    PyTuple*            m_stationExtraInfo;
    PyObject*           m_pseudoSecurities;

    std::multimap<uint32, uint32>        m_regionJumps;  //fromSys/toSys
    std::multimap<uint32, uint32>        m_constJumps;   //fromSys/toSys
    std::multimap<uint32, uint32>        m_systemJumps;  //fromSys/toSys
};


//Singleton
#define sMapData \
( MapData::get() )


#endif  // _EVE_MAP_MAPDATA_H_

/**
 *   the purpose of this class is to have a common location with methods used to define
 *      random points in solar systems based on planet and moon positions.
 *   GetRandPointOnPlanet() will query solar systems for planets, pick a random planet,
 *      and define a coordnate within that planet's bubble.
 *   GetRandPointOnMoon() does same as above, but using moons.
 *   Get2RandPlanets() will pick a random point between 2 planets, for warp-out/warp-in
 *      and other things as we see fit.
 *   Get3RandPlanets() does same as above, but using 3 planets.
 *   GetRandPointInSystem() will define a random point within a given system.
 *
 *   this class of methods should be used for positioning mission space, cosmic signatures,
 *      anomolies, complexes, and other things needing a random position in a given system.
 *
 *   these methods will also be used for random npc spawns and their warping/movement.
 *
 *
 *  NOTE i seem to remember that ALL COSMIC SPAWNS are within 6au from planets.
 *       ....cant find that info now.  -allan 31Jul14
 */
