
 /**
  * @name Moon.h
  *   Specific Class for individual moons.
  * this class will hold all moon data and relative info for each moon.
  *
  * @Author:         Allan
  * @date:   30 April 2016
  */


#ifndef EVEMU_PLANET_MOON_H_
#define EVEMU_PLANET_MOON_H_

#include "system/SystemEntity.h"

class PyServiceMgr;
class SystemManager;

class Moon
{
public:
    Moon();
    ~Moon()                                             { /* do nothing here */ }

};

class MoonSE
: public StaticSystemEntity
{
public:
    MoonSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~MoonSE()                                   { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual MoonSE* GetMoonSE()                         { return this; }
    /* class type tests. */
    virtual bool IsMoonSE()                             { return true; }

    /* virtual functions default to base class and overridden as needed */
    virtual bool LoadExtras();


    /* specific functions handled in this class. */
    StructureSE* GetMyTower()                           { return m_towerSE; }
    bool HasTower()                                     { return (m_towerSE != nullptr); }
    void SetTower(StructureSE* pSE)                     { m_towerSE = pSE; }

    std::map<uint16, uint8>::iterator GooEnd()          { return m_resources.end(); }
    std::map<uint16, uint8>::iterator GooBegin()        { return m_resources.begin(); }


private:
    StructureSE* m_towerSE;

    std::map<uint16, uint8> m_resources;     // moon goo (typeID, amt)

};



#endif  // EVEMU_PLANET_MOON_H_