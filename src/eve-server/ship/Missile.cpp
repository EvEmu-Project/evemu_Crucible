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
    Author:     Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "EVEServerConfig.h"
#include "Profiler.h"
#include "character/Character.h"
#include "inventory/AttributeEnum.h"
#include "system/DestinyManager.h"
#include "ship/Missile.h"
#include "ship/Ship.h"
#include "ship/modules/GenericModule.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"
#include "EVE_WeaponFormulas.h"

#include <algorithm>
#include <cmath>

Missile::Missile( InventoryItemRef self, EVEServiceManager& services, SystemManager* pSystem, InventoryItemRef modRef, SystemEntity* tSE, SystemEntity* pSE, GenericModule* pMod)
: DynamicSystemEntity(self, services, pSystem),
  m_modRef(modRef),
  m_targetSE(tSE),
  m_fromSE(pSE),
  m_hitTimer(0),
  m_lifeTimer(0),
  m_alive(true),
  m_orbitingID(tSE != nullptr ? tSE->GetID() : 0),
  m_launcherID(pSE != nullptr ? pSE->GetID() : 0),
  m_damageMod(1),
  m_speed(0),
  m_hullHP(self->GetAttribute(AttrHP).get_int())
{
    if (pSE->HasPilot()) {
        m_ownerID = pSE->GetPilot()->GetChar()->itemID();
    } else {
        m_ownerID = self->ownerID();
    }

    m_warID = pSE->GetWarFactionID();
    m_allyID = pSE->GetAllianceID();
    m_corpID = pSE->GetCorporationID();

    double flightTime = self->GetAttribute(AttrExplosionDelay).get_float();

    // missile skills do not apply correctly in fx processor.  not sure why yet.
    if (pSE->HasPilot()) {
        Character* pChar = pSE->GetPilot()->GetChar().get();
        flightTime *= (1 + (0.1f * (pChar->GetSkillLevel(EvESkill::MissileBombardment, true)))); // 10% increase in flightTime

        self->MultiplyAttribute(AttrMaxVelocity, (1 + (0.1f * (pChar->GetSkillLevel(EvESkill::MissileProjection, true))))); // 10% increase in velocity
        // Crucible Guided Missile Precision only affects guided rounds:
        // light, heavy and cruise missiles (including FoF/T2 variants).
        // Rockets, heavy assault missiles and torpedoes are unguided.
        switch (m_self->groupID()) {
            case EVEDB::invGroups::Light_Missile:
            case EVEDB::invGroups::FoF_Light_Missile:
            case EVEDB::invGroups::Advanced_Light_Missile:
            case EVEDB::invGroups::Heavy_Missile:
            case EVEDB::invGroups::FoF_Heavy_Missile:
            case EVEDB::invGroups::Advanced_Heavy_Missile:
            case EVEDB::invGroups::Cruise_Missile:
            case EVEDB::invGroups::FoF_Cruise_Missile:
            case EVEDB::invGroups::Advanced_Cruise_Missile:
                self->MultiplyAttribute(AttrAoeCloudSize,
                    (1 - (0.05f * pChar->GetSkillLevel(EvESkill::GuidedMissilePrecision, true))));
                break;
            default: break;
        }
        self->MultiplyAttribute(AttrAoeVelocity, (1 + (0.1f * (pChar->GetSkillLevel(EvESkill::TargetNavigationPrediction, true)))));  // 10% increase in exp velocity

        m_damageMod *= (1 + (0.02f * (pChar->GetSkillLevel(EvESkill::WarheadUpgrades, true)))); // Crucible: 2% damage per level
        switch (m_self->groupID()) {
            case EVEDB::invGroups::Light_Missile:
            case EVEDB::invGroups::FoF_Light_Missile:
            case EVEDB::invGroups::Advanced_Light_Missile: {
                m_damageMod *= (1 + (0.05f * (pChar->GetSkillLevel(EvESkill::LightMissiles, true)))); // 5% increase in damage
                // Light Missile Specialization modifies launcher rate of fire, not per-warhead damage.
            } break;
            case EVEDB::invGroups::Heavy_Missile:
            case EVEDB::invGroups::FoF_Heavy_Missile:
            case EVEDB::invGroups::Advanced_Heavy_Missile: {
                m_damageMod *= (1 + (0.05f * (pChar->GetSkillLevel(EvESkill::HeavyMissiles, true)))); // 5% increase in damage
                // Heavy Missile Specialization is a launcher rate-of-fire modifier.
            } break;
            case EVEDB::invGroups::Cruise_Missile:
            case EVEDB::invGroups::FoF_Cruise_Missile:
            case EVEDB::invGroups::Advanced_Cruise_Missile: {
                m_damageMod *= (1 + (0.05f * (pChar->GetSkillLevel(EvESkill::CruiseMissiles, true)))); // 5% increase in
                // Cruise Missile Specialization is a launcher rate-of-fire modifier.
            } break;
            case EVEDB::invGroups::Torpedo:
            case EVEDB::invGroups::Advanced_Torpedo: {
                m_damageMod *= (1 + (0.05f * (pChar->GetSkillLevel(EvESkill::Torpedoes, true)))); // 5% increase in damage
                // Torpedo Specialization is a launcher rate-of-fire modifier.
            } break;
            case EVEDB::invGroups::Rocket:
            case EVEDB::invGroups::Advanced_Rocket: {
                m_damageMod *= (1 + (0.05f * (pChar->GetSkillLevel(EvESkill::Rockets, true)))); // 5% increase in damage
                // Rocket Specialization is a launcher rate-of-fire modifier.
            } break;
            case EVEDB::invGroups::Defender_Missile: {
                // Crucible Defender Missiles skill grants +5% interceptor
                // flight velocity per level, not damage to ships.
                self->MultiplyAttribute(AttrMaxVelocity,
                    (1 + (0.05f * pChar->GetSkillLevel(EvESkill::DefenderMissiles, true))));
            } break;
            case EVEDB::invGroups::Assault_Missile:
            case EVEDB::invGroups::Advanced_Assault_Missile: {
                m_damageMod *= (1 + (0.05f * (pChar->GetSkillLevel(EvESkill::HeavyAssaultMissiles, true)))); // 5% increase in damage
                // Heavy Assault Missile Specialization is a launcher rate-of-fire modifier.
            } break;
            case EVEDB::invGroups::Citadel_Cruise: {
                m_damageMod *= (1 + (0.05f * (pChar->GetSkillLevel(EvESkill::CitadelCruiseMissiles, true)))); // 5% increase in damage
            } break;
            case EVEDB::invGroups::Citadel_Torpedo: {
                m_damageMod *= (1 + (0.05f * (pChar->GetSkillLevel(EvESkill::CitadelTorpedoes, true)))); // 5% increase in damage
            } break;
        }
    }

    /** @todo update this when rack overloading is implemented... */
    if (IsOverloaded())
        m_damageMod *= (1 + self->GetAttribute(AttrOverloadDamageModifier).get_float());

    flightTime *= sConfig.rates.missileTime;

    // if linked, update appropriate attributes
    if (pMod != nullptr)
        if (pMod->IsLinked()) {
            uint8 mod = m_fromSE->GetShipSE()->GetShipItemRef()->GetLinkedCount(pMod);
            m_damageMod *= mod;
            // Weapon grouping combines volley damage but does NOT increase
            // individual missile flight lifetime or effective range.
            m_hullHP *= mod;
        }

    if (!std::isfinite(flightTime) || flightTime <= 0.0) {
        _log(DAMAGE__ERROR, "Missile %u has invalid flight lifetime %.3f ms. Expiring safely.",
             m_self->itemID(), flightTime);
        flightTime = 1.0;
    }
    m_lifeTimer.Start(static_cast<uint32>(flightTime));

    //_log(DAMAGE__MESSAGE, "Created Missile object for %s (%u)", self.get()->name(), self.get()->itemID());
}

void Missile::Process() {
    if (!m_alive) {
        Delete();
        return;
    }
    if (m_lifeTimer.Check(false)) {
        EndOfLife();
        return;
    }
    double profileStartTime(GetTimeUSeconds());
    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();

    if (m_hitTimer.Check(false)) {
        m_hitTimer.Disable();
        HitTarget();
    }
    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::missile, GetTimeUSeconds() - profileStartTime);
}

void Missile::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;
    BallHeader head = BallHeader();
        head.entityID = GetID();
        head.mode = Ball::Mode::MISSILE;
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
        data.maxSpeed = m_speed;
        data.velX = m_destiny->GetVelocity().x;
        data.velY = m_destiny->GetVelocity().y;
        data.velZ = m_destiny->GetVelocity().z;
        data.inertia = m_destiny->GetInertia();
        data.speedfraction = m_destiny->GetSpeedFraction();
    into.Append( data );
    MISSILE_Struct miss;
        miss.ownerID = m_ownerID;
        miss.formationID = 0xFF;
        miss.effectStamp = m_destiny->GetStateStamp();
        miss.targetID = m_destiny->GetTargetID();
        miss.followRange = (float)m_destiny->GetFollowDistance();
        miss.x = x();
        miss.y = y();
        miss.z = z();
    into.Append(miss);

    _log(SE__DESTINY, "Missile::EncodeDestiny(): %s - id:%lli, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict* Missile::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for Missile %u", m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",                   new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",                   new PyInt(m_self->typeID()));
        slim->SetItemString("groupID",                  new PyInt(m_self->groupID()));
        slim->SetItemString("categoryID",               new PyInt(m_self->categoryID()));
        slim->SetItemString("name",                     new PyString(m_self->itemName()));
        slim->SetItemString("sourceModuleID",           new PyInt(m_modRef->itemID()));
        slim->SetItemString("corpID",                   IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
        slim->SetItemString("allianceID",               IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
        slim->SetItemString("warFactionID",             IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
        slim->SetItemString("securityStatus",           new PyFloat(0/*pChar->GetSecurityRating()*/)); /** @todo (allan) fix this */
        slim->SetItemString("ownerID",                  new PyInt(m_ownerID)); // this is corp ID??
        slim->SetItemString("numLaunchers",             PyStatic.NewOne());  /** @todo (allan) fix this */
        slim->SetItemString("nameID",                   new PyInt(0));  /** @todo (allan) fix this */
    return(slim);
}

void Missile::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = 1;
    into.recharge = 10000;
    into.timestamp = GetFileTimeNow();
    into.armor = 1;
    into.structure = 1.0f - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

bool Missile::ApplyDefenderDamage(double damage) {
    if (!m_alive || !std::isfinite(damage) || damage <= 0.0)
        return false;
    m_hullHP = CalculateRemainingMissileHP(m_hullHP, damage);
    if (m_hullHP <= 0.0) {
        m_alive = false;
        return true;
    }
    return false;
}

void Missile::HitTarget() {
    // Resolve both endpoints through the live system registry. Cached raw
    // pointers may have been invalidated while the missile was in flight.
    SystemManager* system = SystemMgr();
    SystemEntity* liveTarget = (system != nullptr && m_orbitingID != 0)
        ? system->GetSE(m_orbitingID) : nullptr;
    SystemEntity* liveSource = (system != nullptr && m_launcherID != 0)
        ? system->GetSE(m_launcherID) : nullptr;

    if (liveTarget == nullptr || liveTarget != m_targetSE
        || liveSource == nullptr || liveSource != m_fromSE
        || liveTarget->GetSelf().get() == nullptr
        || m_self.get() == nullptr || m_modRef.get() == nullptr) {
        _log(DAMAGE__WARNING, "Missile::HitTarget() - missile %u lost a valid source, module, or target; discarding impact.",
             GetID());
        m_alive = false;
        return;
    }

    // Defender missiles are interception projectiles, not ordinary warheads.
    // They remove the hostile missile on impact instead of applying ship damage.
    if (m_modRef->groupID() == EVEDB::invGroups::Missile_Launcher_Defender
        && liveTarget->IsMissileSE()) {
        Missile* intercepted = liveTarget->GetMissileSE();
        if (intercepted != nullptr && intercepted->IsAlive()) {
            const double defenderDamage =
                m_self->GetAttribute(AttrKineticDamage).get_double()
                + m_self->GetAttribute(AttrThermalDamage).get_double()
                + m_self->GetAttribute(AttrEmDamage).get_double()
                + m_self->GetAttribute(AttrExplosiveDamage).get_double();
            const bool destroyed = intercepted->ApplyDefenderDamage(
                defenderDamage * m_damageMod * sConfig.rates.missileDamage
                * sConfig.rates.damageRate);
            _log(DAMAGE__TRACE,
                 "Defender missile %u hit incoming missile %u for %.1f raw damage; destroyed=%u.",
                 GetID(), intercepted->GetID(), defenderDamage * m_damageMod, destroyed ? 1 : 0);
        }
        m_alive = false;
        return;
    }

    const double explosionRadius = m_self->GetAttribute(AttrAoeCloudSize).get_float();
    const double explosionVelocity = m_self->GetAttribute(AttrAoeVelocity).get_float();
    const double reductionFactor = m_self->GetAttribute(AttrAoeDamageReductionFactor).get_float();
    const double reductionSensitivity = m_self->GetAttribute(AttrAoeDamageReductionSensitivity).get_float();

    // Bomb launchers detonate at the impact point and damage every eligible
    // dynamic entity inside the bomb's data-driven blast radius.
    if (m_modRef->groupID() == EVEDB::invGroups::Missile_Launcher_Bomb) {
        SystemBubble* bubble = SysBubble();
        if (bubble == nullptr || system == nullptr
            || !std::isfinite(explosionRadius) || explosionRadius <= 0.0) {
            _log(DAMAGE__WARNING, "Bomb missile %u has no valid bubble or AoE radius; discarding detonation.",
                 GetID());
            m_alive = false;
            return;
        }

        const GPoint impactPosition(liveTarget->GetPosition());
        std::map<uint32, SystemEntity*> entities;
        bubble->GetAllEntities(entities);

        for (const auto& entry : entities) {
            if (entry.first == GetID())
                continue;

            SystemEntity* areaTarget = system->GetSE(entry.first);
            if (areaTarget == nullptr || areaTarget != entry.second || areaTarget->IsDead())
                continue;
            if (!(areaTarget->IsShipSE() || areaTarget->IsNPCSE() || areaTarget->IsDroneSE()
                  || areaTarget->IsPOSSE() || areaTarget->IsDeployableSE() || areaTarget->IsSentrySE()
                  || areaTarget->IsWeaponSE() || areaTarget->IsBatterySE() || areaTarget->IsTowerSE()))
                continue;

            InventoryItemRef targetItem = areaTarget->GetSelf();
            if (targetItem.get() == nullptr
                || impactPosition.distance(areaTarget->GetPosition()) > explosionRadius)
                continue;

            double signatureRadius = targetItem->GetAttribute(AttrSignatureRadius).get_float();
            if (signatureRadius <= 0.0)
                signatureRadius = targetItem->GetAttribute(AttrRadius).get_float() / 10.0;

            const double damageFactor = CalculateMissileDamageFactor(
                signatureRadius, explosionRadius, explosionVelocity,
                areaTarget->GetVelocity().length(), reductionFactor, reductionSensitivity);
            if (damageFactor <= 0.0)
                continue;

            Damage damage(liveSource, m_modRef, m_self, EVEEffectID::missileLaunching);
            damage *= m_damageMod;
            damage *= static_cast<float>(damageFactor);
            areaTarget->ApplyDamage(damage);
        }

        m_alive = false;
        return;
    }

    /* Standard missile damage formula:
     * Damage = D * MIN(1, Sr/Er, (Ev/V * Sr/Er)^(ln(DRF) / ln(DRS)) )
     */
    InventoryItemRef targetItem = liveTarget->GetSelf();
    double signatureRadius = targetItem->GetAttribute(AttrSignatureRadius).get_float();
    if (signatureRadius <= 0.0)
        signatureRadius = targetItem->GetAttribute(AttrRadius).get_float() / 10.0;

    const double damageFactor = CalculateMissileDamageFactor(
        signatureRadius, explosionRadius, explosionVelocity,
        liveTarget->GetVelocity().length(), reductionFactor, reductionSensitivity);

    if (!std::isfinite(damageFactor) || damageFactor <= 0.0) {
        m_alive = false;
        return;
    }
    Damage damage(liveSource, m_modRef, m_self, EVEEffectID::missileLaunching);
    damage *= m_damageMod;
    damage *= static_cast<float>(damageFactor);

    liveTarget->ApplyDamage(damage);
    m_alive = false;
}

void Missile::EndOfLife() {
    m_alive = false;
    Delete();
}

void Missile::Delete() {
    //  cleanup here
    if (m_alive)
        return;
    // do we need to do anything else here?
    SystemEntity::Delete();
}
