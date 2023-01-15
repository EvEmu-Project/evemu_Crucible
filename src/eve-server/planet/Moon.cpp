
/**
 * @name Moon.cpp
 *   Specific Class for individual moons.
 * this class will hold all moon data and relative info for each moon.
 *
 * @Author:         Allan
 * @date:   30 April 2016
 */

#include "Moon.h"
#include "pos/Structure.h"
#include "StaticDataMgr.h"
#include "system/Celestial.h"
#include "system/SystemManager.h"

/** @note  general design notes
 * moonse will have a Moon class to hold data and call other functions/methods as needed
 * the PlanetMgr class will manage all aspects of moon data, init'd as a single instance (no reason for multiples)
 *
 *
 */

/*  moon probes, cause i dont where else to put them offhand...
 *   Probe     Travel_Time  Skill_Needed
 *   Quest     40 minutes   Survey III, Astrometrics III
 *   Discovery 10 minutes   Survey III, Astrometrics III
 *   Gaze      5 Minutes    Survey V, Astrometrics V
 *

    About 50% of moons will yield nothing.
    About 39% of moons will yield only gases.
    The rest will yield some combination of gasses and/or minerals.

    Normal skills to reduce probing time do not apply to survey probes

    The Covert Ops 10% time reduction skill per level DOES apply to survey probes when fired from a covops. If you are scanning a large number of moons, it may be better time-wise to use a covops. A covops is also highly recommended when scanning in hostile systems.


   What you need to Probe Moons

    Skills
    Survey III
    Astrometrics III
    Expanded Probe Launcher (NOT a Core Probe Launcher)
    Survey Probes
 */

Moon::Moon()
{

}


MoonSE::MoonSE(InventoryItemRef self, EVEServiceManager &services, SystemManager* system)
: StaticSystemEntity(self, services, system),
m_towerSE(nullptr)
{
    /** @todo finish moon resources...this is hacked */
    m_resources.clear();
/*
          [PyTuple 2 items]
            [PyInt 40126701]
            [PyList 20 items]
              [PyTuple 2 items]
                [PyInt 16633]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16634]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16635]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16636]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16637]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16638]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16639]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16640]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16641]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16642]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16643]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16644]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16646]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16647]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16648]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16649]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16650]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16651]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16652]
                [PyInt 4]
              [PyTuple 2 items]
                [PyInt 16653]
                [PyInt 4]
                */
}

bool MoonSE::LoadExtras() {
    if (!StaticSystemEntity::LoadExtras())
        return false;

    /** @todo use this to initialize moongoo data, create planet manager for moon, or whatever else
     * i decide is needed for moon management
     *  this is called after SE is created.
     */

    /*  moon goo data (notes for me)
     * goo rarity 4/8/16/32/64
     *
     *

Moon materials have different rarity classes, starting with R4 being the most common and R64 being the most rare.

    R4 (Gases) : Atmospheric Gases, Evaporite, Hydrocarbons, Silicates.
    R8 (Metals): Cobalt, Scandium, Titanium, Tungsten
    R16 (Metals): Cadmium, Chromium, Platinum, Vanadium
    R32 (Metals): Caesium, Hafnium, Mercury, Technetium
    R64 (Metals): Dysprosium, Neodymium, Promethium, Thulium

    Intermediate Materials include:

    Caesarium Cadmide
    Carbon Polymers
    Ceramic Powder
    Crystallite Alloy
    Dysporite
    Fernite Alloy
    Ferrofluid
    Fluxed Condensates
    Hexite
    Hyperflurite
    Neo Mercurite
    Platinum Technite
    Prometium
    Rolled Tungsten Alloy
    Silicon Diborite
    Solerium
    Sulfuric Acid
    Titanium Chromide
    Vanadium Hafnite
     */
    std::map<uint16, uint8> data;
    sDataMgr.GetMoonResouces(data);

    for (auto cur : data)
        switch (cur.second) {
            case 1: {
                m_resources.insert(std::pair<uint16, uint8>(cur.first, 6));
            } break;
            case 4: {
                m_resources.insert(std::pair<uint16, uint8>(cur.first, 4));
            } break;
            case 6:{
                m_resources.insert(std::pair<uint16, uint8>(cur.first, 3));
            } break;
            case 8:{
                m_resources.insert(std::pair<uint16, uint8>(cur.first, 2));
            } break;
            case 10:{
                m_resources.insert(std::pair<uint16, uint8>(cur.first, 1));
            } break;
        }

    return true;
}
