/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2021 The EVEmu Team
 *    For the latest information visit https://evemu.dev
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:        Zhur
 *    Updates:        Allan
 */

#include <algorithm>

#include "eve-server.h"
#include "EVEServerConfig.h"

#include "Client.h"
#include "EntityList.h"
#include "npc/Drone.h"
#include "npc/NPC.h"
#include "system/BubbleManager.h"
#include "system/Container.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/BeltMgr.h"


SystemBubble::SystemBubble(SystemManager* pSystem, const GPoint& center, double radius)
: m_system(pSystem),
m_center(center),
m_radius(radius),
m_tcuSE(nullptr),
m_sbuSE(nullptr),
m_ihubSE(nullptr),
m_towerSE(nullptr),
m_centerSE(nullptr),
m_spawnTimer(0)
{
    m_ice = false;
    m_belt = false;
    m_gate = false;
    m_anomaly = false;
    m_mission = false;
    m_spawned = false;
    m_incursion = false;
    m_hasBubble = false;
    m_hasMarkers = false;

    m_markers.clear();
    m_players.clear();
    m_entities.clear();
    m_dynamicEntities.clear();

    m_systemID = pSystem->GetID();
    m_bubbleID = sBubbleMgr.GetBubbleID();

    _log(DESTINY__BUBBLE_TRACE, "SystemBubble::Constructor - Created new bubble %u(%p) at (%.2f,%.2f,%.2f)[%.1f].",\
	     m_bubbleID, this, m_center.x, m_center.y, m_center.z, m_radius);
}

SystemBubble::~SystemBubble()
{
    if (m_hasMarkers)
        for (auto cur : m_markers) {
            cur.second->Delete(); // delete marker cans here
            SafeDelete(cur.second);
        }
}

void SystemBubble::clear() {
    if (m_hasMarkers)
        for (auto cur : m_markers) {
            cur.second->Delete(); // delete marker cans here
            SafeDelete(cur.second);
        }

    m_ice = false;
    m_belt = false;
    m_gate = false;
    m_anomaly = false;
    m_mission = false;
    m_spawned = false;
    m_incursion = false;
    m_hasBubble = false;
    m_hasMarkers = false;

    m_markers.clear();
    m_players.clear();
    m_entities.clear();
    m_dynamicEntities.clear();
}

void SystemBubble::Process()
{
    /* this will need to process:
     *    belt and gate for spawn/respawn
     *    missions for ??
     *    incursions for ??
     */
    if (m_belt and (m_system->GetSystemSecurityRating() > 0.90)) // make config option here to spawn rats in secure empire space?   nope.
        return;
    if (m_spawned) {
        m_spawnTimer.Disable();
        return;
    }

    // this must run a second time for spawn to actually hit.  first time only sets main system spawn timer.
    // may be nuts, but will remain enabled as long as player in bubble and bubble has no rats.
    if (m_spawnTimer.Enabled()) {
        if (m_spawnTimer.Check()) {
            if (!m_players.empty()) {
                m_system->DoSpawnForBubble(this);
            } else {
                m_spawnTimer.Disable();
            }
        }
    }
}

//called every 30s from the bubble manager.
//verifies that each entity is still in this bubble.
//if any entity is no longer in the bubble, they are removed
//from the bubble and stuck into the vector for re-classification.
void SystemBubble::ProcessWander(std::vector<SystemEntity*> &wanderers) {
    DynamicSystemEntity* pDSE(nullptr);
    std::map<uint32, SystemEntity*>::iterator itr = m_dynamicEntities.begin();
    while (itr != m_dynamicEntities.end()) {
        if (itr->second == nullptr) {
            itr = m_dynamicEntities.erase(itr);
            continue;
        }
        pDSE = itr->second->GetDynamicSE();
        if (pDSE == nullptr) {
            itr = m_dynamicEntities.erase(itr);
            continue;
        }
        if ((pDSE->DestinyMgr() == nullptr) or pDSE->DestinyMgr()->IsWarping()) {
            ++itr;
            continue;
        }
        // is this shit really needed??
        if (pDSE->SystemMgr()->GetID() != m_systemID) {
            // this entity is in a different system!  this shouldnt happen....
            // remove this entity, insert into wanderers, and continue
            wanderers.push_back(pDSE);
            _log(DESTINY__WARNING, "SystemBubble::ProcessWander() - entity %u is in %u but this is %u.", \
                                pDSE->GetID(), pDSE->SystemMgr()->GetID(), m_systemID);
            itr = m_dynamicEntities.erase(itr);
            pDSE = nullptr;
            continue;
        }
        if (!InBubble(pDSE->GetPosition())) {
            wanderers.push_back(pDSE);
            //17:38:57 [DestinyWarning] SystemBubble::ProcessWander() - entity 140006173(sys:30002507) not in bubble 1 for systemID 30002510.
            _log(DESTINY__WARNING, "SystemBubble::ProcessWander() - entity %u(sys:%u) not in bubble %u for systemID %u.", \
                        pDSE->GetID(), pDSE->SystemMgr()->GetID(), m_bubbleID, m_systemID);
            itr = m_dynamicEntities.erase(itr);
            pDSE = nullptr;
            continue;
        }
        ++itr;
    }
    pDSE = nullptr;

    if (!m_players.empty() and m_spawned)
        ResetBubbleRatSpawn();
}

void SystemBubble::Add(SystemEntity* pSE)
{
    //if they are already in this bubble, do not continue.
    if (m_entities.find(pSE->GetID()) != m_entities.end()) {
        _log(DESTINY__BUBBLE_TRACE, "SystemBubble::Add() - Tried to add Static Entity %u to bubble %u, but it is already in here.",\
             pSE->GetID(), m_bubbleID);
        return;
    }

    pSE->m_bubble = this;
    // global entities also in SystemMgr's static list.  this is used for SystemBubble->IsEmpty() deletion check
    if (pSE->IsStaticEntity() or pSE->isGlobal()) {
        _log(DESTINY__BUBBLE_TRACE, "SystemBubble::Add() - Entity %s(%u) is static or global or both.", pSE->GetName(), pSE->GetID() );
        // all static and global entities (stations, gates, asteroid fields, cyno fields, etc) are put into bubble's staticEntity map
        m_entities[pSE->GetID()] = pSE;
        return;
    }

    //if they are already in this bubble, do not continue.
    if (m_dynamicEntities.find(pSE->GetID()) != m_dynamicEntities.end()) {
        _log(DESTINY__BUBBLE_TRACE, "SystemBubble::Add() - Tried to add Dynamic Entity %u to bubble %u, but it is already in here.",\
                pSE->GetID(), m_bubbleID);
        return;
    }

    _log(DESTINY__BUBBLE_TRACE, "SystemBubble::Add() - Adding entity %u to bubble %u.  Dist to center: %.2f", \
            pSE->GetID(), m_bubbleID, m_center.distance(pSE->GetPosition()));

    if (is_log_enabled(DESTINY__BUBBLE_DEBUG)) {
        GPoint startPoint( pSE->GetPosition() );
        GVector direction(startPoint, NULL_ORIGIN);
        double rangeToStar = direction.length();
        rangeToStar /= ONE_AU_IN_METERS;
        _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::Add() - Distance to Star %.2f AU.  %u/%u Entities in bubble %u",\
                rangeToStar, m_entities.size(), m_dynamicEntities.size(), m_bubbleID);
        //if (sConfig.debug.StackTrace)
        //    EvE::traceStack();
    }

    if (pSE->HasPilot()) {
        // Set spawn timer for this bubble, if needed
        if (m_belt) {
            // check for roids and load/spawn as needed.
            m_system->GetBeltMgr()->CheckSpawn(m_bubbleID);
            if (sConfig.npc.RoamingSpawns)
                if (!m_spawnTimer.Enabled())
                    SetSpawnTimer(true);
        }
        if (m_gate and sConfig.npc.StaticSpawns)
            if (!m_spawnTimer.Enabled())
                SetSpawnTimer(false);

        Client* pClient(pSE->GetPilot());
        SendAddBalls( pSE );
        if (!m_players.empty())
            AddBallExclusive(pSE);  // adds new player to all players in bubble, if any

        m_players[pClient->GetCharacterID()] = pClient;   //add to bubble's player list
    } else {
        if (!m_players.empty())
            AddBallExclusive(pSE);
        if (pSE->IsDroneSE())
            m_drones[pSE->GetID()] = pSE->GetDroneSE();
    }

    // all non-global entities (players, npcs, roids, containers, etc) are put into bubble's dynamicEntity map
    m_dynamicEntities[pSE->GetID()] = pSE;
}

void SystemBubble::Remove(SystemEntity *pSE) {
    //assume that the entity is properly registered for its ID
    if (pSE->m_bubble == nullptr) {
        if (sConfig.debug.StackTrace)
            EvE::traceStack();
		return;
    }

    _log(DESTINY__BUBBLE_TRACE, "SystemBubble::Remove() - Removing entity %u from bubble %u", pSE->GetID(), m_bubbleID);

    m_entities.erase(pSE->GetID());
    m_dynamicEntities.erase(pSE->GetID());

    if (pSE->HasPilot()) {
        m_players.erase(pSE->GetPilot()->GetCharacterID());
        RemoveBalls(pSE);
    }

    //notify everybody else in the bubble of the removal.
    if (!m_players.empty())
        RemoveBall(pSE);

    if (pSE->IsDroneSE())
        m_drones.erase(pSE->GetID());

    if (is_log_enabled(DESTINY__BUBBLE_DEBUG))
        sLog.Warning("SystemBubble::Remove()", "Removing entity %u from bubble %u", pSE->GetID(), m_bubbleID);

    pSE->m_bubble = nullptr;
}

void SystemBubble::RemoveExclusive(SystemEntity *pSE) {
    if (pSE->m_bubble == nullptr)
        return;

    _log(DESTINY__BUBBLE_TRACE, "SystemBubble::RemoveExclusive() - Removing entity %u from bubble %u", pSE->GetID(), m_bubbleID);
    RemoveBallExclusive(pSE);
}

void SystemBubble::ResetBubbleRatSpawn()
{
    /* the current spawn in this bubble was killed off, so reset timers accordingly
     *   once the timer hits, it will do all needed checks for players and respawn as needed.
     *  this enables creating a new spawn after previous group was killed off
     */
    m_spawned = false;
    if (m_belt and sConfig.npc.RoamingSpawns)
        if (!m_spawnTimer.Enabled())
            SetSpawnTimer(true);
    if (m_gate and sConfig.npc.StaticSpawns) /* m_gate = false.  will fix when gate spawns are finished */
        if (!m_spawnTimer.Enabled())
            SetSpawnTimer(false);
}

void SystemBubble::SetSpawnTimer(bool isBelt/*false*/)
{
    if (m_system->GetSystemSecurityRating() > 0.90)
        return;
    if (sConfig.debug.SpawnTest) {
        m_spawnTimer.Start(5000); /* 5s for testing */
    } else {
        // these randoms should be changed to reflect this npc's faction presence in system
        if (isBelt) {
            m_spawnTimer.Start(MakeRandomInt(30, sConfig.npc.RoamingTimer) *1000);
        } else {
            m_spawnTimer.Start(MakeRandomInt(60, sConfig.npc.StaticTimer) *1000);
        }
    }
}

void SystemBubble::SetBelt(InventoryItemRef itemRef)
{
    m_belt = true;
    sBubbleMgr.AddSpawnID(m_bubbleID, itemRef->itemID());
    m_system->GetBeltMgr()->RegisterBelt(itemRef);
    if (itemRef->typeID() == 17774)
        m_ice = true;
}

void SystemBubble::SetGate(uint32 gateID)
{
    m_gate = true;
    sBubbleMgr.AddSpawnID(m_bubbleID, gateID);
}

SystemEntity* const SystemBubble::GetEntity(uint32 entityID) const {
    /* updated to send ONLY dynamic entities to the following:          -allan 17Apr15
     *     ModuleManager::Activate()       --for module activation (with a target)
     */
    std::map<uint32, SystemEntity*>::const_iterator itr = m_dynamicEntities.find(entityID);
    if (itr != m_dynamicEntities.end())
        return itr->second;

    return nullptr;
}

void SystemBubble::GetEntities(std::map<uint32, SystemEntity*> &into) const {
    /* updated to send non-cloaked dynamic entities to the following:         -allan 14Feb15
     *    SystemManager::MakeSetState()   --for player entering new system
     *    Command_killallnpcs()           --GM command
     *    StructureSE::InitData()         --Get TowerSE for pos items
     */
    if (m_dynamicEntities.empty())
        return;

    for (auto cur : m_dynamicEntities) {
        if (cur.second->DestinyMgr() != nullptr)
            if (cur.second->DestinyMgr()->IsCloaked())
                continue;
        into.emplace(cur.first, cur.second);
    }
}

void SystemBubble::GetAllEntities(std::map< uint32, SystemEntity* >& into) const
{
    if (m_dynamicEntities.empty())
        return;

    for (auto cur : m_dynamicEntities)
        into.emplace(cur.first, cur.second);
}


void SystemBubble::GetEntityVec(std::vector< SystemEntity* >& into) const
{
    if (m_players.empty())
        return;

    for (auto cur : m_dynamicEntities)
        into.push_back(cur.second);
}

void SystemBubble::GetPlayers(std::vector<Client*> &into) const {
    /* updated to send ONLY players to the following:         -allan 14Feb15
     *    NPCAIMgr::Process()             --for npc targeting
     *    SpawnEntry::Process()           --for npc spawning
     *
     * this will also send player drones once that system is completed
     */
    into.clear();
    if (m_players.empty())
        return;

    for (auto cur : m_players)
        into.push_back(cur.second);
}

SystemEntity* SystemBubble::GetRandomEntity()
{
    // this is used for idle npc's as a orbit target while waiting for something to pewpew
    if (m_dynamicEntities.empty())
        return nullptr;

    for (auto cur : m_dynamicEntities) {
        if (cur.second->IsWreckSE())
            return cur.second;
        if (cur.second->IsObjectEntity())
            return cur.second;
    }
    return nullptr;
}

uint32 SystemBubble::CountNPCs() {
    uint32 count = 0;
    for (auto cur : m_dynamicEntities)
        if (cur.second->IsNPCSE())
            ++count;

    return count;
}

bool SystemBubble::InBubble(const GPoint& pt, bool inWarp/*false*/) const
{
    if (is_log_enabled(DESTINY__BUBBLE_DEBUG)) {
        float distance = m_center.distance(pt);
        bool check = false;
        if (distance < m_radius + 5000)  // 5k is the grey area between bubbles
            check = true;

        _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::InBubble(%u) - center: %.1f,%.1f,%.1f - distance: %.1f, check: %s", \
                m_bubbleID, m_center.x, m_center.y, m_center.z, distance, check?"true":"false");
        return check;
    }

    return (m_center.distance(pt) < m_radius);
}

bool SystemBubble::IsOverlap( const GPoint& pt ) const
{
    if (is_log_enabled(DESTINY__BUBBLE_DEBUG)) {
        float distance(m_center.distance(pt));
        bool check(false);
        if (distance < m_radius * 2 + 10)
            check = true;

        _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::IsOverlap(%u) - center: %.1f,%.1f,%.1f - distance: %.1f, check: %s", \
                m_bubbleID, m_center.x, m_center.y, m_center.z, distance, check?"true":"false");
        return check;
    }

    return (m_center.distance(pt) < (m_radius * 2));
}

void SystemBubble::PrintEntityList() {
    bool found(false);
    for (auto cur : m_dynamicEntities) {
        found = false;
        if (cur.second->isGlobal())  //this should only hit beacons and cynos as global AND not static
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is Global.", cur.second->GetName(), cur.second->GetID() );
        if (cur.second->IsShipSE()) {
            if (cur.second->HasPilot()) {
                sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is Player Ship.", cur.second->GetName(), cur.second->GetID() ); found = true;
            } else {
                sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is Empty Player Ship.", cur.second->GetName(), cur.second->GetID() ); found = true;
            }
        }
        if (cur.second->IsNPCSE()) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is NPC.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsJumpBridgeSE()) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is JumpBridge.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsTCUSE()) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is TCU.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsSBUSE()) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is SBU.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsIHubSE()) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is IHub.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsCOSE()) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is Customs Office.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsTowerSE()) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is Tower.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsPOSSE() and !found) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is other POS.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsContainerSE()) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is Container.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsWreckSE()) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is Wreck.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsOutpostSE()) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is Outpost.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsAsteroidSE()) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is Asteroid.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsDeployableSE()) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is Deployable.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsStaticEntity() and !found) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is Static.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsItemEntity() and !found) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is Item.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsObjectEntity() and !found) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is Object.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (cur.second->IsDynamicEntity() and !found) {
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is Dynamic.", cur.second->GetName(), cur.second->GetID() ); found = true;
        }
        if (!found)
            sLog.Warning("SystemBubble::PrintEntityList()", "entity %s(%u) is None of the Above.", cur.second->GetName(), cur.second->GetID() );
    }
}

void SystemBubble::SendAddBalls(SystemEntity* to_who) {
    if (!m_system->IsLoaded())
        return;
    if (m_dynamicEntities.empty())
        return;
    if (!to_who->HasPilot())
        return;
    Client* pClient = to_who->GetPilot();
    if (pClient == nullptr)
        return;
    if (is_log_enabled(DESTINY__BUBBLE_DEBUG))
        PrintEntityList();

    Buffer* destinyBuffer = new Buffer();

    Destiny::AddBall_header head = Destiny::AddBall_header();
        head.packet_type = 1;   // 0 = full state   1 = balls
        head.stamp = sEntityList.GetStamp();
    destinyBuffer->Append(head);

    AddBalls addballs;
    addballs.slims = new PyList();

    for (auto cur : m_dynamicEntities) {
        if (cur.second->DestinyMgr() != nullptr)
            if (cur.second->DestinyMgr()->IsCloaked())
                continue;
        if (!cur.second->IsMissileSE() or !cur.second->IsFieldSE())
            addballs.damageDict[cur.first] = cur.second->MakeDamageState();
        addballs.slims->AddItem( new PyObject( "foo.SlimItem", cur.second->MakeSlimItem() ) );
        cur.second->EncodeDestiny( *destinyBuffer );
    }

    if (addballs.slims->empty()) {
        SafeDelete( destinyBuffer );
        return;
    }

    addballs.state = new PyBuffer( &destinyBuffer );

    _log(DESTINY__MESSAGE, "SystemBubble::SendAddBalls() to %s", pClient->GetName());
    if (is_log_enabled(DESTINY__BALL_DUMP))
        addballs.Dump( DESTINY__BALL_DUMP, "    " );
    _log( DESTINY__BALL_DECODE, "    Ball Decoded:" );
    if (is_log_enabled(DESTINY__BALL_DECODE))
        Destiny::DumpUpdate( DESTINY__BALL_DECODE, &( addballs.state->content() )[0], (uint32)addballs.state->content().size() );
    PyTuple* t = addballs.Encode();
    pClient->QueueDestinyUpdate( &t );    //consumed
}

void SystemBubble::SendAddBalls2( SystemEntity* to_who ) {
    if (!m_system->IsLoaded())
        return;
    if (m_dynamicEntities.empty())
        return;
    if (!to_who->HasPilot())
        return;
    Client* pClient = to_who->GetPilot();
    if (pClient == nullptr)
        return;
    if (is_log_enabled(DESTINY__BUBBLE_TRACE))
        PrintEntityList();

    Buffer* destinyBuffer = new Buffer();

    Destiny::AddBall_header head = Destiny::AddBall_header();
        head.packet_type = 1;   // 0 = full state   1 = balls
        head.stamp = sEntityList.GetStamp();
    destinyBuffer->Append(head);

    AddBalls2 addballs2;
    addballs2.stateStamp = sEntityList.GetStamp();
    addballs2.extraBallData = new PyList();

    for (auto cur : m_dynamicEntities) {
        if (cur.second->IsMissileSE() or cur.second->IsContainerSE()) {
            addballs2.extraBallData->AddItem(cur.second->MakeSlimItem());
        } else {
            PyTuple* balls = new PyTuple(2);
                balls->SetItem(0, cur.second->MakeSlimItem());
                balls->SetItem(1, cur.second->MakeDamageState());
            addballs2.extraBallData->AddItem(balls);
        }
        cur.second->EncodeDestiny(*destinyBuffer);
    }

    if (addballs2.extraBallData->size() < 1) {
        SafeDelete( destinyBuffer );
        return;
    }

    addballs2.state = new PyBuffer(&destinyBuffer); //consumed
    SafeDelete( destinyBuffer );

    _log( DESTINY__MESSAGE, "SystemBubble::SendAddBalls2() to %s", pClient->GetName());
    if (is_log_enabled(DESTINY__BALL_DUMP))
        addballs2.Dump( DESTINY__BALL_DUMP, "    " );
    //_log( DESTINY__TRACE, "    Ball Binary:" );
    //_hex( DESTINY__TRACE, &( addballs2.state->content() )[0], (uint32)addballs2.state->content().size() );
    /*  note:  this shows up in valgrind as an uninitialized value   -allan 24Mar16
     * Conditional jump or move depends on uninitialised value(s)  SystemBubble.cpp:484 (uncorrected line#)
     * Uninitialised value was created by a heap allocation  SystemBubble.cpp:472
     */
    _log( DESTINY__BALL_DECODE, "    Ball Decoded:" );
    if (is_log_enabled(DESTINY__BALL_DECODE))
        Destiny::DumpUpdate( DESTINY__BALL_DECODE, &( addballs2.state->content() )[0], (uint32)addballs2.state->content().size() );
    PyTuple* t = addballs2.Encode();
    pClient->QueueDestinyUpdate(&t, true);    //consumed
}

void SystemBubble::AddBallExclusive( SystemEntity* pSE ) {
    if (!m_system->IsLoaded())
        return;
    if (pSE->DestinyMgr() != nullptr)
        if (pSE->DestinyMgr()->IsCloaked())
            return;

    Buffer* destinyBuffer = new Buffer();

    //create AddBalls header
    Destiny::AddBall_header head = Destiny::AddBall_header();
        head.packet_type = 1;   // 0 = full state   1 = balls
        head.stamp = sEntityList.GetStamp();
    destinyBuffer->Append( head );

    AddBalls addballs;
    //encode destiny binary
    pSE->EncodeDestiny( *destinyBuffer );
    addballs.state = new PyBuffer( &destinyBuffer );
	//encode damage state
    addballs.damageDict[ pSE->GetID() ] = pSE->MakeDamageState();
	//encode SlimItem
    addballs.slims = new PyList();
    addballs.slims->AddItem( new PyObject( "foo.SlimItem", pSE->MakeSlimItem() ) );

    _log(DESTINY__BUBBLE_TRACE, "SystemBubble::AddBallExclusive() - Adding entity %u to bubble %u", pSE->GetID(), m_bubbleID);
    if (is_log_enabled(DESTINY__BALL_DUMP))
        addballs.Dump( DESTINY__BALL_DUMP, "    " );
    _log( DESTINY__BALL_DECODE, "    Ball Decoded:" );
    if (is_log_enabled(DESTINY__BALL_DECODE))
        Destiny::DumpUpdate( DESTINY__BALL_DECODE, &( addballs.state->content() )[0], (uint32)addballs.state->content().size() );
    //bubblecast the update
    PyTuple* t = addballs.Encode();
    BubblecastDestinyUpdateExclusive( &t, "AddBall", pSE );
    PySafeDecRef( t );
}

/*  NOTE   lil insight into clients code for RemoveBall
 * RemoveBall is function to remove all data associated with a particular ballID.
 * this call is only effective when a SlimItem for that ball is currently active in clients bubble,
 * and the ballID is > destiny.dstLocalBalls (which i dont know exactly what that is yet)
 * RemoveBalls is called when there is an associated TerminalExplosion with that ballID
 * [code]
 *          if funcName == 'RemoveBalls':
                exploders = [ x[1][1][0] for x in state if x[1][0] == 'TerminalExplosion' ]
 * [/code]
 * RemoveBalls is then called on the entire group, and will call RemoveBall(ballID, terminal) on each ball.
 *  the bool 'terminal' is initially false, then set to true if there is an associated TerminalExplosion for that ballID.
 *
 * see also DestinyManager::SendTerminalExplosion()
 *      NOTE  RemoveBall doesnt not work as i thought it should....doesnt trigger explosion.
 */
//TODO  update these based on above notes   (also look into better (non-ambigious) naming)
void SystemBubble::RemoveBall(SystemEntity *about_who) {
    // RemoveBallFromBP removeball;
    //    removeball.entityID = about_who->GetID();
    // using RemoveBalls instead of RemoveBall because client
    // seems not to trigger explosion on RemoveBall
    if (!m_system->IsLoaded())
        return;
    RemoveBallsFromBP removeball;
    removeball.balls.push_back(about_who->GetID());

    _log(DESTINY__MESSAGE, "SystemBubble::RemoveBall()");
    if (is_log_enabled(DESTINY__BALL_DUMP))
        removeball.Dump( DESTINY__BALL_DUMP, "    " );

    PyTuple *tmp = removeball.Encode();
    BubblecastDestinyUpdate(&tmp, "RemoveBall");
    PySafeDecRef( tmp );
}

// this *should* only be called from DestinyMgr::Cloak() and DestinyMgr::Jump()
void SystemBubble::RemoveBallExclusive(SystemEntity *about_who) {
    RemoveBallFromBP removeball;
        removeball.entityID = about_who->GetID();
    // RemoveBalls removeball;
    //removeball.balls.push_back(about_who->GetID());

    _log(DESTINY__MESSAGE, "SystemBubble::RemoveBallExclusive()");
    if (is_log_enabled(DESTINY__BALL_DUMP))
        removeball.Dump( DESTINY__BALL_DUMP, "    " );

    PyTuple *tmp = removeball.Encode();
    BubblecastDestinyUpdateExclusive(&tmp, "RemoveBall", about_who);
    PySafeDecRef( tmp );
}

void SystemBubble::RemoveBalls( SystemEntity* to_who ) {
    if (!m_system->IsLoaded())
        return;
    if (m_dynamicEntities.empty())
        return;
    if ((!to_who->HasPilot()) or (to_who->SysBubble() == nullptr))
        return;
    Client* pClient = to_who->GetPilot();
    if ((pClient == nullptr) or pClient->IsDock() or pClient->IsDocked())
        return;

    RemoveBallsFromBP remove_balls;

    for (auto cur : m_dynamicEntities)
        remove_balls.balls.push_back(cur.first);

    if (remove_balls.balls.empty())
        return;

    _log( DESTINY__MESSAGE, "SystemBubble::RemoveBalls() - sending to %s", pClient->GetName());
    if (is_log_enabled(DESTINY__BALL_DUMP))
        remove_balls.Dump( DESTINY__BALL_DUMP, "    " );

    PyTuple* tmp = remove_balls.Encode();
    pClient->QueueDestinyUpdate( &tmp );
}

PyObject* SystemBubble::GetDroneState() const
{
    PyList* header = new PyList(7);
        header->SetItemString(0, "droneID");
        header->SetItemString(1, "ownerID");
        header->SetItemString(2, "controllerID");
        header->SetItemString(3, "activityState");
        header->SetItemString(4, "typeID");
        header->SetItemString(5, "controllerOwnerID");
        header->SetItemString(6, "targetID");
    PyList* lines = new PyList();
    for (auto cur : m_drones) {
        PyList* line = new PyList(7);
            line->SetItem(0, new PyInt(cur.first));
            line->SetItem(1, new PyInt(cur.second->GetOwnerID()));
            line->SetItem(2, new PyInt(cur.second->GetControllerID()));
            line->SetItem(3, new PyInt(cur.second->GetState()));
            line->SetItem(4, new PyInt(cur.second->GetSelf()->typeID()));
            line->SetItem(5, new PyInt(cur.second->GetControllerOwnerID()));
            line->SetItem(6, new PyInt(cur.second->GetTargetID()));
        lines->AddItem(line);
    }

    PyDict* dict = new PyDict();
        dict->SetItemString("header", header);
        dict->SetItemString("RowClass", new PyToken("util.Row"));
        dict->SetItemString("lines", lines);

    return new PyObject("util.Rowset", dict);
}

void SystemBubble::SyncPos() {
    // send positions of all dSE in bubble to all players in bubble
    for (auto player : m_players)
        for (auto dse : m_dynamicEntities) {
            SetBallPosition du;
                du.entityID = dse.first;
                du.x = dse.second->GetPosition().x;
                du.y = dse.second->GetPosition().y;
                du.z = dse.second->GetPosition().z;
            PyTuple* up = du.Encode();
            player.second->GetShipSE()->DestinyMgr()->SendSingleDestinyUpdate(&up);
        }
}

void SystemBubble::CmdDropLoot()
{
    for (auto dse : m_dynamicEntities) {
        if (dse.second->IsNPCSE())
            dse.second->GetNPCSE()->CmdDropLoot();
    }
}


void SystemBubble::RemoveMarkers()
{
    if (m_hasMarkers)
        for (auto cur : m_markers) {
            m_system->RemoveEntity(cur.second);
            cur.second->Delete(); // delete marker cans here
            SafeDelete(cur.second);
        }
    m_markers.clear();
    m_centerSE = nullptr;
    m_hasMarkers = false;
}


void SystemBubble::MarkCenter()
{
    // we are not creating markers on system boot.
    if (!m_system->IsLoaded())
        return;
    if (m_hasMarkers)
        return;
    // create jetcan to mark bubble center
    std::string str = "Center Marker for Bubble #", desc = "Bubble Center"; //std::to_string(m_bubbleID);
    str += std::to_string(m_bubbleID);
    MarkBubble(m_center, str, desc, true);

    // create jetcan to mark bubble x
    GPoint center = m_center;
    center.x += BUBBLE_RADIUS_METERS - 5;
    str.clear();
    str = "Bubble #";
    str += std::to_string(m_bubbleID);
    str += " +X";
    desc = "Bubble x";
    MarkBubble(center, str, desc);

    // create jetcan to mark bubble -x
    center = m_center;
    center.x -= BUBBLE_RADIUS_METERS - 5;
    str.clear();
    str = "Bubble #";
    str += std::to_string(m_bubbleID);
    str += " -X";
    desc = "Bubble -x";
    MarkBubble(center, str, desc);

    // create jetcan to mark bubble y
    center = m_center;
    center.y += BUBBLE_RADIUS_METERS - 5;
    str.clear();
    str = "Bubble #";
    str += std::to_string(m_bubbleID);
    str += " +Y";
    desc = "Bubble y";
    MarkBubble(center, str, desc);

    // create jetcan to mark bubble -y
    center = m_center;
    center.y -= BUBBLE_RADIUS_METERS - 5;
    str.clear();
    str = "Bubble #";
    str += std::to_string(m_bubbleID);
    str +=  " -Y";
    desc = "Bubble -y";
    MarkBubble(center, str, desc);

    // create jetcan to mark bubble z
    center = m_center;
    center.z += BUBBLE_RADIUS_METERS - 5;
    str.clear();
    str = "Bubble #";
    str += std::to_string(m_bubbleID);
    str += " +Z";
    desc = "Bubble z";
    MarkBubble(center, str, desc);

    // create jetcan to mark bubble -z
    center = m_center;
    center.z -= BUBBLE_RADIUS_METERS - 5;
    str.clear();
    str = "Bubble #";
    str += std::to_string(m_bubbleID);
    str +=  " -Z";
    desc = "Bubble -z";
    MarkBubble(center, str, desc);

    m_hasMarkers = true;
}

void SystemBubble::MarkBubble(const GPoint& position, std::string& name, std::string& desc, bool center/*false*/)
{
    // create new container item
    ItemData idata(23, ownerSystem, m_systemID, flagNone, name.c_str(), position, desc.c_str());
    CargoContainerRef cRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if ( cRef.get() == nullptr) {
        _log(DESTINY__WARNING, "MarkBubble() could not create Item for %s (%s)", name.c_str(), desc.c_str());
        return;
    }

    // create SE for item
    FactionData jetcanData = FactionData();
    ContainerSE* cSE = new ContainerSE( cRef, *(m_system->GetServiceMgr()), m_system, jetcanData);
    if (cSE == nullptr) {
        _log(DESTINY__WARNING, "MarkBubble() could not create SE for %s (%s)", name.c_str(), desc.c_str());
        return;
    }
    cRef->SetMySE(cSE);
    cSE->AnchorContainer();
    m_markers.emplace(cRef->itemID(), cSE);
    if (center) {
        // only setting centers as global
        cSE->SetGlobal(true);
        m_centerSE = cSE;
        m_system->AddMarker(cSE, true, true);
    } else {
        m_system->AddEntity(cSE, false);
    }
}


void SystemBubble::BubblecastDestiny(std::vector<PyTuple *> &updates, std::vector<PyTuple *> &events, const char *desc) const {
    if (m_players.empty())
        return;

    BubblecastDestinyUpdate(updates, desc);
    BubblecastDestinyEvent(events, desc);
}

void SystemBubble::BubblecastDestinyUpdate(std::vector<PyTuple *> &updates, const char *desc) const {
    for (std::vector<PyTuple *>::iterator itr = updates.begin(); itr != updates.end(); ++itr)
        BubblecastDestinyUpdate(&(*itr), desc);

    updates.clear();
}

void SystemBubble::BubblecastDestinyEvent(std::vector<PyTuple *> &events, const char *desc) const {
    for (std::vector<PyTuple *>::iterator itr = events.begin(); itr != events.end(); ++itr)
        BubblecastDestinyEvent(&(*itr), desc);

    events.clear();
}

void SystemBubble::BubblecastDestinyUpdate( PyTuple** payload, const char* desc ) const
{
    if (is_log_enabled(DESTINY__BUBBLECAST_DUMP))
        (*payload)->Dump(DESTINY__BUBBLECAST_DUMP, "    ");
    for (auto cur : m_players) {
        _log( DESTINY__BUBBLECAST, "Bubblecast %s update to %s(%u)", desc, cur.second->GetName(), cur.first );
        PyIncRef(*payload);
        cur.second->QueueDestinyUpdate(payload);
    }
}

void SystemBubble::BubblecastDestinyUpdateExclusive( PyTuple** payload, const char* desc, SystemEntity* pSE ) const
{
    for (auto cur : m_players) {
        // Only queue a Destiny update for this bubble if the current SystemEntity is not 'pSE':
        // (this is an update to all client objects in the bubble EXCLUDING 'pSE')
        if (cur.second->GetShipSE() != pSE) {
            _log( DESTINY__BUBBLECAST, "Exclusive Bubblecast %s update to %s(%u)", desc, cur.second->GetName(), cur.first );
            PyIncRef(*payload);
            cur.second->QueueDestinyUpdate(payload);
        }
    }
}

void SystemBubble::BubblecastDestinyEvent( PyTuple** payload, const char* desc ) const
{
    if (is_log_enabled(DESTINY__BUBBLECAST_DUMP))
        (*payload)->Dump(DESTINY__BUBBLECAST_DUMP, "    ");
    for (auto cur : m_players) {
        _log( DESTINY__BUBBLECAST, "Bubblecast %s event to %s(%u)", desc, cur.second->GetName(), cur.first );
        PyIncRef(*payload);
        cur.second->QueueDestinyEvent(payload);
    }
}

void SystemBubble::BubblecastSendNotification(const char* notifyType, const char* idType, PyTuple** payload, bool seq)
{
    for (auto cur : m_players) {
        _log( DESTINY__BUBBLECAST, "BubblecastNotify %s to %s(%u)", notifyType, cur.second->GetName(), cur.first );
        PyIncRef(*payload);
        cur.second->SendNotification( notifyType, idType, payload, seq );
    }
}
