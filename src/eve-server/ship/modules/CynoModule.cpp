
 /**
  * @name CynoModule.cpp
  *   Cynosural field generator module class
  * @Author: James
  * @date:   8 October 2021
  */

#include "StaticDataMgr.h"
#include "StatisticMgr.h"
#include "ship/modules/CynoModule.h"
#include "system/Container.h"
#include "system/SystemManager.h"
#include "fleet/FleetService.h"

CynoModule::CynoModule(ModuleItemRef mRef, ShipItemRef sRef)
: ActiveModule(mRef, sRef)
{
    if (!m_shipRef->HasPilot())
        return;

    pChar = m_shipRef->GetPilot()->GetChar().get();
    pClient = pChar->GetClient();

    // increase scan speed by level of survey skill
    float cycleTime = GetAttribute(AttrDuration).get_float();
    cycleTime *= (1 + (0.03f * (pChar->GetSkillLevel(EvESkill::Survey, true))));
    SetAttribute(AttrDuration, cycleTime);
}

void CynoModule::Update()
{
    if (!m_shipRef->HasPilot())
        return;

    // increase scan speed by level of survey skill
    float cycleTime = GetAttribute(AttrDuration).get_float();
    cycleTime *= (1 + (0.03f * (pChar->GetSkillLevel(EvESkill::Survey, true))));
    SetAttribute(AttrDuration, cycleTime);

    ActiveModule::Update();
}

void CynoModule::Activate(uint16 effectID, uint32 targetID, int16 repeat)
{
    m_firstRun = true;
    ActiveModule::Activate(effectID, targetID, repeat);
}

void CynoModule::DeactivateCycle(bool abort)
{
    SendOnJumpBeaconChange(false);
    cSE->Delete();
    SafeDelete(cSE);
    return ActiveModule::DeactivateCycle(abort);
}

void CynoModule::AbortCycle()
{
    if (cSE != nullptr) {
        Deactivate();
    }
    ActiveModule::AbortCycle();
}

bool CynoModule::CanActivate()
{
    //Send a message if its not allowed
    if (pClient->InFleet()) {
        return ActiveModule::CanActivate();
    } else {
        pClient->SendNotifyMsg("You must be in a fleet to run this module.");
        return false;
    }

    //Make sure player is not in high-sec (with config option for this)
}

uint32 CynoModule::DoCycle()
{
    uint32 retVal = ActiveModule::DoCycle();
    if (retVal == 0) {
        return 0;
    }
    if (m_firstRun) {
        m_firstRun = false;

        _log(MODULE__DEBUG, "Cynosural field generated activated in %s", pClient->SystemMgr()->GetName());

        // Create Cyno field here
        ItemData cData(EVEDB::invTypes::CynosuralFieldI, pChar->itemID(), pChar->solarSystemID(), flagNone);
        InventoryItemRef cRef = sItemFactory.SpawnItem(cData);

        cSE = new SystemEntity(cRef, pClient->services(), m_sysMgr);
        GPoint location(pClient->GetShipSE()->GetPosition());
        location.MakeRandomPointOnSphere(1500.0 + cRef->type().radius());
        cSE->SetPosition(location);
        cRef->SaveItem();
        pClient->SystemMgr()->AddEntity(cSE);
        pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();

        SendOnJumpBeaconChange(true);
    } 
    return retVal;
}

void CynoModule::SendOnJumpBeaconChange(bool status) {
        //Send ProcessSovStatusChanged Notification
        _log(MODULE__DEBUG, "Sending OnJumpBeaconChange (adding cyno to jump list for fleet members)");

        uint32 fieldID;
        if (cSE == nullptr) {
            fieldID = 0;
        } else {
            fieldID = cSE->GetID();
        }

        PyTuple* data = new PyTuple(4);
            data->SetItem(0, new PyInt(pClient->GetCharacterID()));
            data->SetItem(1, new PyInt(pClient->GetSystemID()));
            data->SetItem(2, new PyInt(fieldID));
            data->SetItem(3, new PyBool(status));

        std::vector<Client *> fleetClients;
        fleetClients = sFltSvc.GetFleetClients(pClient->GetFleetID());
        for (auto cur : fleetClients)
        {
            if (cur->GetChar().get() != nullptr)
            {
                cur->SendNotification("OnJumpBeaconChange", "clientID", &data);
                _log(MODULE__DEBUG, "OnJumpBeaconChange sent to client %u", cur->GetClientID());
            }
        }
}