
 /**
  * @name Planet.h
  *   Specific Class for individual planets.
  * this class will hold all planet data and relative info for each planet.
  *
  * @Author:         Allan
  * @date:   30 April 2016
  */


#ifndef EVEMU_PLANET_PLANET_H_
#define EVEMU_PLANET_PLANET_H_

#include "EntityList.h"
#include "StaticDataMgr.h"
#include "system/SystemEntity.h"

/** @todo update this to create a planet item instead of the default celestial item */
class Planet
{
public:
    Planet();
    ~Planet()                                           { /* do nothing here */ }

protected:
private:

};


class Colony;
class PyServiceMgr;
class SystemManager;
class CustomsSE;
class Call_ResourceDataDict;

class PlanetSE
: public StaticSystemEntity
{
public:
    PlanetSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~PlanetSE();

    /* Process Calls - Overridden as needed in derived classes */
    virtual void                Process();

    /* class type pointer querys. */
    virtual PlanetSE*           GetPlanetSE()           { return this; }
    /* class type tests. */
    virtual bool                IsPlanetSE()            { return true; }

    /* virtual functions default to base class and overridden as needed */
    virtual bool                LoadExtras();

    /* specific functions for this class */
    PyRep*                      GetPlanetInfo(Colony* pColony);
    PyRep*                      GetResourceData(Call_ResourceDataDict& dict);
    PyRep*                      GetPlanetResourceInfo();
    PyRep*                      GetExtractorsForPlanet(int32 planetID);

    void                        AbandonColony(Colony* pColony);
    Colony*                     GetColony(Client* pClient);

    bool                        HasCOSE()               { return pCO != nullptr; }
    void                        CreateCustomsOffice();
    void                SetCustomsOffice(CustomsSE* pSE){ pCO = pSE; }
    CustomsSE*                  GetCustomsOffice()      { return pCO; }

    bool                        HasColony()             { return !m_colonies.empty(); }

    StructureSE* GetMyTCU()                             { return m_tcuSE; }
    bool HasTCU()                                     { return (m_tcuSE != nullptr); }
    void SetTCU(StructureSE* pSE)                     { m_tcuSE = pSE; }

protected:
    CustomsSE*                  pCO;  // our Customs Office SE  - we dont own this
    PlanetResourceData          m_data;

private:
    std::map<uint16, std::string> m_typeBuffers;

    /* map of charID, Colony* for this planet.
     *   this is a hack, as the client will not reuse planet bound objects, instead calling for a new object on every call.
     *  this scheme will prevent data races on colony calls
     *  Colony* is owned by its planetSE
     */
    std::map<uint32, Colony*>   m_colonies;

    StructureSE* m_tcuSE;
};

#endif  // EVEMU_PLANET_PLANET_H_


/*
        minBand, maxBand = const.planetResourceProximityLimits[info.proximity]
        info.newBand = min(maxBand, minBand + info.planetology + info.advancedPlanetology * 2)
        requiredSkill = 5 - info.proximity
        if info.remoteSensing < requiredSkill:
            info.requiredSkill = requiredSkill
        for i, scanRange in enumerate(const.planetResourceScanningRanges):
            if scanRange >= dist:
                info.proximity = i

planetResourceScanDistance = 1000000000

planetResourceProximityDistant = 0
planetResourceProximityRegion = 1
planetResourceProximityConstellation = 2
planetResourceProximitySystem = 3
planetResourceProximityPlanet = 4

planetResourceProximityLimits =
[(2, 6),
 (4, 10),
 (6, 15),
 (10, 20),
 (15, 30)]
planetResourceScanningRanges =
[9.0,
 7.0,
 5.0,
 3.0,
 1.0]
planetResourceUpdateTime = 1 * HOUR

planetResourceMaxValue = 1.21
MAX_DISPLAY_QUALTY = const.planetResourceMaxValue * 255 * 0.5
qualityRemapped = quality / MAX_DISPLAY_QUALTY
self.resourceList.AddItem(typeID, quality=max(0, min(1.0, qualityRemapped)))

*/