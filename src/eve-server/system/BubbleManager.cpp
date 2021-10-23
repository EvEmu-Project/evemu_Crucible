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

#include <algorithm>
#include <functional>
#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "EVE_Scanning.h"
#include "EVEServerConfig.h"
#include "system/BubbleManager.h"
#include "system/Container.h"
#include "system/SystemBubble.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"


BubbleManager::BubbleManager()
: m_wanderTimer(0),
m_emptyTimer(0),
m_bubbleID(0)
{
    m_bubbles.clear();
    m_wanderers.clear();
    m_bubbleIDMap.clear();
    m_sysBubbleMap.clear();
}

BubbleManager::~BubbleManager() {
}

int BubbleManager::Initialize() {
    // start timers
    m_emptyTimer.Start(60000);  //60s
    m_wanderTimer.Start(60000); //60s

    sLog.Blue("        BubbleMgr", "Bubble Manager Initialized.");
    return 1;
}

void BubbleManager::clear() {
    for (auto cur : m_bubbles)
        SafeDelete(cur);

    sLog.Warning("        BubbleMgr", "Bubble Manager has been closed." );
}

void BubbleManager::Process() {
    double profileStartTime(GetTimeUSeconds());

    for (auto cur : m_bubbles) {
        // process each belt and gate bubble for spawns
        if (cur->IsBelt() or cur->IsGate())
            cur->Process();
        // process each mission and incursion bubble  -placeholder
        if (cur->IsMission() or cur->IsIncursion())
            cur->Process();
    }

    if (m_wanderTimer.Check()) {    //60s
        m_wanderers.clear();
        std::list<SystemBubble*>::iterator itr = m_bubbles.begin();
        while (itr != m_bubbles.end()) {
            if (*itr == nullptr) {
                itr = m_bubbles.erase(itr);
                continue;
            }
            if ((*itr)->HasDynamics())
                (*itr)->ProcessWander(m_wanderers);
            ++itr;
        }

        if (!m_wanderers.empty()) {
            for (auto cur : m_wanderers) {
                // do we really want to check this?
                if (cur->GetPosition().isNaN() or cur->GetPosition().isInf() or cur->GetPosition().isZero()) {
                    // position error.  this will screw things up.  if haspilot, send error.
                    if (cur->HasPilot())
                        cur->GetPilot()->SendErrorMsg("Internal Server Error.  Ref: ServerError 35148<br>Please either dock or relog.");
                    continue;
                }
                _log(DESTINY__WARNING, "BubbleManager::Process() - Wanderer %s(%u) in system %s(%u) is being added to a bubble.", \
                        cur->GetName(), cur->GetID(), cur->SystemMgr()->GetName(), cur->SystemMgr()->GetID());
                CheckBubble(cur);
            }
            m_wanderers.clear();
        }
    }

    if (m_emptyTimer.Check())   //60s
        RemoveEmpty();

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::bubbles, GetTimeUSeconds() - profileStartTime);
}

void BubbleManager::CheckBubble(SystemEntity *pSE) {
    SystemBubble *pBubble = pSE->SysBubble();
    if (pBubble != nullptr) {
        if (pBubble->InBubble(pSE->GetPosition())) {
            _log(DESTINY__BUBBLE_DEBUG, "BubbleManager::CheckBubble() - Entity '%s'(%u) at (%.2f,%.2f,%.2f) is still located in bubble %u at %.2f,%.2f,%.2f.",\
                 pSE->GetName(), pSE->GetID(), pSE->GetPosition().x, pSE->GetPosition().y, pSE->GetPosition().z,\
                 pBubble->GetID(), pBubble->x(), pBubble->y(), pBubble->z());
            return;
        }

        _log(DESTINY__BUBBLE_DEBUG, "BubbleManager::CheckBubble() - Entity '%s'(%u) at (%.2f,%.2f,%.2f) is no longer located in bubble %u at %.2f,%.2f,%.2f.  Removing...",\
             pSE->GetName(), pSE->GetID(), pSE->GetPosition().x, pSE->GetPosition().y, pSE->GetPosition().z,\
             pBubble->GetID(), pBubble->x(), pBubble->y(), pBubble->z());
        pBubble->Remove(pSE);
    }

    _log(DESTINY__BUBBLE_DEBUG, "BubbleManager::CheckBubble() - SystemEntity '%s'(%u) not currently in any Bubble...adding", pSE->GetName(), pSE->GetID() );
    Add(pSE);
}

void BubbleManager::RemoveEmpty()
{
    std::list<SystemBubble*>::iterator itr = m_bubbles.begin();
    while (itr != m_bubbles.end()) {
        if ((*itr)->IsEmpty()) {
            _log(DESTINY__BUBBLE_DEBUG, "BubbleManager::RemoveEmpty() - Bubble %u is empty and is being deleted from the system.", (*itr)->GetID() );
            RemoveBubble((*itr)->GetSystem()->GetID(), (*itr));
            itr = m_bubbles.erase(itr);
        } else {
            ++itr;
        }
    }
}

void BubbleManager::Add(SystemEntity* pSE, bool isPostWarp /*false*/) {
    if (pSE == nullptr)
        return;

    if (pSE->GetPosition().isZero()) {
        SystemGPoint sGP;
        pSE->DestinyMgr()->SetPosition(sGP.GetRandPointOnPlanet(pSE->SystemMgr()->GetID()));
    }

    GPoint center(pSE->GetPosition());
    if (isPostWarp) {
        // Calculate new bubble's center based on entity's velocity and current position
        NewBubbleCenter( pSE->GetVelocity(), center );
    }

    SystemBubble* pBubble(GetBubble(pSE->SystemMgr(), center));
    if (pBubble != nullptr) {
        if (pSE->SysBubble() != nullptr) {
            if (pBubble->GetSystemID() != pSE->SystemMgr()->GetID()) {
                // this is an error.  bad bubble
                _log(DESTINY__ERROR, "BubbleManager::Add(): bubble SysID %u != pSE SysID %u", pBubble->GetSystemID(), pSE->SystemMgr()->GetID() );
                pSE->SysBubble()->Remove(pSE);
            } else if (pSE->SysBubble() != pBubble) {
                _log(DESTINY__BUBBLE_TRACE, "BubbleManager::Add(): bubbleID %u != pSE bubbleID %u", pBubble->GetID(), pSE->SysBubble()->GetID() );
                pSE->SysBubble()->Remove(pSE);
            } else if (pSE->SysBubble()->InBubble(pSE->GetPosition()))  {
                _log(DESTINY__BUBBLE_TRACE, "BubbleManager::Add(): Entity %s(%u) still in Bubble %u", pSE->GetName(), pSE->GetID(), pBubble->GetID() );
                return;
            }
        }
        _log(DESTINY__BUBBLE_TRACE, "BubbleManager::Add(): Entity %s(%u) being added to Bubble %u", pSE->GetName(), pSE->GetID(), pBubble->GetID() );
        pBubble->Add(pSE);
    } else {
        _log(DESTINY__ERROR, "BubbleManager::Add(): GetBubble() returned nullptr for %s:%u, at (%.2f, %.2f, %.2f).", \
                    pSE->SystemMgr()->GetName(), pSE->SystemMgr()->GetID(), center.x, center.y, center.z );
    }
}

void BubbleManager::NewBubbleCenter(GVector shipVelocity, GPoint &newCenter) {
    shipVelocity.normalize();
    newCenter += (shipVelocity * (BUBBLE_RADIUS_METERS /2));
}

void BubbleManager::Remove(SystemEntity *ent) {
    // suns, planets and moons arent in bubbles
    //if (ent->IsStaticEntity())
    //    return;
    if (ent->SysBubble() != nullptr) {
        _log(DESTINY__BUBBLE_TRACE, "BubbleManager::Remove(): Entity %s(%u) being removed from Bubble %u", ent->GetName(), ent->GetID(), ent->SysBubble()->GetID() );
        ent->SysBubble()->Remove(ent);
    }
}

/** UPDATE  large items, (sun, planet, moons) are no longer in bubbles.
 * this cuts number of bubbles drastically, dropping average to 10bbl/system,
 * and allowing a much larger amount of bubbles per system without
 * introducing lag from bubble processing.
 *
 * NOTE:  these are only used here...
 */
SystemBubble* BubbleManager::FindBubble(SystemEntity *ent) const {
    return FindBubble(ent->SystemMgr()->GetID(), ent->GetPosition());
}

SystemBubble* BubbleManager::FindBubble(uint32 systemID, const GPoint &pos) const {
    // Finds a range containing all elements whose key is k.
    // pair<iterator, iterator> equal_range(const key_type& k)
    _log(DESTINY__BUBBLE_DEBUG, "BubbleManager::FindBubble() - Searching point %.1f, %.1f, %.1f in system %u.", \
                pos.x, pos.y, pos.z, systemID);

    auto range = m_sysBubbleMap.equal_range(systemID);
    for ( auto itr = range.first; itr != range.second; ++itr )
        if (itr->second->InBubble(pos))
            return itr->second;

    //not in any existing bubble.
    return nullptr;
}

SystemBubble* BubbleManager::GetBubble(SystemManager* sysMgr, const GPoint& pos)
{
    SystemBubble* pBubble(FindBubble(sysMgr->GetID(), pos));
    if (pBubble == nullptr)
        pBubble = MakeBubble(sysMgr, pos);

    return pBubble;
}

SystemBubble* BubbleManager::MakeBubble(SystemManager* sysMgr, GPoint pos) {
    // determine if new center (pos) is within 2x radius of another bubble center. (overlap)
    auto range = m_sysBubbleMap.equal_range(sysMgr->GetID());
    for ( auto itr = range.first; itr != range.second; ++itr )
        if (itr->second->IsOverlap(pos)) {
            GVector dir(itr->second->GetCenter(), pos);
            dir.normalize();
            _log(DESTINY__BUBBLE_DEBUG, "BubbleManager::MakeBubble()::IsOverlap() - dir: %.3f,%.3f,%.3f", dir.x, dir.y, dir.z);
            // move pos away from center
            pos = itr->second->GetCenter() + (dir * (BUBBLE_RADIUS_METERS * 2));
            break;
        }

    SystemBubble* pBubble = new SystemBubble(sysMgr, pos, BUBBLE_RADIUS_METERS);
    if (pBubble != nullptr) {
        m_bubbles.push_back(pBubble);
        m_bubbleIDMap.emplace(pBubble->GetID(), pBubble);
        m_sysBubbleMap.emplace(sysMgr->GetID(), pBubble);
        if (sConfig.debug.BubbleTrack)
            pBubble->MarkCenter();
    }
    return pBubble;
}

SystemBubble* BubbleManager::FindBubbleByID(uint16 bubbleID)
{
    std::map<uint32, SystemBubble*>::iterator itr = m_bubbleIDMap.find(bubbleID);
    if (itr != m_bubbleIDMap.end())
        return itr->second;
    return nullptr;
}

void BubbleManager::ClearSystemBubbles(uint32 systemID)
{
    auto range = m_sysBubbleMap.equal_range(systemID);
    for (auto itr = range.first; itr != range.second; ++itr){
        m_bubbles.remove(itr->second);
        m_bubbleIDMap.erase(itr->second->GetID());
    }

    m_sysBubbleMap.erase(systemID);
}

void BubbleManager::RemoveBubble(uint32 systemID, SystemBubble* pSB)
{
    auto range = m_sysBubbleMap.equal_range(systemID);
    for (auto itr = range.first; itr != range.second; ++itr)
        if (itr->second == pSB) {
            m_sysBubbleMap.erase(itr);
            return;
        }
    std::map<uint32, SystemBubble*>::iterator itr = m_bubbleIDMap.find(pSB->GetID());
    if (itr != m_bubbleIDMap.end())
        m_bubbleIDMap.erase(itr);
}

/* for beltmgr */
void BubbleManager::AddSpawnID(uint16 bubbleID, uint32 spawnID)
{
    m_spawnIDs.emplace(bubbleID, spawnID);
}

void BubbleManager::RemoveSpawnID(uint16 bubbleID, uint32 spawnID)
{
    // is this right??
    auto range = m_spawnIDs.equal_range(bubbleID);
    for (auto itr = range.first; itr != range.second; ++itr )
        if (itr->second == spawnID) {
            m_spawnIDs.erase(itr);
            return;
        }
}

uint32 BubbleManager::GetBeltID(uint16 bubbleID)
{
    std::map<uint16, uint32>::iterator itr = m_spawnIDs.find(bubbleID);
    if (itr == m_spawnIDs.end())
        return 0;
    return itr->second;
}

uint32 BubbleManager::GetBubbleCount(uint32 systemID) {
    uint32 count = 0;
    auto range = m_sysBubbleMap.equal_range(systemID);
    for (auto itr = range.first; itr != range.second; ++itr)
        ++count;
    return count;
}

void BubbleManager::GetBubbleCenterMarkers(std::vector<CosmicSignature>& anom) {
    ContainerSE* cSE(nullptr);
    for (auto cur : m_sysBubbleMap) {
        cSE = cur.second->GetCenterMarker();
        if (cSE == nullptr)
            continue;
        CosmicSignature sig = CosmicSignature();
            sig.ownerID = cSE->GetOwnerID();
            sig.sigID = cSE->GetSelf()->customInfo();           // result.id
            sig.sigItemID = cSE->GetID();
            sig.sigName = cSE->GetName();                       // result.DungeonName
            //sig.sigGroupID = EVEDB::invGroups::Cosmic_Anomaly;  // result.groupID
            sig.sigStrength = 1.0;
            //sig.sigTypeID = EVEDB::invTypes::CosmicAnomaly;     // result.typeID
            sig.systemID = cur.first;
            sig.position = cSE->GetPosition();
            sig.scanAttributeID = AttrScanMagnetometricStrength;   // result.strengthAttributeID
            sig.scanGroupID = Scanning::Group::Signature;
        anom.push_back(sig);
        cSE = nullptr;
    }
}

void BubbleManager::GetBubbleCenterMarkers(uint32 systemID, std::vector<CosmicSignature>& anom) {
    ContainerSE* cSE(nullptr);
    auto range = m_sysBubbleMap.equal_range(systemID);
    for (auto itr = range.first; itr != range.second; ++itr) {
        cSE = itr->second->GetCenterMarker();
        if (cSE == nullptr)
            continue;
        CosmicSignature sig = CosmicSignature();
            sig.ownerID = cSE->GetOwnerID();
            sig.sigID = cSE->GetSelf()->customInfo();
            sig.sigItemID = cSE->GetID();
            sig.sigName = cSE->GetName();
            //sig.sigGroupID = EVEDB::invGroups::Cosmic_Anomaly;
            sig.sigStrength = 1.0;
            //sig.sigTypeID = EVEDB::invTypes::CosmicAnomaly;
            sig.systemID = systemID;
            sig.position = cSE->GetPosition();
            sig.scanAttributeID = AttrScanMagnetometricStrength;
            sig.scanGroupID = Scanning::Group::Signature;
        anom.push_back(sig);
        cSE = nullptr;
    }
}


void BubbleManager::MarkCenters() {
    for (auto cur : m_sysBubbleMap)
        cur.second->MarkCenter();
}

void BubbleManager::RemoveMarkers() {
    for (auto cur : m_sysBubbleMap)
        cur.second->RemoveMarkers();
}

void BubbleManager::MarkCenters(uint32 systemID) {
    auto range = m_sysBubbleMap.equal_range(systemID);
    for (auto itr = range.first; itr != range.second; ++itr)
        itr->second->MarkCenter();
}

void BubbleManager::RemoveMarkers(uint32 systemID) {
    auto range = m_sysBubbleMap.equal_range(systemID);
    for (auto itr = range.first; itr != range.second; ++itr)
        itr->second->RemoveMarkers();
}
