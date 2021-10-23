/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.
    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:     Zhur
    Rewrite:    Allan
    Updates:    James
*/

#include "eve-server.h"

#include "Client.h"
#include "EVEServerConfig.h"
#include "account/AccountService.h"
#include "chat/LSCService.h"
#include "exploration/Probes.h"
#include "npc/Drone.h"
#include "npc/NPC.h"
#include "npc/Sentry.h"
#include "packets/Destiny.h"
#include "planet/Planet.h"
#include "planet/Moon.h"
#include "planet/CustomsOffice.h"
#include "pos/Array.h"
#include "pos/Battery.h"
#include "pos/Module.h"
#include "pos/Structure.h"
#include "pos/Tower.h"
#include "pos/sovStructures/TCU.h"
#include "pos/sovStructures/SBU.h"
#include "pos/sovStructures/IHub.h"
#include "pos/JumpBridge.h"
#include "pos/Weapon.h"
#include "ship/Missile.h"
#include "ship/Ship.h"
#include "station/Station.h"
#include "system/Asteroid.h"
#include "system/Container.h"
#include "system/DestinyManager.h"
#include "system/SolarSystem.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "system/cosmicMgrs/DungeonMgr.h"
#include "system/cosmicMgrs/SpawnMgr.h"

SystemManager::SystemManager(uint32 systemID, PyServiceMgr &svc)
:m_services(svc),
m_bountyTimer(0),
m_minutetimer(0, true),
m_anomMgr(new AnomalyMgr(this, svc)),
m_beltMgr(new BeltMgr(this, svc)),
m_dungMgr(new DungeonMgr(this, svc)),
m_spawnMgr(new SpawnMgr(this, svc)),
m_loaded(false),
m_entityChanged(false),
m_docked(0),
m_players(0),
m_beltCount(0),
m_gateCount(0),
m_activityTime(0),
m_activeRatSpawns(0),
m_activeGateSpawns(0),
m_activeRoidSpawns(0),
m_secValue(1.1f)
{
    m_minutes = 0;

    m_npcs.clear();
    m_clients.clear();
    m_jumpMap.clear();
    m_moonMap.clear();
    m_entities.clear();
    m_planetMap.clear();
    m_gateMap.clear();
    m_ratBubbles.clear();
    m_beltVector.clear();
    m_roidBubbles.clear();
    m_ticEntities.clear();
    m_staticEntities.clear();
    m_opStaticEntities.clear();

    // zero-init our data containers
    m_data = SystemData();
    m_killData = SystemKillData();

    sDataMgr.GetSystemData(systemID, m_data);   // system data is now an internal memory (cached) object.  db is hit once at system boot.
    m_secValue -= m_data.securityRating;  // range is 0.1 for 1.0 system to 2.0 for -0.9 system

    _log(COMMON__MESSAGE, "Created SystemManager %p for System %s(%u)", this, m_data.name.c_str(), m_data.systemID);
}

SystemManager::~SystemManager() {
    if (m_players or !m_clients.empty()) {
        _log(COMMON__ERROR, "D'tor called for System %u with %u players and/or %u clients in mmaps", m_data.systemID, m_players, m_clients.size());
        for (auto cur : m_clients)
            sEntityList.Remove(cur.second);
    }

    if (m_loaded)
        UnloadSystem();

    SafeDelete(m_dungMgr);
    SafeDelete(m_anomMgr);
    SafeDelete(m_beltMgr);
    SafeDelete(m_spawnMgr);
}

bool SystemManager::BootSystem() {
    // dont fuck with this order...

    m_solarSystemRef = sItemFactory.GetSolarSystem(m_data.systemID);
    assert(m_solarSystemRef.get() != nullptr);

    if (!LoadSystemStatics()) {
        _log(SERVICE__ERROR, "Unable to load System Statics during boot of system %u.", m_data.systemID);
        return false;
    }

    if (!LoadCosmicMgrs()) {
        _log(SERVICE__ERROR, "Unable to load Cosmic Managers during boot of system %u.", m_data.systemID);
        return false;
    }

    if (!LoadSystemDynamics()) {
        _log(SERVICE__ERROR, "Unable to load System Dynamics during boot of system %u.", m_data.systemID);
        return false;
    }

    if (!LoadPlayerDynamics()) {
        _log(SERVICE__ERROR, "Unable to load System Dynamics during boot of system %u.", m_data.systemID);
        return false;
    }

    // check for operational static entities which need to be initialized (such as sovereignty structures)
    for (auto cur: m_opStaticEntities)
        if (cur.second ->IsTCUSE())
            cur.second->GetTCUSE()->Init();
        else if (cur.second ->IsSBUSE())
            cur.second->GetSBUSE()->Init();
        else if (cur.second ->IsIHubSE())
            cur.second->GetIHubSE()->Init();

    // system is loaded.  check for items that need initialization
    for (auto cur : m_ticEntities)
        if (cur.second->IsPOSSE())
            cur.second->GetPOSSE()->Init();

    // check planets for colony/customs office
    /* does not work as intended
    for (auto cur : m_planetMap)
        if (cur.second->GetPlanetSE()->HasColony())
            if (!cur.second->GetPlanetSE()->HasCOSE())
                cur.second->GetPlanetSE()->CreateCustomsOffice();
            */

    if (sConfig.server.BountyPayoutDelayed)
        m_bountyTimer.Start(sConfig.server.BountyPayoutTimer * 60 * 1000);

    //create our chat channels
    m_services.lsc_service->CreateSystemChannel(m_data.regionID);
    m_services.lsc_service->CreateSystemChannel(m_data.constellationID);
    m_services.lsc_service->CreateSystemChannel(m_data.systemID);

    // inform MarketBot of loaded system and stations in this system.
    //sMktBotMgr.AddSystem();

    // set system active for system status page
    MapDB::SetSystemActive(m_data.systemID, true);

    // load dynamic map data
    MapDB::LoadDynamicData(m_data.systemID, m_killData);

    //start minute timer
    m_minutetimer.Start(60000);

    return (m_loaded = true);
}

bool SystemManager::LoadCosmicMgrs()
{
    if (!m_spawnMgr->Init()) {
        _log(SERVICE__ERROR, "Unable to load Spawn Manager during boot of system %u.", m_data.systemID);
        return false;
    }

    if (!m_dungMgr->Init(m_anomMgr, m_spawnMgr)) {
        _log(SERVICE__ERROR, "Unable to load Dungeon Manager during boot of system %u.", m_data.systemID);
        return false;
    }

    if (m_beltCount)
        m_beltMgr->Init(m_data.regionID);  //nothing to check for in this init.

    if (!m_anomMgr->Init(m_beltMgr, m_dungMgr, m_spawnMgr)) {
        _log(SERVICE__ERROR, "Unable to load Anomaly Manager during boot of system %u.", m_data.systemID);
        return false;
    }

    return true;
}

//called once per second from EntityList. (1Hz Tic)
bool SystemManager::ProcessTic() {
    double profileStartTime(GetTimeUSeconds());

    /* the idea here is entities map NEVER has invalid items in it, but our iterator may become invalid
     *      when SE->Process() returns because Process() will add/remove from the map as needed
     *      (new objects, destroyed objects, moved objects, etc)
     *  std::map internally orders items by key(itemID here), so use an int var to hold last-processed itemID (mLast).
     *  when iteration starts over, increment until cur > mLast and continue from there to end of list.
     */
    std::map<uint32, SystemEntity*>::iterator itr = m_ticEntities.begin();
    uint32 mLast(0);
    while (itr != m_ticEntities.end()) {
        if (mLast >= itr->first) {
            ++itr;
            continue;
        }

         /* main process call. */
        mLast = itr->first;     // not sure if this will slow this down or not.  check profile
        itr->second->Process();

        if (m_entityChanged) {
            m_entityChanged = false;
            itr = m_ticEntities.begin();
            continue;
        }
        ++itr;
    }

    // tic for sov structures (as they aren't in ticEntities)
    for (auto cur : m_opStaticEntities)
        if (cur.second->IsTCUSE())
            cur.second->GetTCUSE()->Process();
        else if (cur.second ->IsSBUSE())
            cur.second->GetSBUSE()->Process();
        else if (cur.second ->IsIHubSE())
            cur.second->GetIHubSE()->Process();

    // check bounty timer
    if (m_bountyTimer.Check(sConfig.server.BountyPayoutDelayed))
        PayBounties();
    /* the following are coded for single-tic calls */
    m_anomMgr->Process();
    if (m_beltCount)
        m_beltMgr->Process();
    m_dungMgr->Process();
    m_spawnMgr->Process();

    // process planets for PI
    if (m_minutetimer.Check()) {
        //++m_minutes;  // not used at this time
        for (auto cur : m_planetMap)
            cur.second->Process();
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::system, GetTimeUSeconds() - profileStartTime);

    return SystemActivity();
}

bool SystemManager::SystemActivity() {
    if (m_activityTime == 0)
        return true;
    if ((sEntityList.GetStamp() - m_activityTime) > 60)
        return false;

    return true;
}

// called from EntityList::Process() and EntityList::Close()
void SystemManager::UnloadSystem() {
    if (!m_loaded)
        return;

    sLog.Magenta("    SystemManager", "UnloadSystem() called for %s(%u).", m_data.name.c_str(), m_data.systemID);

    // inform MarketBot of system unloading to remove system stations from proc loop.
    //sMktBotMgr.RemoveSystem();

    // system is being unloaded.  pay bounties now
    PayBounties();
    // unload belts, which saves and removes roids from system
    m_beltMgr->ClearAll();
    // close anomaly mgr, which saves and removes sigs from system
    m_anomMgr->Close();

    // remove static and dynamic entities
    std::map<uint32, SystemEntity*>::iterator itr = m_entities.begin();
    SystemEntity* pSE(nullptr);
    while (itr != m_entities.end()) {
        if ((itr->first == 0) or (itr->second == nullptr)) {
            itr = m_entities.erase(itr);
            continue;
        }
        pSE = itr->second;

        if (pSE->TargetMgr() != nullptr)
            pSE->TargetMgr()->Unload();

        if (pSE->IsStaticEntity() or pSE->isGlobal()) {
            if (pSE->IsStationSE()) {
                pSE->GetStationSE()->UnloadStation();
                sEntityList.RemoveStation(itr->first);
            }
            m_staticEntities.erase(m_staticEntities.find(itr->first));
        } else if (pSE->IsShipSE()) {
            pSE->GetShipSE()->GetShipItemRef()->LogOut();
        } else if (pSE->IsNPCSE()) {
            sEntityList.RemoveNPC();    // this is for loaded npc count.
            pSE->GetSelf()->Delete();
        } else if (pSE->IsProbeSE()) {
            sEntityList.RemoveProbe(itr->first);
        }

        if (pSE->IsOperSE()) { //Remove operational statics from list
            m_opStaticEntities.erase(m_opStaticEntities.find(itr->first));
        }

        sItemFactory.RemoveItem(itr->first);
        itr = m_entities.erase(itr);
        sBubbleMgr.Remove(pSE);
        SafeDelete(pSE);
    }

    // save items, then remove from system inventory, item factory and decrement item count
    m_solarSystemRef->GetMyInventory()->Unload();
    _log(PHYSICS__MESSAGE, "SystemManager::UnloadSystem() - map count after unload: %u npcs, %u entities, %u statics.", \
                m_npcs.size(), m_entities.size(), m_staticEntities.size());

    // this is dupe container. contents unloaded in another call
    m_npcs.clear();
    // at this point, system entity list should be clear...but just in case, hit it again
    m_entities.clear();
    // this is dupe container. contents unloaded in another call
    m_ticEntities.clear();
    // at this point, system static entity list should be clear...but just in case, hit it again
    m_staticEntities.clear();
    // clear operational static entity list too
    m_opStaticEntities.clear();

    // this still needs some work... seems ok to me.  26Dec18
    sBubbleMgr.ClearSystemBubbles(m_data.systemID);

    // remove dungeon shit for this system?
    //  are we saving it for later or is it make-on-boot?
    ManagerDB::ClearDungeons(m_data.systemID);

    // set system inactive for system status page
    MapDB::SetSystemActive(m_data.systemID, false);

    /** @todo finish this for lsc */
    m_services.lsc_service->SystemUnload(m_data.systemID, m_data.constellationID, m_data.regionID);

    // remove solar system item from ItemFactory
    sItemFactory.RemoveItem(m_data.systemID);
    m_loaded = false;
}

bool SystemManager::LoadSystemStatics() {
    std::vector<DBSystemEntity> entities;
    entities.clear();
    m_entities.clear();
    m_staticEntities.clear();
    if (!SystemDB::LoadSystemStaticEntities(m_data.systemID, entities)) {
        sLog.Error( "SystemManager::LoadSystemStatics()", "Unable to load celestial entities during boot of %s(%u).", m_data.name.c_str(), m_data.systemID);
        return false;
    }

    SystemEntity* pSE(nullptr);
    for (auto cur : entities) {
        switch (cur.groupID) {
            case EVEDB::invGroups::Station: {
                /** @todo (Allan) outposts are group::station - may need to hack this */
                /*  types 12242 - 22298 in group 15 are outposts */
                /*  types 29323 - 29390 in group 15 are wrecked stations */
                StationItemRef itemRef = sItemFactory.GetStationItem(cur.itemID);
                StationSE *pSSE = new StationSE(itemRef, *(GetServiceMgr()), this);
                sEntityList.AddStation(cur.itemID, itemRef);
                pSE = pSSE;
            } break;
            case EVEDB::invGroups::Asteroid_Belt: {
                CelestialObjectRef itemRef = sItemFactory.GetCelestialObject(cur.itemID);
                BeltSE *pBSE = new BeltSE(itemRef, *(GetServiceMgr()), this);
                pBSE->SetBeltMgr(m_beltMgr);
                ++m_beltCount;
                pSE = pBSE;
            } break;
            case EVEDB::invGroups::Stargate: {
                CelestialObjectRef itemRef = sItemFactory.GetCelestialObject(cur.itemID);
                itemRef->SetAttribute(AttrRadius, cur.radius, false);
                StargateSE *pSSE = new StargateSE(itemRef, *(GetServiceMgr()), this);
                m_gateMap.insert(std::pair<uint32, SystemEntity*>(cur.itemID, pSSE));
                ++m_gateCount;
                pSE = pSSE;
            } break;
            case EVEDB::invGroups::Planet: {
                CelestialObjectRef itemRef = sItemFactory.GetCelestialObject(cur.itemID);
                itemRef->SetAttribute(AttrRadius, cur.radius, false);
                PlanetSE *pPSE = new PlanetSE(itemRef, *(GetServiceMgr()), this);
                m_planetMap.insert(std::pair<uint32, SystemEntity*>(cur.itemID, pPSE));
                pSE = pPSE;
            } break;
            case EVEDB::invGroups::Moon: {
                CelestialObjectRef itemRef = sItemFactory.GetCelestialObject(cur.itemID);
                itemRef->SetAttribute(AttrRadius, cur.radius, false);
                MoonSE *pMSE = new MoonSE(itemRef, *(GetServiceMgr()), this);
                m_moonMap.insert(std::pair<uint32, SystemEntity*>(cur.itemID, pMSE));
                pSE = pMSE;
            } break;
            case EVEDB::invGroups::Sun: {    // suns dont have anything special, so they are generic SSEs
                CelestialObjectRef itemRef = sItemFactory.GetCelestialObject(cur.itemID);
                itemRef->SetAttribute(AttrRadius, cur.radius, false);
                StaticSystemEntity *pSSE = new StaticSystemEntity(itemRef, *(GetServiceMgr()), this);
                pSE = pSSE;
            } break;
            default: {
                sLog.Error( "SystemManager::LoadSystemStatics()", "create static entity called for unhandled item %u (grp: %u, type %u)", cur.itemID, cur.groupID, cur.typeID);
                continue;
            }
        }
        if (pSE == nullptr) {
            sLog.Error( "SystemManager::LoadSystemStatics()", "Failed to create entity for item %u (grp: %u, type %u)", cur.itemID, cur.groupID, cur.typeID);
            continue;
        }
        if (pSE->IsGateSE() or pSE->IsStationSE())
            sBubbleMgr.Add(pSE);
        if (pSE->IsBeltSE()) {
            sBubbleMgr.Add(pSE);
            m_beltVector.push_back(cur.itemID);
        }
        if (!pSE->LoadExtras())
            _log(INV__WARNING, "SystemManager::LoadSystemStatics() - Failed to load additional data for entity %u. Continuing.", cur.itemID);

        m_entities[cur.itemID] = pSE;
        m_staticEntities[cur.itemID] = pSE;
        AddItemToInventory(pSE->GetSelf());
    }

    _log(SERVER__INIT, "SystemManager::LoadSystemStatics() - %u Static System entities loaded for %s (%u)", entities.size(), m_data.name.c_str(), m_data.systemID);
    entities.clear();
    return true;
}

bool SystemManager::LoadSystemDynamics() {
    std::vector<DBSystemDynamicEntity> entities;
    entities.clear();
    if (!SystemDB::LoadSystemDynamicEntities(m_data.systemID, entities)) {
        sLog.Error( "SystemManager::LoadSystemDynamics()", "Unable to load dynamic entities during boot of %s(%u).", m_data.name.c_str(), m_data.systemID);
        return false;
    }

    SystemEntity* pSE(nullptr);
    for (auto cur : entities) {
        pSE = DynamicEntityFactory::BuildEntity(*this, cur);
        if (pSE == nullptr) {
            sLog.Error( "SystemManager::LoadSystemDynamics()", "Failed to create entity for item %u (grp: %u, type %u)",
                                cur.itemID, cur.groupID, cur.typeID);
            continue;
        }
        _log(ITEM__TRACE, "SystemManager::LoadSystemDynamics() - Loaded dynamic entity %u of type %u for %s(%u)", \
                    cur.itemID, cur.typeID, m_data.name.c_str(), m_data.systemID);
        if (pSE->GetPosition().isZero())
            pSE->SetPosition(mGP.GetRandPointOnPlanet(m_data.systemID));
            //pSE->SetPosition(mGP.GetRandPointOnMoon(m_data.systemID));
        AddEntity(pSE);
    }
    _log(SERVER__INIT, "SystemManager::LoadSystemDynamics - %u Dynamic System entities loaded for %s(%u)", entities.size(), m_data.name.c_str(),m_data.systemID);

    return true;
}

bool SystemManager::LoadPlayerDynamics() {
    std::vector<DBSystemDynamicEntity> entities;
    entities.clear();
    if (!SystemDB::LoadPlayerDynamicEntities(m_data.systemID, entities)) {
        sLog.Error( "SystemManager::LoadPlayerDynamics()", "Unable to load player dynamic entities in %s(%u).", m_data.name.c_str(), m_data.systemID);
        return false;
    }

    SystemEntity* pSE(nullptr);
    for (auto cur : entities) {
        pSE = DynamicEntityFactory::BuildEntity(*this, cur);
        if (pSE == nullptr) {
            sLog.Error( "SystemManager::LoadPlayerDynamics()", "Failed to create entity for item %u (grp: %u, type %u)", cur.itemID, cur.groupID, cur.typeID);
            continue;
        }
        _log(ITEM__TRACE, "SystemManager::LoadPlayerDynamics() - Loaded dynamic entity %u of type %u for %s(%u)", \
                    cur.itemID, cur.typeID, m_data.name.c_str(),m_data.systemID);
        if (pSE->GetPosition().isZero())
            pSE->SetPosition(mGP.GetRandPointOnMoon(m_data.systemID));
            //pSE->SetPosition(mGP.GetRandPointOnPlanet(m_data.systemID));
        AddEntity(pSE);
    }
    _log(SERVER__INIT, "SystemManager::LoadPlayerDynamics() - %u Dynamic Player entities loaded for %s(%u)", \
                entities.size(), m_data.name.c_str(),m_data.systemID);

    return true;
}

bool SystemManager::BuildDynamicEntity(const DBSystemDynamicEntity& entity, uint32 launcherID/*0*/) {
    SystemEntity* pSE = DynamicEntityFactory::BuildEntity(*this, entity);
    if (pSE == nullptr) {
        sLog.Error( "SystemManager::BuildDynamicEntity()", "Failed to create entity for item %u (grp: %u, type %u)", entity.itemID, entity.groupID, entity.typeID);
        return false;
    }

    _log(ITEM__TRACE, "SystemManager::BuildDynamicEntity() - Created dynamic entity %u of type %u for %s(%u)", \
                entity.itemID, entity.typeID, m_data.name.c_str(),m_data.systemID );
    AddEntity(pSE);

    // this is only used for wrecks...
    if (launcherID) {
        WreckSE* pWE = pSE->GetWreckSE();
        pWE->SetLaunchedByID(launcherID);
        if (IsCharacterID(entity.ownerID)) {
            Client* pClient = sEntityList.FindClientByCharID(entity.ownerID);
            if (pClient->InFleet())
                pWE->SetFleetID(pClient->GetFleetID());
        }
    }
    return true;
}

SystemEntity* DynamicEntityFactory::BuildEntity(SystemManager& sysMgr, const DBSystemDynamicEntity& entity)
{
    FactionData data = FactionData();
        data.allianceID = entity.allianceID;
        data.corporationID = entity.corporationID;
        data.factionID = entity.factionID;
        data.ownerID = entity.ownerID;

    switch (entity.categoryID) {
        case EVEDB::invCategories::Asteroid: {
            InventoryItemRef asteroid = sItemFactory.GetItem( entity.itemID );
            if (asteroid.get() == nullptr)
                ; /** @todo make error msg here */
            AsteroidSE* aSE = new AsteroidSE(asteroid, *(sysMgr.GetServiceMgr()), &sysMgr);
            _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making AsteroidSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
            return aSE;
        } break;
        case EVEDB::invCategories::Ship: {
            ShipItemRef ship = sItemFactory.GetShip( entity.itemID );
            if (ship.get() == nullptr)
                return nullptr;
            /** @todo make error msg here */
            ShipSE* sSE = new ShipSE(ship, *(sysMgr.GetServiceMgr()), &sysMgr, data);
            _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making ShipSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
            return sSE;
        } break;
        case EVEDB::invCategories::Deployable: {
            InventoryItemRef deployable = sItemFactory.GetItem( entity.itemID );
            if (deployable.get() == nullptr)
                return nullptr;
            /** @todo make error msg here */
            deployable->SetAttribute(AttrRadius, deployable->type().radius());     // Can you set this somehow from the type class ?
            DeployableSE* dSE = new DeployableSE(deployable, *(sysMgr.GetServiceMgr()), &sysMgr, data);
            _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making DeployableSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
            return dSE;
        } break;
        //  these should go into m_staticEntities
        case EVEDB::invCategories::StructureUpgrade: // SOV upgrade structures   these may need their own class one day.
        case EVEDB::invCategories::Structure: {         // POS items
            StructureItemRef structure = sItemFactory.GetStructure( entity.itemID );
            if (structure.get() == nullptr)
                return nullptr;
            /** @todo make error msg here */
            StructureSE* pSSE(nullptr);
            switch(entity.groupID) {
                case EVEDB::invGroups::Control_Tower: {
                    TowerSE* tSE = new TowerSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making TowerSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    pSSE = tSE;
                } break;
                case EVEDB::invGroups::Mobile_Missile_Sentry:
                case EVEDB::invGroups::Mobile_Projectile_Sentry:
                case EVEDB::invGroups::Mobile_Laser_Sentry:
                case EVEDB::invGroups::Mobile_Hybrid_Sentry: {
                    WeaponSE* wSE = new WeaponSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making WeaponSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    pSSE =  wSE;
                } break;
                case EVEDB::invGroups::Electronic_Warfare_Battery:
                case EVEDB::invGroups::Sensor_Dampening_Battery:
                case EVEDB::invGroups::Stasis_Webification_Battery:
                case EVEDB::invGroups::Warp_Scrambling_Battery:
                case EVEDB::invGroups::Energy_Neutralizing_Battery:
                case EVEDB::invGroups::Target_Painting_Battery: {
                    BatterySE* bSE = new BatterySE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making BatterySE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    pSSE = bSE;
                } break;
                case EVEDB::invGroups::Refining_Array:
                case EVEDB::invGroups::Ship_Maintenance_Array:
                case EVEDB::invGroups::Assembly_Array:
                case EVEDB::invGroups::Shield_Hardening_Array:
                case EVEDB::invGroups::Force_Field_Array:
                case EVEDB::invGroups::Corporate_Hangar_Array:
                case EVEDB::invGroups::Stealth_Emitter_Array:
                case EVEDB::invGroups::Scanner_Array:
                case EVEDB::invGroups::Logistics_Array:
                case EVEDB::invGroups::Structure_Repair_Array: {
                    ArraySE* aSE = new ArraySE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making ArraySE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    pSSE = aSE;
                } break;
                case EVEDB::invGroups::Silo:
                case EVEDB::invGroups::Moon_Mining:
                case EVEDB::invGroups::Mobile_Reactor: {
                    ReactorSE* rSE = new ReactorSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making ReactorSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    pSSE = rSE;
                } break;
                case EVEDB::invGroups::Jump_Portal_Array: {
                    JumpBridgeSE* jbSE = new JumpBridgeSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making JumpBridgeSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    pSSE = jbSE;
                } break;
                default: {
                    StructureSE* sSE = new StructureSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making StructureSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    pSSE = sSE;
                } break;
            }
            return pSSE;
        } break;
        case EVEDB::invCategories::SovereigntyStructure: {// SOV structures
            //Create item ref
            StructureItemRef structure = sItemFactory.GetStructure( entity.itemID );
            if (structure.get() == nullptr)
                return nullptr;
            StructureSE* sSSE(nullptr);
            //Test for different types of sov structures
            switch(entity.groupID) {
                case EVEDB::invGroups::Territorial_Claim_Units: {
                    TCUSE* sSE = new TCUSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making TCUSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    sSSE = sSE;
                } break;
                case EVEDB::invGroups::Sovereignty_Blockade_Units: {
                    SBUSE* sSE = new SBUSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making SBUSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    sSSE = sSE;
                } break;
                case EVEDB::invGroups::Infrastructure_Hubs: {
                    IHubSE* sSE = new IHubSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making IHubSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    sSSE = sSE;
                } break;
                default: { //Should never be called, therefore print an error log
                    StructureSE* sSE = new StructureSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                    _log(POS__ERROR, "DynamicEntityFactory::BuildEntity() Default sovereignty StructureSE created for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    sSSE = sSE;
                } break;
            }
            return sSSE;
        } break;
        case EVEDB::invCategories::Orbitals: {           // planet orbitals   these should go into m_staticEntities
            StructureItemRef structure = sItemFactory.GetStructure( entity.itemID );
            if (structure.get() == nullptr)
                return nullptr;
                /** @todo make error msg here */
            CustomsSE* pCoSE(nullptr);
            switch(entity.groupID) {
                case EVEDB::invGroups::Test_Orbitals:
                case EVEDB::invGroups::Orbital_Construction_Platform:
                case EVEDB::invGroups::Orbital_Infrastructure: {
                    pCoSE = new CustomsSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                    //structure->GetMyInventory()->LoadContents();  this is called during structureItem creation
                    if ((entity.planetID) and (entity.groupID != EVEDB::invGroups::Test_Orbitals)) {
                        pCoSE->SetPlanet(entity.planetID);
                        SystemEntity* pPE = sysMgr.GetSE(entity.planetID);
                        if ((pPE != nullptr) and pPE->IsPlanetSE())
                            pPE->GetPlanetSE()->SetCustomsOffice(pCoSE);
                    }
                    pCoSE->Init();
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making CustomsSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                } break;
            }
            return pCoSE;
        } break;
        case EVEDB::invCategories::Celestial: {
            // TODO: (just use CelestialEntity class for these until their own classes are written)
            // * WarpGateEntity  <-- Warp_Gate
            // * WormholeEntity  <-- Wormhole
            switch (entity.groupID) {
                case EVEDB::invGroups::Wreck: {
                    WreckContainerRef wreck = sItemFactory.GetWreckContainer( entity.itemID );
                    if (wreck.get() == nullptr)
                        return nullptr;
                    /** @todo make error msg here */
                    WreckSE* wSE = new WreckSE(wreck, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                    wreck->GetMyInventory()->LoadContents();
                    wreck->SetMySE(wSE);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making WreckSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return wSE;
                } break;
                case EVEDB::invGroups::Audit_Log_Secure_Container:
                case EVEDB::invGroups::Secure_Cargo_Container:
                case EVEDB::invGroups::Cargo_Container:
                case EVEDB::invGroups::Freight_Container:
                case EVEDB::invGroups::Shipping_Crates: {
                    CargoContainerRef contRef = sItemFactory.GetCargoContainer( entity.itemID );
                    if (contRef.get() == nullptr)
                        return nullptr;
                    /** @todo make error msg here */
                    ContainerSE* cSE = new ContainerSE(contRef, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                    contRef->GetMyInventory()->LoadContents();
                    contRef->SetMySE(cSE);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making ContainerSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return cSE;
                } break;
            /** @todo (Allan)  need to separate these by class to create proper SE (started) */
                case EVEDB::invGroups::Biomass:
                case EVEDB::invGroups::Ring:/*wtf*/
                case EVEDB::invGroups::Secondary_Sun:
                case EVEDB::invGroups::Large_Collidable_Object:
                case EVEDB::invGroups::Large_Collidable_Structure:
                case EVEDB::invGroups::Large_Collidable_Ship:  // this will not hit here.  category 11, entity
                case EVEDB::invGroups::Cloud:
                case EVEDB::invGroups::Landmark:
                case EVEDB::invGroups::Comet:
                case EVEDB::invGroups::Destructable_Station_Services:
                /* test these to see if they are POS types */
                case EVEDB::invGroups::Construction_Platform:
                case EVEDB::invGroups::Station_Improvement_Platform:
                case EVEDB::invGroups::Global_Warp_Disruptor:
                case EVEDB::invGroups::Station_Upgrade_Platform:
                case EVEDB::invGroups::Force_Field:  // <<<  this one is POS type but it IS a plain CSE
                /* these will get their own class eventually */
                case EVEDB::invGroups::Effect_Beacon:
                case EVEDB::invGroups::Beacon:
                case EVEDB::invGroups::Covert_Beacon:
                case EVEDB::invGroups::Harvestable_Cloud:
                case EVEDB::invGroups::Planetary_Cloud: {
                    CelestialObjectRef celestial = sItemFactory.GetCelestialObject( entity.itemID );
                    if (celestial.get() == nullptr)
                        return nullptr;
                    /** @todo make error msg here */
                    CelestialSE* cSE = new CelestialSE(celestial, *(sysMgr.GetServiceMgr()), &sysMgr);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making CelestialSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return cSE;
                } break;
                case EVEDB::invGroups::Wormhole: {
                    CelestialObjectRef celestial = sItemFactory.GetCelestialObject( entity.itemID );
                    if (celestial.get() == nullptr)
                        return nullptr;
                    /** @todo make error msg here */
                    WormholeSE* wSE = new WormholeSE(celestial, *(sysMgr.GetServiceMgr()), &sysMgr);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making WormholeSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return wSE;
                } break;
                case EVEDB::invGroups::Cosmic_Anomaly:
                case EVEDB::invGroups::Cosmic_Signature: {
                    CelestialObjectRef celestial = sItemFactory.GetCelestialObject( entity.itemID );
                    if (celestial.get() == nullptr)
                        return nullptr;
                    /** @todo make error msg here */
                    AnomalySE* aSE = new AnomalySE(celestial, *(sysMgr.GetServiceMgr()), &sysMgr);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making AnomalySE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return aSE;
                } break;
                case EVEDB::invGroups::Warp_Gate: { //accel gate
                    // does this need own item, or celestial, or generic or other?
                    InventoryItemRef iRef = sItemFactory.GetItem( entity.itemID );
                    //CelestialObjectRef celestial = sItemFactory.GetCelestialObject( entity.itemID );
                    if (iRef.get() == nullptr)
                        return nullptr;
                    /** @todo make error msg here */
                    ItemSystemEntity* iSE = new ItemSystemEntity(iRef, *(sysMgr.GetServiceMgr()), &sysMgr);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making ISE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return iSE;
                } break;
            } break;
        } break;
        case EVEDB::invCategories::Entity: {            // Entities
            if (entity.groupID == EVEDB::invGroups::Spawn_Container ) {     // these are destructible objects found in dungeons
                // For category=Entity, group=Spawn Container, create a CargoContainer object:
                /** @todo  this needs its own class....there are 477 types, spawning everything..rats, modules, items, etc. */
                CargoContainerRef contRef = sItemFactory.GetCargoContainer( entity.itemID );
                if (contRef.get() == nullptr)
                    return nullptr;
                /** @todo make error msg here */
                ContainerSE* cSE = new ContainerSE(contRef, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                contRef->GetMyInventory()->LoadContents();
                contRef->SetMySE(cSE);
                _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making ContainerSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                return cSE;
            } else if ((entity.groupID == EVEDB::invGroups::Sentry_Gun) or (entity.groupID == EVEDB::invGroups::Protective_Sentry_Gun)
                or (entity.groupID == EVEDB::invGroups::Destructible_Sentry_Gun) or (entity.groupID == EVEDB::invGroups::Mobile_Sentry_Gun))
            {
                InventoryItemRef sentryRef = sItemFactory.GetItem( entity.itemID );
                if (sentryRef.get() == nullptr)
                    return nullptr;
                /** @todo make error msg here */
                Sentry* SentrySE = new Sentry(sentryRef, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making SentrySE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                return SentrySE;
            }
            // Check for NPC ships/drones here (category 11):   NOT player drones (different category [18])
            else if((entity.groupID == EVEDB::invGroups::Police_Drone) or (entity.groupID == EVEDB::invGroups::Pirate_Drone) or (entity.groupID == EVEDB::invGroups::LCO_Drone)
                or (entity.groupID == EVEDB::invGroups::Tutorial_Drone) or (entity.groupID == EVEDB::invGroups::Rogue_Drone) or (entity.groupID == EVEDB::invGroups::Faction_Drone)
                or (entity.groupID == EVEDB::invGroups::Convoy) or (entity.groupID == EVEDB::invGroups::Convoy_Drone) or (entity.groupID == EVEDB::invGroups::Concord_Drone)
                or (entity.groupID == EVEDB::invGroups::Mission_Drone) or (entity.groupID == EVEDB::invGroups::Deadspace_Overseer) or (entity.groupID == EVEDB::invGroups::Customs_Official)
                or (entity.groupID == EVEDB::invGroups::Deadspace_Overseer_s_Structure) or (entity.groupID == EVEDB::invGroups::Deadspace_Overseer_s_Sentry)
                or (entity.groupID == EVEDB::invGroups::Deadspace_Overseer_s_Belongings) or (entity.groupID == EVEDB::invGroups::Storyline_Frigate)
                or (entity.groupID == EVEDB::invGroups::Storyline_Cruiser) or (entity.groupID == EVEDB::invGroups::Storyline_Battleship) or (entity.groupID == EVEDB::invGroups::Storyline_Mission_Frigate)
                or (entity.groupID == EVEDB::invGroups::Storyline_Mission_Cruiser) or (entity.groupID == EVEDB::invGroups::Storyline_Mission_Battleship)
                or ((entity.groupID >= EVEDB::invGroups::Asteroid_Angel_Cartel_Frigate) and (entity.groupID <= EVEDB::invGroups::Asteroid_Serpentis_BattleCruiser))
                or ((entity.groupID >= EVEDB::invGroups::Deadspace_Angel_Cartel_BattleCruiser) and (entity.groupID <= EVEDB::invGroups::Deadspace_Angel_Cartel_Frigate))
                or ((entity.groupID >= EVEDB::invGroups::Deadspace_Blood_Raiders_BattleCruiser) and (entity.groupID <= EVEDB::invGroups::Deadspace_Blood_Raiders_Frigate))
                or ((entity.groupID >= EVEDB::invGroups::Deadspace_Guristas_BattleCruiser) and (entity.groupID <= EVEDB::invGroups::Deadspace_Guristas_Frigate))
                or ((entity.groupID >= EVEDB::invGroups::Deadspace_Sanshas_Nation_BattleCruiser) and (entity.groupID <= EVEDB::invGroups::Deadspace_Sanshas_Nation_Frigate))
                or ((entity.groupID >= EVEDB::invGroups::Deadspace_Serpentis_BattleCruiser) and (entity.groupID <= EVEDB::invGroups::Deadspace_Serpentis_Frigate))
                or ((entity.groupID >= EVEDB::invGroups::Mission_Amarr_Empire_Frigate) and (entity.groupID <= EVEDB::invGroups::Mission_Minmatar_Republic_Battleship))
                or (entity.groupID == EVEDB::invGroups::Destructible_Agents_In_Space)
                or ((entity.groupID >= EVEDB::invGroups::Asteroid_Rogue_Drone_Battlecruiser) and (entity.groupID <= EVEDB::invGroups::Asteroid_Rogue_Drone_Swarm))
                or (entity.groupID == EVEDB::invGroups::Large_Collidable_Ship) //  this is wreck?  abandoned ship?
                or ((entity.groupID >= EVEDB::invGroups::Asteroid_Angel_Cartel_Commander_Frigate) and (entity.groupID <= EVEDB::invGroups::Asteroid_Serpentis_Commander_Frigate))
                or ((entity.groupID >= EVEDB::invGroups::Mission_Generic_Battleships) and (entity.groupID <= EVEDB::invGroups::Mission_Generic_Destroyers))
                or ((entity.groupID >= EVEDB::invGroups::Asteroid_Rogue_Drone_Commander_Battlecruiser) and (entity.groupID <= EVEDB::invGroups::Asteroid_Serpentis_Commander_Battleship))
                or (entity.groupID == EVEDB::invGroups::Mission_Fighter_Drone)
                or ((entity.groupID >= EVEDB::invGroups::Mission_Amarr_Empire_Carrier) and (entity.groupID <= EVEDB::invGroups::Mission_Minmatar_Republic_Carrier))
                or (entity.groupID == EVEDB::invGroups::Mission_Faction_Transports) or (entity.groupID == EVEDB::invGroups::Mission_Faction_Industrials)
                or (entity.groupID == EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Sentinel) or (entity.groupID == EVEDB::invGroups::Deadspace_Sleeper_Awakened_Sentinel)
                or (entity.groupID == EVEDB::invGroups::Deadspace_Sleeper_Emergent_Sentinel)
                or ((entity.groupID >= EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Defender) and (entity.groupID <= EVEDB::invGroups::Deadspace_Sleeper_Emergent_Patroller))
                or (entity.groupID == EVEDB::invGroups::Mission_Faction_Cruiser) or (entity.groupID == EVEDB::invGroups::Mission_Faction_Frigate)
                or (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Industrial) or (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Capital)
                or (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Frigate) or (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Cruiser)
                or (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Battleship))
            {
                InventoryItemRef npcRef = sItemFactory.GetItem( entity.itemID );
                if (npcRef.get() == nullptr)
                    return nullptr;
                /** @todo make error msg here */
                NPC* npcSE = new NPC(npcRef, *(sysMgr.GetServiceMgr()), &sysMgr, data);
                npcSE->Load();
                sEntityList.AddNPC();
                _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making NPCSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                return npcSE;
            }
            // may have to create unique class for Billboard (EVEDB::invGroups::Billboard)
            else {
                InventoryItemRef iRef = sItemFactory.GetItem( entity.itemID );
                if (iRef.get() == nullptr)
                    return nullptr;
                /** @todo make error msg here */
                ItemSystemEntity* cSE = new ItemSystemEntity(iRef, *(sysMgr.GetServiceMgr()), &sysMgr);
                _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making ISE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                return cSE;
            }
        } break;
        case EVEDB::invCategories::Drone: {             // Player Drones
            InventoryItemRef drone = sItemFactory.GetItem( entity.itemID );
            if (drone.get() == nullptr)
                return nullptr;
            /** @todo make error msg here */
            DroneSE* dSE = new DroneSE(drone, *(sysMgr.GetServiceMgr()), &sysMgr, data);
            _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making DroneSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
            return dSE;
        } break;
        case EVEDB::invCategories::Charge: {
            switch (entity.groupID) {
                case EVEDB::invGroups::Scanner_Probe: {
                    ProbeItemRef pRef = sItemFactory.GetProbeItem(entity.itemID);
                    if (pRef.get() == nullptr)
                        return nullptr;
                        /** @todo make error msg here */
                    // make sure these are owned by eve system
                    pRef->ChangeOwner(1);
                    //these probes are abandoned and offline.  give them 5h lifetime
                    ProbeSE* pSE = new ProbeSE(pRef, *(sysMgr.GetServiceMgr()), &sysMgr);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making ProbeSE for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return pSE;
                } break;
                case EVEDB::invGroups::Survey_Probe: {
                    sLog.Warning("BuildEntity", "Called for Survey_Probe");
                } break;
                case EVEDB::invGroups::Warp_Disruption_Probe: {
                    sLog.Warning("BuildEntity", "Called for Warp_Disruption_Probe");
                } break;
            }
        } break;
    }
    sLog.Warning("BuildEntity", "Unhandled dynamic entity category %u for item %u of type %u", entity.categoryID, entity.itemID, entity.typeID);
    EvE::traceStack();
    return nullptr;
}

void SystemManager::AddClient(Client* pClient, bool count/*false*/, bool jump/*false*/) {
    //called from Client::MoveToLocation() on login and when changing systems
    if (pClient == nullptr)
        return;
    if (m_clients.find(pClient->GetCharacterID()) == m_clients.end()) {
        m_clients[pClient->GetCharacterID()] = pClient;
        _log(PLAYER__TRACE, "%s(%u): Added to system manager for %s(%u) - %u clients now in system. count %s", \
                    pClient->GetName(), pClient->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_clients.size(), count?"true":"false");
    } else {
        // error for player already in client map
        _log(PLAYER__ERROR, "%s(%u): Already in player map for %s(%u)", \
            pClient->GetName(), pClient->GetCharacterID(), m_data.name.c_str(), m_data.systemID);
    }

    m_activityTime = 0;

    if (count) {
        ++m_players;
        _log(PLAYER__INFO, "%s(%u): Added to player count for %s(%u) - new count: %u", \
                pClient->GetName(), pClient->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_players);
    }
    if (jump) {
        //add jump in this system
        MapDB::AddJump(m_data.systemID);

        _log(PLAYER__INFO, "%s(%u): Add Jump to %s(%u)", \
        pClient->GetName(), pClient->GetCharacterID(), m_data.name.c_str(), m_data.systemID);

        uint16 stamp = sEntityList.GetStamp();
        std::map<uint32, uint8>::iterator itr = m_jumpMap.find(stamp);
        if (itr != m_jumpMap.end()) {
            ++(itr->second);
        } else {
            m_jumpMap.emplace(stamp, 1);
        }
    }
}

void SystemManager::RemoveClient(Client* pClient, bool count/*false*/, bool jump/*false*/) {
    //called from Client::~Client() and Client::MoveToLocation() when changing systems
    if (pClient == nullptr)
        return;
    m_clients.erase(pClient->GetCharacterID());
    _log(PLAYER__TRACE, "%s(%u): Removed from system manager for %s(%u) - %u clients still in system.", \
            pClient->GetName(), pClient->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_clients.size());

    if (count) {
        --m_players;
        if (m_players < 0) {
            m_players = 0;
            m_clients.clear();  // redundant but safe
            _log(PLAYER__ERROR, "player count for %s(%u) is < 0", m_data.name.c_str(), m_data.systemID);
        }

        _log(PLAYER__INFO, "%s(%u): Removed from player count for %s(%u) - new count: %u", \
                pClient->GetName(), pClient->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_players);
    }
    if (jump) {
        //add jump in this system
        MapDB::AddJump(m_data.systemID);

        _log(PLAYER__INFO, "%s(%u): Add Jump to %s(%u)", \
                pClient->GetName(), pClient->GetCharacterID(), m_data.name.c_str(), m_data.systemID);

        uint16 stamp = sEntityList.GetStamp();
        std::map<uint32, uint8>::iterator itr = m_jumpMap.find(stamp);
        if (itr != m_jumpMap.end()) {
            ++(itr->second);
        } else {
            m_jumpMap.emplace(stamp, 1);
        }
    }
}

void SystemManager::SetDockCount(Client* pClient, bool docked/*false*/)
{
    if (docked) {
        ++m_docked;
    } else {
        --m_docked;
        if (m_docked < 0) {
            m_docked = 0;
            _log(PLAYER__ERROR, "docked count for %s(%u) is <0.  Setting to 0.", m_data.name.c_str(), m_data.systemID);
        }
    }

    if (m_players > sEntityList.GetPlayerCount())
        GetPlayerCount();
    if (m_docked > m_players)
        GetDockedCount();

    MapDB::UpdatePilotCount(m_data.systemID, m_docked, (m_players - m_docked));

    _log(PLAYER__INFO, "%s(%u): %s docked count for %s(%u) - new count: %u", \
            pClient->GetName(), pClient->GetCharacterID(), docked ? "Added to" : "Removed from",  m_data.name.c_str(), m_data.systemID, m_docked);
}

void SystemManager::AddNPC(NPC* pNPC) {
    if ( pNPC == nullptr)
        return;
    uint32 itemID = pNPC->GetID();
    if (m_npcs.find(itemID) != m_npcs.end()) {
        _log(ITEM__WARNING, "%s(%u): Called AddNPC(), but they're already in %s(%u).  Check bubble.", pNPC->GetName(), itemID, m_data.name.c_str(), m_data.systemID);
    } else {
        m_npcs[itemID] = pNPC;
    }

    _log(NPC__TRACE, "%s(%u): Added to system manager for %s(%u)", pNPC->GetName(), pNPC->GetID(), m_data.name.c_str(), m_data.systemID);
    AddEntity(pNPC, false);
    sEntityList.AddNPC();
}

void SystemManager::RemoveNPC(NPC* pNPC) {
    if ( pNPC == nullptr)
        return;
    auto itr = m_npcs.find(pNPC->GetID());
    if (itr != m_npcs.end())
        m_npcs.erase(itr);

    _log(NPC__TRACE, "%s(%u): Removed from system manager for %s(%u)", pNPC->GetName(), pNPC->GetID(), m_data.name.c_str(), m_data.systemID);
    RemoveEntity(pNPC);
    sEntityList.RemoveNPC();    // this is for loaded npc count.
    pNPC->RemoveNPC();   // this deletes NPC from DB.  NPC's dont jump, so no reason to remove from system unless killed
}

void SystemManager::AddEntity(SystemEntity* pSE, bool addSignal/*true*/) {
    if (pSE == nullptr)
        return;
    uint32 itemID(pSE->GetID());
    if (m_entities.find(itemID) != m_entities.end()) {
        _log(ITEM__WARNING, "%s(%u): Called AddEntity(), but they're already in %s(%u).  Check bubble.", pSE->GetName(), itemID, m_data.name.c_str(), m_data.systemID);
        return;
    } else {
        _log(ITEM__TRACE, "%s(%u): Added to system manager for %s(%u)", pSE->GetName(), itemID, m_data.name.c_str(), m_data.systemID);
        m_entities[itemID] = pSE;

        if ((pSE->IsCOSE())
        or  (pSE->isGlobal())) {
            m_staticEntities[itemID] = pSE;
            if (pSE->IsOperSE()) //Entities which need to be acted upon while nobody is in the system
                m_opStaticEntities[itemID] = pSE;
            if (m_loaded)   // only update when system is already loaded
                SendStaticBall(pSE);
        } else if (pSE->IsProbeSE()) {
            // probes are now running sub-hz tics, so dont add to proc list.
            addSignal = false;  // redundant...called with AddSignal=false
            sEntityList.AddProbe(itemID, pSE->GetProbeSE());
        } else if (!IsStaticItem(itemID)) {
            // *most* dynamic items need proc tics.  add to proc list
            m_entityChanged = true;
            m_ticEntities[itemID] = pSE;
        } else {
            addSignal = false;
        }

        // Add Entity's Item Ref to Solar System Dynamic Inventory:
        m_solarSystemRef->AddItemToInventory( pSE->GetSelf() );
    }
    sBubbleMgr.Add(pSE);
    // add item to our AnomalyMgr
    if (addSignal)
        m_anomMgr->AddSignal(pSE);
}

void SystemManager::RemoveEntity(SystemEntity* pSE) {
    /** @note  this does not remove static balls (bubble center markers) and no clue why */
    if (pSE == nullptr)
        return;
    sBubbleMgr.Remove(pSE);
    // Remove Entity's Item Ref from Solar System Dynamic Inventory:
    RemoveItemFromInventory(pSE->GetSelf());
    // remove entity from our maps
    uint32 itemID(pSE->GetID());
    m_entityChanged = true;
    m_ticEntities.erase(itemID);
    m_staticEntities.erase(itemID);

    // remove from anomaly map, if exists
    m_anomMgr->RemoveSignal(itemID);
}

void SystemManager::AddMarker(SystemEntity* pSE, bool sendBall/*false*/, bool addSignal/*false*/) {
    if (pSE == nullptr)
        return;

    m_entities[pSE->GetID()] = pSE;
    // Add Entity's Item Ref to Solar System Dynamic Inventory:
    //m_solarSystemRef->AddItemToInventory(pSE->GetSelf());

    sBubbleMgr.Add(pSE);
    if (addSignal)
        m_anomMgr->AddSignal(pSE);
    if (sendBall) {
        // modified from SendStaticBall()
        if (m_clients.empty())
            return;

        Buffer* destinyBuffer = new Buffer();
        //create AddBalls header
        Destiny::AddBall_header head = Destiny::AddBall_header();
            head.packet_type = 1;   // 0 = full state   1 = balls
            head.stamp = sEntityList.GetStamp();
        destinyBuffer->Append( head );

        AddBalls2 addballs2;
            addballs2.stateStamp = sEntityList.GetStamp();
            addballs2.extraBallData = new PyList();

        PyTuple* balls = new PyTuple(2);
            balls->SetItem(0, pSE->MakeSlimItem());
            balls->SetItem(1, pSE->MakeDamageState());
        addballs2.extraBallData->AddItem(balls);

        pSE->EncodeDestiny(*destinyBuffer);

        addballs2.state = new PyBuffer(&destinyBuffer); //consumed
        SafeDelete( destinyBuffer );

        if (is_log_enabled(DESTINY__BALL_DUMP))
            addballs2.Dump( DESTINY__BALL_DUMP, "    " );
        //send the update
        PyTuple* up = addballs2.Encode();
        for (auto cur : m_clients) {
            PyIncRef(up);
            cur.second->QueueDestinyUpdate(&up, true);
        }
    }
}


void SystemManager::AddBounty(uint32 charID, BountyData& data)
{
    /*
struct BountyData {     // this is comming from rat killed.
    uint32 fromID;
    uint32 toID;
    double amount;
    uint8 refTypeID;
    uint16 fromKey;
    uint16 toKey;
    std::string reason;  this can stay blank for now.  populate during PayBounties
}; */
    _log(CLIENT__TEXT, "AddBounty called for charID %u in system %s(%u).", charID, m_data.name.c_str(), m_data.systemID);
    std::map<uint32, BountyData>::iterator itr = m_bountyMap.find(charID);
    if (itr != m_bountyMap.end()) {
        itr->second.amount += data.amount;
        std::map<uint32, RatDataMap>::iterator rItr = m_ratMap.find(charID);
        if (rItr != m_ratMap.end()) {
            RatDataMap::iterator it = rItr->second.begin();
            if (it == rItr->second.end()){
                RatDataMap vec;
                vec.emplace(data.fromID, 1);
                rItr->second = vec;
            } else {
                ++(it->second);
            }
        } else {
            RatDataMap vec;
            vec.emplace(data.fromID, 1);
            m_ratMap.emplace(charID, vec);
        }
    } else {
        m_bountyMap.emplace(charID, data);
        RatDataMap vec;
        vec.emplace(data.fromID, 1);
        m_ratMap.emplace(charID, vec);
    }
}

void SystemManager::PayBounties()
{
    _log(CLIENT__TEXT, "PayBounties called for system %s(%u).", m_data.name.c_str(), m_data.systemID);
    int8 count = 0;
        /* recDescNpcBountyList = 'NBL'                <-- descrives a full list of [typeID: qty]
         * recDescNpcBountyListTruncated = 'NBLT'      <-- describes a trunicated list
recDescription = 'DESC'
recDescNpcBountyList = 'NBL'
recDescNpcBountyListTruncated = 'NBLT'
recStoreItems = 'STOREITEMS'
         */

    /** @todo  this isnt right...which is why it's not working.
     * see code in client/script/ui/shared/neocom/wallet.py for more info.  (lots to look over. no time tonite.)
     * more data....
     *
     *   ["description" => <92:         << jnlRef:CorpBountyTax?
            - 1630077495                << total bounties paid on this timer
            - 325275.0                  << corp taxes taken?
            NBL:                        << see above
                11030: 2
                11935: 1
                23323: 1
                23332: 2
                23340: 2
            > [WStr]]
     *
     *
     *
     */
    for (auto cur : m_bountyMap) {
        std::string reason = "NBLT: "; //this needs to be populated as [NBL(T): type:amt, type:amt, ... ] to get proper shit in client
        std::map<uint32, RatDataMap>::iterator itr = m_ratMap.find(cur.first);
        if (itr != m_ratMap.end()) {
            count = itr->second.size();
            for (auto cur : itr->second) {
                reason += std::to_string(cur.first);
                reason += ":";
                reason += std::to_string(cur.second);
                if (count > 1)
                    reason += ",";
                --count;
            }
            // will have to figure out how to *correctly* limit this data to count<20 or so...
        } //else {
            reason += ",...";    // this will show as "truncated" in client
        //}
        AccountService::TranserFunds(corpCONCORD, cur.first, cur.second.amount, reason, Journal::EntryType::BountyPrizes, m_data.systemID);
        count = 0;
    }
    m_ratMap.clear();
    m_bountyMap.clear();
}

void SystemManager::DoSpawnForBubble(SystemBubble* pBubble)
{
    if (!m_spawnMgr->IsInitialized())
        return;

    uint8 count = m_beltCount;
    if (count < 1)
        return;

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "Spawn called for bubble %u(%u) in %s(%u)[%.4f], region %u.",
             pBubble->GetID(), sBubbleMgr.GetBeltID(pBubble->GetID()), m_data.name.c_str(), m_data.systemID, m_data.securityRating, m_data.regionID);
    if (count > 15)
        count = 15;
    if ((m_activeRatSpawns < count ) or (pBubble->IsGate())) {
        if (m_spawnMgr->DoSpawnForBubble(pBubble)) {
            m_ratBubbles.emplace(pBubble->GetID(), pBubble);
            if (is_log_enabled(SPAWN__TRACE))
                _log(SPAWN__TRACE, "SystemManager::DoSpawnForBubble() completed for %s(%u) in bubble %u.  %u items in m_ratBubbles", \
                        m_data.name.c_str(), m_data.systemID, pBubble->GetID(), m_ratBubbles.size());
        } else {
            if (is_log_enabled(SPAWN__TRACE))
                _log(SPAWN__TRACE, "SystemManager::DoSpawnForBubble() returned false for bubble %u.", pBubble->GetID());
        }
    }
}

void SystemManager::GetSpawnBubbles(SpawnBubbleMap* bubbleMap)
{
    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SystemManager::GetSpawnBubbles() - called for %s(%u)", m_data.name.c_str(), m_data.systemID);
    SpawnBubbleMap::iterator itr = m_ratBubbles.begin();
    while (itr != m_ratBubbles.end())
        bubbleMap->emplace(itr->first, itr->second);
}

void SystemManager::RemoveSpawnBubble(SystemBubble* pBubble)
{
    if (pBubble->IsBelt()) {
        m_ratBubbles.erase(pBubble->GetID());
        --m_activeRatSpawns;
    } else if (pBubble->IsGate()) {
        m_ratBubbles.erase(pBubble->GetID());
        --m_activeGateSpawns;
    }
    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SystemManager::RemoveSpawnBubble() - called for bubbleID %u in %s(%u).", pBubble->GetID(), m_data.name.c_str(), m_data.systemID);
}

uint32 SystemManager::GetRandBeltID()
{
    return m_beltVector.at(MakeRandomInt(0, m_beltCount));
}

void SystemManager::MakeSetState(const SystemBubble* pBubble,  SetState& into) const {
    using namespace Destiny;
    Buffer* stateBuffer(new Buffer());

    AddBall_header head = AddBall_header();
        head.packet_type = 0;   // 0 = full state   1 = balls
        head.stamp = into.stamp;
    stateBuffer->Append( head );

    std::map<uint32, SystemEntity*> visibleEntities;

    // get all static entities for this system
    for (auto cur : m_staticEntities)
        visibleEntities.emplace(cur.first, cur.second);

    // get all operational static entities and add to visibleEntities map
    for (auto cur : m_opStaticEntities)
        visibleEntities.emplace(cur.first, cur.second);

    // get our ship.  bubble->GetEntities() does not include cloaked items
    std::map<uint32, SystemEntity*>::const_iterator itr = m_ticEntities.find(into.ego);
    if (itr != m_ticEntities.end())
       visibleEntities.emplace(itr->first, itr->second);

    // query bubble to get dynamic entities
    pBubble->GetEntities(visibleEntities);

    into.slims = new PyList();
    into.slims->clear();
    into.effectStates = new PyList();
    into.effectStates->clear();
    into.allianceBridges = new PyList();
    into.allianceBridges->clear();  //activeBeacon and activeBridge data found in fleetSvc.py

    //go through all visible entities and gather the info we need...
    for (auto cur : visibleEntities) {
        if (!cur.second->IsMissileSE() or !cur.second->IsFieldSE())
            into.damageState[ cur.first ] = cur.second->MakeDamageState();

        into.slims->AddItem( new PyObject( "foo.SlimItem", cur.second->MakeSlimItem()));

        //append the destiny binary data...
        cur.second->EncodeDestiny( *stateBuffer );

        // get tower effect state (if applicable)
        if (cur.second->IsTowerSE())
            cur.second->GetTowerSE()->GetEffectState(*(into.effectStates));

        /**  @todo (allan)  this needs more work.  should be done same as damageState.  28.2.16
        //ss.aggressors is for players undocking/jumping with aggression (uses GetCriminalTimeStamps)  ** see notes in Client::GetAggressors()
        if (cur.second->HasPilot() and cur.second->HasAggression())
            ss.aggressors[ cur.first ] = cur.second->GetAggressors());
        */

        /** @todo (allan)  to be written   -jumpbridges is a PyList */
        //  if (cur.second->IsJumpBridgeSE)
        //ss.allianceBridges -- jumpbridges et al.
        //  [for shipID, toSolarsystemID, toBeaconID in bag.allianceBridges:]
    }

    into.destiny_state = new PyBuffer( &stateBuffer );
    into.droneState = pBubble->GetDroneState(); //SystemDB::GetSolDroneState( m_data.systemID );

    /* SolarSystem info.  this avoids the old way of a DB hit for every call.  */
    PyPackedRow* row = new PyPackedRow(sDataMgr.CreateHeader());
        row->SetField("itemID",        new PyLong(m_data.systemID));
        row->SetField("typeID",        new PyInt(5));
        row->SetField("ownerID",       PyStatic.NewOne());  // should this be owning factionID?  yes
        row->SetField("locationID",    new PyInt(m_data.constellationID));
        row->SetField("flagID",        PyStatic.NewZero());
        row->SetField("quantity",      PyStatic.NewNegOne());
        row->SetField("groupID",       new PyInt(5));
        row->SetField("categoryID",    new PyInt(2));
        row->SetField("customInfo",    new PyString(""));
    into.solItem = row;

    if (is_log_enabled(DESTINY__SETSTATE)) {
        _log( DESTINY__SETSTATE, "Current State of %s", m_data.name.c_str() );
        into.Dump( DESTINY__SETSTATE, "    " );
    }

    if (is_log_enabled(DESTINY__SETSTATE_DECODE)) {
        _log( DESTINY__SETSTATE_DECODE, "    Decoded:" );
        Destiny::DumpUpdate( DESTINY__SETSTATE_DECODE, &( into.destiny_state->content() )[0], (uint32)into.destiny_state->content().size() );
    }
}

void SystemManager::SendStaticBall(SystemEntity* pSE)
{
    if (m_clients.empty())
        return;

    if (is_log_enabled(DESTINY__MESSAGE)) {
        if (pSE->SysBubble() != nullptr) { //Don't attempt to log if bubble is null (ie, on static structure initial launch)
            GPoint bCenter(pSE->SysBubble()->GetCenter());
            _log(DESTINY__MESSAGE, "SystemManager::SendStaticBall() - Adding static entity %s(%u) to bubble %u.  Dist to center: %.2f", \
            pSE->GetName(), pSE->GetID(), pSE->SysBubble()->GetID(), bCenter.distance(pSE->GetPosition()));
        }
    }

    Buffer* destinyBuffer = new Buffer();
    //create AddBalls header
    Destiny::AddBall_header head = Destiny::AddBall_header();
        head.packet_type = 1;   // 0 = full state   1 = balls
        head.stamp = sEntityList.GetStamp();
    destinyBuffer->Append( head );

    AddBalls2 addballs2;
    addballs2.stateStamp = sEntityList.GetStamp();
    addballs2.extraBallData = new PyList();

    if (pSE->IsContainerSE()) {
        addballs2.extraBallData->AddItem(pSE->MakeSlimItem());
    } else {
        PyTuple* balls = new PyTuple(2);
        balls->SetItem(0, pSE->MakeSlimItem());
        balls->SetItem(1, pSE->MakeDamageState());
        addballs2.extraBallData->AddItem(balls);
    }

    if (addballs2.extraBallData->size() < 1) {
        SafeDelete( destinyBuffer );
        return;
    }

    pSE->EncodeDestiny(*destinyBuffer);
    addballs2.state = new PyBuffer(&destinyBuffer); //consumed

    if (is_log_enabled(DESTINY__BALL_DUMP))
        addballs2.Dump( DESTINY__BALL_DUMP, "    " );
    _log( DESTINY__BALL_DECODE, "    Ball Decoded:" );
    if (is_log_enabled(DESTINY__BALL_DECODE))
        Destiny::DumpUpdate( DESTINY__BALL_DECODE, &( addballs2.state->content() )[0], (uint32)addballs2.state->content().size() );

    //send the update
    PyTuple* rsp = addballs2.Encode();
    // does this need to be incremented?  the others do...
    for (auto cur : m_clients) {
        PyIncRef(rsp);
        cur.second->QueueDestinyUpdate(&rsp, true);
    }

    //cleanup
    SafeDelete(destinyBuffer);
}

void SystemManager::AddItemToInventory(InventoryItemRef iRef)
{
    m_solarSystemRef->AddItemToInventory( iRef );
}

void SystemManager::RemoveItemFromInventory(InventoryItemRef iRef)
{
    // just in case this is called from elsewhere (which it may be), make sure we remove entity from our map.
    auto itr = m_entities.find(iRef->itemID());
    if (itr != m_entities.end()) {
        _log(ITEM__TRACE, "%s(%u): Removed from system manager for %s(%u)", iRef->name(), iRef->itemID(), m_data.name.c_str(), m_data.systemID);
        m_entities.erase(itr);
    } else {
        _log(ITEM__WARNING, "%s(%u): Called RemoveEntity(), but they weren\'t found in system manager for %s(%u)", \
                iRef->name(), iRef->itemID(), m_data.name.c_str(), m_data.systemID);
    }

    m_solarSystemRef->RemoveItemFromInventory( iRef );
}

SystemEntity* SystemManager::GetSE(uint32 entityID) const {
    std::map<uint32, SystemEntity*>::const_iterator itr = m_entities.find(entityID);
    if (itr == m_entities.end())
        return nullptr;
    return itr->second;
}

NPC* SystemManager::GetNPCSE(uint32 entityID) const
{
    std::map<uint32, NPC*>::const_iterator itr = m_npcs.find(entityID);
    if (itr == m_npcs.end())
        return nullptr;
    return itr->second;
}

ShipItemRef SystemManager::GetShipFromInventory(uint32 shipID)
{
    return ShipItemRef::StaticCast( m_solarSystemRef->GetMyInventory()->GetByID( shipID ) );
}

CargoContainerRef SystemManager::GetContainerFromInventory(uint32 contID)
{
    return CargoContainerRef::StaticCast( m_solarSystemRef->GetMyInventory()->GetByID( contID ) );
}

StationItemRef SystemManager::GetStationFromInventory(uint32 stationID)
{
    return StationItemRef::StaticCast( m_solarSystemRef->GetMyInventory()->GetByID( stationID ) );
}

SystemEntity* SystemManager::GetPlanet(uint32 planetID)
{
    std::map<uint32, SystemEntity*>::iterator itr = m_planetMap.find(planetID);
    if (itr != m_planetMap.end())
        return itr->second;
    return nullptr;
}

uint32 SystemManager::GetClosestPlanetID(const GPoint& myPos)
{
    std::map<double, SystemEntity*> sorted;
    for (auto cur : m_planetMap)
        sorted.insert(std::pair<double, SystemEntity*>(myPos.distance(cur.second->GetPosition()), cur.second));

    std::map<double, SystemEntity*>::iterator itr = sorted.begin();

    return itr->second->GetID();
}

SystemEntity* SystemManager::GetClosestPlanetSE(const GPoint& myPos)
{
    std::map<double, SystemEntity*> sorted;
    for (auto cur : m_planetMap)
        sorted.insert(std::pair<double, SystemEntity*>(myPos.distance(cur.second->GetPosition()), cur.second));

    std::map<double, SystemEntity*>::iterator itr = sorted.begin();

    return itr->second;
}

SystemEntity* SystemManager::GetClosestGateSE(const GPoint& myPos)
{
    std::map<double, SystemEntity*> sorted;
    for (auto cur : m_gateMap)
        sorted.insert(std::pair<double, SystemEntity*>(myPos.distance(cur.second->GetPosition()), cur.second));

    std::map<double, SystemEntity*>::iterator itr = sorted.begin();

    return itr->second;
}

SystemEntity* SystemManager::GetClosestMoonSE(const GPoint& myPos)
{
    std::map<double, SystemEntity*> sorted;
    for (auto cur : m_moonMap)
        sorted.insert(std::pair<double, SystemEntity*>(myPos.distance(cur.second->GetPosition()), cur.second));

    std::map<double, SystemEntity*>::iterator itr = sorted.begin();

    return itr->second;
}

void SystemManager::GetClientList(std::vector< Client* >& cVec)
{
    for (auto cur : m_clients)
        cVec.push_back(cur.second);
}

void SystemManager::DScan(int64 range, const GPoint& pos, std::vector<SystemEntity*>& vector )
{
    /** @todo finish this for correct dscan entity reporting
     * all ships (not cloaked)
     * all celestials
     * all structures
     * wrecks
     * drones
     * probes (core and combat only, i think)
     * spheres and bubbles (ewar shit)
     * some npcs ('normal' like rats dont show)
     * clouds(groups 227 & 711) and asteroids are coded in client but not included here (yet)
     *   group 227 only shown if role_mod or role_gml
     *
     * may not be in this version, but check for "scan inhibitor" POS module; ships in it are invis to dscan
     * AttrDScanImmune is from rhea expansion.  may be able to implement here.
     */
    for (auto cur : m_entities) {
        // these dont show on dscan
        if (IsTempItem(cur.first))
            continue;
        if (IsAsteroidID(cur.first))
            if (!sConfig.server.AsteroidsOnDScan)
                continue;
        if (IsNPC(cur.first))
            continue;
        if (cur.second->IsDeployableSE())       // not sure if this is right or not
            continue;
        if (cur.second->IsShipSE()) {
            if (cur.second->GetGroupID() == EVEDB::invGroups::CovertOps)
                continue;
            if (cur.second->GetGroupID() == EVEDB::invGroups::CombatRecon)
                continue;
        }
        if (cur.second->DestinyMgr() != nullptr)
            if (cur.second->DestinyMgr()->IsCloaked())
                continue;
        // made it this far.  add item to scan list
        if (pos.distance(cur.second->GetPosition()) < range)
            vector.push_back(cur.second);
    }
}

PyRep* SystemManager::GetCurrentEntities()
{
    /*  return list of dict
     * itemID, typeID, catID, name, pos[x,y,z]
     *
     * already have statics, so add players, empty ships, pos', npcs, drones  (anything that requires a tic)
     */

    PyList* list = new PyList();
    for (auto cur : m_ticEntities) {
        PyDict* dict = new PyDict();
            dict->SetItemString("itemID", new PyInt(cur.first));
            dict->SetItemString("ownerName", new PyString(sDataMgr.GetOwnerName(cur.second->GetOwnerID())));
            dict->SetItemString("typeID", new PyInt(cur.second->GetTypeID()));
            dict->SetItemString("catID", new PyInt(cur.second->GetCategoryID()));
            dict->SetItemString("name", new PyString(cur.second->GetName()));
            dict->SetItemString("x", new PyLong(cur.second->x()));
            dict->SetItemString("y", new PyLong(cur.second->y()));
            dict->SetItemString("z", new PyLong(cur.second->z()));
        list->AddItem(dict);
    }
    return list;
}

void SystemManager::GetAllEntities(std::vector< CosmicSignature >& vector)
{
    /** @todo this will need to put entity's sigID into anomaly map for Scan::WarpTo object */
    /** @todo this should be updated/current/correct in system's AnomalyMgr.  try to get data from there for this list  */
    for (auto cur : m_ticEntities) {
        CosmicSignature sig = CosmicSignature();
        sig.dungeonType = Dungeon::Type::Anomaly;
        sig.ownerID = cur.second->GetOwnerID();
        sig.sigID = sEntityList.GetAnomalyID();         // result.id
        sig.sigItemID = cur.first;
        sig.sigStrength = 0.9f; // these arent warpable yet
        sig.systemID = m_data.systemID;
        sig.position = cur.second->GetPosition();
        sig.sigGroupID = cur.second->GetGroupID();      // result.groupID
        sig.sigTypeID = cur.second->GetTypeID();        // result.typeID
        // if scanGroupID is anom or sig, use scanAttributeID to determine site type (in client code)
        // scanGroupID must be one of the 5 groups coded in client (sig, anom, ship, drone, structure)
        // scanGroupID of sig and anom are cached on client side
        switch (cur.second->GetCategoryID()) {
            case EVEDB::invCategories::Drone:
            case EVEDB::invCategories::Charge: { // probes, missiles (at time of scan), and ??
                sig.scanAttributeID = AttrScanStrengthDronesProbes;   // result.strengthAttributeID
                sig.scanGroupID = Scanning::Group::DroneOrProbe;
            } break;
            case EVEDB::invCategories::Orbitals:
            case EVEDB::invCategories::Structure:
            case EVEDB::invCategories::StructureUpgrade:
            case EVEDB::invCategories::SovereigntyStructure: {
                sig.scanAttributeID = AttrScanStrengthStructures;   // result.strengthAttributeID
                sig.scanGroupID = Scanning::Group::Structure;
            } break;
            case EVEDB::invCategories::Ship: {
                sig.scanAttributeID = AttrScanStrengthShips;   // result.strengthAttributeID
                sig.scanGroupID = Scanning::Group::Ship;
            } break;
            case EVEDB::invCategories::Entity: {
                sig.scanAttributeID = AttrScanStrengthSignatures;       // result.strengthAttributeID
                sig.scanGroupID = Scanning::Group::Signature;    // Scrap(1) is for filter only
                sig.sigName = cur.second->GetName(); // result.DungeonName  -  only used when scanGroupID is sig or anom
            } break;
            case EVEDB::invCategories::Asteroid:
            case EVEDB::invCategories::Celestial:
            case EVEDB::invCategories::Deployable: // mobile warp disruptor
            default: {
                sig.scanAttributeID = AttrScanAllStrength;     // result.strengthAttributeID (Unknown)
                sig.scanGroupID = Scanning::Group::Anomaly; // Celestial(64) is only for filter
                sig.sigName = cur.second->GetName(); // result.DungeonName  -  only used when scanGroupID is sig or anom
            } break;
        }
        vector.push_back(sig);
    }
}


//  time related methods to manipulate hour/24hour map data
void SystemManager::UpdateData()
{
    MapDB::UpdatePilotCount(m_data.systemID, m_docked, (m_players - m_docked));

    uint16 jumps = 0;
    uint16 stamp = sEntityList.GetStamp() -60;
    std::map<uint32, uint8>::iterator itr = m_jumpMap.begin();
    while (itr != m_jumpMap.end()) {
        if (itr->first < stamp) {
            itr = m_jumpMap.erase(itr);
            continue;
        } else {
            jumps += itr->second;
            ++itr;
        }
    }

    // this is jumps/hour data
    //MapDB::UpdateJumps(m_data.systemID, jumps);

    // if system and jumpmap are both empty, set activity time for unload timer
    if (SafeToUnload())
        if (m_activityTime == 0)
            if (m_clients.empty())
                if (m_jumpMap.empty())
                    m_activityTime = sEntityList.GetStamp() -50;
    ManipulateTimeData();
}

// checks for if it is safe to mark the system for unloading
bool SystemManager::SafeToUnload()
{
    for (auto cur: GetOperationalStatics()) {
        //If there are any ongoing operations by operational static structures, we don't want to unload the system until this is complete
        if (cur.second->IsPOSSE()) {
            if ((cur.second->GetPOSSE()->GetProcState() == EVEPOS::ProcState::Unanchoring) or
            (cur.second->GetPOSSE()->GetProcState() == EVEPOS::ProcState::Anchoring) or
            (cur.second->GetPOSSE()->GetProcState() == EVEPOS::ProcState::Offlining) or
            (cur.second->GetPOSSE()->GetProcState() == EVEPOS::ProcState::Onlining)) {
                return false;
            }
        }
    }
    return true; //by default, its always safe to unload
}

// not sure how to do this one yet...
void SystemManager::ManipulateTimeData()
{
    int64 timeNow = GetFileTimeNow();
    timeNow += EvE::Time::Hour;

    //if (m_killData.killsDateTime < timeNow)

    MapDB::UpdateKillData(m_data.systemID, m_killData);
}

void SystemManager::GetDockedCount()
{
    m_docked = 0;
    for (auto cur : m_clients)
        if (cur.second->IsDocked())
            ++m_docked;
}

void SystemManager::GetPlayerCount()
{
    // not used yet
}

/*
    uint16 killsHour;
    uint16 kills24Hour;
    uint16 factionKills;
    uint16 factionKills24Hour;
    uint16 podKillsHour;
    uint16 podKills24Hour;

    int64 killsDateTime;
    int64 kills24DateTime;
    int64 factionDateTime;
    int64 faction24DateTime;
    int64 podDateTime;
    int64 pod24DateTime;
    */


bool SystemManager::IsNull(std::map<uint32, SystemEntity*>::iterator& i)
{
    /* you have to change this parameter to the type of the container
     * you are using and the type of the element inside the container.
     *   if this finds use again, it should be changed to a template.
     */

    uint8 buffer[sizeof(i)];
    memset(buffer, 0, sizeof(i));
    memcpy(buffer, &i, sizeof(i));
    return *buffer == 0;
    /* I found that the size of any iterator is 12 bytes long.
     * I also found that if the first byte of the iterator that
     * is copy to the buffer is zero, then the iterator is invalid.
     * Otherwise it is valid. I like to call invalid iterators also as "null iterators".
     */
}
