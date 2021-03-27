
 /**
  * @name MiningLaser.cpp
  *   mining module class
  * @Author:         Allan
  * @date:      10 June 2015   -UD/RW 02 April 2017
  * @revised:  4 August 2017
  */


#include "eve-server.h"

#include "PyServiceMgr.h"
#include "StaticDataMgr.h"
#include "StatisticMgr.h"
#include "character/Character.h"
#include "ship/Ship.h"
#include "ship/modules/MiningLaser.h"
//#include "system/SystemBubble.h"
//#include "system/SystemManager.h"
#include "system/cosmicMgrs/BeltMgr.h"


MiningLaser::MiningLaser(ModuleItemRef mRef, ShipItemRef sRef)
: ActiveModule(mRef, sRef)
{
    m_IsInitialCycle = true;
    m_rMiner = m_dcMiner = m_iMiner = m_gMiner = false;
    m_crystalDmg = m_crystalRoidGrp = m_crystalDmgAmount = m_crystalDmgChance = 0;

    if (m_modRef->groupID() == EVEDB::invGroups::Mining_Laser) {
        m_rMiner = true;
    } else if ((m_modRef->typeID() == 12108) or (m_modRef->typeID() == 18068) or (m_modRef->typeID() == 24305) or (m_modRef->typeID() == 28748)) {
        m_dcMiner = true;
    } else if ((m_modRef->typeID() == 16278) or (m_modRef->typeID() == 22229) or (m_modRef->typeID() == 22589) or (m_modRef->typeID() == 22591)
        or (m_modRef->typeID() == 22597) or (m_modRef->typeID() == 22599) or (m_modRef->typeID() == 28752)) {
        /* this includes 'dev testing modules'  */
        m_iMiner = true;
    } else if (m_modRef->groupID() == EVEDB::invGroups::Gas_Cloud_Harvester) {
        m_gMiner = true;
    } else if (m_modRef->groupID() == EVEDB::invGroups::Frequency_Mining_Laser) {
        m_rMiner = true;
        m_reloadTime = 8000;    // this is not set in ActiveModule c'tor.  easier/cheaper to set here.
    } else if(m_modRef->groupID() == EVEDB::invGroups::Strip_Miner) {
        m_rMiner = true;
    }

    m_holdFlag = flagCargoHold;
    _log(MINING__TRACE, "MiningLaser Created for %s with %ums Duration.", mRef->name(), GetAttribute(AttrDuration).get_int());
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
    if (m_targetSE == nullptr){
        _log(MINING__WARNING, "Activate() - Invalid target: m_targetSE == nullptr");
        if (m_shipRef->HasPilot())
            m_shipRef->GetPilot()->SendNotifyMsg("Module Activate: Invalid target - Ref: ServerError 15628");
        return false;
    }

    bool canActivate(false);
    // verify module vs target for activation.  disallow if not compatible.  set special ore hold if applicable
    if (m_rMiner) {
        if ((m_targetSE->GetSelf()->categoryID() == EVEDB::invCategories::Asteroid)
        and (m_targetSE->GetSelf()->groupID() != EVEDB::invGroups::Mercoxit)) {
            canActivate = true;
            if (m_shipRef->HasAttribute(AttrOreHoldCapacity))
                m_holdFlag = flagOreHold;
        }
    } else if (m_dcMiner) {
        if (m_targetSE->GetSelf()->groupID() == EVEDB::invGroups::Mercoxit) {
            canActivate = true;
            if (m_shipRef->HasAttribute(AttrOreHoldCapacity))
                m_holdFlag = flagOreHold;
        }
    } else if (m_iMiner) {
        if (m_targetSE->GetSelf()->groupID() == EVEDB::invGroups::Ice) {
            canActivate = true;
            if (m_shipRef->HasAttribute(AttrOreHoldCapacity))
                m_holdFlag = flagOreHold;
        }
    } else if (m_gMiner) {
        if (m_targetSE->GetSelf()->groupID() == EVEDB::invGroups::Harvestable_Cloud) {
            canActivate = true;
            if (m_shipRef->HasAttribute(AttrGasHoldCapacity))
                m_holdFlag = flagGasHold;
        }
    }

    if (canActivate) {
        // so far, checks have passed and proper hold is set.
        // check for capacity as final test before allowing mining (ship would know)

        // should we also test for target volume here? ...it's done on every ProcessCycle() call...
        if (m_shipRef->GetRemainingVolumeByFlag(m_holdFlag) < GetMiningVolume()) {
            _log(MINING__WARNING, "Activate() - Cargo full.  Denying Activate() on %s", m_targetSE->GetName());
            if (m_shipRef->HasPilot())
                m_shipRef->GetPilot()->SendNotifyMsg("Module Activate: Your Cargo is full. - Ref: ServerError 65125");

            return false;
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
    }

    return ActiveModule::DoCycle();
}

void MiningLaser::DeactivateCycle(bool abort/*false*/)
{
    if (m_ModuleState != Module::State::Deactivating)
        return;

    ApplyEffect(FX::State::Active, false);
    ShowEffect(false, abort);

    ProcessCycle(abort);
    m_holdFlag = flagCargoHold;

    SetModuleState(Module::State::Online);
    Clear();
}

// note:  gas cloud contains radius/10 units of gas.
/** @todo verify for ice and gas */
void MiningLaser::ProcessCycle(bool abort/*false*/)
{
    float cycleVol(GetMiningVolume());

    InventoryItemRef roidRef(m_targetSE->GetSelf());
    // verify gas clouds have volume attr.
    float oreVolume(roidRef->GetAttribute(AttrVolume).get_float());

    if ((cycleVol < oreVolume) or (cycleVol <= 0) or (oreVolume <= 0)) {
        _log(MINING__ERROR, "%s(%u) - Mining Laser could not extract ore from %s(%u)", m_modRef->name(), m_modRef->itemID(), roidRef->name(), m_targetSE->GetID() );
        m_shipRef->GetPilot()->SendNotifyMsg("Your %s deactivates because there was an error in it's processing.  Ref: ServerError 06428.", m_modRef->name());
        ActiveModule::DeactivateCycle(true);
        return;
    }

    float oreAmount((cycleVol /oreVolume));
    if (abort) {
        // adjust amount AND cycle for partial cycle
        float delta = 1 - (GetRemainingCycleTimeMS() / GetAttribute(AttrDuration).get_float());
        cycleVol *= delta;
        oreAmount *= delta;
        if (m_iMiner or m_gMiner)
            oreAmount = floor(oreAmount);
        _log(MINING__DEBUG, "ProcessCycle(abort) -  cycleVol:%.2f, oreAmount:%.2f, delta:%.5f", cycleVol, oreAmount, delta);
    }

    float roidQuantity(roidRef->GetAttribute(AttrQuantity).get_float());
    if (oreAmount > roidQuantity)
        oreAmount = roidQuantity;

    float remainingCargoVolume(m_shipRef->GetRemainingVolumeByFlag(m_holdFlag));
    if (remainingCargoVolume < cycleVol) {
        // cargohold is full.  this module will fill to available volume and trash the rest
        if (remainingCargoVolume > oreVolume) {
            oreAmount = remainingCargoVolume /oreVolume;
        } else {
            oreAmount = 0;
        }
        // check for other lasers running, and deactivate them also.
        // the ship will tell all miners the current hold is full, and they automagically deactivate.
        // this is a feature.
        m_shipRef->CargoFull();

        // explicitly calling base class method here will negate the possibility of running a loop from DeactivateCycle override and overfilling cargo (elusive error)
        ActiveModule::DeactivateCycle(true);
        if (!abort) // dont notify client if they deactivated laser
            m_shipRef->GetPilot()->SendNotifyMsg("Your %s deactivates because your cargohold is full.", m_modRef->name());
    }

    _log(MINING__DEBUG, "ProcessCycle(%s) -  cycleVol:%.2f, roidQuantity:%.2f, remainingCargoVolume:%.2f/%.2f, oreAmount:%.2f", \
                (abort?"true":"false"), cycleVol, roidQuantity, remainingCargoVolume, (remainingCargoVolume -cycleVol), oreAmount);

    if (oreAmount <= 0)
        return;

    roidQuantity -= oreAmount;
    _log(MINING__TRACE, "new roidQuantity %.3f", roidQuantity);

    if (roidQuantity > 0.0f) {
        roidRef->SetAttribute(AttrQuantity, roidQuantity);
        // do not reset ice radius (our huge-ass chunks will probably never expire)
        if (!m_iMiner) {
            /* reversing the radius-to-quantity formula, we get radius = exp((quantity + 112404.8) /25000)  */
            double radius = exp((roidQuantity + 112404.8) /25000);
            // need to update players in bubble of this change.  not sure how yet
            roidRef->SetAttribute(AttrRadius, radius);
        }
    } else {
        //rock is depleted.
        // this will get all active miners on depleted rock and set mined amount accordingly for each.
        m_targetSE->TargetMgr()->Depleted(this);
        SystemEntity* pSE(m_targetSE);
        RemoveTarget(m_targetSE);
        // m_targetSE is null after above call returns
        pSE->Delete();
        SafeDelete(pSE);
        return;
    }

    // at this point, there is still plenty of ore in rock
    ItemData idata(roidRef->typeID(), m_shipRef->ownerID(), locTemp, flagNone, oreAmount);
    InventoryItemRef oRef(sItemFactory.SpawnItem(idata));
    if (oRef.get() == nullptr) {
        _log(MINING__ERROR, "Could not create mined ore for %s(%u)", m_shipRef->name(), m_shipRef->itemID() );
        return;
    }

    bool oreError(true);
    if (m_shipRef->GetMyInventory()->HasAvailableSpace(m_holdFlag, oRef)) {
        oreError = false;
        oRef->MergeTypesInCargo(m_shipRef.get(), m_holdFlag);
    }

    // add data to StatisticMgr
    sStatMgr.Add(Stat::oreMined, cycleVol);

    if (oreError and !abort) {
        m_shipRef->GetPilot()->SendNotifyMsg("Your %s deactivates as it couldn't add the %s to your %s.", \
                m_modRef->name(), oRef->name(), sDataMgr.GetFlagName(m_holdFlag));
        _log(MINING__ERROR, "Could not add ore to hold for %s(%u)", m_shipRef->name(), m_shipRef->itemID() );
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
    float total = GetMiningVolume(), percent = 0.0f;
    for (auto cur : mMap)
        total += cur.first;

    /** @todo check for mining drones here!!!  */

    InventoryItemRef roidRef(m_targetSE->GetSelf());
    float oreVolume(roidRef->GetAttribute(AttrVolume).get_float());
    if (oreVolume <= 0) {
        _log(MINING__ERROR, "%s(%u) - Depleted() -  oreVolume is <0 for %s(%u)", m_modRef->name(), m_modRef->itemID(), roidRef->name(), m_targetSE->GetID() );

        // send error and deactivate all active modules here
        for (auto cur : mMap) {
            cur.second->GetShipRef()->GetPilot()->SendNotifyMsg("Your %s deactivates because there was a processing error.  Ref: ServerError 03123.", \
                        cur.second->GetSelf()->name());
            cur.second->CancelOnError();
        }
        return;
    }

    float roidQuantity(roidRef->GetAttribute(AttrQuantity).get_float());
    double oreAmount(0);

    for (auto cur : mMap) {
        if ((cur.first < oreVolume) or (cur.first < 0.1)) {
            _log(MINING__ERROR, "%s(%u) - Depleted() -  Mining Laser could not extract ore from %s(%u)", \
                        cur.second->GetSelf()->name(), cur.second->GetSelf()->itemID(), roidRef->name(), m_targetSE->GetID() );
            cur.second->GetShipRef()->GetPilot()->SendNotifyMsg("Your %s deactivates because there was an error in it's processing.  Ref: ServerError 06428.",\
                        cur.second->GetSelf()->name());
            cur.second->CancelOnError();
            continue;
        }

        // calculate ore for this laser
        percent = cur.first /total;
        oreAmount = roidQuantity * percent;

        // create and add ore to cargo for this laser
        cur.second->AddOreAndDeactivate(roidRef->typeID(), oreAmount);

        // inform pilot of asteroid depleted  ...no clue if it actually works like this
        PyTuple* tuple = new PyTuple(2);
            tuple->SetItem(0, new PyString("MiningItemDepleted"));
        PyDict* dict = new PyDict();
            dict->SetItemString("modulename", new PyString(cur.second->GetSelf()->itemName()));
            tuple->SetItem(1, dict);
        cur.second->GetShipRef()->GetPilot()->QueueDestinyUpdate(&tuple);
    }

    // calculate ore for this laser
    percent = GetMiningVolume() /total;
    // create and add ore to cargo for this laser
    AddOreAndDeactivate(roidRef->typeID(), roidQuantity * percent, false);
}

void MiningLaser::AddOreAndDeactivate(uint16 typeID, float amt, bool slave/*true*/) {

    ItemData idata(typeID, m_shipRef->ownerID(), locTemp, flagNone, amt);
    InventoryItemRef oRef(sItemFactory.SpawnItem( idata ));
    if (oRef.get() == nullptr) {
        _log(MINING__ERROR, "AddOreAndDeactivate() - Could not create mined ore for %s(%u)", m_shipRef->name(), m_shipRef->itemID() );
        return;
    }

    if (!m_shipRef->AddItemByFlag(m_holdFlag, oRef))
        _log(MINING__ERROR, "AddOreAndDeactivate() - Could not add ore to hold for %s(%u)", m_shipRef->name(), m_shipRef->itemID() );

    // send pilot msg about depletion for this module
    m_shipRef->GetPilot()->SendNotifyMsg("Your %s deactivates; Its target has been depleted.", m_modRef->name());

    if (slave)
        ActiveModule::DeactivateCycle(true);

    // add data to StatisticMgr
    sStatMgr.Add(Stat::oreMined, amt);
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
    /*if (pShip != nullptr)
        if (pShip->IsBoosted())
            cycleVol *= (1 + (0.03f * pShip->GetMiningBoostAmount())); // 3% increase/level*/

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