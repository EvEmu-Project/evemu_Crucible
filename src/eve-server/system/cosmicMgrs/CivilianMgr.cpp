
/**
 * @name CivilianMgr.cpp
 *     Civilian (non-combatant NPC) management system for EVEmu
 *
 * @Author:        Allan
 * @date:          12 Feb 2017
 *
 */


#include "eve-server.h"

#include "EVEServerConfig.h"

#include "system/cosmicMgrs/CivilianMgr.h"

/*  this class will be in charge of all non-combatant npcs ingame, hereinafter refered to as NC.
 * the purpose here is to simulate civilian actions by having ships travel from station to station.
 * this will also include jumping systems
 *
 *  mostly, it's simple AI to run an NC from point A to point B to simulate normal system traffic.
 *  Process() will check for player activity before creating/moving NCs.
 * NCs jumping into empty system will be deleted. (no reason to simulate traffic in empty system)
 * any NCs in system are 'paused' if last player jumps out.  processing will continue if a player enters system.
 * system creation will create NCs, configure their routing and begin processing
 * system unloading will delete any NCs in that system and remove their routing
 *
 *  as the system matures, it may be possible for 'shadier' npcs to travel to unknown areas (to do their deeds in secret locations)
 *    (the astute capsuleer will notice the ship NOT traveling to a planet, gate, or station, and will then know the general area to scan)
 *
 *  this will be a singleton class, in order for ships to correctly span multiple systems.
 */


CivilianMgr::CivilianMgr()
{
    m_initalized = false;
}

void CivilianMgr::Initialize() {
    m_initalized = true;
    sLog.Blue(" Civilian Manager", "Civilian Manager Initialized.");
    /* load current data, start timers, process current data, and create new items, if needed */
}

void CivilianMgr::Process() {
    if (!m_initalized) return;

    /* do something useful here */
}

