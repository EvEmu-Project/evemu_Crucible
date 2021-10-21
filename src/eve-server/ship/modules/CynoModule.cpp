 /**
  * @name CynoModule.cpp
  *   Cynosural field generator module class
  * @Author: James
  * @date:   8 October 2021
  */

#include "ship/modules/CynoModule.h"
#include "system/SystemManager.h"
#include "fleet/FleetService.h"
#include "pos/Tower.h"
#include "system/sov/SovereigntyDataMgr.h"

CynoModule::CynoModule(ModuleItemRef mRef, ShipItemRef sRef)
: ActiveModule(mRef, sRef),
cSE(nullptr),
pClient(nullptr),
pShipSE(nullptr),
m_firstRun(true),
m_shipVelocity(0.0f)
{
    if (!m_shipRef->HasPilot())
        return;

    pClient = m_shipRef->GetPilot();

    // increase scan speed by level of survey skill
    float cycleTime = GetAttribute(AttrDuration).get_float();
    cycleTime *= (1 + (0.03f * (pClient->GetChar()->GetSkillLevel(EvESkill::Survey, true))));
    SetAttribute(AttrDuration, cycleTime);
}

void CynoModule::Activate(uint16 effectID, uint32 targetID, int16 repeat)
{
    pShipSE = pClient->GetShipSE();

    m_firstRun = true;
    ActiveModule::Activate(effectID, targetID, repeat);

    // check to see if Activate() was denied.
    if (m_Stop)
        return;

    _log(MODULE__DEBUG, "Cynosural field generator activated by %s in %s", pClient->GetName(), m_sysMgr->GetName());

    // hack to disable ship movement here
    m_shipVelocity = pShipSE->DestinyMgr()->GetMaxVelocity();
    pShipSE->DestinyMgr()->SetFrozen(true);
}

void CynoModule::DeactivateCycle(bool abort)
{
    SendOnJumpBeaconChange(false);
    cSE->Delete();
    SafeDelete(cSE);
    ActiveModule::DeactivateCycle(abort);

    // hack to reinstate ship movement here
    // may have to reset/reapply all fx for ship movement
    pShipSE->GetSelf()->SetAttribute(AttrMaxVelocity, m_shipVelocity);
    pShipSE->DestinyMgr()->SetFrozen(false);
}

bool CynoModule::CanActivate()
{
    if (!pClient->InFleet())
        throw UserError("CynoMustBeInFleet");

    if (pShipSE->SysBubble()->HasTower()) {
        TowerSE* ptSE = pShipSE->SysBubble()->GetTowerSE();
        if (ptSE->HasForceField())
            if (pShipSE->GetPosition().distance(ptSE->GetPosition()) < ptSE->GetSOI())
                throw UserError("NoCynoInPOSShields");
    }

    /** @todo check for active cyno jammer */

    //if (m_sysMgr->HasCynoJammer())
    //    throw UserError("CynosuralGenerationJammed");

    //Send message if the system is being jammed
    SovereigntyData sovData = svDataMgr.GetSovereigntyData(pClient->GetLocationID());
    if (sovData.jammerID != 0) {
        pClient->SendNotifyMsg("This system is currently being jammed.");
        return false;
    }

    //Make sure player is not in high-sec (configurable)
    if (!sConfig.world.highSecCyno) {
        if (pClient->SystemMgr()->GetSecValue() >= 0.5f) {
            pClient->SendNotifyMsg("This module may not be used in high security space.");
            return false;
        }
    }

    // all specific checks pass.  run generic checks in base class
    return ActiveModule::CanActivate();
}

uint32 CynoModule::DoCycle()
{
    // i really dont like this, but cant get it to work anywhere else...
    uint32 retVal(0);
    if (retVal = ActiveModule::DoCycle())
        if (m_firstRun) {
            m_firstRun = false;
            CreateCyno();
        }

    return retVal;
}

void CynoModule::CreateCyno()
{
    // do we already have a cyno field created?
    if (cSE != nullptr)
        return;

    // Create Cyno field here
    ItemData cData(EVEDB::invTypes::CynosuralFieldI, pClient->GetCharacterID(), m_sysMgr->GetID(), flagNone);
    InventoryItemRef cRef = sItemFactory.SpawnItem(cData);

    _log(MODULE__DEBUG, "Creating Cynosural field");

    cSE = new ItemSystemEntity(cRef, pClient->services(), m_sysMgr);
    GPoint location(pShipSE->GetPosition());
    location.MakeRandomPointOnSphere(1500.0f + cRef->type().radius());
    cSE->SetPosition(location);
    cRef->SaveItem();
    m_sysMgr->AddEntity(cSE);

    SendOnJumpBeaconChange(true);
}

void CynoModule::SendOnJumpBeaconChange(bool active/*false*/) {
        //Send ProcessSovStatusChanged Notification
        _log(MODULE__DEBUG, "Sending OnJumpBeaconChange (active = %s)", active ? "true" : "false");

        uint32 fieldID(0);
        if (cSE != nullptr)
            fieldID = cSE->GetID();

        PyTuple* data = new PyTuple(4);
            data->SetItem(0, new PyInt(pClient->GetCharacterID()));
            data->SetItem(1, new PyInt(m_sysMgr->GetID()));
            data->SetItem(2, new PyInt(fieldID));
            data->SetItem(3, new PyBool(active));

        std::vector<Client *> fleetClients;
        fleetClients = sFltSvc.GetFleetClients(pClient->GetFleetID());
        for (auto cur : fleetClients)
            if (cur != nullptr) {
                cur->SendNotification("OnJumpBeaconChange", "clientID", &data);
                _log(MODULE__DEBUG, "OnJumpBeaconChange sent to %s (%u)", cur->GetName(), cur->GetCharID());
            }
}

/*
 * {'messageKey': 'CynoMustBeInFleet', 'dataID': 17879144, 'suppressable': False, 'bodyID': 257897, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257896, 'messageID': 2253}
 * {'messageKey': 'CynosuralGenerationJammed', 'dataID': 17880043, 'suppressable': False, 'bodyID': 258240, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2249}
 * {'messageKey': 'NoActiveBeacon', 'dataID': 17879973, 'suppressable': False, 'bodyID': 258214, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258213, 'messageID': 2211}
 * {'messageKey': 'NoCynoInPOSShields', 'dataID': 17878872, 'suppressable': False, 'bodyID': 257791, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2339}
 *
 */

/*
 * {'FullPath': u'UI/Messages', 'messageID': 257896, 'label': u'CynoMustBeInFleetTitle'}(u'Cannot start Cynosural Field', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257897, 'label': u'CynoMustBeInFleetBody'}(u'You must be in a fleet to activate your Cynosural Field.', None, None)
 *
 * {'FullPath': u'UI/Messages', 'messageID': 258213, 'label': u'NoActiveBeaconTitle'}(u'No Active Beacon', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258214, 'label': u'NoActiveBeaconBody'}(u'You do not have an active Cynosural Field Beacon.', None, None)
 *
 * {'FullPath': u'UI/Messages', 'messageID': 258240, 'label': u'CynosuralGenerationJammedBody'}(u'You are unable to generate a cynosural field while there is a jammer present and active somewhere in this solar system.', None, None)
 *
 * {'FullPath': u'UI/Messages', 'messageID': 257791, 'label': u'NoCynoInPOSShieldsBody'}(u'You cannot set up a cynosural field within the force field of a Control Tower', None, None)
 */
