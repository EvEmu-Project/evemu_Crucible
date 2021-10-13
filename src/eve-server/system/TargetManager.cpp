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
/** @todo (Allan)  add target lost and target fail reasons.
 * maybe make common function, and pass "add", "clear", "otheradd", reason, etc ??
 *
 * NOTE:  the above suggestions have been completed and are in TargetManager.new
 *      still have bugs to work out.
 */

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "Profiler.h"
#include "Client.h"
#include "inventory/AttributeEnum.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "pos/Structure.h"
#include "pos/Tower.h"
#include "pos/sovStructures/TCU.h"
#include "pos/sovStructures/IHub.h"
#include "ship/Ship.h"
#include "ship/modules/ActiveModule.h"
#include "ship/modules/MiningLaser.h"
#include "ship/modules/Prospector.h"
#include "system/TargetManager.h"
#include "system/SystemEntity.h"
#include "system/SystemBubble.h"

TargetManager::TargetManager(SystemEntity *self)
: mySE(self)
{
    m_canAttack = false;

    m_modules.clear();
    m_targets.clear();
    m_targetedBy.clear();
}

bool TargetManager::Process() {
    double profileStartTime = GetTimeUSeconds();

    if (m_targets.empty())
        return false;

    //process outgoing targeting (outgoing will call incoming as needed)
    std::map<SystemEntity*, TargetEntry*>::iterator itr = m_targets.begin();
    while (itr != m_targets.end()) {
        if ((itr->first == nullptr) or (itr->second == nullptr)) {
            itr = m_targets.erase(itr);
            continue;
        }
        switch (itr->second->state) {
            case TargMgr::State::Idle:
            case TargMgr::State::Locked:{          //do nothing
            } break;
            case TargMgr::State::Passive:   // this will be used with stealth modules (which, ofc, are not written yet)
            case TargMgr::State::Locking: {
                if (itr->second->timer.Check(false)) {
                    itr->second->timer.Disable();
                    itr->second->state = TargMgr::State::Locked;
                    _log(TARGET__TRACE, "%s(%u) has finished locking %s(%u)", \
                            mySE->GetName(), mySE->GetID(), itr->first->GetName(), itr->first->GetID());
                    TargetAdded(itr->first);
                    itr->first->TargetMgr()->TargetedByLocked(mySE);
                    m_canAttack = true;
                }
            } break;
        }
        ++itr;
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::targets, GetTimeUSeconds() - profileStartTime);

    return true;
}

void TargetManager::Unload() {
    for (auto cur : m_targets)
        SafeDelete(cur.second);
    m_targets.clear();
    for (auto cur : m_targetedBy)
        SafeDelete(cur.second);
    m_targetedBy.clear();
}

bool TargetManager::StartTargeting(SystemEntity *tSE, ShipItemRef sRef)
{       // NOTE this is for players and CAN throw (client calls this inside try/catch block)
    if (!mySE->HasPilot()) {
        codelog(TARGET__ERROR, "StartTargeting() called by pilot-less ship %s(%u) to target %s(%u)", \
        mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID());
        return false;
    }

    //first make sure they are not already in the list
    if (m_targets.find(tSE) != m_targets.end()) {
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but we are already targeting them. Ignoring request.", \
        mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID());
        return false;
    }
    // get lower of ship and char target skills, with minimum of 1
    uint8 maxLockedTargets = 1;
    uint8 maxCharTargets = mySE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::Targeting);
    maxCharTargets += mySE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::Multitasking);
    if (maxCharTargets > 0)
        if (maxLockedTargets < maxCharTargets)
            maxLockedTargets = maxCharTargets;

    uint8 maxShipTargets = (uint8)sRef->GetAttribute(AttrMaxLockedTargets).get_uint32();
    if (maxShipTargets > 0)
        if (maxLockedTargets > maxShipTargets)
            maxLockedTargets = maxShipTargets;

    if (m_targets.size() >= maxLockedTargets) {
        mySE->GetPilot()->SendNotifyMsg("Your ship and skills combination can only handle %u targets at a time.", maxLockedTargets);
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but we already have max targets of %u.  Ignoring request.", \
                mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID(), maxLockedTargets);
        return false;
    }

    // Check if target is an invulnerable structure
    if (tSE->IsTCUSE()) {
        if (tSE->GetTCUSE()->GetState() == EVEPOS::StructureState::Online) {
            mySE->GetPilot()->SendNotifyMsg("You cannot target an invulnerable structure.");
            return false;
        }
    } else if (tSE->IsTowerSE()) {
        if ((tSE->GetTowerSE()->GetState() == EVEPOS::StructureState::Reinforced) || (tSE->GetTowerSE()->GetState() == EVEPOS::StructureState::ArmorReinforced) || (tSE->GetTowerSE()->GetState() == EVEPOS::StructureState::SheildReinforced)) {
            mySE->GetPilot()->SendNotifyMsg("You cannot target an invulnerable structure.");
            return false;
        }
    } else if (tSE->IsIHubSE()) {
        if (tSE->GetIHubSE()->GetState() == EVEPOS::StructureState::Online) {
            mySE->GetPilot()->SendNotifyMsg("You cannot target an invulnerable structure.");
            return false;
        }
    } else if (tSE->IsOutpostSE()) {
        if (tSE->GetOutpostSE()->GetState() == EVEPOS::StructureState::Online) { //TODO: This structure state likely will be different for outposts, will change later.
            mySE->GetPilot()->SendNotifyMsg("You cannot target an invulnerable structure.");
            return false;
        }
    }

    // Check against max target range
    double maxTargetRange = sRef->GetAttribute(AttrMaxTargetRange).get_double();
    GVector rangeToTarget( mySE->GetPosition(), tSE->GetPosition() );
    // adjust for target radius, in case of ice or other large objects..
    double targetDistance = rangeToTarget.length();
    if (tSE->IsAsteroidSE())
        targetDistance -= tSE->GetRadius();
    if (targetDistance > maxTargetRange) {
        mySE->GetPilot()->SendNotifyMsg("Your ship and skills combination can only target to %.0f meters.  %s is %.0f meters away.", \
        maxTargetRange, tSE->GetName(), targetDistance);
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but they are too far away.  Ignoring request.", \
                mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID());
        return false;
    }

    // Calculate Time to Lock target:
    float lockTime = TimeToLock( sRef, tSE );

    TargetEntry *te = new TargetEntry();
        te->state = TargMgr::State::Locking;
        te->timer.Start(lockTime *1000);      //timer has ms resolution
    m_targets[tSE] = te;
    tSE->TargetMgr()->TargetedAdd(mySE);

    _log(TARGET__INFO, "Pilot %s in %s(%u) started targeting %s(%u) (%.2fs lock time)", \
            mySE->GetPilot()->GetName(), mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID(), lockTime);

    sEntityList.AddTargMgr(mySE, this);

    Dump();

    return true;
}

bool TargetManager::StartTargeting(SystemEntity *tSE, float lockTime, uint8 maxLockedTargets, double maxTargetLockRange, bool &chase)
{       // NOTE  this is for npcs
    //first make sure they are not already in the list
    if (m_targets.find(tSE) != m_targets.end()) {
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but we are already targeting them. Ignoring request.", \
        mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID());
        return true;
    }
    // Check against max locked target count
    if (m_targets.size() >= maxLockedTargets){
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but we already have max targets.  Ignoring request.", \
        mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID());
        return false;
    }
    // Check against max target range
    if (mySE->GetPosition().distance(tSE->GetPosition()) > maxTargetLockRange){
        _log(TARGET__TRACE, " %s(%u): Told to target %s(%u), but they are too far away.  Begin Approaching.", \
        mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID());
        chase = true;
        return false;
    }

    TargetEntry *te = new TargetEntry();
        te->state = TargMgr::State::Locking;
        te->timer.Start(lockTime);
    m_targets[tSE] = te;
    tSE->TargetMgr()->TargetedAdd(mySE);

    _log(TARGET__INFO, "NPC %s(%u) started targeting %s(%u) (%.2fs lock time)", \
            mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID(), (lockTime /1000));

    sEntityList.AddTargMgr(mySE, this);

    Dump();

    return true;
}

void TargetManager::RemoveTarget(SystemEntity* tSE) {
    std::map<SystemEntity*, TargetEntry*>::iterator itr = m_targets.find(tSE);
    if (itr != m_targets.end()) {
        SafeDelete(itr->second);
        m_targets.erase(itr);
    }
    if (m_targets.empty()) {
        m_canAttack = false;
        // no targets to process.  remove from proc map
        sEntityList.DeleteTargMgr(mySE);
    }
    _log(TARGET__TRACE, "RemoveTarget:  %s(%u) has removed target %s(%u).", \
            mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID());
}

void TargetManager::ClearTarget(SystemEntity *tSE) {
    //let the other entity know they are no longer targeted.
    tSE->TargetMgr()->TargetedByLost(mySE);
    //clear it from our own state
    TargetLost(tSE);
    if (m_targets.empty()) {
        m_canAttack = false;
        // no targets to process.  remove from proc map
        sEntityList.DeleteTargMgr(mySE);
    }
    _log(TARGET__TRACE, "ClearTarget:  %s(%u) has cleared target %s(%u).", \
            mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID());
}

void TargetManager::ClearModules() {
    auto cur = m_modules.begin ();
    auto end = m_modules.end ();

    ActiveModule* module (nullptr);

    while (cur != end) {
        module = cur->second;

        cur = m_modules.erase (cur);

        module->AbortCycle ();
    }
}

void TargetManager::ClearAllTargets(bool notify/*true*/) {
    ClearTargets(notify);
    ClearFromTargets();
    if (notify)
        TargetsCleared();
    _log(TARGET__TRACE, "ClearAllTargets:  %s(%u) has cleared all targeting information.", mySE->GetName(), mySE->GetID());
}

void TargetManager::ClearTargets(bool notify/*true*/) {
    m_canAttack = false;

    for (auto cur : m_targets) {
        // failsafe   still chance this code is incomplete
        if (cur.first->TargetMgr() != nullptr)
            cur.first->TargetMgr()->TargetedByLost(mySE);
        SafeDelete(cur.second);
        _log(TARGET__TRACE, "ClearTargets() - %s(%u) has cleared target %s(%u).",
                mySE->GetName(), mySE->GetID(), cur.first->GetName(), cur.first->GetID());
    }

    m_targets.clear();

    // no targets to process.  remove from proc map
    sEntityList.DeleteTargMgr(mySE);
}

void TargetManager::ClearFromTargets() {
    if (m_targetedBy.empty())
        return;

    std::vector<SystemEntity *> ToNotify;
    for (auto cur : m_targetedBy) {
        SafeDelete(cur.second);
        //do not notify until we clear our target list! otherwise Bad Things happen. (invalidate iterator here)
        ToNotify.push_back(cur.first);
        _log(TARGET__TRACE, "ClearFrom() - %s(%u) has added %s(%u) to delete list.", \
                mySE->GetName(), mySE->GetID(), cur.first->GetName(), cur.first->GetID());
    }

    m_targetedBy.clear();

    for (auto cur : ToNotify)
        if (cur->TargetMgr() != nullptr)
            cur->TargetMgr()->TargetLost(mySE);
}

void TargetManager::TargetLost(SystemEntity *tSE) {
    std::map<SystemEntity *, TargetEntry *>::iterator itr = m_targets.find(tSE);
    if (itr == m_targets.end())
        return;

    SafeDelete(itr->second);
    m_targets.erase(itr);

    if (m_targets.empty()) {
        m_canAttack = false;
        // no targets to process.  remove from proc map
        sEntityList.DeleteTargMgr(mySE);
    }
    _log(TARGET__INFO, "%s(%u) has lost lock on %s(%u)", mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID());

    if (mySE->IsSentrySE())
        return;

    mySE->DestinyMgr()->EntityRemoved(tSE);
    if (mySE->IsNPCSE())
        mySE->GetNPCSE()->TargetLost(tSE);
    if (!mySE->HasPilot())
        return;
    Notify_OnTarget te;
        te.mode = "lost";
        te.targetID = tSE->GetID();
        //te.reason = "Docking";
    Notify_OnMultiEvent multi;
        multi.events = new PyList();
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();   //this is consumed below
    mySE->GetPilot()->SendNotification("OnMultiEvent", "clientID", &tmp);
}

void TargetManager::TargetedByLocked(SystemEntity* pSE) {
    _log(TARGET__TRACE, "%s(%u) has been locked by %s(%u)", \
            mySE->GetName(), mySE->GetID(), pSE->GetName(), pSE->GetID());
    // i think this is redundant....check
    //mySE->TargetMgr()->TargetedAdd(pSE);
}

void TargetManager::TargetedByLost(SystemEntity* pSE) {
    std::map<SystemEntity *, TargetedByEntry *>::iterator itr = m_targetedBy.find(pSE);
    if (itr == m_targetedBy.end()) {
        _log(TARGET__DEBUG, "%s(%u) TargetByLost() - Tried to notify %s(%u) of target lost, but they did not have us targeted.", \
                mySE->GetName(), mySE->GetID(), pSE->GetName(), pSE->GetID());
        return;
    }

    _log(TARGET__TRACE, "%s(%u) is no longer locked by %s(%u)", \
            mySE->GetName(), mySE->GetID(), pSE->GetName(), pSE->GetID());

    SafeDelete(itr->second);
    m_targetedBy.erase(itr);
    TargetedLost(pSE);
}

/*
    OnTarget.mode
        add - targeting successful
        clear - clear all targets
        lost - target lost
            - Docking
            - Destroyed
        otheradd - somebody else has targeted you
        otherlost - somebody else has stopped targeting you
            - WarpingOut
            - StoppedTargeting
*/

void TargetManager::TargetAdded(SystemEntity* tSE) {
    _log(TARGET__TRACE, "%s(%u) - adding target %s(%u).", \
            mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID());
    if (!mySE->HasPilot())
        return;
    PyTuple* up(nullptr);
    Notify_OnTarget te;
        te.mode = "add";
        te.targetID = tSE->GetID();
    up = te.Encode();
    mySE->GetPilot()->QueueDestinyEvent(&up);
    OnDamageStateChange odsc;
        odsc.entityID = tSE->GetID();
        odsc.state = tSE->MakeDamageState();
    up = odsc.Encode();
    mySE->GetPilot()->QueueDestinyUpdate(&up);
}

void TargetManager::TargetedAdd(SystemEntity *tSE) {
    //first make sure they are not already in the list
    if (m_targetedBy.find(tSE) != m_targetedBy.end()) {
        _log(TARGET__INFO, "Cannot add %s(%u) to %s(%u)'s locked list: they're already in there.", \
                tSE->GetName(), tSE->GetID(), mySE->GetName(), mySE->GetID());
        return;
    } else {
        //new entry.
        _log(TARGET__TRACE, "%s(%u) - %s(%u) has started target lock on me.", \
                mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID());
        TargetedByEntry *te = new TargetedByEntry();
        te->state = TargMgr::State::Locking;
        m_targetedBy[tSE] = te;
    }
    if (mySE->IsNPCSE())
        mySE->GetNPCSE()->TargetedAdd(tSE);
    if (!mySE->HasPilot())
        return;
    Notify_OnTarget te;
        te.mode = "otheradd";
        te.targetID = tSE->GetID();
    Notify_OnMultiEvent multi;
        multi.events = new PyList();
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();
    mySE->GetPilot()->SendNotification("OnMultiEvent", "clientID", &tmp);
}

void TargetManager::TargetedLost(SystemEntity *tSE) {
    _log(TARGET__TRACE, "%s(%u) - %s(%u) has lost target lock on me.", \
            mySE->GetName(), mySE->GetID(), tSE->GetName(), tSE->GetID());
    if (!mySE->HasPilot())
        return;
    Notify_OnTarget te;
        te.mode = "otherlost";
        te.targetID = tSE->GetID();
       // te.reason = "WarpingOut";
       // te.reason = "StoppedTargeting";
    Notify_OnMultiEvent multi;
        multi.events = new PyList();
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();
    mySE->GetPilot()->SendNotification("OnMultiEvent", "clientID", &tmp);
}

void TargetManager::TargetsCleared() {
    _log(TARGET__TRACE, "%s(%u) - i am clearing all target data.", mySE->GetName(), mySE->GetID());
    if (!mySE->HasPilot())
        return;
    Notify_OnTarget te;
        te.mode = "clear";
        te.targetID = 0;
    Notify_OnMultiEvent multi;
        multi.events = new PyList();
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();
    mySE->GetPilot()->SendNotification("OnMultiEvent", "clientID", &tmp);
}

bool TargetManager::IsTargetedBy(SystemEntity* pSE)
{
    return (m_targetedBy.find(pSE) != m_targetedBy.end());
}

SystemEntity* TargetManager::GetFirstTarget(bool need_locked/*false*/) {
    if (m_targets.empty())
        return nullptr;

    if (!need_locked)
        return m_targets.begin()->first;

    std::map<SystemEntity *, TargetEntry *>::iterator itr = m_targets.begin();
    for (; itr != m_targets.end(); ++itr)
        if (itr->second->state == TargMgr::State::Locked)
            return itr->first;

    return nullptr;
}

PyList* TargetManager::GetTargets() const {
    PyList* result = new PyList();
    if (m_targets.empty())
        return result;

    std::map<SystemEntity *, TargetEntry *>::const_iterator itr = m_targets.begin();
    for (; itr != m_targets.end(); ++itr)
        result->AddItemInt( itr->first->GetID() );

    return result;
}

PyList* TargetManager::GetTargeters() const {
    PyList* result = new PyList();
    if (m_targetedBy.empty())
        return result;

    std::map<SystemEntity*, TargetedByEntry*>::const_iterator itr = m_targetedBy.begin();
    for(; itr != m_targetedBy.end(); ++itr)
        result->AddItemInt( itr->first->GetID() );

    return result;
}

// no longer used.  1Feb18
// will be used by advanced NPCs....eventually
SystemEntity* TargetManager::GetTarget(uint32 targetID, bool need_locked/*true*/) const {
    if (m_targets.empty())
        return nullptr;

    std::map<SystemEntity*, TargetEntry*>::const_iterator itr = m_targets.begin();
    for (; itr != m_targets.end(); ++itr) {
        if (itr->first->GetID() != targetID)
            continue;
        //found it...
        if (need_locked and (itr->second->state != TargMgr::State::Locked)) {
            _log(TARGET__INFO, "Found target %u, but it is not locked.", targetID);
            continue;
        }
        _log(TARGET__INFO, "Found target %u: %s (nl? %s)", targetID, itr->first->GetName(), need_locked?"yes":"no");
        return itr->first;
    }
    _log(TARGET__INFO, "Unable to find target %u (nl? %s)", targetID, need_locked?"yes":"no");
    return nullptr;    //not found.
}

void TargetManager::AddTargetModule(ActiveModule* pMod)
{
    _log(TARGET__INFO, "Adding %s:%s to %s's activeModule list.", \
            pMod->GetShipRef()->name(), pMod->GetSelf()->name(), mySE->GetName() );
    // i think this check is redundant...shouldnt be able to activate non-miner on roid.
    if (mySE->IsAsteroidSE())
        if (!pMod->IsMiningLaser())
            return;

    m_modules.emplace(pMod->itemID(), pMod);
}

void TargetManager::RemoveTargetModule(ActiveModule* pMod)
{
    _log(TARGET__INFO, "Removing the %s on %s from %s's activeModule list.", \
            pMod->GetSelf()->name(), pMod->GetShipRef()->name(), mySE->GetName() );
    m_modules.erase(pMod->itemID());
}

void TargetManager::Destroyed()
{
    _log(TARGET__INFO, "%s(%u) has been destroyed. %u modules, %u targets, and %u targeters in maps.", \
            mySE->GetName(), mySE->GetID(), m_modules.size(), m_targets.size(), m_targetedBy.size());

    std::string effect = "TargetDestroyed";

    ClearAllTargets();

    // iterate thru the map of modules targeting this object, and call Deactivate on each.
    auto cur = m_modules.begin ();
    auto end = m_modules.end ();

    ActiveModule* module (nullptr);

    while (cur != end) {
        // TODO: THIS IS A HACK TO FIX A PROBLEM ON THE TARGET MANAGER
        // TODO: WHEN A MODULE'S CYCLE IS ABORTED BY THIS FUNCTION, IT ENDS UP CALLING
        // TODO: ActiveModule::Clear DOWN THE ROAD, WHICH IN TURN REMOVES ITEMS FROM THE m_modules
        // TODO: MAP WHILE WE'RE ITERATING IT, AND THAT'S A NO-NO UNLESS YOU CAN GET THE NEW
        // TODO: ITERATOR FROM THE ERASE FUNCTION, THAT'S WHY IT'S HANDLED HERE INSTEAD OF LETTING
        // TODO: THE ActiveModule::Clear TAKE CARE OF IT
        module = cur->second;

        // this should advance the iterator without needing to do any cur++ or anything
        cur = m_modules.erase (cur);
        //  some modules should immediately cease cycle when target destroyed.  miners are NOT in this call
        switch (module->groupID()) {
            case EVEDB::invGroups::Target_Painter:
            case EVEDB::invGroups::Tracking_Disruptor:
            case EVEDB::invGroups::Remote_Sensor_Damper:
            case EVEDB::invGroups::Remote_Sensor_Booster:
            case EVEDB::invGroups::Armor_Repair_Projector:
            case EVEDB::invGroups::Shield_Transporter:
            case EVEDB::invGroups::Energy_Vampire:
            case EVEDB::invGroups::Energy_Transfer_Array:
            case EVEDB::invGroups::Energy_Destabilizer:
            case EVEDB::invGroups::Tractor_Beam:
            case EVEDB::invGroups::Projected_ECCM:
            case EVEDB::invGroups::Ship_Scanner:
            case EVEDB::invGroups::Cargo_Scanner: {
                module->AbortCycle();
            } break;
            case EVEDB::invGroups::Salvager:
                // set success=false and fall thru
                module->GetProspectModule()->TargetDestroyed();
            default: {
                module->Deactivate(effect);
            } break;
        }
    }

    Dump();
}

// specific for asteroids; only called by asteroids
void TargetManager::Depleted(MiningLaser* pMod)
{
    if (!mySE->IsAsteroidSE()) {
        codelog(MODULE__ERROR, "Depleted() called by Non Astroid %s", mySE->GetName());
        return;
    }
    // remove master module here to avoid placement in map
    m_modules.erase(pMod->itemID());

    std::multimap<float, MiningLaser*> mMap;
    // iterate thru the map of modules and add to map as MiningLasers with their mining volume
    std::map<uint32, ActiveModule*>::iterator itr = m_modules.begin();
    while (itr != m_modules.end()) {
        mMap.emplace(itr->second->GetMiningModule()->GetMiningVolume(), itr->second->GetMiningModule());
        itr = m_modules.erase(itr);     //remove module from map here to avoid segfault on rock delete
    }

    // call Depleted() on master module with map of active modules
    pMod->Depleted(mMap);
}

float TargetManager::TimeToLock ( ShipItemRef sRef, SystemEntity* tSE ) const {
    if ((tSE->IsAsteroidSE()) or (tSE->IsDeployableSE()) or (tSE->IsWreckSE())
        or  (tSE->IsContainerSE()) or (tSE->IsInanimateSE()))
        return 2.0f;

    //  fixed lock time  -allan 24Dec14  -updated 26May15   -revisited after new effects system implementation 25Mar17
    uint32 scanRes = sRef->GetAttribute(AttrScanResolution).get_uint32();
    uint32 sigRad = 25; // set base as capsule with 25m signature radius

    if ( tSE->GetSelf().get() != nullptr )
        if ( tSE->GetSelf()->HasAttribute(AttrSignatureRadius) )
            sigRad = tSE->GetSelf()->GetAttribute(AttrSignatureRadius).get_uint32();

        //https://wiki.eveonline.com/en/wiki/Targeting_speed
        //locktime = 40000/(scanres * asinh(sigrad)^2)
        float time = ( 40000 /(scanRes * std::pow(asinh(sigRad), 2)));   // higher scan res means faster lock time.

        /*  distance-based modifier to targeting speed?         sure, why the hell not?   -allan 27.6.15
         *  +0.1s for each 10k distance
         *     distance = pos - targ.pos
         *     disMod = distance /10k (for 10k increments)
         *     time += disMod * 0.1
         */
        double distance = sRef->position().distance( tSE->GetPosition());
        // check for snipers... >85k distance do NOT need additional 7.5+s to targettime
        // should we check LRT skill for pilots to modify this?  yes....not sure how to modify time using this yet...
        /*
         *    uint8 sLevel = 1;
         *    if (ship->HasPilot()) {
         *        sLevel += ship->GetPilot()->GetChar()->GetSkillLevel(EvESkill::LongRangeTargeting);   // bonus to target range
         *        sLevel += ship->GetPilot()->GetChar()->GetSkillLevel(EvESkill::SignatureAnalysis); // skill at operating target systems
         *        sLevel += ship->GetPilot()->GetChar()->GetSkillLevel(EvESkill::Electronics);   // basic ship sensor and computer systems
}
*/
        //if (mySE->IsNPCSE())      // not all snipers are npc
        if (distance > 85000)
            distance -= 75000;

        float disMod = distance /10000;
        if (disMod < 1)
            disMod = 0.0f;
        time += (disMod * 0.1f);

        return time;
}

void TargetManager::QueueEvent( PyTuple** event ) const
{
    for (auto cur : m_targetedBy)
        if (cur.first->HasPilot()) {
            PyIncRef(*event);
            cur.first->GetPilot()->QueueDestinyEvent(event);
        }
}

void TargetManager::QueueUpdate( PyTuple** update ) const
{
    for (auto cur : m_targetedBy)
        if (cur.first->HasPilot()) {
            PyIncRef(*update);
            cur.first->GetPilot()->QueueDestinyUpdate(update);
        }
}

/* debugging methods */
std::string TargetManager::TargetList(uint16 &length, uint16 &count) {
    std::ostringstream str;
    if (m_targets.empty()) {
        str << "Targets: <br>";
        str << "   *NONE*<br>";
        length += 23;
    } else {
        str << "Targets: <br>";
        length += 11;
        for (auto cur : m_targets) {
            str << "  " << cur.first->GetName();
            str << " (" << cur.first->GetID() << ") <br>";
            length += 35;
            ++count;
        }
    }
    if (m_targetedBy.empty())  {
        str << "Targeted by: <br>";
        str << "   *NONE*<br>";
        length += 28;
    } else {
        str << "Targeted by: <br>";
        length += 15;
        for (auto cur : m_targetedBy) {
            str << "  " << cur.first->GetName();
            str << " (" << cur.first->GetID() << ") <br>";
            length += 35;
            ++count;
        }
    }
    str << "Active Modules: (ship:module)<br>";
    length += 30;
    if (m_modules.empty()) {
        str << "   *NONE*";
        length += 10;
    } else {
        for (auto cur : m_modules) {
            str << "  " << cur.second->GetShipRef()->itemName();
            str << ":" << cur.second->GetSelf()->itemName() << "<br>";
            length += 55;
            ++count;
        }
    }

    return str.str();
}

void TargetManager::Dump() const {
    if (!is_log_enabled(TARGET__DUMP))
        return;

    _log(TARGET__DUMP, "Target Dump for %s(%u):", mySE->GetName(), mySE->GetID());
    if (m_targets.empty()) {
        _log(TARGET__DUMP, "    No Targets");
    } else {
        for (auto cur : m_targets)
            cur.second->Dump(cur.first);
    }
    if (m_targetedBy.empty()) {
        _log(TARGET__DUMP, "    No Targeters");
    } else {
        for (auto cur : m_targetedBy)
            cur.second->Dump(cur.first);
    }

    _log(TARGET__DUMP, "    Active Modules: (ship:module(moduleID))");
    if (m_modules.empty()) {
        _log(TARGET__DUMP, "      *NONE*");
    } else {
        for (auto cur : m_modules)
            _log(TARGET__DUMP, "\t\t %s: %s(%u)", cur.second->GetShipRef()->name(), cur.second->GetSelf()->name(), cur.second->itemID());
    }
}

void TargetManager::TargetEntry::Dump(SystemEntity* pSE) const {
    if (timer.Enabled()) {
        _log(TARGET__DUMP, "    Targeting %s(%u): %s - Timer Running with %ums remaining.", \
                pSE->GetName(), pSE->GetID(), TargetManager::GetStateName(state), timer.GetRemainingTime());
    } else {
        _log(TARGET__DUMP, "    Targeting %s(%u): %s", pSE->GetName(), pSE->GetID(), TargetManager::GetStateName(state));
    }
}

void TargetManager::TargetedByEntry::Dump( SystemEntity* pSE ) const {
    _log(TARGET__DUMP, "    Targeted By %s(%u): %s", pSE->GetName(), pSE->GetID(), TargetManager::GetStateName(state));
}

const char* TargetManager::GetStateName( uint8 state ) {
    switch(state) {
        case TargMgr::State::Idle:      return "Idle";
        case TargMgr::State::Locking:   return "Locking";
        case TargMgr::State::Passive:   return "Passive";
        case TargMgr::State::Locked:    return "Locked";
        default:                        return "Invalid";
    }
}

const char* TargetManager::GetModeName ( uint8 mode ) {

    switch(mode) {
        case TargMgr::Mode::None:       return "None";
        case TargMgr::Mode::Add:        return "Add";
        case TargMgr::Mode::Lost:       return "Lost";
        case TargMgr::Mode::Clear:      return "Clear";
        case TargMgr::Mode::OtherAdd:   return "OtherAdd";
        case TargMgr::Mode::OtherLost:  return "OtherLost";
        case TargMgr::Mode::LockedBy:   return "LockedBy";
        default:                        return "Invalid";
    }
}
