/**
 * DroneAI.cpp
 *      this class is for drone AI
 *
 * @Author:     Allan
 * @Version:    0.15
 * @Date:       27Nov19
*/

#include "eve-server.h"

#include "Client.h"
#include "character/Character.h"
#include "inventory/AttributeEnum.h"
#include "system/DestinyManager.h"
#include "npc/Drone.h"
#include "npc/DroneAI.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

namespace {
// Drone control range is a character attribute (skills/modules may modify it).
// Keep a conservative 20 km baseline only for old characters whose imported
// static attributes do not contain the value.
double GetDroneControlRange(DroneSE* drone) {
    double effectiveRange = 0.0;
    if (drone != nullptr && drone->GetHomeShip() != nullptr
        && drone->GetHomeShip()->GetSelf().get() != nullptr) {
        // The ship value may include fitted drone-link range bonuses.
        effectiveRange = drone->GetHomeShip()->GetSelf()
            ->GetAttribute(AttrDroneControlDistance).get_double();
    }
    if (drone != nullptr && drone->GetOwner() != nullptr) {
        Character* character = drone->GetOwner()->GetChar().get();
        if (character != nullptr) {
            // Character skills may be reflected on the character rather than
            // copied into the ship attribute in older database imports.
            const double characterRange = character
                ->GetAttribute(AttrDroneControlDistance).get_double();
            if (characterRange > effectiveRange)
                effectiveRange = characterRange;
        }
    }
    return (effectiveRange > 0.0) ? effectiveRange : 20000.0;
}

bool IsTargetWithinDroneControlRange(DroneSE* drone, SystemEntity* target) {
    if (drone == nullptr || target == nullptr || drone->GetHomeShip() == nullptr)
        return false;
    return drone->GetHomeShip()->GetPosition().distance(target->GetPosition())
        <= GetDroneControlRange(drone);
}

// When Focus Fire is enabled, all drones controlled by the same ship use the
// first valid target already assigned to any member of that flight. The AI
// stores the assignment before lock completion so the other drones can join
// immediately instead of independently selecting different NPCs.
SystemEntity* FindSharedDroneTarget(DroneSE* drone) {
    if (drone == nullptr || drone->GetSystemManager() == nullptr
        || drone->GetHomeShip() == nullptr || drone->SysBubble() == nullptr)
        return nullptr;

    SystemManager* system = drone->GetSystemManager();
    const uint32 controllerID = drone->GetHomeShip()->GetID();
    const std::map<uint32, SystemEntity*> entities = system->GetEntities();
    for (const auto& entry : entities) {
        SystemEntity* entity = entry.second;
        if (entity == nullptr || !entity->IsDroneSE())
            continue;

        DroneSE* wingDrone = entity->GetDroneSE();
        if (wingDrone == nullptr || !wingDrone->IsEnabled()
            || wingDrone->GetControllerID() != controllerID
            || wingDrone->GetAI() == nullptr)
            continue;

        const uint32 targetID = wingDrone->GetAI()->GetAssignedTargetID();
        if (targetID == 0)
            continue;

        SystemEntity* target = system->GetSE(targetID);
        if (target == nullptr || target->IsDead()
            || target->SysBubble() != drone->SysBubble()
            || target->DestinyMgr() == nullptr || target->DestinyMgr()->IsCloaked()
            || !IsTargetWithinDroneControlRange(drone, target))
            continue;

        return target;
    }
    return nullptr;
}

}

DroneAIMgr::DroneAIMgr(DroneSE* who)
: m_state(DroneAI::State::Idle),
  m_pDrone(who),
  m_assignedShip(nullptr),
  m_pendingTargetID(0),
  m_assignedTargetID(0),
  m_returnToBay(false),
  m_scoopRequested(false),
  m_autoEngageEnabled(true),
  m_mainAttackTimer(0),// dont start timer until we have a target
  m_processTimer(0),
  m_beginFindTarget(0),
  m_warpScramblerTimer(0),     //not implemented yet
  m_webifierTimer(0),             //not implemented yet
  m_sigRadius(who->GetSelf()->GetAttribute(AttrSignatureRadius).get_float()),
  m_attackSpeed(who->GetSelf()->GetAttribute(AttrSpeed).get_float()),
  m_cruiseSpeed(who->GetSelf()->GetAttribute(AttrEntityCruiseSpeed).get_int()),
  m_chaseSpeed(who->GetSelf()->GetAttribute(AttrMaxVelocity).get_int()),
  m_entityFlyRange(who->GetSelf()->GetAttribute(AttrEntityFlyRange).get_float() + who->GetSelf()->GetAttribute(AttrMaxRange).get_float()),
  m_entityChaseRange(who->GetSelf()->GetAttribute(AttrEntityChaseMaxDistance).get_float() *2),
  m_entityOrbitRange(who->GetSelf()->GetAttribute(AttrOrbitRange).get_float()),
  m_entityAttackRange(who->GetSelf()->GetAttribute(AttrEntityAttackRange).get_float() *2),
  m_shieldBoosterDuration(who->GetSelf()->GetAttribute(AttrEntityShieldBoostDuration).get_int()),
  m_armorRepairDuration(who->GetSelf()->GetAttribute(AttrEntityArmorRepairDuration).get_int())
{
    m_processTimer.Start(5000);     //arbitrary.

    // Drone static data is inconsistent between legacy and newer types.
    // Apply safe combat defaults when an attribute is absent/zero so a drone
    // never gets a zero pursuit range, zero orbit radius or zero weapon cycle.
    if (m_attackSpeed <= 0.0)
        m_attackSpeed = 2000.0; // milliseconds
    if (m_chaseSpeed == 0)
        m_chaseSpeed = 2000;
    if (m_cruiseSpeed == 0)
        m_cruiseSpeed = m_chaseSpeed / 2;
    if (m_entityAttackRange <= 0.0)
        m_entityAttackRange = m_entityOrbitRange;
    if (m_entityAttackRange < 10000.0)
        m_entityAttackRange *= 3.0;
    if (m_entityAttackRange <= 0.0)
        m_entityAttackRange = 5000.0;
    if (m_entityFlyRange <= 0.0)
        m_entityFlyRange = m_entityAttackRange;
    if (m_entityChaseRange <= 0.0)
        m_entityChaseRange = (m_entityAttackRange > 60000.0)
            ? m_entityAttackRange : 60000.0;
    if (m_entityOrbitRange <= 0.0) {
        // Match DroneSE's orbit fallback: use the smaller of weapon range and
        // falloff when static data omits orbitRange, not maxRange by itself.
        const double maxRange = who->GetSelf()->GetAttribute(AttrMaxRange).get_double();
        const double falloff = who->GetSelf()->GetAttribute(AttrFalloff).get_double();
        m_entityOrbitRange = (maxRange < falloff) ? maxRange : falloff;
        if (m_entityOrbitRange > 2500.0)
            m_entityOrbitRange = 2500.0;
    }
    if (m_entityOrbitRange <= 0.0)
        m_entityOrbitRange = 500.0;
}

void DroneAIMgr::Process() {
    double profileStartTime(GetTimeUSeconds());

    /* Drone::State definitions   -allan 27Nov19
     *   Invalid
     *   Idle              = 0,  // not doing anything....idle.
     *   Combat            = 1,  // fighting - needs targetID
     *   Mining            = 2,  // unsure - needs targetID
     *   Approaching       = 3,  // too close to chase, but to far to engage
     *   Departing         = 4,  // return to ship
     *   Departing2        = 5,  // leaving.  different from Departing
     *   Pursuit           = 6,  // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
     *   Fleeing           = 7,  // running away
     *   Operating         = 9,  // whats diff from engaged here?  mining maybe?
     *   Engaged           = 10, // non-combat? - needs targetID
     *   // internal only
     *   Unknown           = 8,  // as stated
     *   Guarding          = 11,
     *   Assisting         = 12,
     *   Incapacitated     = 13  // out of control range, but online
     */

    // Focus-fire is a flight-level decision. Keep every controlled drone on
    // the same valid assigned target, including while that target is locking.
    if (m_autoEngageEnabled && m_state != DroneAI::State::Departing
        && m_pDrone->GetSelf().get() != nullptr
        && m_pDrone->GetSelf()->GetAttribute(AttrDroneFocusFire).get_bool()) {
        SystemEntity* sharedTarget = FindSharedDroneTarget(m_pDrone);
        if (sharedTarget != nullptr && sharedTarget->GetID() != m_assignedTargetID) {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Focus Fire joining flight target %s(%u).",
                 m_pDrone->GetName(), m_pDrone->GetID(), sharedTarget->GetName(), sharedTarget->GetID());
            Target(sharedTarget);
        }
    }

    // A target outside immediate lock range is retained while the drone
    // approaches. Without this pending-target path, StartTargeting() returns
    // false and the drone silently abandons otherwise valid engage commands.
    if (m_pendingTargetID != 0) {
        SystemManager* system = m_pDrone->GetSystemManager();
        SystemEntity* pending = (system != nullptr) ? system->GetSE(m_pendingTargetID) : nullptr;
        if (pending == nullptr || pending->SysBubble() == nullptr
            || pending->SysBubble() != m_pDrone->SysBubble()
            || pending->DestinyMgr() == nullptr) {
            m_pendingTargetID = 0;
            SetIdle();
            return;
        }
        const double commandRange = (m_entityChaseRange > m_entityAttackRange)
            ? m_entityChaseRange : m_entityAttackRange;
        const double distance = m_pDrone->GetPosition().distance(pending->GetPosition());
        if (distance <= commandRange) {
            m_pendingTargetID = 0;
            Target(pending);
        } else {
            if (m_pDrone->DestinyMgr() != nullptr
                && (!m_pDrone->DestinyMgr()->IsFollowing()
                    || m_pDrone->DestinyMgr()->GetTargetID() != pending->GetID())) {
                m_pDrone->DestinyMgr()->SetMaxVelocity(m_chaseSpeed);
                m_pDrone->DestinyMgr()->Follow(pending, m_entityFlyRange);
                m_state = DroneAI::State::Approaching;
                m_pDrone->StateChange();
            }
            return;
        }
    }

    // test for drone attributes here - aggressive, focus fire, attack/follow
    // test for control distance here also.  offline drones outside this  (AttrDroneControlDistance)
    switch(m_state) {
        case DroneAI::State::Invalid: {
            // check everything in this state.   return to ship?
        } break;
        case DroneAI::State::Idle: {
            // Combat drones automatically acquire the nearest hostile NPC in
            // their current bubble. After a target dies/losses lock, the next
            // idle tick selects the next NPC, so a flight clears the pocket.
            if (m_autoEngageEnabled && m_pDrone->GetSelf().get() != nullptr
                && m_pDrone->GetSelf()->GetAttribute(AttrDroneIsAgressive).get_bool()
                && m_pDrone->GetHomeShip() != nullptr
                && m_pDrone->GetSystemManager() != nullptr
                && m_pDrone->SysBubble() != nullptr) {
                SystemEntity* nearestNPC = nullptr;
                double nearestDistance = 1.0e100;
                const std::map<uint32, SystemEntity*> entities = m_pDrone->GetSystemManager()->GetEntities();
                for (const auto& entry : entities) {
                    SystemEntity* candidate = entry.second;
                    if (candidate == nullptr || candidate == m_pDrone || !candidate->IsNPCSE()
                        || candidate->IsDead() || candidate->SysBubble() != m_pDrone->SysBubble()
                        || candidate->DestinyMgr() == nullptr || candidate->DestinyMgr()->IsCloaked())
                        continue;
                    // Do not attack NPCs explicitly owned by the drone's pilot,
                    // or NPCs belonging to the same non-zero faction.
                    if (candidate->GetOwnerID() == m_pDrone->GetOwnerID())
                        continue;
                    const int32 npcFaction = candidate->GetWarFactionID();
                    const int32 droneFaction = m_pDrone->GetWarFactionID();
                    if (npcFaction != 0 && droneFaction != 0 && npcFaction == droneFaction)
                        continue;
                    // Automatic acquisition must stay inside the pilot's drone
                    // control range. Scanning every NPC in the bubble caused
                    // drones to select distant pockets and fly across the system.
                    if (!IsTargetWithinDroneControlRange(m_pDrone, candidate))
                        continue;
                    const double distance = m_pDrone->GetPosition().distance(candidate->GetPosition());
                    if (distance < nearestDistance) {
                        nearestDistance = distance;
                        nearestNPC = candidate;
                    }
                }
                if (nearestNPC != nullptr) {
                    _log(DRONE__AI_TRACE, "Drone %s(%u): Auto-acquiring hostile NPC %s(%u) at %.0f m.",
                         m_pDrone->GetName(), m_pDrone->GetID(), nearestNPC->GetName(), nearestNPC->GetID(), nearestDistance);
                    Target(nearestNPC);
                }
            }
        } break;
        // Combat (state 1) is the client-defined active weapon state. Engaged
        // (state 10) is retained for compatibility with drones already in that state.
        case DroneAI::State::Combat:
        case DroneAI::State::Engaged:
        case DroneAI::State::Approaching:
        case DroneAI::State::Pursuit: {
            // TargetManager can be removed after its final target is pruned.
            // Never assume it still exists just because the AI was previously
            // in a combat state.
            TargetManager* targetMgr = m_pDrone->TargetMgr();
            if (targetMgr == nullptr) {
                SetIdle();
                return;
            }

            // Only process the assigned, fully locked target. Pending locks
            // must not be treated as active combat.
            SystemEntity* pTarget = (m_assignedTargetID != 0)
                ? targetMgr->GetTarget(m_assignedTargetID, true) : nullptr;
            if (pTarget == nullptr && m_assignedTargetID == 0)
                pTarget = targetMgr->GetFirstTarget(true);
            if (pTarget != nullptr && m_assignedTargetID == 0)
                m_assignedTargetID = pTarget->GetID();
            if (pTarget == nullptr) {
                if (targetMgr->HasNoTargets()) {
                    _log(DRONE__AI_TRACE, "Drone %s(%u): Stopped engagement, assigned target is not locked.",
                         m_pDrone->GetName(), m_pDrone->GetID());
                    SetIdle();
                }
                return;
            } else if (pTarget->SysBubble() == nullptr ||
                       pTarget->SysBubble() != m_pDrone->SysBubble()) {
                ClearTarget(pTarget);
                return;
            }
            CheckDistance(pTarget);
        } break;

        case DroneAI::State::Departing: { // return to ship.  when close enough, set lazy orbit
            // The owning ship may have been destroyed or the drone may have been orphaned.
            m_assignedShip = m_pDrone->GetHomeShip();
            if (m_assignedShip == nullptr) {
                _log(DRONE__AI_TRACE, "Drone %s(%u): Return target is unavailable; stopping return state.",
                     m_pDrone->GetName(), m_pDrone->GetID());
                SetIdle();
                break;
            }
            if (m_pDrone->GetPosition().distance(m_assignedShip->GetPosition()) <= 2500.0) {
                if (m_returnToBay) {
                    m_scoopRequested = true;
                    return;
                }
                SetIdle();
            }
        } break;
        // not sure how im gonna do these...
        case DroneAI::State::Fleeing:
        case DroneAI::State::Operating:
        case DroneAI::State::Unknown:
        case DroneAI::State::Incapacitated:
        case DroneAI::State::Guarding:
        case DroneAI::State::Assisting:
        case DroneAI::State::Mining:
        case DroneAI::State::Departing2: {
           // do nothing here yet
        } break;

    //no default on purpose
    }
    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::drone, GetTimeUSeconds() - profileStartTime);
}

int8 DroneAIMgr::GetState() {
    switch (m_state) {
        case DroneAI::State::Invalid:
        case DroneAI::State::Unknown:
        case DroneAI::State::Incapacitated:
            return DroneAI::State::Idle;
        case DroneAI::State::Guarding:
        case DroneAI::State::Assisting:
            return DroneAI::State::Engaged;
        default:
            return m_state;
    }
}

void DroneAIMgr::Return(bool returnToBay) {
    m_assignedTargetID = 0;
    // A deliberate return order suppresses automatic reacquisition until the
    // drone is launched/online again or receives another engage order.
    m_autoEngageEnabled = false;
    m_assignedShip = m_pDrone->GetHomeShip();
    if (m_assignedShip == nullptr || m_pDrone->DestinyMgr() == nullptr) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Cannot return; home ship or destiny manager is unavailable.",
             m_pDrone->GetName(), m_pDrone->GetID());
        SetIdle();
        return;
    }

    if (m_pDrone->TargetMgr() != nullptr)
        m_pDrone->TargetMgr()->ClearAllTargets();

    m_pendingTargetID = 0;
    m_returnToBay = returnToBay;
    m_scoopRequested = false;
    m_mainAttackTimer.Disable();
    m_beginFindTarget.Disable();
    m_webifierTimer.Disable();
    m_warpScramblerTimer.Disable();
    m_pDrone->DestinyMgr()->SetMaxVelocity(m_chaseSpeed);
    m_pDrone->DestinyMgr()->Follow(m_assignedShip, returnToBay ? 1500.0 : m_entityOrbitRange);
    m_state = DroneAI::State::Departing;
    m_pDrone->StateChange();
}

void DroneAIMgr::SetIdle() {
    if (m_state == DroneAI::State::Idle) {
        m_pendingTargetID = 0;
        m_assignedTargetID = 0;
        return;
    }
    // not doing anything....idle.
    _log(DRONE__AI_TRACE, "Drone %s(%u): SetIdle: returning to idle.",
         m_pDrone->GetName(), m_pDrone->GetID());
    m_state = DroneAI::State::Idle;
    m_pendingTargetID = 0;
    m_assignedTargetID = 0;
    m_returnToBay = false;
    m_scoopRequested = false;

    // disable ewar timers
    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Disable();
    m_warpScramblerTimer.Disable();

    // Orbit only while the owning ship still exists. An orphaned drone remains idle.
    m_assignedShip = m_pDrone->GetHomeShip();
    if (m_assignedShip != nullptr)
        m_pDrone->IdleOrbit(m_assignedShip);
    m_pDrone->StateChange();
}

void DroneAIMgr::SetEngaged(SystemEntity* pTarget) {
    if (pTarget == nullptr || m_pDrone->DestinyMgr() == nullptr)
        return;
    if ((m_state == DroneAI::State::Combat || m_state == DroneAI::State::Engaged)
        && m_pDrone->DestinyMgr()->GetTargetID() == pTarget->GetID())
        return;
    _log(DRONE__AI_TRACE, "Drone %s(%u): SetEngaged: %s(%u) begin engaging.",
         m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively fighting
    //   not sure of the actual orbit speed of npc's, but their 'cruise speed' seems a bit slow.
    //   this sets orbit speed between cruise speed and quarter of max speed (whether mwb or ab)
    //   this will also enable this npc to have a variable speed, instead of fixed upon creation.
    const uint32 orbitSpeedQuarter = m_chaseSpeed / 4;
    const uint32 orbitSpeedMin = (m_cruiseSpeed < orbitSpeedQuarter)
        ? m_cruiseSpeed : orbitSpeedQuarter;
    const uint32 orbitSpeedMax = (m_cruiseSpeed > orbitSpeedQuarter)
        ? m_cruiseSpeed : orbitSpeedQuarter;
    m_pDrone->DestinyMgr()->SetMaxVelocity(MakeRandomFloat(orbitSpeedMin, orbitSpeedMax));
    m_pDrone->DestinyMgr()->Orbit(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    // Combat is the actual client activity state for weapon engagement.
    // State 10 (Engaged) is not the combat state and can be rendered as a
    // non-combat activity by the client.
    m_state = DroneAI::State::Combat;
    m_pDrone->StateChange();
}

void DroneAIMgr::CheckDistance(SystemEntity* pSE)
{
    if (m_pDrone == nullptr || m_pDrone->DestinyMgr() == nullptr) {
        SetIdle();
        return;
    }
    if (pSE == nullptr || pSE->IsDead() || m_pDrone->SysBubble() == nullptr) {
        if (pSE != nullptr)
            ClearTarget(pSE);
        else
            SetIdle();
        return;
    }
    //rewrote distance checks for correct logic this time
    const double dist = m_pDrone->GetPosition().distance(pSE->GetPosition());
    // EntityChaseMaxDistance is the outer pursuit envelope. EntityAttackRange
    // remains a fallback for drone types with incomplete static attributes.
    const double maxChaseRange = (m_entityChaseRange > m_entityAttackRange)
        ? m_entityChaseRange : m_entityAttackRange;
    if (dist > maxChaseRange) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): CheckDistance: %s(%u) is outside chase range (%.0f m > %.0f m).",
             m_pDrone->GetName(), m_pDrone->GetID(), pSE->GetName(), pSE->GetID(), dist, maxChaseRange);
        if (m_state != DroneAI::State::Idle)
            ClearTarget(pSE);
        return;
    }

    // EntityAttackRange is the outer combat envelope; EntityFlyRange plus
    // MaxRange is the older inner approach threshold. Using only the latter
    // made light drones oscillate between Approach and Combat near the boundary.
    const double engageRange = (m_entityAttackRange > m_entityFlyRange)
        ? m_entityAttackRange : m_entityFlyRange;
    if (dist <= engageRange) {
        SetEngaged(pSE); // inside the weapon envelope: orbit and attack
    } else {
        // Keep pursuing a valid, player-designated target until it enters the
        // combat envelope instead of dropping it at the inner fly threshold.
        if (m_state != DroneAI::State::Approaching || !m_pDrone->DestinyMgr()->IsFollowing()) {
            m_pDrone->DestinyMgr()->SetMaxVelocity(m_chaseSpeed);
            m_pDrone->DestinyMgr()->Follow(pSE, static_cast<uint32>(m_entityOrbitRange));
            m_state = DroneAI::State::Approaching;
            m_mainAttackTimer.Disable();
            m_pDrone->StateChange();
        }
        return;
    }

    if (!m_mainAttackTimer.Enabled())
        m_mainAttackTimer.Start(m_attackSpeed);

    Attack(pSE);
}

void DroneAIMgr::ClearTargets() {
    if (m_pDrone != nullptr && m_pDrone->TargetMgr() != nullptr)
        m_pDrone->TargetMgr()->ClearTargets();
}

void DroneAIMgr::ClearAllTargets() {
    if (m_pDrone != nullptr && m_pDrone->TargetMgr() != nullptr)
        m_pDrone->TargetMgr()->ClearAllTargets();
    //m_pDrone->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::ClientReq);
}

void DroneAIMgr::TargetAcquired(SystemEntity* pTarget) {
    if (pTarget == nullptr || m_pDrone == nullptr)
        return;
    if (m_assignedTargetID == 0)
        m_assignedTargetID = pTarget->GetID();
    CheckDistance(pTarget);
}

void DroneAIMgr::Target(SystemEntity* pTarget) {
    m_autoEngageEnabled = true;
    if (pTarget == nullptr || m_pDrone == nullptr || m_pDrone->GetSelf().get() == nullptr
        || m_pDrone->TargetMgr() == nullptr || m_pDrone->DestinyMgr() == nullptr
        || pTarget->DestinyMgr() == nullptr || pTarget->SysBubble() == nullptr
        || pTarget->SysBubble() != m_pDrone->SysBubble()) {
        SetIdle();
        return;
    }
    if (!IsTargetWithinDroneControlRange(m_pDrone, pTarget)) {
        if (m_pDrone->GetOwner() != nullptr)
            m_pDrone->GetOwner()->SendNotifyMsg("That target is outside your drone control range.");
        _log(DRONE__AI_TRACE, "Drone %s(%u): Rejecting target %s(%u), outside %.0f m control range of its home ship.",
             m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID(),
             GetDroneControlRange(m_pDrone));
        return;
    }
    // An explicit engage order replaces the previous target rather than
    // leaving the drone with a full/stale target list and an idle AI state.
    if (m_pDrone->TargetMgr()->GetTarget(pTarget->GetID(), false) == nullptr)
        m_pDrone->TargetMgr()->ClearAllTargets();

    bool chase = false;
    m_assignedTargetID = pTarget->GetID();
    const double commandRange = (m_entityChaseRange > m_entityAttackRange)
        ? m_entityChaseRange : m_entityAttackRange;
    const double distance = m_pDrone->GetPosition().distance(pTarget->GetPosition());
    if (distance > commandRange) {
        m_pendingTargetID = pTarget->GetID();
        m_pDrone->DestinyMgr()->SetMaxVelocity(m_chaseSpeed);
        m_pDrone->DestinyMgr()->Follow(pTarget, m_entityFlyRange);
        m_state = DroneAI::State::Approaching;
        m_mainAttackTimer.Disable();
        m_pDrone->StateChange();
        _log(DRONE__AI_TRACE, "Drone %s(%u): Approaching target %s(%u) to enter %.0f m lock range.",
             m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID(), commandRange);
        return;
    }
    m_pendingTargetID = 0;
    uint32 maxTargets = m_pDrone->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_uint32();
    if (maxTargets == 0)
        maxTargets = 1; // many combat drone types omit this NPC targeting attribute
    if (maxTargets > 255)
        maxTargets = 255;
    uint32 lockTime = m_pDrone->GetSelf()->GetAttribute(AttrScanSpeed).get_uint32();
    if (lockTime == 0)
        lockTime = 1000;
    if (!m_pDrone->TargetMgr()->StartTargeting(pTarget, static_cast<float>(lockTime),
                                               static_cast<uint8>(maxTargets), commandRange, chase)) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.",
             m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
        //ClearAllTargets();
        SetIdle();
        return;
    }
    m_beginFindTarget.Disable();
    CheckDistance(pTarget);

    /*
    std::map<std::string, PyRep *> arg;
    arg["target"] = new PyInt(args.arg);
    throw PyException(MakeUserError("DeniedDroneTargetForceField", arg));
    */
 //DeniedDroneTargetForceField
}

void DroneAIMgr::Targeted(SystemEntity* pAgressor) {
    if (m_pDrone == nullptr || pAgressor == nullptr) {
        _log(DRONE__AI_TRACE, "Drone AI received a target notification without a valid drone or aggressor.");
        return;
    }
    _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while %s.",
                m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID(), GetStateName(m_state).c_str());

    // Aggressive drones retaliate against NPCs that actively target them.
    // Passive drones remain under explicit player command.
    if (m_state == DroneAI::State::Idle && pAgressor->IsNPCSE()
        && m_pDrone->GetSelf().get() != nullptr
        && m_pDrone->GetSelf()->GetAttribute(AttrDroneIsAgressive).get_bool()) {
        Target(pAgressor);
        return;
    }

    switch(m_state) {
        case DroneAI::State::Idle: {
        } break;
        case DroneAI::State::Operating: {
        } break;
        case DroneAI::State::Unknown: {
        } break;
        case DroneAI::State::Engaged: {
        } break;
        case DroneAI::State::Fleeing: {
        } break;
        case DroneAI::State::Incapacitated: {
        } break;
        case DroneAI::State::Guarding: {
        } break;
        case DroneAI::State::Assisting: {
        } break;
        case DroneAI::State::Combat: {
        } break;
        case DroneAI::State::Mining: {
        } break;
        case DroneAI::State::Approaching: {
        } break;
        case DroneAI::State::Departing: {
        } break;
        case DroneAI::State::Departing2: {
        } break;
        case DroneAI::State::Pursuit: {
        } break;
    }
}

void DroneAIMgr::TargetLost(SystemEntity* pTarget) {
    if (m_pDrone == nullptr || m_pDrone->TargetMgr() == nullptr)
        return;

    if (pTarget != nullptr && m_pendingTargetID == pTarget->GetID())
        m_pendingTargetID = 0;
    if (pTarget == nullptr || m_assignedTargetID == (pTarget != nullptr ? pTarget->GetID() : 0)) {
        m_assignedTargetID = 0;
        SystemEntity* nextTarget = m_pDrone->TargetMgr()->GetFirstTarget(false);
        if (nextTarget != nullptr)
            m_assignedTargetID = nextTarget->GetID();
    }
    const char* targetName = (pTarget != nullptr) ? pTarget->GetName() : "<unknown>";
    const uint32 targetID = (pTarget != nullptr) ? pTarget->GetID() : 0;
    if (m_pDrone->TargetMgr()->HasNoTargets()) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) lost. No targets remain; returning to idle.",
             m_pDrone->GetName(), m_pDrone->GetID(), targetName, targetID);
        SetIdle();
    } else if (m_state == DroneAI::State::Combat ||
               m_state == DroneAI::State::Engaged ||
               m_state == DroneAI::State::Approaching ||
               m_state == DroneAI::State::Pursuit) {
        // Process() will select the next fully locked target and recalculate movement.
        _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) lost; another target remains.",
             m_pDrone->GetName(), m_pDrone->GetID(), targetName, targetID);
    }
}

void DroneAIMgr::Attack(SystemEntity* pSE)
{
    if (m_mainAttackTimer.Check()) {
        if (m_pDrone->SysBubble() == nullptr) {
            SetIdle();
            return;
        }
        if (pSE == nullptr)
            return;
        if (pSE->IsDead()) {
            ClearTarget(pSE);
            return;
        }
        // Check to see if the target still in the bubble (Client warped out)
        // fighters/bombers are able to follow.
        if (!m_pDrone->SysBubble()->InBubble(pSE->GetPosition())) {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) no longer in bubble.  Clear target and move on",
                 m_pDrone->GetName(), m_pDrone->GetID(), pSE->GetName(), pSE->GetID());
            ClearTarget(pSE);
            return;
        }
        DestinyManager* pDestiny = pSE->DestinyMgr();
        if (pDestiny == nullptr) {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) has no destiny manager.  Clear target and move on",
                 m_pDrone->GetName(), m_pDrone->GetID(), pSE->GetName(), pSE->GetID());
            ClearTarget(pSE);
            return;
        }
        // Check to see if the target is not cloaked:
        if (pDestiny->IsCloaked()) {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) is cloaked.  Clear target and move on",
                 m_pDrone->GetName(), m_pDrone->GetID(), pSE->GetName(), pSE->GetID());
            ClearTarget(pSE);
            return;
        }

        // Match the NPC combat path: TargetManager only permits weapon fire
        // after its lock-completion process has enabled attacks. pSE is already
        // the first fully locked target selected by Process().
        if (m_pDrone->TargetMgr() != nullptr && m_pDrone->TargetMgr()->CanAttack())
            AttackTarget(pSE);
        else
            _log(DRONE__AI_TRACE, "Drone %s(%u): Waiting for target lock before firing on %s(%u).",
                 m_pDrone->GetName(), m_pDrone->GetID(), pSE->GetName(), pSE->GetID());
    }
}

void DroneAIMgr::ClearTarget(SystemEntity* pSE) {
    if (m_pDrone == nullptr || m_pDrone->TargetMgr() == nullptr || pSE == nullptr) {
        SetIdle();
        return;
    }
    m_pDrone->TargetMgr()->ClearTarget(pSE);

    if (m_pDrone->TargetMgr() == nullptr || m_pDrone->TargetMgr()->HasNoTargets())
        SetIdle();
}

//also check for special effects and write code to implement them
//modifyTargetSpeedRange, modifyTargetSpeedChance
//entityWarpScrambleChance

void DroneAIMgr::AttackTarget(SystemEntity* pTarget) {
    if (pTarget == nullptr || pTarget->GetSelf().get() == nullptr
        || m_pDrone == nullptr || m_pDrone->GetSelf().get() == nullptr
        || m_pDrone->DestinyMgr() == nullptr)
        return;
    /** @todo  not all drones use lazors...fix this */
    //  woot!! --> group:1010        cat:8       Compact Citadel Torpedo         Citadel torpedoes for fighter-bombers

    // effects are listed in EVE_Effects.h
    //  NOTE: drones are called 'entities' in client; EVE_Effects has 'entityxxx' for gfx
    std::string guid = "effects.Laser"; // client looks for 'turret' in ship.ball.modules for 'effects.laser'
    //effects.ProjectileFiredForEntities
    uint32 gfxID = 0;
    if (m_pDrone->GetSelf()->HasAttribute(AttrGfxTurretID))// graphicID for turret for drone type ships
        gfxID = m_pDrone->GetSelf()->GetAttribute(AttrGfxTurretID).get_uint32();
    m_pDrone->DestinyMgr()->SendSpecialEffect(m_pDrone->GetSelf()->itemID(),
                                             m_pDrone->GetSelf()->itemID(),
                                             m_pDrone->GetSelf()->typeID(), //m_pDrone->GetSelf()->GetAttribute(AttrGfxTurretID).get_int(),
                                             pTarget->GetID(),
                                             0,guid,1,1,1,m_attackSpeed,0,gfxID);

    const float hitQuality = m_formula.GetDroneToHit(m_pDrone, pTarget);
    const float baseEM = m_pDrone->GetSelf()->GetAttribute(AttrEmDamage).get_float();
    const float baseKinetic = m_pDrone->GetSelf()->GetAttribute(AttrKineticDamage).get_float();
    const float baseThermal = m_pDrone->GetSelf()->GetAttribute(AttrThermalDamage).get_float();
    const float baseExplosive = m_pDrone->GetSelf()->GetAttribute(AttrExplosiveDamage).get_float();
    Damage d(m_pDrone,
             m_pDrone->GetSelf(),
             m_pDrone->GetKinetic(),
             m_pDrone->GetThermal(),
             m_pDrone->GetEM(),
             m_pDrone->GetExplosive(),
             hitQuality,
             EVEEffectID::targetAttack
            );

    float damageMultiplier = m_pDrone->GetSelf()->GetAttribute(AttrDamageMultiplier).get_float();
    if (damageMultiplier <= 0.0f)
        damageMultiplier = 1.0f;

    if (d.GetTotal() <= 0.0f) {
        _log(DRONE__WARNING, "Drone %s(%u): Cannot damage %s(%u); drone damage attributes are all zero.",
             m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
        return;
    }

    d *= damageMultiplier;
    _log(DRONE__MESSAGE, "Drone %s(%u): Firing on %s(%u), EM %.2f K %.2f T %.2f EX %.2f, base %.2f, multiplier %.2f, hit quality %.2f.",
         m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID(),
         baseEM, baseKinetic, baseThermal, baseExplosive, d.GetTotal(), damageMultiplier, hitQuality);
    const double shieldBefore = pTarget->GetSelf()->GetAttribute(AttrShieldCharge).get_double();
    const double armorBefore = pTarget->GetSelf()->GetAttribute(AttrArmorDamage).get_double();
    const double hullBefore = pTarget->GetSelf()->GetAttribute(AttrDamage).get_double();
    // ApplyDamage() applies the global damageRate to every damage source.
    // Applying it here as well squared the configured rate for drone attacks.
    const bool killed = pTarget->ApplyDamage(d);
    if (!killed && pTarget->GetSelf().get() != nullptr) {
        _log(DRONE__MESSAGE, "Drone %s(%u): Damage result on %s(%u): shield %.3f, armor +%.3f, hull +%.3f.",
             m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID(),
             shieldBefore - pTarget->GetSelf()->GetAttribute(AttrShieldCharge).get_double(),
             pTarget->GetSelf()->GetAttribute(AttrArmorDamage).get_double() - armorBefore,
             pTarget->GetSelf()->GetAttribute(AttrDamage).get_double() - hullBefore);
    }
}


std::string DroneAIMgr::GetStateName(int8 stateID)
{
    switch (stateID) {
        case DroneAI::State::Idle:            return "Idle";
        case DroneAI::State::Combat:          return "Combat";
        case DroneAI::State::Mining:          return "Mining";
        case DroneAI::State::Approaching:     return "Approaching";
        case DroneAI::State::Departing:       return "Returning to ship";
        case DroneAI::State::Departing2:      return "Departing2";
        case DroneAI::State::Pursuit:         return "Pursuit";
        case DroneAI::State::Engaged:         return "Engaged";
        case DroneAI::State::Fleeing:         return "Fleeing";
        case DroneAI::State::Unknown:         return "Unknown";
        case DroneAI::State::Operating:       return "Operating";
        case DroneAI::State::Assisting:       return "Assisting";
        case DroneAI::State::Guarding:        return "Guarding";
        case DroneAI::State::Incapacitated:   return "Incapacitated";
        default:                              return "Invalid";
    }
}
