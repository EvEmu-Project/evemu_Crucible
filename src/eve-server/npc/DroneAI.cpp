/**
 * DroneAI.cpp
 *      this class is for drone AI
 *
 * @Author:     Allan
 * @Version:    0.15
 * @Date:       27Nov19
*/

/* How drones behave (Crucible)
 *
 *  - idle drones orbit their controlling ship at their orbit range and orbit velocity.  if the ship warps off or
 *     leaves their bubble they wait where they are
 *  - engaging: approach the target at full speed, then orbit it at the drone's orbit range (entityFlyRange) and
 *     orbit velocity (entityCruiseSpeed), using the main effect once the target is in range and locked.
 *     the target has to stay within the pilot's drone control range; if it leaves, the drones return and orbit
 *  - when the target is gone, the drones return to orbiting the ship.  aggressive drones then go after whoever
 *     attacked the pilot or the drones (focus fire puts all of the pilot's drones on the same target)
 *  - mining drones fly to the asteroid and mine one cycle.  at the end of the cycle the ore goes straight into the
 *     ship's cargo hold, provided the ship is within the drone's range.  mine repeatedly keeps going
 *  - return to bay: fly to the ship and enter the drone bay on arrival
 *
 * Movement is done with ordinary destiny Orbit/Follow commands, which the client simulates the same way we do.
 *  each call below is idempotent (destiny ignores a command that changes nothing), so they are simply repeated every tick.
 */

#include "eve-server.h"

#include "Client.h"
#include "EVEServerConfig.h"
#include "EntityList.h"
#include "StatisticMgr.h"
#include "character/Character.h"
#include "inventory/AttributeEnum.h"
#include "inventory/Inventory.h"
#include "npc/Drone.h"
#include "npc/DroneAI.h"
#include "ship/Ship.h"
#include "system/Damage.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "packets/General.h"

namespace {
    // shows one of the client's own messages (a 'notify' popup for the mining ones)
    void SendDroneMessage(Client* pClient, const char* msgKey, std::map<std::string, PyRep*> args) {
        Notify_OnRemoteMessage n;
            n.msgType = msgKey;
        for (auto cur : args)
            n.args[cur.first] = cur.second;
        PyTuple* tmp = n.Encode();
        pClient->SendNotification("OnRemoteMessage", "charid", &tmp);
    }

    // gap between two balls' surfaces
    double Gap(SystemEntity* pA, SystemEntity* pB) {
        return pA->GetPosition().distance(pB->GetPosition()) - pA->GetRadius() - pB->GetRadius();
    }

    // drones enter the bay once this close to the ship's surface
    const double DRONE_BAY_ENTRY_DISTANCE = 300.0;
    // drones approach at full speed until about this far outside their orbit range, then orbit (with hysteresis)
    const double DRONE_APPROACH_MARGIN = 500.0;
}

DroneAIMgr::DroneAIMgr(DroneSE* pDrone)
: m_pDrone(pDrone),
  m_state(DroneAI::State::Idle),
  m_order(DroneAI::Order::Idle),
  m_action(DroneAI::Action::None),
  m_moveMode(0),
  m_targetID(0),
  m_activityCharID(0),
  m_moveTargetID(0),
  m_aggressorID(0),
  m_webbedID(0),
  m_moveRange(0.0),
  m_moveSpeed(0.0),
  m_cycleActive(false),
  m_cycleTimer(0)
{
}

void DroneAIMgr::Process() {
    double profileStartTime(GetTimeUSeconds());

    ShipSE* pController(m_pDrone->GetController());
    if (pController == nullptr) {
        // our controller is gone without telling us (shouldn't happen)
        m_pDrone->ReleaseControl();
        return;
    }
    m_action = m_pDrone->Stats().action;

    switch (m_order) {
        case DroneAI::Order::Idle:              ProcessIdle(pController);           break;
        case DroneAI::Order::ReturnHome:        ProcessReturn(pController, false);  break;
        case DroneAI::Order::ReturnBay:         ProcessReturn(pController, true);   break;
        case DroneAI::Order::Engage:            ProcessEngage(pController);         break;
        case DroneAI::Order::Mine:
        case DroneAI::Order::MineRepeatedly:    ProcessMine(pController);           break;
        case DroneAI::Order::Guard:
        case DroneAI::Order::Assist:            ProcessGuard(pController);          break;
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::drone, GetTimeUSeconds() - profileStartTime);
}

/* ======================================================================================
 *  orders
 * ====================================================================================== */

void DroneAIMgr::SetOrder(uint8 order, uint32 targetID/*0*/) {
    StopCycle();
    if (targetID != m_targetID) {
        StopWeb();
        ReleaseTarget();
    }
    m_order = order;
    m_targetID = targetID;
}

void DroneAIMgr::SetState(uint8 state) {
    if (state == m_state)
        return;
    _log(DRONE__AI_TRACE, "Drone %s(%u): state %u -> %u (order %u, target %u)", m_pDrone->GetName(), m_pDrone->GetID(), m_state, state, m_order, m_targetID);
    m_state = state;
    m_pDrone->SendStateChange();
}

void DroneAIMgr::Engage(SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return;
    bool changed(pTarget->GetID() != m_targetID);
    uint8 order(((m_order == DroneAI::Order::Guard) or (m_order == DroneAI::Order::Assist)) ? m_order : DroneAI::Order::Engage);
    SetOrder(order, pTarget->GetID());
    m_state = DroneAI::State::Approaching;
    if (changed)
        m_pDrone->SendStateChange();

    // drones assisting our pilot join in
    if (m_order != DroneAI::Order::Engage)
        return;
    for (auto cur : m_pDrone->SystemMgr()->GetDrones()) {
        DroneSE* pDrone(cur.second);
        if ((pDrone == m_pDrone) or !pDrone->IsControlled())
            continue;
        DroneAIMgr* pAI(pDrone->GetAI());
        if ((pAI->m_order == DroneAI::Order::Assist) and (pAI->m_activityCharID == m_pDrone->GetControllerOwnerID())
        and (pAI->m_targetID != pTarget->GetID()))
            pAI->Engage(pTarget);
    }
}

void DroneAIMgr::Mine(SystemEntity* pTarget, bool repeat) {
    if (pTarget == nullptr)
        return;
    SetOrder(repeat ? DroneAI::Order::MineRepeatedly : DroneAI::Order::Mine, pTarget->GetID());
    m_state = DroneAI::State::Approaching;
    m_pDrone->SendStateChange();
}

void DroneAIMgr::ReturnHome() {
    SetOrder(DroneAI::Order::ReturnHome);
    SetState(DroneAI::State::Returning);
}

void DroneAIMgr::ReturnToBay() {
    SetOrder(DroneAI::Order::ReturnBay);
    SetState(DroneAI::State::ReturningToBay);
}

void DroneAIMgr::Guard(uint32 charID) {
    SetOrder(DroneAI::Order::Guard);
    m_activityCharID = charID;
    m_aggressorID = 0;
    SetState(DroneAI::State::Following);
}

void DroneAIMgr::Assist(uint32 charID) {
    SetOrder(DroneAI::Order::Assist);
    m_activityCharID = charID;
    m_aggressorID = 0;
    SetState(DroneAI::State::Following);
}

void DroneAIMgr::SetIdle() {
    SetOrder(DroneAI::Order::Idle);
    m_activityCharID = 0;
    SetState(DroneAI::State::Idle);
}

// control lost.  nobody is listening, so nothing is sent
void DroneAIMgr::Stop() {
    StopCycle();
    StopWeb();
    ReleaseTarget();
    m_order = DroneAI::Order::Idle;
    m_state = DroneAI::State::Idle;
    m_targetID = 0;
    m_activityCharID = 0;
    m_aggressorID = 0;
    StopMoving();
}

void DroneAIMgr::TargetLost(SystemEntity* pTarget) {
    // a lock we dropped ourselves (ReleaseTarget()) has already been forgotten
    if ((pTarget == nullptr) or (pTarget->GetID() != m_targetID))
        return;
    _log(DRONE__AI_TRACE, "Drone %s(%u): lost lock on %s(%u)", m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
    if ((m_order == DroneAI::Order::Guard) or (m_order == DroneAI::Order::Assist)) {
        StopCycle();
        StopWeb();
        m_targetID = 0;
        m_pDrone->SendStateChange();
    } else if (m_order == DroneAI::Order::Engage) {
        SetIdle();
    }
}

void DroneAIMgr::Aggressed(SystemEntity* pAttacker) {
    if (pAttacker == nullptr)
        return;
    m_aggressorID = pAttacker->GetID();
}

/* ======================================================================================
 *  per-order processing
 * ====================================================================================== */

void DroneAIMgr::ProcessIdle(ShipSE* pController) {
    // our ship has gone elsewhere.  wait here
    if ((pController->SysBubble() != m_pDrone->SysBubble()) or pController->DestinyMgr()->IsWarping()) {
        StopMoving();
        SetState(DroneAI::State::Idle);
        return;
    }

    Client* pClient(pController->GetPilot());
    if ((m_action == DroneAI::Action::Attack) or (m_action == DroneAI::Action::Web) or (m_action == DroneAI::Action::Ewar))
        if (pClient->GetChar()->GetAttribute(AttrDroneIsAgressive).get_int() != 0) {
            SystemEntity* pTarget(PickAggressor());
            if (pTarget != nullptr) {
                _log(DRONE__AI_TRACE, "Drone %s(%u): aggressive, engaging %s(%u)", m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
                Engage(pTarget);
                return;
            }
        }

    OrbitTarget(pController, m_pDrone->Stats().idleOrbitRange, m_pDrone->Stats().orbitVelocity);
    SetState(DroneAI::State::Idle);
}

void DroneAIMgr::ProcessReturn(ShipSE* pController, bool toBay) {
    if ((pController->SysBubble() != m_pDrone->SysBubble()) or pController->DestinyMgr()->IsWarping()) {
        StopMoving();
        return;
    }
    const DroneStats& stats(m_pDrone->Stats());
    double gap(Gap(m_pDrone, pController));

    if (toBay) {
        ApproachTarget(pController, 0.0, stats.maxVelocity);
        SetState(DroneAI::State::ReturningToBay);
        if (gap <= DRONE_BAY_ENTRY_DISTANCE)
            m_pDrone->RequestReturnToBay();
        return;
    }

    if (gap <= stats.idleOrbitRange + DRONE_APPROACH_MARGIN) {
        SetOrder(DroneAI::Order::Idle);
        ProcessIdle(pController);
        return;
    }
    ApproachTarget(pController, stats.idleOrbitRange, stats.maxVelocity);
    SetState(DroneAI::State::Returning);
}

void DroneAIMgr::ProcessEngage(ShipSE* pController) {
    SystemEntity* pTarget(m_pDrone->SystemMgr()->GetSE(m_targetID));
    bool usable((pTarget != nullptr) and !pTarget->IsDead() and (pTarget->SysBubble() == m_pDrone->SysBubble()));
    if (usable and (pTarget->DestinyMgr() != nullptr))
        if (pTarget->DestinyMgr()->IsCloaked() or pTarget->DestinyMgr()->IsWarping())
            usable = false;
    if (!usable) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): target %u is gone", m_pDrone->GetName(), m_pDrone->GetID(), m_targetID);
        if (m_order == DroneAI::Order::Engage) {
            SetIdle();
        } else {
            StopCycle();
            StopWeb();
            ReleaseTarget();
            m_targetID = 0;
            m_pDrone->SendStateChange();
        }
        return;
    }

    // the target must stay within the pilot's drone control range
    if (Gap(pController, pTarget) > m_pDrone->ControlRange()) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): target %s(%u) left control range", m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
        ReturnHome();
        return;
    }

    const DroneStats& stats(m_pDrone->Stats());
    double gap(Gap(m_pDrone, pTarget));
    // close in at full speed, then settle into orbit at orbit velocity.  the effect is used whenever in range
    bool approaching(m_pDrone->DestinyMgr()->IsFollowing());
    double approachUntil(stats.orbitRange + (approaching ? DRONE_APPROACH_MARGIN : DRONE_APPROACH_MARGIN * 2.0));
    if (gap > approachUntil) {
        ApproachTarget(pTarget, stats.orbitRange, stats.maxVelocity);
    } else {
        OrbitTarget(pTarget, stats.orbitRange, stats.orbitVelocity);
    }

    if ((gap > stats.effectRange) or !LockTarget(pTarget)) {
        StopCycle();
        SetState(DroneAI::State::Approaching);
        return;
    }

    switch (m_action) {
        case DroneAI::Action::RepairShield:
        case DroneAI::Action::RepairArmor:  SetState(DroneAI::State::Repairing);    break;
        case DroneAI::Action::Web:
        case DroneAI::Action::Ewar:         SetState(DroneAI::State::Operating);    break;
        default:                            SetState(DroneAI::State::Combat);       break;
    }

    if (!m_cycleActive) {
        m_cycleActive = true;
        m_cycleTimer.Start(static_cast<uint32>(stats.cycleTime));
        Fire(pTarget);
    } else if (m_cycleTimer.Check()) {
        Fire(pTarget);
    }
}

void DroneAIMgr::ProcessMine(ShipSE* pController) {
    Client* pClient(pController->GetPilot());
    SystemEntity* pRoid(m_pDrone->SystemMgr()->GetSE(m_targetID));
    if ((pRoid == nullptr) or !pRoid->IsAsteroidSE() or (pRoid->GetSelf()->GetAttribute(AttrQuantity).get_double() <= 0.0)) {
        //{'FullPath': u'UI/Messages', 'messageID': 259462, 'label': u'MiningDronesDeactivatedAsteroidEmptyBody'}(u'{modulename} deactivates as it finds the resource it was harvesting a pale shadow of its former glory.')
        SendDroneMessage(pClient, "MiningDronesDeactivatedAsteroidEmpty", {{"modulename", new PyString(m_pDrone->GetName())}});
        ReturnHome();
        return;
    }

    const DroneStats& stats(m_pDrone->Stats());
    double gap(Gap(m_pDrone, pRoid));
    if (!m_cycleActive) {
        if (gap > stats.effectRange) {
            ApproachTarget(pRoid, stats.orbitRange, stats.maxVelocity);
            SetState(DroneAI::State::Approaching);
            return;
        }
        m_cycleActive = true;
        m_cycleTimer.Start(static_cast<uint32>(stats.cycleTime));
        SendEffect(pRoid, true);
    }
    OrbitTarget(pRoid, stats.orbitRange, stats.orbitVelocity);
    SetState(DroneAI::State::Mining);

    if (!m_cycleTimer.Check(false))
        return;
    m_cycleTimer.Disable();
    m_cycleActive = false;
    SendEffect(pRoid, false);

    uint8 order(m_order);
    FinishMiningCycle(pController, pRoid);
    // pRoid may be gone now
    if (m_order != order)
        return;     // the cycle failed and sent us home
    if (order == DroneAI::Order::Mine) {
        ReturnHome();
        return;
    }
    // mine repeatedly: next cycle starts on the next tick
}

void DroneAIMgr::ProcessGuard(ShipSE* pController) {
    Client* pPilot(sEntityList.FindClientByCharID(m_activityCharID));
    ShipSE* pShip((pPilot == nullptr) ? nullptr : pPilot->GetShipSE());
    if ((pShip == nullptr) or (pShip->SystemMgr() != m_pDrone->SystemMgr()) or pPilot->InPod()) {
        // nobody left to guard or assist
        Client* pClient(pController->GetPilot());
        PyTuple* ev = new PyTuple(3);
            ev->SetItem(0, new PyInt(m_pDrone->GetID()));
            ev->SetItem(1, PyStatic.NewNone());
            ev->SetItem(2, PyStatic.NewNone());
        pClient->SendNotification("OnDroneActivityChange", "charid", &ev);
        SetIdle();
        return;
    }

    if (m_targetID == 0) {
        SystemEntity* pTarget(nullptr);
        if (m_order == DroneAI::Order::Assist) {
            // go for whatever the assisted pilot's drones are engaging
            for (auto cur : m_pDrone->SystemMgr()->GetDrones())
                if ((cur.second->GetControllerOwnerID() == m_activityCharID) and (cur.second->GetAI()->GetOrder() == DroneAI::Order::Engage)) {
                    pTarget = m_pDrone->SystemMgr()->GetSE(cur.second->GetAI()->GetTargetID());
                    if (pTarget != nullptr)
                        break;
                }
        } else {
            pTarget = PickAggressor();
        }
        if ((pTarget != nullptr) and (pTarget->SysBubble() == m_pDrone->SysBubble())) {
            m_targetID = pTarget->GetID();
            m_pDrone->SendStateChange();
        }
    }

    if (m_targetID != 0) {
        ProcessEngage(pController);
        return;
    }

    if (pShip->SysBubble() != m_pDrone->SysBubble()) {
        StopMoving();
    } else {
        OrbitTarget(pShip, m_pDrone->Stats().idleOrbitRange, m_pDrone->Stats().orbitVelocity);
    }
    SetState(DroneAI::State::Following);
}

// our last attacker, if we can still go after them.  with focus fire, the target the pilot's other drones already have
SystemEntity* DroneAIMgr::PickAggressor() {
    SystemManager* pSystem(m_pDrone->SystemMgr());
    ShipSE* pController(m_pDrone->GetController());
    if (pController == nullptr)
        return nullptr;

    auto usable = [&](SystemEntity* pSE) -> bool {
        if ((pSE == nullptr) or pSE->IsDead() or (pSE->SysBubble() != m_pDrone->SysBubble()))
            return false;
        if ((pSE->DestinyMgr() != nullptr) and (pSE->DestinyMgr()->IsCloaked() or pSE->DestinyMgr()->IsWarping()))
            return false;
        return (Gap(pController, pSE) <= m_pDrone->ControlRange());
    };

    if (pController->GetPilot()->GetChar()->GetAttribute(AttrDroneFocusFire).get_int() != 0)
        for (auto cur : pSystem->GetDrones()) {
            DroneSE* pDrone(cur.second);
            if ((pDrone == m_pDrone) or (pDrone->GetControllerID() != m_pDrone->GetControllerID()))
                continue;
            if (pDrone->GetAI()->GetOrder() != DroneAI::Order::Engage)
                continue;
            SystemEntity* pTarget(pSystem->GetSE(pDrone->GetAI()->GetTargetID()));
            if (usable(pTarget))
                return pTarget;
        }

    if (m_aggressorID == 0)
        return nullptr;
    SystemEntity* pTarget(pSystem->GetSE(m_aggressorID));
    if (!usable(pTarget)) {
        m_aggressorID = 0;
        return nullptr;
    }
    return pTarget;
}

/* ======================================================================================
 *  movement
 * ====================================================================================== */

void DroneAIMgr::OrbitTarget(SystemEntity* pTarget, double range, double speed) {
    DestinyManager* pDestiny(m_pDrone->DestinyMgr());
    pDestiny->SetMaxVelocity(static_cast<float>(speed));
    pDestiny->SetSpeedFraction(1.0f);
    pDestiny->Orbit(pTarget, static_cast<uint32>(range));
}

void DroneAIMgr::ApproachTarget(SystemEntity* pTarget, double range, double speed) {
    DestinyManager* pDestiny(m_pDrone->DestinyMgr());
    pDestiny->SetMaxVelocity(static_cast<float>(speed));
    pDestiny->SetSpeedFraction(1.0f);
    pDestiny->Follow(pTarget, static_cast<uint32>(range));
}

void DroneAIMgr::StopMoving() {
    m_pDrone->DestinyMgr()->Stop();
}

/* ======================================================================================
 *  using the main effect
 * ====================================================================================== */

bool DroneAIMgr::LockTarget(SystemEntity* pTarget) {
    TargetManager* pTM(m_pDrone->TargetMgr());
    if ((pTM == nullptr) or (pTarget->TargetMgr() == nullptr))
        return true;    // nothing to lock with.  just use the effect
    if (pTM->GetTarget(pTarget->GetID(), true) != nullptr)
        return true;
    if (pTM->GetTarget(pTarget->GetID(), false) != nullptr)
        return false;   // still locking

    // drones work one target at a time
    pTM->ClearTargets();
    bool chase(false);
    double lockTime(m_pDrone->GetSelf()->type().HasAttribute(AttrScanSpeed) ? m_pDrone->GetSelf()->type().GetAttribute(AttrScanSpeed).get_double() : 2000.0);
    pTM->StartTargeting(pTarget, static_cast<float>(lockTime), 1, 1.0e9, chase);
    return false;
}

void DroneAIMgr::ReleaseTarget() {
    if (m_targetID == 0)
        return;
    uint32 targetID(m_targetID);
    m_targetID = 0;     // so the TargetLost() callback below is ignored
    TargetManager* pTM(m_pDrone->TargetMgr());
    if (pTM == nullptr)
        return;
    SystemEntity* pTarget(pTM->GetTarget(targetID, false));
    if (pTarget != nullptr)
        pTM->ClearTarget(pTarget);
}

void DroneAIMgr::StopCycle() {
    if (!m_cycleActive)
        return;
    m_cycleActive = false;
    m_cycleTimer.Disable();
    SystemEntity* pTarget(m_pDrone->SystemMgr()->GetSE(m_targetID));
    if (pTarget != nullptr)
        SendEffect(pTarget, false);
}

// the drone's turret is keyed by its own ID, so both shipID and moduleID are the drone
void DroneAIMgr::SendEffect(SystemEntity* pTarget, bool start) {
    const DroneStats& stats(m_pDrone->Stats());
    if (stats.effectGuid.empty())
        return;
    uint32 gfxID(0);
    if (m_pDrone->GetSelf()->HasAttribute(AttrGfxTurretID))
        gfxID = m_pDrone->GetSelf()->GetAttribute(AttrGfxTurretID).get_uint32();
    m_pDrone->DestinyMgr()->SendSpecialEffect(m_pDrone->GetID(), m_pDrone->GetID(), m_pDrone->GetSelf()->typeID(), pTarget->GetID(), 0,
                                              stats.effectGuid, stats.offensive, start, start, static_cast<int32>(stats.cycleTime), 0, gfxID);
}

void DroneAIMgr::Fire(SystemEntity* pTarget) {
    const DroneStats& stats(m_pDrone->Stats());
    SendEffect(pTarget, true);

    switch (m_action) {
        case DroneAI::Action::Attack: {
            const ItemType& type(m_pDrone->GetSelf()->type());
            auto base = [&](uint16 attrID) -> float {
                return (type.HasAttribute(attrID) ? type.GetAttribute(attrID).get_float() : 0.0f);
            };
            Damage d(m_pDrone,
                     m_pDrone->GetSelf(),
                     base(AttrKineticDamage),
                     base(AttrThermalDamage),
                     base(AttrEmDamage),
                     base(AttrExplosiveDamage),
                     m_formula.GetDroneToHit(m_pDrone, pTarget),
                     stats.effectID);
            d *= static_cast<float>(stats.damageMultiplier);
            d *= sConfig.rates.damageRate;
            // this may destroy the target, and with it our lock
            pTarget->ApplyDamage(d);
        } break;
        case DroneAI::Action::RepairShield: {
            InventoryItemRef tRef(pTarget->GetSelf());
            double capacity(tRef->GetAttribute(AttrShieldCapacity).get_double());
            double charge(tRef->GetAttribute(AttrShieldCharge).get_double() + stats.repairAmount);
            tRef->SetAttribute(AttrShieldCharge, (charge > capacity ? capacity : charge));
            pTarget->SendDamageStateChanged();
        } break;
        case DroneAI::Action::RepairArmor: {
            InventoryItemRef tRef(pTarget->GetSelf());
            double damage(tRef->GetAttribute(AttrArmorDamage).get_double() - stats.repairAmount);
            tRef->SetAttribute(AttrArmorDamage, (damage < 0.0 ? 0.0 : damage));
            pTarget->SendDamageStateChanged();
        } break;
        case DroneAI::Action::Web: {
            StartWeb(pTarget);
        } break;
        default:
            // electronic warfare and neutralizing drones show their effect, but have none yet
            break;
    }
}

void DroneAIMgr::StartWeb(SystemEntity* pTarget) {
    if (pTarget->GetID() == m_webbedID)
        return;
    StopWeb();
    if (pTarget->DestinyMgr() == nullptr)
        return;
    pTarget->DestinyMgr()->WebbedMe(m_pDrone->GetSelf(), true);
    m_webbedID = pTarget->GetID();
}

void DroneAIMgr::StopWeb() {
    if (m_webbedID == 0)
        return;
    SystemEntity* pTarget(m_pDrone->SystemMgr()->GetSE(m_webbedID));
    m_webbedID = 0;
    if ((pTarget != nullptr) and (pTarget->DestinyMgr() != nullptr))
        pTarget->DestinyMgr()->WebbedMe(m_pDrone->GetSelf(), false);
}

/* end of a mining cycle.  the ore goes straight into the ship's cargo hold, if the ship is within the drone's range.
 *  mining lasers on the same rock are left to deplete it, as they share out what's left between themselves
 */
void DroneAIMgr::FinishMiningCycle(ShipSE* pController, SystemEntity* pRoid) {
    Client* pClient(pController->GetPilot());
    const DroneStats& stats(m_pDrone->Stats());

    double distance(Gap(m_pDrone, pController));
    if (distance > stats.effectRange) {
        //{'FullPath': u'UI/Messages', 'messageID': 259422, 'label': u'MiningDronesDeactivatedOutOfRangeBody'}(u'{[item]module.name} deactivates without transfering ore to your cargo hold because your ship has strayed to a distance of {[numeric]distance} m, beyond its mining range of {[numeric]range} m.')
        SendDroneMessage(pClient, "MiningDronesDeactivatedOutOfRange", {{"module", new PyInt(m_pDrone->GetSelf()->typeID())},
                {"distance", new PyInt(static_cast<int32>(distance))}, {"range", new PyInt(static_cast<int32>(stats.effectRange))}});
        ReturnHome();
        return;
    }

    InventoryItemRef roidRef(pRoid->GetSelf());
    double oreVolume(roidRef->GetAttribute(AttrVolume).get_double());
    double roidQuantity(roidRef->GetAttribute(AttrQuantity).get_double());
    if (oreVolume <= 0.0)
        return;
    double units(std::floor(stats.miningAmount / oreVolume));
    bool lasersOnRock((pRoid->TargetMgr() != nullptr) and pRoid->TargetMgr()->HasActiveModules());
    double available(lasersOnRock ? std::floor(roidQuantity) - 1.0 : std::floor(roidQuantity));
    if (units > available)
        units = available;
    if (units < 1.0)
        return;

    ShipItemRef shipRef(pController->GetShipItemRef());
    if (shipRef->GetRemainingVolumeByFlag(flagCargoHold) < units * oreVolume) {
        //{'FullPath': u'UI/Messages', 'messageID': 259442, 'label': u'MiningDronesDeactivatedCargoHoldFullBody'}(u'{modulename} disperses its load of freshly mined ore into space unrecoverably, then deactivates. Your cargo hold is full.')
        SendDroneMessage(pClient, "MiningDronesDeactivatedCargoHoldFull", {{"modulename", new PyString(m_pDrone->GetName())}});
        ReturnHome();
        return;
    }

    ItemData idata(roidRef->typeID(), pClient->GetCharacterID(), locTemp, flagNone, static_cast<int32>(units));
    InventoryItemRef oRef(sItemFactory.SpawnItem(idata));
    if (oRef.get() == nullptr) {
        _log(DRONE__ERROR, "Drone %s(%u): could not create mined ore for %s", m_pDrone->GetName(), m_pDrone->GetID(), pClient->GetName());
        return;
    }
    oRef->MergeTypesInCargo(shipRef.get(), flagCargoHold);
    sStatMgr.Add(Stat::oreMined, units * oreVolume);

    roidQuantity -= units;
    if (roidQuantity > 0.0) {
        roidRef->SetAttribute(AttrQuantity, roidQuantity);
        // same radius-to-quantity relation the mining lasers use
        roidRef->SetAttribute(AttrRadius, std::exp((roidQuantity + 112404.8) / 25000));
        return;
    }

    // rock is depleted
    pRoid->Delete();
    SafeDelete(pRoid);
}
