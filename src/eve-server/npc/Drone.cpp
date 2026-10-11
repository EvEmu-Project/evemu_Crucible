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
    Author:     Aknor Jaden
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "EntityList.h"
#include "effects/EffectsDataMgr.h"
#include "inventory/AttributeEnum.h"
#include "npc/Drone.h"
#include "ship/Ship.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"

namespace {
    // drones with no mass would break the movement equations (on the client too).  some mining drones have none
    const double DRONE_DEFAULT_MASS = 3000.0;
    // base drone control range, if the character doesn't have the attribute
    const double DRONE_BASE_CONTROL_RANGE = 20000.0;

    // classifies a drone's main effect by what it does
    uint8 ActionForEffect(const std::string& name, bool offensive, bool assistance) {
        using namespace DroneAI::Action;
        if ((name == "targetAttack") or (name == "fighterMissile") or (name == "missileLaunchingForEntity"))
            return Attack;
        if (name == "mining")
            return Mine;
        if (name == "shieldTransfer")
            return RepairShield;
        if (name == "targetArmorRepair")
            return RepairArmor;
        if (name == "decreaseTargetSpeed")
            return Web;
        if (offensive or assistance)
            return Ewar;
        return None;
    }
}

DroneSE::DroneSE(InventoryItemRef drone, EVEServiceManager &services, SystemManager* pSystem, const FactionData& data)
: DynamicSystemEntity(drone, services, pSystem),
  m_AI(nullptr),
  m_system(pSystem),
  m_controllerID(0),
  m_controllerOwnerID(0),
  m_returnRequested(false),
  m_stats()
{
    assert (m_system != nullptr);

    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;

    // attributes the movement and damage code expect.  existing damage is kept: launching does not repair a drone
    if (!m_self->HasAttribute(AttrDamage))
        m_self->SetAttribute(AttrDamage,          EvilZero, false);
    if (!m_self->HasAttribute(AttrArmorDamage))
        m_self->SetAttribute(AttrArmorDamage,     EvilZero, false);
    if (!m_self->HasAttribute(AttrShieldCharge))
        m_self->SetAttribute(AttrShieldCharge,    m_self->GetAttribute(AttrShieldCapacity), false);
    m_self->SetAttribute(AttrWarpCapacitorNeed,   0.00001, false);
    double mass(m_self->type().mass());
    if (mass <= 0.0)
        mass = DRONE_DEFAULT_MASS;
    m_self->SetAttribute(AttrMass,                mass, false);
    m_self->SetAttribute(AttrRadius,              m_self->type().radius(), false);
    m_self->SetAttribute(AttrVolume,              m_self->type().volume(), false);
    m_self->SetAttribute(AttrCapacity,            m_self->type().capacity(), false);
    m_self->SetAttribute(AttrCapacitorCharge,     m_self->GetAttribute(AttrCapacitorCapacity), false);

    SetResists();

    m_AI = new DroneAIMgr(this);
    UpdateStats(nullptr);
    m_destiny->UpdateShipVariables();

    _log(DRONE__TRACE, "Created Drone object for %s (%u)", drone.get()->name(), drone.get()->itemID());
}

DroneSE::~DroneSE() {
    SafeDelete(m_AI);
}

void DroneSE::Process() {
    if (m_killed)
        return;
    double profileStartTime(GetTimeUSeconds());

    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();

    if (IsControlled())
        m_AI->Process();

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::drone, GetTimeUSeconds() - profileStartTime);
}

/* works out the drone's working numbers from its type (not its item, which holds our modified values) and the
 *  pilot's skills.  bonus sizes are from the skills' own attributes in the static data:
 *   Drone Interfacing          +20% damage and mining yield (all drones)
 *   Combat Drone Operation     +5% damage, drones requiring Scout Drone Operation (light/medium)
 *   Heavy Drone Operation      +5% damage, heavy drones
 *   Sentry Drone Interfacing   +5% damage, sentries
 *   racial specializations     +2% damage, drones requiring them
 *   Drone Navigation           +5% max velocity
 *   Drone Sharpshooting        +5% optimal range
 *   Mining Drone Operation     +5% mining yield, Mining Drone Specialization +3%
 *   Repair Drone Operation     +5% repair amount
 *  ship bonuses to drones (Vexor, Ishtar...) are not applied.
 */
void DroneSE::UpdateStats(Client* pController) {
    using namespace EvESkill;
    const ItemType& type(m_self->type());
    auto base = [&](uint16 attrID, double def) -> double {
        return (type.HasAttribute(attrID) ? type.GetAttribute(attrID).get_double() : def);
    };
    Character* pChar((pController == nullptr) ? nullptr : pController->GetChar().get());
    auto level = [&](uint16 skillID) -> double {
        if (pChar == nullptr)
            return 0.0;
        int8 lvl(pChar->GetSkillLevel(skillID));
        return (lvl < 0 ? 0.0 : static_cast<double>(lvl));
    };
    auto needsSkill = [&](uint16 skillID) -> bool {
        static const uint16 reqAttrs[] = { AttrRequiredSkill1, AttrRequiredSkill2, AttrRequiredSkill3, AttrRequiredSkill4 };
        for (auto attrID : reqAttrs)
            if (static_cast<uint16>(base(attrID, 0.0)) == skillID)
                return true;
        return false;
    };

    DroneStats s = DroneStats();

    // main effect
    std::vector<TypeEffects> fx;
    sFxDataMgr.GetTypeEffect(m_self->typeID(), fx);
    Effect effect = Effect();
    for (auto cur : fx) {
        Effect e(sFxDataMgr.GetEffect(cur.effectID));
        uint8 action(ActionForEffect(e.effectName, e.isOffensive, e.isAssistance));
        if (action == DroneAI::Action::None)
            continue;
        // targetAttack beats anything else on test types that have several
        if (cur.isDefault or (s.action == DroneAI::Action::None) or (action == DroneAI::Action::Attack)) {
            s.action = action;
            effect = e;
        }
        if (cur.isDefault)
            break;
    }
    s.effectID = effect.effectID;
    s.offensive = effect.isOffensive;
    s.assistance = effect.isAssistance;
    s.effectGuid = effect.guid;

    double navBonus(1.0 + 0.05 * level(DroneNavigation));
    s.maxVelocity = base(AttrMaxVelocity, 0.0) * navBonus;
    s.orbitVelocity = base(AttrEntityCruiseSpeed, 0.0);
    if ((s.orbitVelocity <= 0.0) or (s.orbitVelocity > s.maxVelocity))
        s.orbitVelocity = s.maxVelocity;

    s.optimalRange = base(AttrMaxRange, 0.0) * (1.0 + 0.05 * level(DroneSharpshooting));
    s.falloff = base(AttrFalloff, 0.0);
    s.tracking = base(AttrTrackingSpeed, 0.0);
    s.sigResolution = base(AttrOptimalSigRadius, 40.0);
    if (s.action == DroneAI::Action::Attack) {
        // turrets keep firing into falloff, with falling hit chance
        s.effectRange = s.optimalRange + s.falloff * 2.0;
    } else if (effect.rangeAttributeID != 0) {
        s.effectRange = base(effect.rangeAttributeID, s.optimalRange);
        if (effect.rangeAttributeID == AttrMaxRange)
            s.effectRange = s.optimalRange;
    } else {
        s.effectRange = s.optimalRange;
    }
    if (s.effectRange <= 0.0)
        s.effectRange = 1000.0;

    s.orbitRange = base(AttrEntityFlyRange, 0.0);
    if (s.orbitRange <= 0.0)
        s.orbitRange = base(AttrOrbitRange, 0.0);
    if (s.orbitRange <= 0.0)
        s.orbitRange = 500.0;
    if (s.orbitRange > s.effectRange)
        s.orbitRange = s.effectRange * 0.8;
    s.idleOrbitRange = s.orbitRange;

    s.cycleTime = (effect.durationAttributeID != 0 ? base(effect.durationAttributeID, 0.0) : 0.0);
    if (s.cycleTime <= 0.0)
        s.cycleTime = base(AttrSpeed, 0.0);
    if (s.cycleTime <= 0.0)
        s.cycleTime = 5000.0;

    s.damageMultiplier = base(AttrDamageMultiplier, 1.0) * (1.0 + 0.20 * level(DroneInterfacing));
    if (needsSkill(ScoutDroneOperation))
        s.damageMultiplier *= (1.0 + 0.05 * level(CombatDroneOperation));
    if (needsSkill(HeavyDroneOperation))
        s.damageMultiplier *= (1.0 + 0.05 * level(HeavyDroneOperation));
    if (needsSkill(SentryDroneInterfacing))
        s.damageMultiplier *= (1.0 + 0.05 * level(SentryDroneInterfacing));
    for (auto spec : { AmarrDroneSpecialization, MinmatarDroneSpecialization, GallenteDroneSpecialization, CaldariDroneSpecialization })
        if (needsSkill(spec))
            s.damageMultiplier *= (1.0 + 0.02 * level(spec));

    s.miningAmount = base(AttrMiningAmount, 0.0) * (1.0 + 0.20 * level(DroneInterfacing)) * (1.0 + 0.05 * level(MiningDroneOperation));
    if (needsSkill(MiningDroneSpecialization))
        s.miningAmount *= (1.0 + 0.03 * level(MiningDroneSpecialization));

    if (s.action == DroneAI::Action::RepairShield) {
        s.repairAmount = base(AttrShieldBonus, 0.0);
    } else if (s.action == DroneAI::Action::RepairArmor) {
        s.repairAmount = base(AttrArmorDamageAmount, 0.0);
    }
    s.repairAmount *= (1.0 + 0.05 * level(RepairDroneOperation));

    s.bandwidth = base(AttrDroneBandwidthUsed, 0.0);
    m_stats = s;

    // the movement code caps our speed at this attribute.  it is recomputed from the type each time, so never compounds
    m_self->SetAttribute(AttrMaxVelocity, s.maxVelocity, false);

    _log(DRONE__TRACE, "Drone %s(%u) stats: action %u (%s), speed %.0f/%.0f, orbit %.0f, range %.0f+%.0f (use %.0f), cycle %.0fms, dmg x%.3f, mine %.1f, rep %.1f",
         GetName(), GetID(), s.action, effect.effectName.c_str(), s.maxVelocity, s.orbitVelocity, s.orbitRange, s.optimalRange,
         s.falloff, s.effectRange, s.cycleTime, s.damageMultiplier, s.miningAmount, s.repairAmount);
}

/* ======================================================================================
 *  control
 * ====================================================================================== */

ShipSE* DroneSE::GetController() {
    if (m_controllerID == 0)
        return nullptr;
    SystemEntity* pSE(m_system->GetSE(m_controllerID));
    if ((pSE == nullptr) or !pSE->IsShipSE() or !pSE->HasPilot())
        return nullptr;
    if (static_cast<uint32>(pSE->GetPilot()->GetCharacterID()) != m_controllerOwnerID)
        return nullptr;
    return pSE->GetShipSE();
}

Client* DroneSE::GetControllerClient() {
    ShipSE* pShip(GetController());
    return (pShip == nullptr ? nullptr : pShip->GetPilot());
}

Client* DroneSE::GetOwnerClient() {
    return sEntityList.FindClientByCharID(m_ownerID);
}

void DroneSE::SetController(ShipSE* pShip) {
    if ((pShip == nullptr) or !pShip->HasPilot())
        return;
    Client* pClient(pShip->GetPilot());
    m_controllerID = pShip->GetID();
    m_controllerOwnerID = pClient->GetCharacterID();
    UpdateStats(pClient);
    m_AI->SetIdle();
    SendStateChange();
    UpdateBandwidthLoad(pShip);
    _log(DRONE__TRACE, "Drone %s(%u) now controlled by %s's %s(%u)", GetName(), GetID(), pClient->GetName(), pShip->GetName(), pShip->GetID());
}

void DroneSE::ReleaseControl() {
    if (!IsControlled())
        return;

    uint32 shipID(m_controllerID);
    Client* pController(sEntityList.FindClientByCharID(m_controllerOwnerID));
    Client* pOwner(GetOwnerClient());

    m_AI->Stop();
    m_controllerID = 0;
    m_controllerOwnerID = 0;

    SendStateChangeTo(pController, true);
    if (pOwner != pController)
        SendStateChangeTo(pOwner, true);

    SystemEntity* pShip(m_system->GetSE(shipID));
    if ((pShip != nullptr) and pShip->IsShipSE())
        UpdateBandwidthLoad(pShip->GetShipSE());

    _log(DRONE__TRACE, "Drone %s(%u) lost control from ship %u", GetName(), GetID(), shipID);

    // a delegated drone goes back to its owner, if they can take it
    if ((pOwner != nullptr) and (pOwner != pController)) {
        ShipSE* pOwnerShip(pOwner->GetShipSE());
        if ((pOwnerShip != nullptr) and (pOwnerShip->SystemMgr() == m_system) and (pOwnerShip->SysBubble() == m_bubble)
        and !pOwner->InPod() and (CountControlledBy(m_system, m_ownerID) < pOwner->GetChar()->GetAttribute(AttrMaxActiveDrones).get_uint32()))
            SetController(pOwnerShip);
    }
}

void DroneSE::SendStateChange() {
    if (!IsControlled())
        return;
    Client* pController(GetControllerClient());
    Client* pOwner(GetOwnerClient());
    SendStateChangeTo(pController, false);
    if (pOwner != pController)
        SendStateChangeTo(pOwner, false);
}

//OnDroneStateChange(droneID, ownerID, controllerID, activityState, droneTypeID, controllerOwnerID, targetID)
// the client drops the drone from its list when neither ownerID nor controllerID are the pilot's, so all None removes it
void DroneSE::SendStateChangeTo(Client* pClient, bool remove) {
    if ((pClient == nullptr) or !pClient->IsInSpace() or (pClient->GetShipSE() == nullptr))
        return;
    PyTuple* args = new PyTuple(7);
        args->SetItem(0, new PyInt(GetID()));
    if (remove) {
        for (uint8 i = 1; i < 7; ++i)
            args->SetItem(i, PyStatic.NewNone());
    } else {
        uint32 targetID(m_AI->GetTargetID());
        args->SetItem(1, new PyInt(m_ownerID));
        args->SetItem(2, new PyInt(m_controllerID));
        args->SetItem(3, new PyInt(m_AI->GetState()));
        args->SetItem(4, new PyInt(m_self->typeID()));
        args->SetItem(5, new PyInt(m_controllerOwnerID));
        args->SetItem(6, (targetID == 0 ? PyStatic.NewNone() : new PyInt(targetID)));
    }
    PyTuple* up = new PyTuple(2);
        up->SetItem(0, new PyString("OnDroneStateChange"));
        up->SetItem(1, args);
    pClient->QueueDestinyUpdate(&up);
}

double DroneSE::ControllerDistance() {
    ShipSE* pShip(GetController());
    if (pShip == nullptr)
        return -1.0;
    return GetPosition().distance(pShip->GetPosition()) - GetRadius() - pShip->GetRadius();
}

double DroneSE::ControlRange() {
    Client* pClient(GetControllerClient());
    if (pClient == nullptr)
        return DRONE_BASE_CONTROL_RANGE;
    double range(pClient->GetChar()->GetAttribute(AttrDroneControlDistance).get_double());
    return (range > 0.0 ? range : DRONE_BASE_CONTROL_RANGE);
}

uint32 DroneSE::GetBounty() {
    Client* pOwner(GetOwnerClient());
    return (pOwner == nullptr ? 0 : pOwner->GetChar()->bounty());
}

float DroneSE::GetSecurityRating() {
    Client* pOwner(GetOwnerClient());
    return (pOwner == nullptr ? 0.0f : pOwner->GetChar()->GetSecurityRating());
}

/* ======================================================================================
 *  scooping and returning to the bay
 * ====================================================================================== */

void DroneSE::RequestReturnToBay() {
    if (m_returnRequested)
        return;
    m_returnRequested = true;
    m_system->QueueDroneReturn(GetID());
}

bool DroneSE::FinishReturnToBay() {
    m_returnRequested = false;
    ShipSE* pShip(GetController());
    if (pShip == nullptr)
        return false;
    try {
        ScoopTo(pShip, flagDroneBay);
        return true;
    } catch (PyException& e) {
        // no room in the bay (or the hold won't take it).  wait by the ship
        pShip->GetPilot()->SendNotifyMsg("Your %s cannot return to your drone bay.", GetName());
        m_AI->SetIdle();
    }
    return false;
}

// the hold checks throw, before anything changes
void DroneSE::ScoopTo(ShipSE* pShip, EVEItemFlags flag) {
    Client* pClient(pShip->GetPilot());
    ShipItemRef shipRef(pShip->GetShipItemRef());
    InventoryItemRef iRef(m_self);
    if (flag == flagDroneBay)
        shipRef->VerifyHoldType(flagDroneBay, iRef, pClient);
    shipRef->GetMyInventory()->ValidateAddItem(flag, iRef);

    uint32 oldShipID(m_controllerID);
    ReleaseControl();

    iRef->ChangeOwner(pClient->GetCharacterID(), true);
    pClient->MoveItem(iRef->itemID(), shipRef->itemID(), flag);
    m_system->RemoveEntity(this);

    // drones without structure or armor damage go back onto their stack
    if ((iRef->GetAttribute(AttrDamage).get_double() <= 0.0) and (iRef->GetAttribute(AttrArmorDamage).get_double() <= 0.0)) {
        iRef->ChangeSingleton(false, true);
        iRef->MergeTypesInCargo(shipRef.get(), flag);
    }

    if (oldShipID != pShip->GetID()) {
        SystemEntity* pOld(m_system->GetSE(oldShipID));
        if ((pOld != nullptr) and pOld->IsShipSE())
            UpdateBandwidthLoad(pOld->GetShipSE());
    }
    _log(DRONE__TRACE, "Drone %u scooped to %s of %s(%u)", iRef->itemID(), sDataMgr.GetFlagName(flag), pShip->GetName(), pShip->GetID());
}

/* ======================================================================================
 *  helpers for all drones in a system
 * ====================================================================================== */

uint8 DroneSE::CountControlledBy(SystemManager* pSystem, uint32 charID) {
    uint8 count(0);
    for (auto cur : pSystem->GetDrones())
        if (cur.second->IsControlled() and (cur.second->GetControllerOwnerID() == charID))
            ++count;
    return count;
}

double DroneSE::BandwidthUsedBy(SystemManager* pSystem, uint32 shipID) {
    double used(0.0);
    for (auto cur : pSystem->GetDrones())
        if (cur.second->GetControllerID() == shipID)
            used += cur.second->Stats().bandwidth;
    return used;
}

void DroneSE::ReleaseDronesOf(SystemManager* pSystem, uint32 shipID) {
    std::vector<DroneSE*> drones;
    for (auto cur : pSystem->GetDrones())
        if (cur.second->GetControllerID() == shipID)
            drones.push_back(cur.second);
    for (auto cur : drones)
        cur->ReleaseControl();
}

void DroneSE::UpdateBandwidthLoad(ShipSE* pShip) {
    if (pShip == nullptr)
        return;
    double load(BandwidthUsedBy(pShip->SystemMgr(), pShip->GetID()));
    pShip->GetShipItemRef()->SetAttribute(AttrDroneBandwidthLoad, load, true);
}

void DroneSE::OnAggression(SystemEntity* pVictim, SystemEntity* pAttacker) {
    if ((pVictim == nullptr) or (pAttacker == nullptr) or (pVictim == pAttacker))
        return;
    SystemManager* pSystem(pVictim->SystemMgr());
    if (pSystem == nullptr)
        return;

    uint32 shipID(0), charID(0);
    if (pVictim->IsShipSE() and pVictim->HasPilot()) {
        shipID = pVictim->GetID();
        charID = pVictim->GetPilot()->GetCharacterID();
    } else if (pVictim->IsDroneSE()) {
        shipID = pVictim->GetDroneSE()->GetControllerID();
        charID = pVictim->GetDroneSE()->GetControllerOwnerID();
    }
    if (shipID == 0)
        return;
    // don't turn on our own ship or drones
    if (pAttacker->GetID() == shipID)
        return;
    if (pAttacker->IsDroneSE() and (pAttacker->GetDroneSE()->GetControllerID() == shipID))
        return;

    for (auto cur : pSystem->GetDrones()) {
        DroneSE* pDrone(cur.second);
        if (!pDrone->IsControlled())
            continue;
        if ((pDrone->GetControllerID() == shipID)
        or  ((pDrone->GetAI()->GetOrder() == DroneAI::Order::Guard) and (pDrone->GetAI()->GetActivityCharID() == charID)))
            pDrone->GetAI()->Aggressed(pAttacker);
    }
}

/* ======================================================================================
 *  SystemEntity interface
 * ====================================================================================== */

void DroneSE::Abandon() {
    ReleaseControl();
    SystemEntity::Abandon();

    // owner changed.  let everyone here know, so its owner is no longer offered to reconnect to it
    if (m_bubble == nullptr)
        return;
    PyTuple* data = new PyTuple(2);
        data->SetItem(0, new PyLong(GetID()));
        data->SetItem(1, new PyObject("foo.SlimItem", MakeSlimItem()));
    PyTuple* up = new PyTuple(2);
        up->SetItem(0, new PyString("OnSlimItemChange"));
        up->SetItem(1, data);
    m_bubble->BubblecastDestinyUpdate(&up, "destiny");
}

void DroneSE::TargetLost(SystemEntity* who) {
    m_AI->TargetLost(who);
}

void DroneSE::TargetedAdd(SystemEntity* who) {
    // being locked isn't aggression.  damage is (see OnAggression())
}

PyDict* DroneSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for Drone %u ", m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",          new PyInt(m_self->groupID()));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        slim->SetItemString("ownerID",          new PyInt(m_ownerID));
        slim->SetItemString("corpID",           IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
        slim->SetItemString("allianceID",       IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
        slim->SetItemString("warFactionID",     IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
        slim->SetItemString("bounty",           new PyFloat(GetBounty()));
        slim->SetItemString("securityStatus",   new PyFloat(GetSecurityRating()));
    return slim;
}

void DroneSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    uint8 mode = m_destiny->GetState(); //Ball::Mode::STOP;

    BallHeader head = BallHeader();
        head.entityID = GetID();
        head.mode = mode;
        head.radius = GetRadius();
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.flags = Ball::Flag::IsFree;
    into.Append( head );
    MassSector mass = MassSector();
        mass.mass = m_destiny->GetMass();
        mass.cloak = 0;
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (IsAlliance(m_allyID) ? m_allyID : -1);
    into.Append( mass );
    // movement data and mode-specific part are written by destiny
    m_destiny->EncodeDestiny( into );
    _log(SE__DESTINY, "DroneSE::EncodeDestiny(): %s - id:%lli, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void DroneSE::MakeDamageState(DoDestinyDamageState &into)
{
    auto fraction = [&](uint16 currentAttr, uint16 maxAttr, bool isDamage) -> double {
        double max(m_self->GetAttribute(maxAttr).get_double());
        if (max <= 0.0)
            return 1.0;
        double cur(m_self->GetAttribute(currentAttr).get_double());
        return (isDamage ? 1.0 - (cur / max) : (cur / max));
    };
    into.shield = fraction(AttrShieldCharge, AttrShieldCapacity, false);
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() + 5;
    into.timestamp = GetFileTimeNow();
    into.armor = fraction(AttrArmorDamage, AttrArmorHP, true);
    into.structure = fraction(AttrDamage, AttrHP, true);
}

void DroneSE::SetResists() {
    /* fix for missing resist attribs -allan 18April16  */
    if (!m_self->HasAttribute(AttrShieldEmDamageResonance)) m_self->SetAttribute(AttrShieldEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldExplosiveDamageResonance)) m_self->SetAttribute(AttrShieldExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldKineticDamageResonance)) m_self->SetAttribute(AttrShieldKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldThermalDamageResonance)) m_self->SetAttribute(AttrShieldThermalDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorEmDamageResonance)) m_self->SetAttribute(AttrArmorEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorExplosiveDamageResonance)) m_self->SetAttribute(AttrArmorExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorKineticDamageResonance)) m_self->SetAttribute(AttrArmorKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorThermalDamageResonance)) m_self->SetAttribute(AttrArmorThermalDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrEmDamageResonance)) m_self->SetAttribute(AttrEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrExplosiveDamageResonance)) m_self->SetAttribute(AttrExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrKineticDamageResonance)) m_self->SetAttribute(AttrKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrThermalDamageResonance)) m_self->SetAttribute(AttrThermalDamageResonance, EvilOne, false);
}

// player drones leave no wreck.  SystemEntity::Killed() removes us from the system afterwards
void DroneSE::Killed(Damage &damage) {
    ReleaseControl();
}
