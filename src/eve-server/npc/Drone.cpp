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
#include "inventory/AttributeEnum.h"
#include "npc/Drone.h"
#include "ship/Ship.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/AnomalyMgr.h"

DroneSE::DroneSE(InventoryItemRef drone, EVEServiceManager &services, SystemManager* pSystem, const FactionData& data)
: DynamicSystemEntity(drone, services, pSystem),
  m_AI(new DroneAIMgr(this)),
  m_pShipSE(nullptr),
  m_system(pSystem)
  {
      assert (m_AI != nullptr);
      assert (m_system != nullptr);

    m_online = false;

    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;
    m_targetID = 0;
    m_controllerID = 0;
    m_controllerOwnerID = 0;

    m_orbitRange = m_self->GetAttribute(AttrOrbitRange).get_float();
    if (m_orbitRange < 1) {
        const double maxRange = m_self->GetAttribute(AttrMaxRange).get_double();
        const double falloff = m_self->GetAttribute(AttrFalloff).get_double();
        m_orbitRange = (maxRange < falloff) ? maxRange : falloff;
        // Old drone imports often omit orbitRange and carry very large weapon
        // ranges. Never use those values as the idle orbit radius around ship.
        if (m_orbitRange > 2500.0)
            m_orbitRange = 2500.0;
    }
    if (m_orbitRange < 1.0)
        m_orbitRange = 500.0;

    // Create default dynamic attributes in the AttributeMap:
    m_self->SetAttribute(AttrInertia,             EvilOne, false);
    m_self->SetAttribute(AttrDamage,              EvilZero, false);
    m_self->SetAttribute(AttrArmorDamage,         EvilZero, false);
    m_self->SetAttribute(AttrWarpCapacitorNeed,   0.00001, false);
    m_self->SetAttribute(AttrOrbitRange,          m_orbitRange, false);
    m_self->SetAttribute(AttrMass,                m_self->type().mass(), false);
    m_self->SetAttribute(AttrRadius,              m_self->type().radius(), false);
    m_self->SetAttribute(AttrVolume,              m_self->type().volume(), false);
    m_self->SetAttribute(AttrCapacity,            m_self->type().capacity(), false);
    m_self->SetAttribute(AttrShieldCharge,        m_self->GetAttribute(AttrShieldCapacity), false);
    m_self->SetAttribute(AttrCapacitorCharge,     m_self->GetAttribute(AttrCapacitorCapacity), false);

    /** @todo update attribs from char skills here....it's not done by Fx system */

    m_destiny->UpdateShipVariables();

    SetResists();

    /* Gets the value from the Drone and put on our own vars */
    m_emDamage = m_self->GetAttribute(AttrEmDamage).get_float(),
    m_kinDamage = m_self->GetAttribute(AttrKineticDamage).get_float(),
    m_therDamage = m_self->GetAttribute(AttrThermalDamage).get_float(),
    m_expDamage = m_self->GetAttribute(AttrExplosiveDamage).get_float(),
    m_hullDamage = m_self->GetAttribute(AttrDamage).get_float();
    m_armorDamage = m_self->GetAttribute(AttrArmorDamage).get_float();
    m_shieldCharge = m_self->GetAttribute(AttrShieldCharge).get_float();
    m_shieldCapacity = m_self->GetAttribute(AttrShieldCapacity).get_float();

    _log(DRONE__TRACE, "Created Drone object for %s (%u)", drone.get()->name(), drone.get()->itemID());
}

DroneSE::~DroneSE() {
    SafeDelete(m_AI);
}

Client* DroneSE::GetOwner() const {
    // Client objects are session-owned and may be destroyed on disconnect.
    // Resolve the owner through the live player registry instead of retaining
    // a raw Client pointer for the lifetime of a space drone.
    return (m_ownerID != 0) ? sEntityList.FindClientByCharID(m_ownerID) : nullptr;
}

uint32 DroneSE::GetBounty() const {
    Client* owner = GetOwner();
    if (owner == nullptr)
        return 0;
    auto character = owner->GetChar();
    return (character.get() != nullptr) ? character->bounty() : 0;
}

float DroneSE::GetSecurityRating() const {
    Client* owner = GetOwner();
    if (owner == nullptr)
        return 0.0f;
    auto character = owner->GetChar();
    return (character.get() != nullptr) ? character->GetSecurityRating() : 0.0f;
}

void DroneSE::SetOwner(Client* pClient) {
    if (pClient == nullptr)
        return;

    m_self->ChangeOwner(pClient->GetCharacterID());
    m_ownerID = pClient->GetCharacterID();
    m_corpID = pClient->GetCorporationID();
    m_allyID = pClient->GetAllianceID();
    m_warID = pClient->GetWarFactionID();
    m_pShipSE = pClient->GetShipSE();
    m_controllerID = (m_pShipSE != nullptr) ? m_pShipSE->GetID() : 0;
    m_controllerOwnerID = (m_pShipSE != nullptr) ? m_pShipSE->GetOwnerID() : 0;
}

ShipSE* DroneSE::GetHomeShip() {
    // m_pShipSE is a non-owning pointer and can become stale if the controlling
    // ship is destroyed or replaced. Resolve the controller ID before exposing
    // the pointer to drone AI or scoop/return code.
    if (m_system == nullptr || m_controllerID == 0)
        return nullptr;

    SystemEntity* controller = m_system->GetSE(m_controllerID);
    if (controller == nullptr || !controller->IsShipSE())
        return nullptr;

    ShipSE* liveShip = controller->GetShipSE();
    if (liveShip != m_pShipSE)
        m_pShipSE = liveShip;
    return liveShip;
}

void DroneSE::Process() {
    if (m_killed)
        return;
    double profileStartTime(GetTimeUSeconds());

    // Process movement and target-lock timers before running drone combat AI.
    SystemEntity::Process();

    if (m_online && m_AI != nullptr) {
        m_AI->Process();
        // Complete the inventory/system removal outside the AI call so the AI
        // object is not destroyed while one of its methods is still executing.
        if (m_AI->IsScoopRequested()) {
            if (ScoopToBay())
                return; // this entity has been removed and deleted
            Client* owner = GetOwner();
            if (owner != nullptr)
                owner->SendNotifyMsg("The drone could not be returned to the drone bay. Check your drone bay capacity.");
            m_AI->Return(false);
        }
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::drone, GetTimeUSeconds() - profileStartTime);
}

void DroneSE::SaveDrone() {
    m_self->SaveItem();
}

void DroneSE::RemoveDrone() {
    // this seems to work properly
    m_self->Delete();
    delete this;
}

void DroneSE::Launch(ShipSE* pShipSE) {
    m_pShipSE = pShipSE;

    m_controllerID = pShipSE->GetID();
    m_controllerOwnerID = pShipSE->GetOwnerID();

    m_system->AddEntity(this);

    assert (m_bubble != nullptr);
}

void DroneSE::Online(ShipSE* pShipSE/*nullptr*/) {
    m_online = true;
    if (m_AI != nullptr)
        m_AI->SetAutoEngage(true);
    StateChange();

    if (pShipSE == nullptr)
        pShipSE = GetHomeShip();
    if (pShipSE == nullptr) {
        m_online = false;
        StateChange();
        return;
    }

    m_AI->AssignShip(pShipSE);
}

void DroneSE::Offline() {
    // This is called by scoop and abandon. Clear combat locks before releasing
    // control so neither side retains stale target references.
    if (m_AI != nullptr)
        m_AI->ClearAllTargets();
    if (m_destiny != nullptr)
        m_destiny->Stop();
    if (m_AI != nullptr)
        m_AI->AssignShip(nullptr);
    m_online = false;
    StateChange();
}

bool DroneSE::ScoopToBay() {
    ShipSE* homeShip = GetHomeShip();
    Client* owner = GetOwner();
    if (!m_online || owner == nullptr || homeShip == nullptr
        || m_system == nullptr || m_self.get() == nullptr)
        return false;

    // EVE's normal drone scoop interaction range is 2,500 metres.
    if (GetPosition().distance(homeShip->GetPosition()) > 2500.0)
        return false;

    ShipItemRef ship = owner->GetShip();
    if (ship.get() == nullptr || ship->itemID() != homeShip->GetID())
        return false;

    try {
        if (!ship->GetMyInventory()->ValidateAddItem(flagDroneBay, m_self))
            return false;

        m_self->ChangeOwner(owner->GetCharacterID(), true);
        owner->MoveItem(m_self->itemID(), ship->itemID(), flagDroneBay);
    } catch (...) {
        // Capacity or inventory errors must not escape the system process loop.
        return false;
    }

    homeShip->ScoopDrone(this);
    m_system->RemoveEntity(this);
    delete this;
    return true;
}

void DroneSE::IdleOrbit(ShipSE* pShipSE/*nullptr*/) {
    if (pShipSE == nullptr)
        pShipSE = GetHomeShip();

    if (!m_online || pShipSE == nullptr || m_destiny == nullptr)
        return;

    // TODO:  fix these speeds
    // set speed and begin orbit
    m_destiny->SetMaxVelocity(500);
    m_destiny->SetSpeedFraction(0.6f);
    m_destiny->Orbit(pShipSE, m_orbitRange);
}

void DroneSE::Abandon() {
    SystemEntity::Abandon();
    Offline();
    ClearShipControl();
}

/*   when drone is scooped up....
 *
 *                    [PyTuple 2 items]
 *                      [PyString "OnDroneStateChange"]
 *                      [PyList 7 items]
 *                        [PyIntegerVar 1540263056]
 *                        [PyNone]
 *                        [PyNone]
 *                        [PyNone]
 *                        [PyNone]
 *                        [PyNone]
 *                        [PyNone]
 */

void DroneSE::StateChange() {
    if (m_bubble == nullptr)
        return;

    //OnDroneStateChange(droneID, ownerID, controllerID, activityState, droneTypeID, controllerOwnerID, targetID)
    if (m_online) {
        OnDroneStateChange du;
            du.droneID = m_self->itemID();
            du.ownerID = m_ownerID;
            du.droneTypeID = m_self->typeID();
            du.controllerID = m_controllerID;
            du.controllerOwnerID = m_controllerOwnerID;
            du.activityState = m_AI->GetState();
            du.targetID = m_targetID;
        PyTuple* up = du.Encode();
        // bubblecast is faster than destiny::update
        m_bubble->BubblecastDestinyUpdate(&up, "destiny");
        //pShipSE->DestinyMgr()->SendSingleDestinyUpdate(&up);
    } else {
        PyList* list = new PyList();
            list->AddItemInt(m_self->itemID());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
        PyTuple* tuple = new PyTuple(2);
            tuple->SetItem(0, new PyString("OnDroneStateChange"));
            tuple->SetItem(1, list);
        m_bubble->BubblecastDestinyUpdate(&tuple, "destiny");
    }
}

void DroneSE::TargetAdded(SystemEntity* who) {
    if (who == nullptr)
        return;

    SetTarget(who);
    m_AI->TargetAcquired(who);
    StateChange();
}

void DroneSE::TargetLost(SystemEntity *who) {
    if (who == nullptr || m_targetID == who->GetID())
        SetTarget();

    m_AI->TargetLost(who);

    // Keep the client-visible target in sync when another locked target remains.
    if (TargetMgr() != nullptr) {
        SystemEntity* nextTarget = TargetMgr()->GetFirstTarget(true);
        if (nextTarget != nullptr)
            SetTarget(nextTarget);
    }
    StateChange();
}

void DroneSE::TargetedAdd(SystemEntity *who) {
    m_AI->Targeted(who);
}

void DroneSE::TargetedLost(SystemEntity* who) {
    // This callback means another entity stopped locking this drone. It does not
    // change the drone's own target or combat state.
    if (who != nullptr)
        _log(DRONE__AI_TRACE, "Drone %s(%u): %s(%u) stopped targeting it.",
             GetName(), GetID(), who->GetName(), who->GetID());
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

    // drone id's begin at 500m
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
    DataSector data = DataSector();
        data.maxSpeed = m_destiny->GetMaxVelocity();
        data.velX = m_destiny->GetVelocity().x;
        data.velY = m_destiny->GetVelocity().y;
        data.velZ = m_destiny->GetVelocity().z;
        data.inertia = m_destiny->GetInertia();
        data.speedfraction = m_destiny->GetCommandedSpeedFraction();
    into.Append( data );
    switch (mode) {
        case Ball::Mode::WARP: {
            m_destiny->EncodeWarpState(into);
        }  break;
        case Ball::Mode::FOLLOW: {
            FOLLOW_Struct follow;
                follow.followID = m_destiny->GetTargetID();
                follow.followRange = m_destiny->GetFollowRange();
                follow.formationID = 0xFF;
            into.Append( follow );
        }  break;
        case Ball::Mode::ORBIT: {
            ORBIT_Struct orbit;
                orbit.targetID = m_destiny->GetTargetID();
                orbit.followRange = m_destiny->GetFollowDistance();
                orbit.formationID = 0xFF;
            into.Append( orbit );
        }  break;
        case Ball::Mode::GOTO: {
            GOTO_Struct goto_; 
                goto_.formationID = 0xFF;
                goto_.x = m_destiny->GetTargetPoint().x;
                goto_.y = m_destiny->GetTargetPoint().y;
                goto_.z = m_destiny->GetTargetPoint().z;
            into.Append( goto_ );
        }  break;
        case Ball::Mode::STOP: {
            STOP_Struct stop;
                stop.formationID = 0xFF;
            into.Append( stop );
        }  break;
        default: {
            _log(DRONE__ERROR, "DroneSE::EncodeDestiny - Unknown destiny mode %u for %s", mode, m_self->name());
        }  break;
    }
}


void DroneSE::MakeDamageState(DoDestinyDamageState& into) {
    const auto clamp01 = [](double value) -> double {
        if (value < 0.0) return 0.0;
        if (value > 1.0) return 1.0;
        return value;
    };
    const double shieldCapacity = m_self->GetAttribute(AttrShieldCapacity).get_double();
    const double armorCapacity = m_self->GetAttribute(AttrArmorHP).get_double();
    const double hullCapacity = m_self->GetAttribute(AttrHP).get_double();
    into.shield = (shieldCapacity > 0.0)
        ? clamp01(m_self->GetAttribute(AttrShieldCharge).get_double() / shieldCapacity) : 0.0;
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_double() + 5.0;
    into.timestamp = GetFileTimeNow();
    into.armor = (armorCapacity > 0.0)
        ? clamp01(1.0 - m_self->GetAttribute(AttrArmorDamage).get_double() / armorCapacity) : 0.0;
    into.structure = (hullCapacity > 0.0)
        ? clamp01(1.0 - m_self->GetAttribute(AttrDamage).get_double() / hullCapacity) : 0.0;
}

void DroneSE::SetResists() {
    // Some legacy drone types are missing resistance attributes in static data.
    // Add defaults only where an attribute is absent.
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

void DroneSE::Killed(Damage& damage) {
    // Unregister from the controlling ship before the item is deleted.
    // Resolve the controller ID first; m_pShipSE is a non-owning cached pointer
    // and may have become stale if the ship was removed before this drone died.
    ShipSE* homeShip = GetHomeShip();
    if (homeShip != nullptr)
        homeShip->ScoopDrone(this);

    if (m_bubble == nullptr || m_destiny == nullptr || m_system == nullptr || m_self.get() == nullptr) {
        sLog.Error("DroneSE::Killed()", "Cannot create drone wreck: required state unavailable for drone %u", GetID());
        return;
    }

    SystemEntity* killer = damage.srcSE;
    Client* pClient = nullptr;
    uint32 killerID = 0;
    if (killer != nullptr) {
        if (killer->HasPilot()) {
            pClient = killer->GetPilot();
            if (pClient != nullptr) killerID = pClient->GetCharacterID();
        } else if (killer->IsDroneSE()) {
            pClient = killer->GetDroneSE()->GetOwner();
            if (pClient != nullptr) killerID = pClient->GetCharacterID();
        } else {
            killerID = killer->GetID();
        }
    }

    if (pClient != nullptr) {
        AwardBounty(pClient);
        if (m_system->GetSystemSecurityRating() > 0.0)
            AwardSecurityStatus(m_self, pClient->GetChar().get());
    }

    const GPoint wreckPosition = m_destiny->GetPosition();
    if (wreckPosition.isNaN()) {
        sLog.Error("DroneSE::Killed()", "Wreck position is invalid for drone %u", GetID());
        return;
    }

    const uint32 wreckTypeID = 26972; // Faction drone wreck.
    const std::string wreckName = m_self->itemName() + " Wreck";
    ItemData wreckItemData(wreckTypeID, killerID, m_system->GetID(), flagNone,
                           wreckName.c_str(), wreckPosition, itoa(m_allyID));
    WreckContainerRef wreckItemRef = sItemFactory.SpawnWreckContainer(wreckItemData);
    if (wreckItemRef.get() == nullptr) {
        sLog.Error("DroneSE::Killed()", "Creating wreck failed for drone %s(%u)", m_self->name(), m_self->itemID());
        return;
    }

    if (is_log_enabled(PHYSICS__TRACE)) {
        _log(PHYSICS__TRACE,
             "DroneSE::Killed() - Drone %s(%u) Position: %.2f,%.2f,%.2f. Wreck %s(%u) Position: %.2f,%.2f,%.2f.",
             GetName(), GetID(), x(), y(), z(), wreckItemRef->name(), wreckItemRef->itemID(),
             wreckPosition.x, wreckPosition.y, wreckPosition.z);
    }

    DBSystemDynamicEntity wreckEntity = DBSystemDynamicEntity();
    wreckEntity.allianceID = (killer != nullptr && killer->GetAllianceID() != 0) ? killer->GetAllianceID() : m_allyID;
    wreckEntity.categoryID = EVEDB::invCategories::Celestial;
    wreckEntity.corporationID = (killer != nullptr) ? killer->GetCorporationID() : m_corpID;
    wreckEntity.factionID = (killer != nullptr && killer->GetWarFactionID() != 0) ? killer->GetWarFactionID() : m_warID;
    wreckEntity.groupID = EVEDB::invGroups::Wreck;
    wreckEntity.itemID = wreckItemRef->itemID();
    wreckEntity.itemName = wreckName;
    wreckEntity.ownerID = killerID;
    wreckEntity.typeID = wreckTypeID;
    wreckEntity.position = wreckPosition;

    if (!m_system->BuildDynamicEntity(wreckEntity, m_self->itemID())) {
        sLog.Error("DroneSE::Killed()", "Spawning wreck failed for drone typeID %u", m_self->typeID());
        wreckItemRef->Delete();
        return;
    }
    m_destiny->SendJettisonPacket();
}
