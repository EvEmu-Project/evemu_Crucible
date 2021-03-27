
 /**
  * @name SystemGPoint.cpp
  *   a group of methods and functions to get random points in a given solarsystem
  * @Author:         Allan
  * @date:   31 July 2014
  */


//work in progress
/** @todo  move this into MapData class  */


#include "eve-server.h"

#include "system/SystemGPoint.h"
#include "system/SystemManager.h"

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
 *   class DBGPointEntity has index, itemID, radius, and position, and is found in SystemDB.
 *      see copy of class declaration below
 *
 *  NOTE i seem to remember that ALL COSMIC SPAWNS are within 6au from planets.
 *       ....cant find that info now.  -allan 31Jul14
 */


/// int64 MakeRandomInt( int64 low, int64 high )
/// double MakeRandomFloat( double low, double high )

/*   copied from system/SystemDB.cpp
 * namespace SystemDB:
 * class DBGPointEntity {
 *  public:
 *	   uint8 idx;
 *    uint32 itemID;
 *    GPoint position;
 *    double radius;
 *    double x;
 *    double y;
 *    double z;
 * };
 *
*/

void SystemGPoint::GetPlanets(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> planetIDs;
    planetIDs.clear();
    m_db.GetPlanets(systemID, planetIDs, total);
}

void SystemGPoint::GetMoons(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> moonIDs;
    moonIDs.clear();
    m_db.GetMoons(systemID, moonIDs, total);
}

const GPoint SystemGPoint::GetRandPointOnPlanet(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> planetIDs;
    planetIDs.clear();
    m_db.GetPlanets(systemID, planetIDs, total);

    if (planetIDs.empty())
        return NULL_ORIGIN;

    uint16 i = MakeRandomInt(1, total);
    return (planetIDs[i].position + planetIDs[i].radius + 50000);
}

const GPoint SystemGPoint::GetRandPointOnMoon(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> moonIDs;
    moonIDs.clear();
    m_db.GetMoons(systemID, moonIDs, total);

    if (moonIDs.empty())
        return NULL_ORIGIN;

    uint16 i = MakeRandomInt(1, total);
    return (moonIDs[i].position + moonIDs[i].radius + 10000);
}

uint32 SystemGPoint::GetRandPlanet(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> planetIDs;
    planetIDs.clear();
    m_db.GetPlanets(systemID, planetIDs, total);

    if (planetIDs.empty())
        return 0;

    uint16 i = MakeRandomInt(1, total);
    return planetIDs[i].itemID;
}

const GPoint SystemGPoint::Get2RandPlanets(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> planetIDs;
    planetIDs.clear();
    m_db.GetPlanets(systemID, planetIDs, total);
    /** @todo finish this */
    return NULL_ORIGIN;
}

const GPoint SystemGPoint::Get3RandPlanets(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> planetIDs;
    planetIDs.clear();
    m_db.GetPlanets(systemID, planetIDs, total);
    /** @todo finish this */

    return NULL_ORIGIN;
}

uint32 SystemGPoint::GetRandMoon(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBGPointEntity> moonIDs;
    moonIDs.clear();
    m_db.GetMoons(systemID, moonIDs, total);

    if (moonIDs.empty())
        return 0;

    uint16 i = MakeRandomInt(1, total);
    return moonIDs[i].itemID;
}

const GPoint SystemGPoint::GetRandPointInSystem(uint32 systemID, int64 distance) {
    // get system max diameter, verify distance is within system.

    return NULL_ORIGIN;
}

const GPoint SystemGPoint::GetAnomalyPoint(SystemManager* pSys)
{
    uint8 total = 0;
    std::vector<DBGPointEntity> planetIDs;
    planetIDs.clear();
    m_db.GetPlanets(pSys->GetID(), planetIDs, total);

    SystemEntity* pSE(pSys->GetSE(planetIDs[MakeRandomInt(0, total)].itemID));

    GPoint pos(pSE->GetPosition());
    pos.MakeRandomPointOnSphereLayer(ONE_AU_IN_METERS / 3, ONE_AU_IN_METERS * 4);
    return pos;
}

