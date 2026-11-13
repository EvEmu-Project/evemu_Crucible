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
  m_pClient(nullptr),
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
    m_pClient = sEntityList.FindClientByCharID(data.ownerID);
    m_targetID = 0;
    m_controllerID = 0;
    m_controllerOwnerID = 0;

    m_orbitRange = m_self->GetAttribute(AttrOrbitRange).get_float();
    if (m_orbitRange < 1) {
        if (m_self->GetAttribute(AttrMaxRange) < m_self->GetAttribute(AttrFalloff)) {
            m_orbitRange = m_self->GetAttribute(AttrMaxRange).get_float();
        } else {
            m_orbitRange = m_self->GetAttribute(AttrFalloff).get_float();
        }
    }

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

void DroneSE::SetOwner(Client* pClient) {
    m_self->ChangeOwner(pClient->GetCharacterID());
    m_pClient = pClient;
    m_ownerID = pClient->GetCharacterID();
    m_corpID = pClient->GetCorporationID();
    m_allyID = pClient->GetAllianceID();
    m_warID = pClient->GetWarFactionID();
    m_pShipSE = pClient->GetShipSE();
    m_controllerID = pClient->GetShipID();
    m_controllerOwnerID = m_pShipSE->GetOwnerID();
}

void DroneSE::Process() {
    if (m_killed)
        return;
    double profileStartTime(GetTimeUSeconds());

    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();

    /** @todo (allan) finish drone AI and processing */
    if (m_online)
        m_AI->Process();

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
    StateChange();

    if (pShipSE == nullptr)
        pShipSE = m_pShipSE;

    m_AI->AssignShip(pShipSE);
}

void DroneSE::Offline() {
    // this is called by abandon also
    m_destiny->Stop();
    m_AI->AssignShip(nullptr);
    m_online = false;
    StateChange();
}

void DroneSE::IdleOrbit(ShipSE* pShipSE/*nullptr*/) {
    if (pShipSE == nullptr)
        pShipSE = m_pShipSE;

    if (!m_online)
        return;         // error here?

    // TODO:  fix these speeds
    // set speed and begin orbit
    m_destiny->SetMaxVelocity(500);
    m_destiny->SetSpeedFraction(0.6f);
    m_destiny->Orbit(pShipSE, m_orbitRange);
}

void DroneSE::Abandon() {
    SystemEntity::Abandon();
    Offline();
    m_pClient = nullptr;
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
    /** @todo (Allan) will need code once drones are implemented */
}

void DroneSE::TargetLost(SystemEntity *who) {
    m_AI->TargetLost(who);
}

void DroneSE::TargetedAdd(SystemEntity *who) {
    m_AI->Targeted(who);
}

void DroneSE::TargetedLost(SystemEntity* who) {
    /** @todo (Allan) will need code once drones are implemented */
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
        data.speedfraction = m_destiny->GetSpeedFraction();
    into.Append( data );
    switch (mode) {
        case Ball::Mode::WARP: {
            m_destiny->EncodeWarpState(into);
        }  break;
        case Ball::Mode::FOLLOW: {
            FOLLOW_Struct follow;
                follow.followID = m_destiny->GetTargetID();
                follow.followRange = m_destiny->GetFollowDistance();
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
    if (m_pShipSE != nullptr)
        m_pShipSE->ScoopDrone(this);

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
