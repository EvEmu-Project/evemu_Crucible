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
    Author:        Zhur
    Rewrite:    Allan
*/

#ifndef __SYSTEMMANAGER_H_INCL__
#define __SYSTEMMANAGER_H_INCL__

#include "system/BubbleManager.h"
#include "system/SolarSystem.h"
#include "system/SystemDB.h"


class PyRep;
class PyDict;
class PyTuple;
class PyList;
class Client;
class NPC;
class InventoryItem;
class SystemEntity;
class SystemBubble;
class SetState;
class DestinyManager;

class AnomalyMgr;
class BeltMgr;
class DungeonMgr;
class SpawnMgr;
class PyServiceMgr;

class DynamicEntityFactory {
public:
    // you MUST call (your SystemManager)->AddEntity([this returned object]) after this to actually put the entity in space
    static SystemEntity* BuildEntity(SystemManager& pSysMgr, const DBSystemDynamicEntity& entity);
};

class SystemManager
{
public:
    SystemManager(uint32 systemID, PyServiceMgr &svc);//, ItemData idata);
    ~SystemManager();

    bool ProcessTic();          // called at 1Hz.
    bool BootSystem();
    void UnloadSystem();
    void UpdateData();          // called from EntityList every 5m for active systems

    bool IsLoaded()                                     { return m_loaded; }

    SystemEntity* GetSE(uint32 entityID) const;
    NPC* GetNPCSE(uint32 entityID) const;
    ShipItemRef GetShipFromInventory(uint32 shipID);
    StationItemRef GetStationFromInventory(uint32 stationID);
    CargoContainerRef GetContainerFromInventory(uint32 contID);


    uint32 GetID() const                                { return m_data.systemID; }
    uint32 GetRegionID()                                { return m_data.regionID; }
    uint32 GetConstellationID()                         { return m_data.regionID; }
    const std::string& GetNameStr() const               { return m_data.name; }
    const char* GetName() const                         { return m_data.name.c_str(); }
    const char* GetSystemSecurityClass()                { return m_data.securityClass.c_str(); }
    const float GetSystemSecurityRating()               { return m_data.securityRating; }

    PyServiceMgr* GetServiceMgr()                       { return &m_services; }
    Inventory* GetSystemInv()                           { return m_solarSystemRef->GetMyInventory(); }
    SolarSystemRef GetSystemRef()                       { return m_solarSystemRef; }

    // for spawn system     -allan 15July15     (not complete)
    typedef std::map<uint32, SystemBubble*> SpawnBubbleMap;
    void RemoveSpawnBubble(SystemBubble* pBubble);
    void GetSpawnBubbles(SpawnBubbleMap* bubbleMap);
    void IncRatSpawnCount()                             { ++m_activeRatSpawns; }
    void DecRatSpawnCount()                             { --m_activeRatSpawns; }
    void IncGateSpawnCount()                            { ++m_activeGateSpawns; }
    void DecGateSpawnCount()                            { --m_activeGateSpawns; }
    void IncRoidSpawnCount()                            { ++m_activeRoidSpawns; }
    void DecRoidSpawnCount()                            { --m_activeRoidSpawns; }
    uint8 BeltCount()                                   { return m_beltCount; }
    uint8 GetRatSpawnCount()                            { return m_activeRatSpawns; }
    uint16 GetRoidSpawnCount()                          { return m_activeRoidSpawns; }
    uint32 PlayerCount()                                { return m_players; }
    uint32 GetSysNPCCount()                             { return m_npcs.size(); }

    // CosmicMgr interface
    BeltMgr* GetBeltMgr()                               { return m_beltMgr; }
    SpawnMgr* GetSpawnMgr()                             { return m_spawnMgr; }
    AnomalyMgr* GetAnomMgr()                            { return m_anomMgr; }
    DungeonMgr* GetDungMgr()                            { return m_dungMgr; }

    // range is 0.1 for 1.0 system to 2.0 for -0.9 system
    float GetSecValue()                                 { return m_secValue; }

    bool BuildDynamicEntity(const DBSystemDynamicEntity& entity, uint32 launcherID=0);

    void AddNPC(NPC* pNPC);
    void RemoveNPC(NPC* pNPC);
    void AddEntity(SystemEntity* pSE, bool addSignal=true);    // add entity to system, and (optionally) add signal to AnomalyMgr
    void RemoveEntity(SystemEntity* pSE);   // this also removes SE* from bubble and sig from AnomalyMgr (if applicable)
    void AddClient(Client* pClient, bool count=false, bool jump=false);
    void AddMarker(SystemEntity* pSE, bool sendBall=false, bool addSignal=false);    // rather specific here.
    void RemoveClient(Client* pClient, bool count=false, bool jump=false);
    void SetDockCount(Client* pClient, bool docked=false);

    void AddItemToInventory(InventoryItemRef item);
    void RemoveItemFromInventory(InventoryItemRef item);
    void DoSpawnForBubble(SystemBubble* pBubble);

    /* this is to send new global ball added to system AFTER SetState has been sent */
    void SendStaticBall(SystemEntity* pSE);
    void MakeSetState(const SystemBubble* pBubble, SetState& into) const;

    uint32 GetRandBeltID();
    uint32 GetClosestPlanetID(const GPoint& myPos);

    // system bounty timer system.  20m delay
    void AddBounty(uint32 charID, BountyData& data);

    SystemEntity* GetClosestMoonSE(const GPoint& myPos);
    SystemEntity* GetClosestPlanetSE(const GPoint& myPos);
    SystemEntity* GetClosestGateSE(const GPoint& myPos);

    // this returns entities in range for display on dscan.
    void DScan(int64 range, const GPoint& pos, std::vector< SystemEntity* >& vector);
    // this returns entities in system for display on Groove's Entity Map in client
    PyRep* GetCurrentEntities();
    // this returns entities in system for display on ship scanner when enabled.
    void GetAllEntities(std::vector<CosmicSignature>& vector);

    std::map<uint32, SystemEntity*> GetOperationalStatics() { return m_opStaticEntities; }
    std::map<uint32, SystemEntity*> GetGates() { return m_gateMap; }

    SystemEntity* GetEntityByID(uint32 itemID) { return m_entities.find(itemID)->second; }

    void GetClientList(std::vector<Client*>& cVec);

    std::map< uint32, SystemEntity* > GetEntities()     { return m_entities; }

    SystemEntity* GetPlanet(uint32 planetID);


protected:
    /** @todo  this needs more work */
    void PayBounties();

    /* hack to avoid errors with dock count */
    void GetDockedCount();
    void GetPlayerCount();

    bool LoadCosmicMgrs();
    bool LoadSystemStatics();
    bool LoadSystemDynamics();
    bool LoadPlayerDynamics();

private:
    AnomalyMgr* m_anomMgr;      //we own this, never NULL.
    BeltMgr* m_beltMgr;         //we own this, never NULL.
    DungeonMgr* m_dungMgr;      //we own this, never NULL.
    SpawnMgr* m_spawnMgr;       //we own this, never NULL.

    PyServiceMgr& m_services;
    SolarSystemRef m_solarSystemRef;

    // static system data
    SystemData m_data;

    float m_secValue;  // range is 0.1 for 1.0 system to 2.0 for -0.9 system

    // for dynamic data system  -allan 10June2019
    SystemKillData m_killData;
    uint16 m_docked;
    void ManipulateTimeData();
    std::map<uint32, uint8> m_jumpMap;  // timestamp/jumps
    // may have to do kill data like jumps above

    // for spawn systems     -allan 15July15
    uint8 m_beltCount;
    uint8 m_gateCount;
    uint8 m_activeRatSpawns;
    uint8 m_activeGateSpawns;
    uint16 m_activeRoidSpawns;
    std::vector<uint32> m_beltVector;
    SpawnBubbleMap m_ratBubbles;  // map of id/bubble with rat spawns  - not actually used yet
    SpawnBubbleMap m_roidBubbles;  // map of id/bubble with roid spawns  - not actually used yet

    // for POS system       -allan 23July17
    std::map<uint32, SystemEntity*> m_moonMap;        // our container, but we DONT own the SE*
    std::map<uint32, SystemEntity*> m_planetMap;      // our container, but we DONT own the SE*
    std::map<uint32, SystemEntity*> m_gateMap;      // our container, but we DONT own the SE*

    // for grid Unloading system  -allan  27June2015
    bool m_loaded;
    bool SystemActivity();
    bool SafeToUnload();
    uint16 m_players;           // current total count
    uint32 m_activityTime;

    // system entity lists:
    bool m_entityChanged :1;
    std::map<uint32, NPC*> m_npcs;
    std::map<uint32, Client*> m_clients;
    std::map<uint32, SystemEntity*> m_entities;         // this list is all entities in this system.  we own these.
    std::map<uint32, SystemEntity*> m_ticEntities;      // this list is for entities that need process tics (objects, npc, client ships)
    std::map<uint32, SystemEntity*> m_staticEntities;   // this list is for static entities to send in setstate
    std::map<uint32, SystemEntity*> m_opStaticEntities; // this list is for static entities which are operational and need to be initialized and operated upon even when system is empty

    // for bounty processing (20m timer)
    Timer m_bountyTimer;
    typedef std::map<uint16, uint8> RatDataMap;  // typeID/amt
    std::map<uint32, BountyData> m_bountyMap;  // charID/data
    std::map<uint32, RatDataMap> m_ratMap;  // charID/rat data

    // for PI timing
    Timer m_minutetimer;
    uint32 m_minutes;

    // check for null iterator.  this will need to be moved to a memory code file eventually.
    // unfortunately, this is very specific for which iterators it can check.  see notes in code.
    bool IsNull(std::map<uint32, SystemEntity*>::iterator& i);
};

#endif