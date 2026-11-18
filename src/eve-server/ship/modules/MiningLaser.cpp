
 /**
  * @name MiningLaser.cpp
  *   mining module class
  * @Author:         Allan
  * @date:      10 June 2015   -UD/RW 02 April 2017
  * @revised:  4 August 2017
  */


#include "eve-server.h"


#include "StaticDataMgr.h"
#include "StatisticMgr.h"
#include "character/Character.h"
#include "inventory/Inventory.h"
#include "ship/Ship.h"
#include "ship/modules/MiningLaser.h"
#include "EVE_MiningFormulas.h"
#include "EVE_MovementFormulas.h"
//#include "system/SystemBubble.h"
//#include "system/SystemManager.h"
#include "system/cosmicMgrs/BeltMgr.h"


MiningLaser::MiningLaser(ModuleItemRef mRef, ShipItemRef sRef)
: ActiveModule(mRef, sRef)
{
    m_IsInitialCycle = true;
    m_rMiner = m_dcMiner = m_iMiner = m_gMiner = false;
    m_crystalDmg = m_crystalRoidGrp = m_crystalDmgAmount = m_crystalDmgChance = 0;

    if ((m_modRef->typeID() == 12108) || (m_modRef->typeID() == 18068)
        || (m_modRef->typeID() == 24305) || (m_modRef->typeID() == 28748)) {
        m_dcMiner = true;
    } else if (m_modRef->groupID() == EVEDB::invGroups::Mining_Laser) {
        m_rMiner = true;
    } else if ((m_modRef->typeID() == 16278) or (m_modRef->typeID() == 22229) or (m_modRef->typeID() == 22589) or (m_modRef->typeID() == 22591)
        or (m_modRef->typeID() == 22597) or (m_modRef->typeID() == 22599) or (m_modRef->typeID() == 28752)) {
        /* this includes 'dev testing modules'  */
        m_iMiner = true;
    } else if (m_modRef->groupID() == EVEDB::invGroups::Gas_Cloud_Harvester) {
        m_gMiner = true;
    } else if (m_modRef->groupID() == EVEDB::invGroups::Frequency_Mining_Laser) {
        m_rMiner = true;
        m_reloadTime = 8000;    // this is not set in ActiveModule c'tor.  easier/cheaper to set here.
    } else if (m_modRef->groupID() == EVEDB::invGroups::Strip_Miner) {
        m_rMiner = true;
    }

    m_holdFlag = flagCargoHold;
    _log(MINING__TRACE, "MiningLaser Created for %s with %ums Duration.", mRef->name(), GetAttribute(AttrDuration).get_uint32());
}

void MiningLaser::LoadCharge(InventoryItemRef charge)
{
    ActiveModule::LoadCharge(charge);

    if (m_chargeRef.get() == nullptr)
        return;

    m_crystalDmg = m_chargeRef->GetAttribute(AttrDamage).get_float();
    m_crystalRoidGrp = m_chargeRef->GetAttribute(AttrSpecialisationAsteroidGroup).get_float();
    m_crystalDmgAmount = m_chargeRef->GetAttribute(AttrCrystalVolatilityDamage).get_float();
    m_crystalDmgChance = m_chargeRef->GetAttribute(AttrCrystalVolatilityChance).get_float();
    //AttrUsageDamagePercent
}

void MiningLaser::UnloadCharge()
{
    _log(MODULE__TRACE, "%s calling ML::UnloadCharge()", m_modRef->name());
    //AttrUnfitCapCost
    m_crystalDmg = 0;
    m_crystalRoidGrp = 0;
    m_crystalDmgAmount = 0;
    m_crystalDmgChance = 0;

    ActiveModule::UnloadCharge();
}

bool MiningLaser::CanActivate()
{
    m_holdFlag = flagCargoHold;
    if (m_targetSE == nullptr || !m_targetSE->IsAsteroidSE() || m_targetSE->SysBubble() == nullptr) {
        _log(MINING__WARNING, "Activate() - Invalid target: m_targetSE == nullptr");
        if (m_shipRef->HasPilot())
            m_shipRef->GetPilot()->SendNotifyMsg("Module Activate: Invalid target - Ref: ServerError 15628");
        return false;
    }

    bool canActivate(false);
    const InventoryItemRef target = m_targetSE->GetSelf();
    if (target.get() == nullptr)
        return false;

    // Ice harvesters handle ice. Deep-core lasers can mine ordinary ore
    // as well as Mercoxit, which ordinary mining lasers cannot extract.
    if (m_rMiner || m_dcMiner) {
        if (target->categoryID() == EVEDB::invCategories::Asteroid
            && target->groupID() != EVEDB::invGroups::Ice
            && (m_dcMiner || target->groupID() != EVEDB::invGroups::Mercoxit)) {
            canActivate = true;
            if (m_shipRef->GetAttribute(AttrOreHoldCapacity).get_double() > 0.0)
                m_holdFlag = flagOreHold;
        }
    } else if (m_iMiner) {
        if (target->groupID() == EVEDB::invGroups::Ice) {
            canActivate = true;
            if (m_shipRef->GetAttribute(AttrOreHoldCapacity).get_double() > 0.0)
                m_holdFlag = flagOreHold;
        }
    } else if (m_gMiner) {
        if (target->groupID() == EVEDB::invGroups::Harvestable_Cloud) {
            canActivate = true;
            if (m_shipRef->GetAttribute(AttrGasHoldCapacity).get_double() > 0.0)
                m_holdFlag = flagGasHold;
        }
    }

    if (canActivate) {
        m_IsInitialCycle = true;
        m_targetSE->SystemMgr()->GetBeltMgr()->SetActive(m_targetSE->SysBubble()->GetID());

        // mining on current target approved. check for and set crystal variables here
        if (m_chargeLoaded and (m_chargeRef.get() != nullptr) and (m_crystalRoidGrp == 0)) {
            m_crystalDmg = m_chargeRef->GetAttribute(AttrDamage).get_float();
            m_crystalRoidGrp = m_chargeRef->GetAttribute(AttrSpecialisationAsteroidGroup).get_float();
            m_crystalDmgAmount = m_chargeRef->GetAttribute(AttrCrystalVolatilityDamage).get_float();
            m_crystalDmgChance = m_chargeRef->GetAttribute(AttrCrystalVolatilityChance).get_float();
        }

        return ActiveModule::CanActivate();
    } else {
        _log(MINING__WARNING, "Activate() - Invalid target: %s", m_targetSE->GetName());
        if (m_shipRef->HasPilot())
            m_shipRef->GetPilot()->SendNotifyMsg("Module Activate: %s is an invalid target - Ref: ServerError 15628", m_targetSE->GetName());
    }

    return false;
}

uint32 MiningLaser::DoCycle() {
    /* ore is dumped into hold at end of module's cycle.
     * however, code processing runs at beginning of cycle, so this needs to 'fake' the ore acquisition to the end of cycle
     * we accomplish this by doing nothing on first cycle, and call the processing component at beginning of each cycle after that.
     */
    if (m_IsInitialCycle) {
    	m_IsInitialCycle = false;
    } else {
        ProcessCycle();
        if (m_targetSE == nullptr)
            return 0;
    }

    return ActiveModule::DoCycle();
}

void MiningLaser::DeactivateCycle(bool abort/*false*/)
{
    if (m_ModuleState != Module::State::Deactivating)
        return;

    ProcessCycle(abort);
    m_holdFlag = flagCargoHold;
    if (m_ModuleState == Module::State::Deactivating)
        ActiveModule::DeactivateCycle(abort);
}

// note:  gas cloud contains radius/10 units of gas.
/** @todo verify for ice and gas */
void MiningLaser::ProcessCycle(bool abort/*false*/)
{
    // A target can disappear between the module timer firing and the cycle callback.
    // Do not dereference a stale SystemEntity when an asteroid is depleted or removed.
    if (m_targetSE == nullptr || m_shipRef.get() == nullptr || m_shipRef->GetPilot() == nullptr) {
        _log(MINING__WARNING, "ProcessCycle() - Mining module %s(%u) has no valid target, ship, or pilot; aborting.",
             m_modRef->name(), m_modRef->itemID());
        ActiveModule::DeactivateCycle(true);
        return;
    }

    float cycleVol(GetMiningVolume());

    InventoryItemRef roidRef(m_targetSE->GetSelf());
    if (roidRef.get() == nullptr) {
        _log(MINING__WARNING, "ProcessCycle() - Mining target for %s(%u) no longer has an inventory item; aborting.",
             m_modRef->name(), m_modRef->itemID());
        ActiveModule::DeactivateCycle(true);
        return;
    }
    // Mining yield is awarded at the end of the cycle. Do not mine
    // remotely when the ship has moved beyond its fitted laser range.
    const float miningRange = GetAttribute(AttrMaxRange).get_float();
    ShipSE* liveShip = m_shipRef->GetPilot()->GetShipSE();
    if (liveShip == nullptr || liveShip->SysBubble() == nullptr
        || liveShip->SysBubble() != m_targetSE->SysBubble()) {
        ActiveModule::DeactivateCycle(true);
        return;
    }
    const double separation = EVEmuMovement::SurfaceSeparation(
        liveShip->GetPosition().distance(m_targetSE->GetPosition()),
        liveShip->GetRadius(), m_targetSE->GetRadius());
    if (!std::isfinite(miningRange) || miningRange <= 0.0f || separation > miningRange) {
        _log(MINING__WARNING,
             "Mining %s(%u) outside effective range: %.1f m (max %.1f m).",
             m_modRef->name(), m_modRef->itemID(), separation, miningRange);
        m_shipRef->GetPilot()->SendNotifyMsg(
            "Your %s deactivates because its target is out of mining range (%.0f m; max %.0f m).",
            m_modRef->name(), separation, miningRange);
        ActiveModule::DeactivateCycle(true);
        return;
    }

    const double oreVolume = roidRef->GetAttribute(AttrVolume).get_double();
    const double remaining = roidRef->GetAttribute(AttrQuantity).get_double();
    if (abort) {
        const uint32 duration = m_timer.GetDuration();
        const double elapsed = duration > 0
            ? 1.0 - static_cast<double>(GetRemainingCycleTimeMS()) / duration : 0.0;
        cycleVol *= std::clamp(elapsed, 0.0, 1.0);
    }
    if (!std::isfinite(oreVolume) || oreVolume <= 0.0 || !std::isfinite(remaining)
        || !std::isfinite(cycleVol) || (!abort && cycleVol < oreVolume)) {
        ActiveModule::DeactivateCycle(true);
        return;
    }

    const int32 extractable = EvEMining::OreUnits(cycleVol, oreVolume, remaining);
    const double remainingHold = m_shipRef->GetRemainingVolumeByFlag(m_holdFlag);
    const int32 oreAmount = EvEMining::OreUnits(remainingHold, oreVolume, extractable);
    const bool holdFull = oreAmount < extractable;
    if (oreAmount == 0 && remaining >= 1.0) {
        if (holdFull) {
            if (!abort)
                m_shipRef->GetPilot()->SendNotifyMsg("Your %s deactivates because your hold is full.", m_modRef->name());
            ActiveModule::DeactivateCycle(true);
        }
        return;
    }

    const double remainingOre = remaining - oreAmount;
    if (remainingOre < 1.0) {
        // Allocation and deactivation must happen while the rock is still live.
        // Calling RemoveTarget() here would run the mining cycle a second time.
        SystemEntity* depleted = m_targetSE;
        if (depleted->TargetMgr() != nullptr && oreAmount > 0)
            depleted->TargetMgr()->Depleted(this);
        ActiveModule::DeactivateCycle(true);
        depleted->Delete();
        delete depleted;
        return;
    }

    ItemData idata(roidRef->typeID(), m_shipRef->ownerID(), locTemp, flagNone, oreAmount);
    InventoryItemRef ore = sItemFactory.SpawnItem(idata);
    if (ore.get() == nullptr) {
        _log(MINING__ERROR, "Could not create mined ore for %s(%u)", m_shipRef->name(), m_shipRef->itemID());
        ActiveModule::DeactivateCycle(true);
        return;
    }
    ore->MergeTypesInCargo(m_shipRef.get(), m_holdFlag);
    roidRef->SetAttribute(AttrQuantity, remainingOre);
    if (!m_iMiner)
        roidRef->SetAttribute(AttrRadius, std::exp((remainingOre + 112404.8) / 25000));
    sStatMgr.Add(Stat::oreMined, oreAmount * oreVolume);

    if (holdFull) {
        if (!abort)
            m_shipRef->GetPilot()->SendNotifyMsg("Your %s deactivates because your hold is full.", m_modRef->name());
        ActiveModule::DeactivateCycle(true);
        return;
    }

    if (!m_chargeLoaded or (m_chargeRef.get() == nullptr))
        return;

    if (MakeRandomFloat(0,1) < m_crystalDmgChance) {
        m_crystalDmg += m_crystalDmgAmount;
        if (m_crystalDmg > 0.99f) {
            m_shipRef->GetPilot()->SendNotifyMsg("Your %s deactivates due to the destruction of it's %s.", \
                    m_modRef->name(), m_chargeRef->name());
            InventoryItemRef chargeRef(m_chargeRef);   // make a copy of charge's item ref, as m_chargeRef = NULL after next call returns
            m_shipRef->RemoveItem(m_chargeRef);
            chargeRef->Delete();
        } else {
            m_chargeRef->SetAttribute(AttrDamage, m_crystalDmg);
        }
    }
}

void MiningLaser::Depleted(std::multimap<float, MiningLaser*> &mMap) {
    /* instead of calculations and timers from previous iteration, this method will determine how many modules are activated
     * on newly-depleted rock, get total cycle volume, and then divide rock contents between modules, weighted by modules cycle volume.
     */
    // NOTE:  this will be called for EACH active module on rock, with first timer to hit being the 'master' module.

    // determine % of remaining ore for each module active on this rock
    double total = GetMiningVolume();
    for (auto cur : mMap)
        if (std::isfinite(cur.first) && cur.first > 0.0f)
            total += cur.first;

    InventoryItemRef roidRef(m_targetSE->GetSelf());
    const double oreVolume = roidRef->GetAttribute(AttrVolume).get_double();
    if (!std::isfinite(oreVolume) || oreVolume <= 0.0 || !std::isfinite(total) || total <= 0.0) {
        _log(MINING__ERROR, "%s(%u) - Depleted() -  oreVolume is <0 for %s(%u)", m_modRef->name(), m_modRef->itemID(), roidRef->name(), m_targetSE->GetID() );

        // send error and deactivate all active modules here
        for (auto cur : mMap) {
            cur.second->GetShipRef()->GetPilot()->SendNotifyMsg("Your %s deactivates because there was a processing error.  Ref: ServerError 03123.", \
                        cur.second->GetSelf()->name());
            cur.second->CancelOnError();
        }
        return;
    }

    const int32 roidQuantity = EvEMining::OreUnits(
        roidRef->GetAttribute(AttrQuantity).get_double(), 1.0, std::numeric_limits<int32>::max());
    int32 unallocated = roidQuantity;

    for (auto cur : mMap) {
        if (!std::isfinite(cur.first) || cur.first < oreVolume) {
            _log(MINING__ERROR, "%s(%u) - Depleted() -  Mining Laser could not extract ore from %s(%u)", \
                        cur.second->GetSelf()->name(), cur.second->GetSelf()->itemID(), roidRef->name(), m_targetSE->GetID() );
            cur.second->GetShipRef()->GetPilot()->SendNotifyMsg("Your %s deactivates because there was an error in it's processing.  Ref: ServerError 06428.",\
                        cur.second->GetSelf()->name());
            cur.second->CancelOnError();
            continue;
        }

        const int32 oreAmount = EvEMining::OreUnits(roidQuantity * (cur.first / total), 1.0, unallocated);
        unallocated -= oreAmount;
        cur.second->AddOreAndDeactivate(roidRef->typeID(), oreAmount, oreVolume);

        // inform pilot of asteroid depleted  ...no clue if it actually works like this
        PyTuple* tuple = new PyTuple(2);
            tuple->SetItem(0, new PyString("MiningItemDepleted"));
        PyDict* dict = new PyDict();
            dict->SetItemString("modulename", new PyString(cur.second->GetSelf()->itemName()));
            tuple->SetItem(1, dict);
        cur.second->GetShipRef()->GetPilot()->QueueDestinyUpdate(&tuple);
    }

    // Credit remaining whole units to the module that completed its cycle.
    AddOreAndDeactivate(roidRef->typeID(), unallocated, oreVolume, false);
}

void MiningLaser::AddOreAndDeactivate(uint16 typeID, int32 units, double unitVolume, bool slave/*true*/) {
    const int32 storedUnits = EvEMining::OreUnits(m_shipRef->GetRemainingVolumeByFlag(m_holdFlag), unitVolume, units);
    if (storedUnits > 0) {
        ItemData data(typeID, m_shipRef->ownerID(), locTemp, flagNone, storedUnits);
        InventoryItemRef ore = sItemFactory.SpawnItem(data);
        if (ore.get() != nullptr) {
            ore->MergeTypesInCargo(m_shipRef.get(), m_holdFlag);
            sStatMgr.Add(Stat::oreMined, storedUnits * unitVolume);
        } else {
            _log(MINING__ERROR, "Could not create mined ore for %s(%u)", m_shipRef->name(), m_shipRef->itemID());
        }
    }

    m_shipRef->GetPilot()->SendNotifyMsg("Your %s deactivates; Its target has been depleted.", m_modRef->name());

    if (slave)
        ActiveModule::DeactivateCycle(true);
}

float MiningLaser::GetMiningVolume()
{
    float cycleVol(GetAttribute(AttrMiningAmount).get_float());
    if (m_chargeLoaded)
        if (m_targetSE->GetGroupID() == m_crystalRoidGrp)
            cycleVol = GetAttribute(AttrSpecialtyMiningAmount).get_float();

    // fleet involvement enhances mining amount using MiningForeman of highest member (3%/lvl)
    // this should apply to modules/ship when boost activated, but this is easier at this time.
    //  downside is client will have the original, lower cycle amount as this isnt set in module (but should be)
    ShipSE* pShip(m_shipRef->GetPilot()->GetShipSE());
    if (pShip != nullptr)
        if (pShip->IsBoosted())
            cycleVol *= (1 + (0.03f * pShip->GetMiningBoostAmount())); // 3% increase/level

    return cycleVol;
}


/*
 * {'messageKey': 'MiningCrystalDestroyed', 'dataID': 17883202, 'suppressable': False, 'bodyID': 259420, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1167}
 * {'messageKey': 'MiningDronesDeactivatedAsteroidEmpty', 'dataID': 17883322, 'suppressable': False, 'bodyID': 259462, 'messageType': 'notify', 'urlAudio': 'wise:/msg_MiningDronesDeactivatedAsteroidEmpty_play', 'urlIcon': '', 'titleID': None, 'messageID': 1168}
 * {'messageKey': 'MiningDronesDeactivatedCargoHoldFull', 'dataID': 17883265, 'suppressable': False, 'bodyID': 259442, 'messageType': 'notify', 'urlAudio': 'wise:/msg_MiningDronesDeactivatedCargoHoldFull_play', 'urlIcon': '', 'titleID': None, 'messageID': 1169}
 * {'messageKey': 'MiningDronesDeactivatedCargoHoldNowFull', 'dataID': 17883243, 'suppressable': False, 'bodyID': 259434, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1170}
 * {'messageKey': 'MiningDronesDeactivatedOutOfRange', 'dataID': 17883208, 'suppressable': False, 'bodyID': 259422, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1171}
 * {'messageKey': 'MiningItemDepleted', 'dataID': 17879592, 'suppressable': False, 'bodyID': 258064, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2312}
 *
 * {'FullPath': u'UI/Messages', 'messageID': 258064, 'label': u'MiningItemDepletedBody'}(u'{modulename} deactivates as its target has been depleted.', None, {u'{modulename}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'modulename'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259420, 'label': u'MiningCrystalDestroyedBody'}(u'{[item]module.name} deactivates due to the destruction of the {[item]type.name} it was fitted with. \r\n', None, {u'{[item]type.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'type'}, u'{[item]module.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'module'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259422, 'label': u'MiningDronesDeactivatedOutOfRangeBody'}(u'{[item]module.name} deactivates without transfering ore to your cargo hold because your ship has strayed to a distance of {[numeric]distance} m, beyond its mining range of {[numeric]range} m.', None, {u'{[numeric]distance}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'distance'}, u'{[numeric]range}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'range'}, u'{[item]module.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'module'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259434, 'label': u'MiningDronesDeactivatedCargoHoldNowFullBody'}(u'{modulename} deactivates. Your cargo hold does not have sufficient space to contain more ore, so loading algorithms have deactivated the {modulename} before it becomes overloaded.', None, {u'{modulename}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'modulename'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259442, 'label': u'MiningDronesDeactivatedCargoHoldFullBody'}(u'{modulename} disperses its load of freshly mined ore into space unrecoverably, then deactivates. Your cargo hold is full.', None, {u'{modulename}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'modulename'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259462, 'label': u'MiningDronesDeactivatedAsteroidEmptyBody'}(u'{modulename} deactivates as it finds the resource it was harvesting a pale shadow of its former glory.', None, {u'{modulename}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'modulename'}})
 *
 */
