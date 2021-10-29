
 /**
  * @name BeltMgr.h
  *     Asteroid Belt Spawn management system for EVEmu
  *
  * @Author:        Allan
  * @date:          15 April 2016
  *
  */


#ifndef EVEMU_SYSTEM_BELTMGR_H_
#define EVEMU_SYSTEM_BELTMGR_H_

#include <unordered_map>
#include "system/Asteroid.h"
#include "system/SystemEntity.h"
#include "system/cosmicMgrs/ManagerDB.h"

/*  this class will keep track of all asteroid belts in a system
 * it is created on a per-system basis, and will also deal with
 * calling spawn/delete/grow functions for each belt.
 *
 * this class will also be in charge of belts in anomalies
 *
 *  a new iteration of this class is created for each system as that system
 * is booted.
 */


class PyServiceMgr;
class SystemManager;

class BeltMgr
{
public:
    BeltMgr(SystemManager* mgr, PyServiceMgr& svc);
    ~BeltMgr()    { /* do nothing here */ }

    void Init();
    void Save();
    void Process();
    void ClearAll();
    void ClearBelt(uint16 bubbleID);
    void SetActive(uint16 bubbleID, bool active=true);
    void RegisterBelt(InventoryItemRef itemRef);
    bool Create(CosmicSignature& sig, std::unordered_multimap<float, uint16>& roidTypes);

    bool Load(uint16 bubbleID);
    bool IsActive(uint16 bubbleID);
    bool IsSpawned(uint16 bubbleID);
    void CheckSpawn(uint16 bubbleID);

    void GetList(uint32 beltID, std::vector< AsteroidSE* >& list);

    void RemoveAsteroid(uint32 beltID, AsteroidSE* pASE);

protected:
    Timer m_respawnTimer;

    void SpawnBelt(uint16 bubbleID, std::unordered_multimap<float, uint16>& roidTypes, int type = 0, bool anomaly = false);
    void SpawnAsteroid(uint32 beltID, uint32 typeID, double radius, const GPoint& position, bool ice=false);
    void GetIceDist(uint8 quarter, float secStatus, std::unordered_multimap<float, uint16>& roidDist);

    uint32 GetAsteroidType(double p, const std::unordered_multimap<float, uint16>& roids);

private:
    SystemManager* m_system;    //we do not own this
    PyServiceMgr& m_services;    //we do not own this

    bool m_initialized;
    uint32 m_systemID;
    uint32 m_regionID;

    /* map contains beltID, boolean for IsSpawned() */
    std::map<uint32, bool> m_spawned;
    /* map contains beltID, boolean for IsActive() */
    std::map<uint32, bool> m_active;
    /* vector contains belt's itemID, itemRef */
    std::map<uint32, InventoryItemRef> m_belts;
    /*  this map contains beltID, asteroidSE for entire system */
    std::unordered_multimap<uint32, AsteroidSE*> m_asteroids;

};

#endif  // EVEMU_SYSTEM_BELTMGR_H_


/*  data for sovereign nullsec systems that contain an active Ore Prospecting Array.
 Small Asteroid Cluster
Arkonor      9600
Bistot      12800
Crokite     30000
Dark Ochre  16000
Gneiss     170000
Spodumain  300000

 Medium Asteroid Cluster
Arkonor     28000
Bistot      38700
Crokite     84700
Dark Ochre  31000
Gneiss     340000
Spodumain  270000
Mercoxit     2600

 Large Asteroid Cluster
Arkonor     29900
Bistot      57000
Crokite    124000
Dark Ochre  60000
Gneiss     313500
Spodumain  368100
Mercoxit     3500

 Enormous Asteroid Cluster
Arkonor     58000
Bistot      86000
Crokite    169000
Dark Ochre  50000
Gneiss     540000
Spodumain  542000
Mercoxit     5200

 Colossal Asteroid Cluster
Arkonor      60800
Bistot      114300
Crokite     225200
Dark Ochre  115000
Gneiss      630000
Spodumain   736200
Mercoxit      7000

*/