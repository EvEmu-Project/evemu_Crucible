
 /**
  * @name TurretModule.cpp
  *   turret module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */


#include "eve-server.h"

#include "StaticDataMgr.h"
#include "StatisticMgr.h"
#include "ship/modules/TurretModule.h"
#include "system/Damage.h"

TurretModule::TurretModule(ModuleItemRef mRef, ShipItemRef sRef)
 : ActiveModule(mRef, sRef),
 m_crystalDmg(0),
 m_crystalDmgAmount(0),
 m_crystalDmgChance(0)
{
    /* this may lead to some weird shit, but civilian turrets dont use charges.
     *  these turrets have the damage attribs set in turret item, so set chargeRef to turret item for damage calcs
     */
    if (!m_needsCharge)
        m_chargeRef = mRef;
}

void TurretModule::LoadCharge(InventoryItemRef cRef)
{
    _log(MODULE__TRACE, "%s calling TM::LoadCharge()", m_modRef->name());

    ActiveModule::LoadCharge(cRef);

    if (m_chargeRef.get() == nullptr)
        return;

    m_crystalDmg        = m_chargeRef->GetAttribute(AttrDamage).get_float();
    m_crystalDmgAmount  = m_chargeRef->GetAttribute(AttrCrystalVolatilityDamage).get_float();
    m_crystalDmgChance  = m_chargeRef->GetAttribute(AttrCrystalVolatilityChance).get_float();
}

void TurretModule::UnloadCharge()
{
    _log(MODULE__TRACE, "%s calling TM::UnloadCharge()", m_modRef->name());

    m_crystalDmg        = 0;
    m_crystalDmgAmount  = 0;
    m_crystalDmgChance  = 0;

    ActiveModule::UnloadCharge();
}

void TurretModule::ApplyDamage()
{
    Client* pilot = m_shipRef->GetPilot();
    ShipSE* sourceSE = (pilot != nullptr) ? pilot->GetShipSE() : nullptr;
    SystemManager* system = (sourceSE != nullptr) ? sourceSE->SystemMgr() : nullptr;
    SystemEntity* liveTarget = (system != nullptr && m_targetID != 0)
        ? system->GetSE(m_targetID) : nullptr;

    // Turret modules must never fire without a live, current target and source.
    // Resolve by ID before touching the cached target pointer, which may have
    // been invalidated by target removal during the previous server tick.
    if (pilot == nullptr || sourceSE == nullptr || liveTarget == nullptr
        || liveTarget != m_targetSE || liveTarget->DestinyMgr() == nullptr
        || liveTarget->GetSelf().get() == nullptr) {
        _log(MODULE__WARNING, "TurretModule::ApplyDamage() - module %s(%u) has no live source/target; stopping cycle.",
             m_modRef->name(), m_modRef->itemID());
        m_targetSE = nullptr;
        Deactivate("TargetDestroyed");
        return;
    }

    if (m_chargeRef.get() == nullptr) {
        pilot->SendErrorMsg("Your %s in %s doesnt have a charge registered.<br>You can try Unload/Reload, but if this error happens again, dock or relog.",
                m_modRef->name(), sDataMgr.GetFlagName(m_modRef->flag()));
        _log(MODULE__ERROR, "TurretModule::ApplyDamage() - module %s(%u) does not have a m_chargeRef.",
             m_modRef->name(), m_modRef->itemID());
        Deactivate();
        return;
    }

    sStatMgr.Increment(Stat::pcShots);

    Damage d(sourceSE,
            m_modRef,
            m_chargeRef->GetAttribute(AttrKineticDamage).get_float(),
            m_chargeRef->GetAttribute(AttrThermalDamage).get_float(),
            m_chargeRef->GetAttribute(AttrEmDamage).get_float(),
            m_chargeRef->GetAttribute(AttrExplosiveDamage).get_float(),
            m_formula.GetToHit(m_shipRef, this, liveTarget),
            m_effectID
    );

    d *= GetAttribute(AttrDamageMultiplier).get_float();
    d *= sConfig.rates.turretDamage;

    if (m_linkMaster)
        d *= m_shipRef->GetLoadedLinkedCount(this);

    liveTarget->ApplyDamage(d);

    switch (m_modRef->groupID()) {
        case EVEDB::invGroups::Projectile_Weapon:
        case EVEDB::invGroups::Hybrid_Weapon: {
            ConsumeCharge();
        } break;
        case EVEDB::invGroups::Energy_Weapon: {
            if (m_chargeRef->HasAttribute(AttrCrystalsGetDamaged))
                if (MakeRandomFloat(0, 1) < m_crystalDmgChance) {
                    m_crystalDmg += m_crystalDmgAmount;
                    if (m_crystalDmg > 1.0f) {
                        pilot->SendNotifyMsg("Your %s loaded in %s has been destroyed.", m_chargeRef->name(), m_modRef->name());
                        InventoryItemRef chargeRef = m_chargeRef;
                        m_shipRef->RemoveItem(m_chargeRef);
                        chargeRef->Delete();
                    } else {
                        m_chargeRef->SetAttribute(AttrDamage, m_crystalDmg);
                    }
                }
        } break;
    }
}
