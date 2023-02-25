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
    Author:        Zhur
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "EVEServerConfig.h"


#include "cache/ObjCacheService.h"
#include "dogmaim/DogmaIMService.h"
#include "pos/Tower.h"
#include "ship/modules/GenericModule.h"
#include "system/Container.h"
#include "system/SystemManager.h"
#include "station/Station.h"

DogmaIMService::DogmaIMService(EVEServiceManager& mgr) :
    BindableService("dogmaIM", mgr)  // IM = Instance Manager, also LM = Location Manager
{
    this->Add("GetAttributeTypes", &DogmaIMService::GetAttributeTypes);

    this->m_cache = this->GetServiceManager().Lookup <ObjCacheService>("objectCaching");
}

PyResult DogmaIMService::GetAttributeTypes(PyCallArgs& call) {
    PyString* str = new PyString("dogmaIM.attributesByName" );
    PyRep* result = this->m_cache->GetCacheHint( str );
    PyDecRef( str );
    return result;
}

BoundDispatcher* DogmaIMService::BindObject(Client* client, PyRep* bindParameters) {
    DogmaLM_BindArgs args;
    //crap
    PyRep* tmp(bindParameters->Clone());
    if (!args.Decode(&tmp)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode bind args.", GetName().c_str());
        return nullptr;
    }

    return new DogmaIMBound(this->GetServiceManager(), *this, args.locationID, args.groupID);
}

void DogmaIMService::BoundReleased (DogmaIMBound* bound) {

}

DogmaIMBound::DogmaIMBound(EVEServiceManager& mgr, DogmaIMService& parent, uint32 locationID, uint32 groupID) :
    EVEBoundObject(mgr, parent),
    m_locationID(locationID),
    m_groupID(groupID)
{
    this->Add("ChangeDroneSettings", &DogmaIMBound::ChangeDroneSettings);
    this->Add("LinkWeapons", &DogmaIMBound::LinkWeapons);
    this->Add("LinkAllWeapons", &DogmaIMBound::LinkAllWeapons);
    this->Add("UnlinkModule", &DogmaIMBound::UnlinkModule);
    this->Add("UnlinkAllModules", &DogmaIMBound::UnlinkAllModules);
    this->Add("OverloadRack", &DogmaIMBound::OverloadRack);
    this->Add("StopOverloadRack", &DogmaIMBound::StopOverloadRack);
    this->Add("CharGetInfo", &DogmaIMBound::CharGetInfo);
    this->Add("ItemGetInfo", &DogmaIMBound::ItemGetInfo);
    this->Add("GetAllInfo", &DogmaIMBound::GetAllInfo);
    this->Add("DestroyWeaponBank", &DogmaIMBound::DestroyWeaponBank);
    this->Add("GetCharacterBaseAttributes", &DogmaIMBound::GetCharacterBaseAttributes);
    this->Add("Activate", static_cast <PyResult(DogmaIMBound::*)(PyCallArgs&, PyInt*, PyInt*)> (&DogmaIMBound::Activate));
    this->Add("Activate", static_cast <PyResult(DogmaIMBound::*)(PyCallArgs&, PyInt*, PyWString*, std::optional <PyInt*>, PyInt*)> (&DogmaIMBound::Activate));
    this->Add("Deactivate", static_cast <PyResult(DogmaIMBound::*)(PyCallArgs&, PyInt*, PyWString*)> (&DogmaIMBound::Deactivate));
    this->Add("Deactivate", static_cast <PyResult(DogmaIMBound::*)(PyCallArgs&, PyInt*, PyInt*)> (&DogmaIMBound::Deactivate));
    this->Add("Overload", &DogmaIMBound::Overload);
    this->Add("StopOverload", &DogmaIMBound::StopOverload);
    this->Add("CancelOverloading", &DogmaIMBound::CancelOverloading);
    this->Add("SetModuleOnline", &DogmaIMBound::SetModuleOnline);
    this->Add("TakeModuleOffline", &DogmaIMBound::TakeModuleOffline);
    this->Add("LoadAmmoToBank", &DogmaIMBound::LoadAmmoToBank);
    this->Add("LoadAmmoToModules", &DogmaIMBound::LoadAmmoToModules);
    this->Add("GetTargets", &DogmaIMBound::GetTargets);
    this->Add("GetTargeters", &DogmaIMBound::GetTargeters);
    this->Add("AddTarget", &DogmaIMBound::AddTarget);       //AddTargetOBO
    this->Add("RemoveTarget", &DogmaIMBound::RemoveTarget);
    this->Add("ClearTargets", &DogmaIMBound::ClearTargets);
    this->Add("InitiateModuleRepair", &DogmaIMBound::InitiateModuleRepair);
    this->Add("StopModuleRepair", &DogmaIMBound::StopModuleRepair);
    this->Add("MergeModuleGroups", &DogmaIMBound::MergeModuleGroups);
    this->Add("PeelAndLink", &DogmaIMBound::PeelAndLink);
}

PyResult DogmaIMBound::CharGetInfo(PyCallArgs& call) {
    return call.client->GetChar()->GetCharInfo();
}

PyResult DogmaIMBound::ClearTargets(PyCallArgs& call) {
    call.client->GetShipSE()->TargetMgr()->ClearTargets();
    //call.client->GetShipSE()->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::ClientReq);
    return nullptr;
}

PyResult DogmaIMBound::GetTargets(PyCallArgs& call) {
    return call.client->GetShipSE()->TargetMgr()->GetTargets();
}

PyResult DogmaIMBound::GetTargeters(PyCallArgs& call) {
    return call.client->GetShipSE()->TargetMgr()->GetTargeters();
}

PyResult DogmaIMBound::GetCharacterBaseAttributes(PyCallArgs& call)
{
    CharacterRef cref = call.client->GetChar();
    PyDict* result = new PyDict();
        result->SetItem(new PyInt(AttrIntelligence), cref->GetAttribute(AttrIntelligence).GetPyObject());
        result->SetItem(new PyInt(AttrPerception), cref->GetAttribute(AttrPerception).GetPyObject());
        result->SetItem(new PyInt(AttrCharisma), cref->GetAttribute(AttrCharisma).GetPyObject());
        result->SetItem(new PyInt(AttrWillpower), cref->GetAttribute(AttrWillpower).GetPyObject());
        result->SetItem(new PyInt(AttrMemory), cref->GetAttribute(AttrMemory).GetPyObject());
    return result;
}


PyResult DogmaIMBound::ItemGetInfo(PyCallArgs& call, PyInt* itemID) {
    // called when item 'row' info not in shipState data from GetAllInfo() return
    InventoryItemRef itemRef = sItemFactory.GetItemRef(itemID->value());
    if (itemRef.get() == nullptr ) {
        _log(INV__ERROR, "Unable to load item %u", itemID->value());
        return PyStatic.NewNone();
    }

    PyObject* obj = itemRef->ItemGetInfo();
    if (is_log_enabled(ITEM__DEBUG))
        obj->Dump(ITEM__DEBUG, "    ");
    return obj;
}

PyResult DogmaIMBound::SetModuleOnline(PyCallArgs& call, PyInt* locationID, PyInt* moduleID) {
    Client* pClient(call.client);

    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (pDestiny == nullptr) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return PyStatic.NewNone();
        } else if (pDestiny->IsWarping()) {
            /** @todo  warpsafe modules can be activated while warping.  fix this */
            pClient->SendNotifyMsg("You can't do this while warping");
            return PyStatic.NewNone();
        }
    }

	pClient->GetShip()->Online(moduleID->value());

    // returns nodeID and timestamp
    return this->GetOID();
}

PyResult DogmaIMBound::TakeModuleOffline(PyCallArgs& call, PyInt* locationID, PyInt* moduleID) {
    Client* pClient(call.client);

    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (pDestiny == nullptr) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return PyStatic.NewNone();
        } else if (pDestiny->IsWarping()) {
            /** @todo  warpsafe modules can be deactivated while warping.  fix this */
            pClient->SendNotifyMsg("You can't do this while warping");
            return PyStatic.NewNone();
        }
    }

	pClient->GetShip()->Offline(moduleID->value());

    // returns nodeID and timestamp
    return this->GetOID();
}

PyResult DogmaIMBound::LoadAmmoToModules(PyCallArgs& call, PyInt* shipID, PyList* cModuleIDs, PyInt* chargeTypeID, PyInt* itemID, PyInt* ammoLocationId) {
    //  self.remoteDogmaLM.LoadAmmoToModules(shipID, moduleIDs, chargeTypeID, itemID, ammoLocationID, qty=qty)
    //  NOTE:  this call seems to be a list of moduleIDs with ONLY a single module.
    /* 02:13:11 [SvcCall]       Tuple: 5 elements
     * 02:13:11 [SvcCall]         [ 0] Integer field: 140000602     <- ship
     * 02:13:11 [SvcCall]         [ 1] List: 1 elements
     * 02:13:11 [SvcCall]         [ 1]   [ 0] Integer field: 140000454  <- moduleList
     * 02:13:11 [SvcCall]         [ 2] Integer field: 196   <- charge type
     * 02:13:11 [SvcCall]         [ 3] Integer field: 140000623 <- charge item
     * 02:13:11 [SvcCall]         [ 4] Integer field: 60014749  <- charge location
     * 02:13:11 [SvcCall]   Call Named Arguments:
     * 02:13:11 [SvcCall]     Argument 'qty':
     * 02:13:11 [SvcCall]         (None)
     */
    _log(MODULE__TRACE, "DogmaIMBound::Handle_LoadAmmoToModules()");
    call.Dump(MODULE__TRACE);
    std::vector<int32> moduleIDs;

    PyList::const_iterator list_2_cur = cModuleIDs->begin();
    for (size_t list_2_index(0); list_2_cur != cModuleIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_Dogma_LoadAmmoToModules failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        moduleIDs.push_back(t->value());
    }

    if (moduleIDs.empty())
        return nullptr;
    if (moduleIDs.size() > 1) {
        sLog.Error("DogmaIMBound::Handle_LoadAmmoToModules()", "args.moduleIDs.size = %lu.", moduleIDs.size() );
        call.Dump(MODULE__WARNING);
    }

    // Get Reference to Ship and Charge
    ShipItemRef sRef = call.client->GetShip();
    GenericModule* pMod = sRef->GetModule(sItemFactory.GetItemRef(moduleIDs[0])->flag());
    if (pMod == nullptr)
        throw UserError ("ModuleNoLongerPresentForCharges");

    InventoryItemRef cRef = sItemFactory.GetItemRef(itemID->value());
    sRef->LoadCharge(cRef, pMod->flag());

    // returns nodeID and timestamp
    return this->GetOID();
}

PyResult DogmaIMBound::LoadAmmoToBank(PyCallArgs& call, PyInt* shipID, PyInt* masterID, PyInt* chargeTypeID, PyList* cItemIDs, PyInt* chargeLocationID, PyInt* qty) {
  /*   NOTE:  this to load ALL modules in weapon bank, if possible.
   * self.remoteDogmaLM.LoadAmmoToBank(shipID, masterID, chargeTypeID,  itemIDs,  chargeLocationID,   qty)
   *                                    ship,   module,  charge type, charge item, charge location, stack qty (usually none - havent found otherwise)
   *   *******    UPDATED VAR NAMES TO MATCH CLIENT CODE  -allan 26Jul14  *************
   */
  _log(MODULE__TRACE, "DogmaIMBound::Handle_LoadAmmoToBank()");
  call.Dump(MODULE__TRACE);

    /*
    args.shipID
    args.masterID
    args.chargeTypeID
    args.itemIDs
    args.chargeLocationID
    args.qty
    */
    std::vector<int32> itemIDs;

    PyList::const_iterator list_2_cur = cItemIDs->begin();
    for (size_t list_2_index(0); list_2_cur != cItemIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_Dogma_LoadAmmoToModules failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        itemIDs.push_back(t->value());
    }

    if (itemIDs.empty())
        return nullptr;

    // Get Reference to Ship, Module, and Charge
    ShipItemRef sRef = call.client->GetShip();
    if (sRef->itemID() != shipID->value())
        sLog.Error("DogmaIMBound::Handle_LoadAmmoToBank()", "passed shipID %u != current shipID %u.", shipID->value(), sRef->itemID() );

    // if shipID passed in call isnt active ship (from client->GetShip()), would this work right?
    GenericModule* pMod = sRef->GetModule(sItemFactory.GetItemRef(masterID->value())->flag());
    if (pMod == nullptr)
        throw UserError ("ModuleNoLongerPresentForCharges");

    // figure out how to use args.qty for loading less-than-full charges
    //  LoadCharge() can be easily updated to do this.
    if (pMod->IsLinked()) {
        sRef->LoadLinkedWeapons(pMod, itemIDs);
    } else {
        sRef->LoadCharge(sItemFactory.GetItemRef(itemIDs.at(0)), pMod->flag());
    }

    // not sure why im not using this, as call is to load bank...
    //sRef->LoadChargesToBank(pMod->flag(), args.itemIDs);

    // returns nodeID and timestamp
    return this->GetOID();
}

PyResult DogmaIMBound::AddTarget(PyCallArgs& call, PyInt* targetID) {
   // flag, targetList = self.GetDogmaLM().AddTargetOBO(sid, tid)
    // flag, targetList = self.GetDogmaLM().AddTarget(tid)
    // as a note, targetList isnt used once call returns to client
    // throws here void returns.  client will raise throw (and ignore return)
    // calling client tests
    Client* pClient(call.client);
    if (pClient->IsJump())
        throw UserError ("CantTargetWhileJumping");

    if (pClient->GetShipID() == targetID->value())
        throw UserError ("DeniedTargetSelf");

    /** @todo pClient->IsUncloak() incomplete */
    if (pClient->IsUncloak())
        throw UserError ("DeinedTargetAfterCloak");

    if (pClient->IsDocked()) {
        pClient->SendNotifyMsg("You can't do this while docked");
        Rsp_Dogma_AddTarget rsp;
        rsp.flag = false;
        rsp.targetList.push_back(targetID->value());
        return rsp.Encode();
    }
    // caller ship tests
    ShipSE* mySE = pClient->GetShipSE();
    if ( mySE->TargetMgr() == nullptr)
        throw UserError ("DeniedTargetingAttemptFailed")
                .AddFormatValue ("target", new PyInt (targetID->value()));
    /** @todo SE->IsFrozen() incomplete */
    if (mySE->IsFrozen())
        throw UserError ("DeniedTargetSelfFrozen")
                .AddFormatValue ("targetName", new PyString (mySE->GetName ()));
    /** @todo SE->IsInvul() incomplete */
    if (mySE->IsInvul())
        throw UserError ("DeniedInvulnerable");

    if (mySE->SysBubble() == nullptr) {
        _log(DESTINY__ERROR, "Client %s does not have a bubble.", pClient->GetName());
        throw UserError ("DeniedTargettingAttemptFailed")
                .AddFormatValue ("target", new PyInt (targetID->value()));
    }
    if (mySE->SysBubble()->HasTower()) {
        TowerSE* ptSE = mySE->SysBubble()->GetTowerSE();
        if (ptSE->HasForceField() && mySE->GetPosition().distance(ptSE->GetPosition()) < ptSE->GetSOI())
                throw UserError ("DeniedTargetingInsideField")
                        .AddFormatValue ("target", new PyInt (targetID->value()));
    }

    // caller destiny tests
    DestinyManager* pMyDestiny = mySE->DestinyMgr();
    if (pMyDestiny == nullptr) {
        _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
        throw UserError ("DeniedTargetingAttemptFailed")
                .AddFormatValue ("target", new PyInt (targetID->value()));
    }
    if (pMyDestiny->IsCloaked())
        throw UserError ("CantTargetWhileCloaked");
       // throw UserError ("DeniedTargetingCloaked");

    // verify caller sysMgr
    SystemManager* pSysMgr = pClient->SystemMgr();
    if (pSysMgr == nullptr) {
        _log(PLAYER__WARNING, "Unable to find system manager for '%s'", pClient->GetName());
        throw UserError ("DeniedTargetingAttemptFailed")
                .AddFormatValue ("target", new PyInt (targetID->value()));
    }

    // target tests
    SystemEntity* tSE = pSysMgr->GetSE(targetID->value());
    if (tSE == nullptr) {
        _log(INV__WARNING, "Unable to find entity %u in system %u from '%s'", targetID->value(), pSysMgr->GetID(), pClient->GetName());
        throw UserError ("DeniedTargetingAttemptFailed")
                .AddFormatValue ("target", new PyInt (targetID->value()));
    }
    if ((tSE->IsStaticEntity())
    or (tSE->IsLogin())/** @todo SE->IsLogin() incomplete */
    or (tSE->GetSelf()->HasAttribute(AttrUntargetable)))
        throw UserError ("DeniedTargetEvadesSensors")
                .AddFormatValue ("targetName", new PyString (tSE->GetName ()));
    /** @todo SE->IsInvul() incomplete */
    if (tSE->IsInvul())
        throw CustomError ("Cannot Engage %s as they are invulnerable.", tSE->GetName());
        //throw UserError ("DeniedTargetInvulnerable");
    /** @todo SE->IsFrozen() incomplete */
    if (tSE->IsFrozen())
        throw UserError ("DeniedTargetOtherFrozen")
                .AddFormatValue ("targetName", new PyString (tSE->GetName ()));

    if (tSE->HasPilot()) {
        /** @todo SE->IsInvul() incomplete */
        if ( tSE->GetPilot()->IsInvul())
            throw CustomError ("Cannot Engage %s as they are invulnerable.", tSE->GetName());
        //throw UserError ("DeniedTargetInvulnerable");
        if ( tSE->GetPilot()->IsSessionChange())
            throw UserError ("DeniedTargetEvadesSensors")
                    .AddFormatValue ("targetName", new PyString (tSE->GetName ()));
    }

    // this is to allow use of target name, after tSE is init'd
    if (pMyDestiny->IsWarping()) {
        _log(TARGET__WARNING, "Handle_AddTarget - TargMgr.StartTargeting() failed - warping.");
        throw UserError ("DeniedTargetSelfWarping")
                .AddFormatValue ("targetName", new PyString (tSE->GetName ()));
    }
    if (tSE->DestinyMgr() != nullptr) {
        if (tSE->DestinyMgr()->IsCloaked())
            throw UserError ("DeniedTargetingTargetCloaked");
        if (tSE->DestinyMgr()->IsWarping()) {
            _log(TARGET__WARNING, "Handle_AddTarget - TargMgr.StartTargeting() failed - target warping.");
            throw UserError ("DeniedTargetOtherWarping")
                    .AddFormatValue ("targetName", new PyString (tSE->GetName ()));
        }
    }

    // target bubble tests
    if (tSE->SysBubble() == nullptr) {
        _log(DESTINY__ERROR, "Target %s does not have a bubble.", tSE->GetName());
        throw UserError ("DeniedTargetingAttemptFailed")
                .AddFormatValue ("target", new PyInt (targetID->value()));
    }
    if (tSE->IsPOSSE())
        if (tSE->GetPOSSE()->IsReinforced())
            throw UserError ("DeniedTargetReinforcedStructure")
                    .AddFormatValue ("target", new PyInt (targetID->value()));
    if (tSE->SysBubble()->HasTower()) {
        TowerSE* ptSE = tSE->SysBubble()->GetTowerSE();
        if (ptSE->HasForceField() && tSE->GetPosition().distance(ptSE->GetPosition()) < ptSE->GetSOI())
                throw UserError ("DeniedTargetForceField")
                        .AddFormatValue ("target", new PyInt (targetID->value()))
                        .AddFormatValue ("range", new PyInt (ptSE->GetSOI ()))
                        .AddFormatValue ("item", new PyInt (ptSE->GetID ()));
    }

    if (!mySE->TargetMgr()->StartTargeting( tSE, pClient->GetShip())) {
        _log(TARGET__WARNING, "AddTarget() - TargMgr.StartTargeting() failed.");
        throw UserError ("DeniedTargetingAttemptFailed")
                .AddFormatValue ("target", new PyInt (targetID->value()));
    }

    Rsp_Dogma_AddTarget rsp;
    rsp.flag = true;    // false = immediate target lock in client, true = wait for OnTarget::add from server for lock
    rsp.targetList.push_back(targetID->value()); // not used in client
    return rsp.Encode();
}

PyResult DogmaIMBound::RemoveTarget(PyCallArgs& call, PyInt* targetID) {
    Client* pClient(call.client);

    SystemManager* pSysMgr = pClient->SystemMgr();
    if (pSysMgr == nullptr) {
        _log(SERVICE__ERROR, "Unable to find system manager for '%s'", pClient->GetName());
        return nullptr;
    }
    SystemEntity* pTSE = pSysMgr->GetSE(targetID->value());
    if (pTSE == nullptr) {
        _log(SERVICE__ERROR, "Unable to find entity %u in system %u for '%s'", targetID->value(), pSysMgr->GetID(), pClient->GetName());
        return nullptr;
    }

    if (sConfig.debug.IsTestServer)
        if (is_log_enabled(TARGET__MESSAGE)) {
            GVector vectorToTarget(pClient->GetShipSE()->GetPosition(), pTSE->GetPosition());
            _log(TARGET__MESSAGE, "Handle_RemoveTarget() - Removed %s(%u) - Range to Target: %.2f meters.", \
                        pTSE->GetName(),pTSE->GetID(), vectorToTarget.length() );
        }

    // tell our ship this target has been removed
    pClient->GetShipSE()->RemoveTarget(pTSE);
    return nullptr;
}


PyResult DogmaIMBound::GetAllInfo(PyCallArgs& call, PyBool* getCharInfo, PyBool* getShipInfo)
{
    // added more return data and updated logic (almost complete and mostly accurate) -allan 26Mar16
    // completed.  -allan 7Jan19
    // Start the Code
    Client* pClient(call.client);

    // Create the response dictionary
    PyDict* rsp = new PyDict();
    rsp->SetItemString("activeShipID", new PyInt(pClient->GetShipID()));
    // Set "locationInfo" in the Dictionary
    /** @todo  havent found a populated item in packet logs
     *
        def ProcessLocationInfo(self, cData):
            for locationID, datas in cData.iteritems():
        --still dont know what 'datas' are
        ** this has *something* to do with POS
        */
    if (getShipInfo->value()) {
        rsp->SetItemString("locationInfo", new PyDict());
    } else {
        rsp->SetItemString("locationInfo", PyStatic.NewNone());
    }

    // Set "shipModifiedCharAttribs" in the Dictionary
    /** @todo  havent found a populated item in packet logs */
    rsp->SetItemString("shipModifiedCharAttribs", PyStatic.NewNone());

    // Set "charInfo" in the Dictionary  -fixed 24Mar16
    sItemFactory.SetUsingClient(pClient);
    if (getCharInfo->value()) {
        PyDict* charResult = pClient->GetChar()->GetCharInfo();
        if (charResult == nullptr) {
            _log(SERVICE__ERROR, "Unable to build char info for char %u", pClient->GetCharacterID());
            sItemFactory.UnsetUsingClient();
            PySafeDecRef(rsp);
            return PyStatic.NewNone();
        }
        rsp->SetItemString("charInfo", charResult);
    } else {
        rsp->SetItemString("charInfo", new PyDict());
    }

    // Set "shipInfo" in the Dictionary  -fixed 26Mar16
    if (getShipInfo->value()) {
        PyDict* shipResult = pClient->GetShip()->GetShipInfo();
        if (shipResult == nullptr) {
            _log(SERVICE__ERROR, "Unable to build ship info for ship %u", pClient->GetShipID());
            sItemFactory.UnsetUsingClient();
            PySafeDecRef(rsp);
            return PyStatic.NewNone();
        }
        rsp->SetItemString("shipInfo", shipResult);
    } else {
        rsp->SetItemString("shipInfo", new PyDict());
    }

    // Set "shipState" in the Dictionary  -fixed 26Mar16  -UD to add linked weapons 7Jan19
    if (pClient->GetShip().get() == nullptr) {
        _log(SERVICE__ERROR, "Unable to build shipState for %u", pClient->GetShipID());
        PySafeDecRef(rsp);
        return PyStatic.NewNone();
    }
    PyTuple* rspShipState = new PyTuple(3);
        rspShipState->items[0] = pClient->GetShip()->GetShipState();        // fitted module list
        rspShipState->items[1] = pClient->GetShip()->GetChargeState();      // loaded charges (subLocation)
        rspShipState->items[2] = pClient->GetShip()->GetLinkedWeapons();    // linked weapons
    rsp->SetItemString("shipState", rspShipState);

    if (is_log_enabled(SHIP__STATE))
        rsp->Dump(SHIP__STATE, "     ");

    sItemFactory.UnsetUsingClient();
    return new PyObject("util.KeyVal", rsp );
}

PyResult DogmaIMBound::LinkWeapons(PyCallArgs& call, PyInt* shipID, PyInt* masterID, PyInt* fromID) {
    /*  data = self.remoteDogmaLM.LinkWeapons(shipID, masterID, fromID)
     *
     *    def SetWeaponBanks(self, shipID, data):
     *        self.slaveModulesByMasterModule[shipID] = defaultdict(set)
     *        if data is None:
     *            return
     *        for masterID, slaveIDs in data.iteritems():
     *            for slaveID in slaveIDs:
     *                self.slaveModulesByMasterModule[shipID][masterID].add(slaveID)
     */

    /* args.shipID
     * args.masterID
     * args.slaveID
     */
    if (!IsPlayerItem(shipID->value()))
        return nullptr;

    SystemManager* pSysMgr(call.client->SystemMgr());
    ShipItemRef sRef(nullptr);
    if (call.client->IsDocked()) {
        sRef = pSysMgr->GetStationFromInventory(call.client->GetStationID())->GetShipFromInventory(shipID->value());
    } else {
        sRef = pSysMgr->GetShipFromInventory(shipID->value());
    }
    if (sRef.get() == nullptr) {
        _log(INV__ERROR, "ShipRef not found in containers inventory for %s", call.client->GetName());
        call.client->SendErrorMsg("Your ship was not found.  Ref: ServerError xxxxx");
        return nullptr;
    }
    sRef->LinkWeapon(masterID->value(), fromID->value());
    return sRef->GetLinkedWeapons();
}

PyResult DogmaIMBound::LinkAllWeapons(PyCallArgs& call, PyInt* shipID) {
    if (!IsPlayerItem(shipID->value()))
        return nullptr;

    SystemManager* pSysMgr(call.client->SystemMgr());
    ShipItemRef sRef(nullptr);
    if (call.client->IsDocked()) {
        sRef = pSysMgr->GetStationFromInventory(call.client->GetStationID())->GetShipFromInventory(shipID->value());
    } else {
        sRef = pSysMgr->GetShipFromInventory(shipID->value());
    }
    if (sRef.get() == nullptr) {
        _log(INV__ERROR, "ShipRef not found in containers inventory for %s", call.client->GetName());
        call.client->SendErrorMsg("Your ship was not found.  Ref: ServerError xxxxx");
        return nullptr;
    }
    // locate and link all weapons on ship, if possible.
    sRef->LinkAllWeapons();
    return sRef->GetLinkedWeapons();
}

PyResult DogmaIMBound::DestroyWeaponBank(PyCallArgs& call, PyInt* shipID, PyInt* itemID) {
    //self.remoteDogmaLM.DestroyWeaponBank(shipID, itemID)
    if (!IsPlayerItem(shipID->value()) or !IsPlayerItem(itemID->value()))
        return nullptr;

    SystemManager* pSysMgr(call.client->SystemMgr());
    ShipItemRef sRef(nullptr);
    if (call.client->IsDocked()) {
        sRef = pSysMgr->GetStationFromInventory(call.client->GetStationID())->GetShipFromInventory(shipID->value());
    } else {
        sRef = pSysMgr->GetShipFromInventory(shipID->value());
    }
    if (sRef.get() == nullptr) {
        _log(INV__ERROR, "ShipRef not found in containers inventory for %s", call.client->GetName());
        call.client->SendErrorMsg("Your ship was not found.  Ref: ServerError xxxxx");
        return nullptr;
    }
    sRef->UnlinkGroup(itemID->value());
    return nullptr;
}

PyResult DogmaIMBound::UnlinkAllModules(PyCallArgs& call, PyInt* shipID) {
    //info = self.remoteDogmaLM.UnlinkAllModules(shipID)
    if (!IsPlayerItem(shipID->value()))
        return nullptr;

    SystemManager* pSysMgr(call.client->SystemMgr());
    ShipItemRef sRef(nullptr);
    if (call.client->IsDocked()) {
        sRef = pSysMgr->GetStationFromInventory(call.client->GetStationID())->GetShipFromInventory(shipID->value());
    } else {
        sRef = pSysMgr->GetShipFromInventory(shipID->value());
    }
    if (sRef.get() == nullptr) {
        _log(INV__ERROR, "ShipRef not found in containers inventory for %s", call.client->GetName());
        call.client->SendErrorMsg("Your ship was not found.  Ref: ServerError xxxxx");
        return nullptr;
    }
    sRef->UnlinkAllWeapons();
    return sRef->GetLinkedWeapons();
}

PyResult DogmaIMBound::UnlinkModule(PyCallArgs& call, PyInt* shipID, PyInt* moduleID) {
    // slaveID = self.remoteDogmaLM.UnlinkModule(shipID, moduleID)
    if (is_log_enabled(SHIP__MESSAGE)) {
        sLog.Warning("DogmaIMBound::Handle_UnlinkModule()", "size=%lu", call.tuple->size());
        call.Dump(SHIP__MESSAGE);
    }

    if (!IsPlayerItem(shipID->value()) or !IsPlayerItem(moduleID->value()))
        return PyStatic.NewZero();

    SystemManager* pSysMgr(call.client->SystemMgr());
    ShipItemRef sRef(nullptr);
    if (call.client->IsDocked()) {
        sRef = pSysMgr->GetStationFromInventory(call.client->GetStationID())->GetShipFromInventory(shipID->value());
    } else {
        sRef = pSysMgr->GetShipFromInventory(shipID->value());
    }
    if (sRef.get() == nullptr) {
        _log(INV__ERROR, "ShipRef not found in containers inventory for %s", call.client->GetName());
        call.client->SendErrorMsg("Your ship was not found.  Ref: ServerError xxxxx");
        return PyStatic.NewZero();
    }

    return new PyInt(sRef->UnlinkWeapon(moduleID->value()));
}

PyResult DogmaIMBound::MergeModuleGroups(PyCallArgs& call, PyInt* shipID, PyInt* masterID, PyInt* slaveID) {
    //info = self.remoteDogmaLM.MergeModuleGroups(shipID, masterID, slaveID)
    if (is_log_enabled(SHIP__MESSAGE)) {
        sLog.Warning("DogmaIMBound::Handle_MergeModuleGroups()", "size=%lu", call.tuple->size());
        call.Dump(SHIP__MESSAGE);
    }

    /* args.shipID
     * args.masterID
     * args.slaveID
     */
    if (!IsPlayerItem(shipID->value()))
        return nullptr;
    SystemManager* pSysMgr(call.client->SystemMgr());
    ShipItemRef sRef(nullptr);
    if (call.client->IsDocked()) {
        sRef = pSysMgr->GetStationFromInventory(call.client->GetStationID())->GetShipFromInventory(shipID->value());
    } else {
        sRef = pSysMgr->GetShipFromInventory(shipID->value());
    }
    if (sRef.get() == nullptr) {
        _log(INV__ERROR, "ShipRef not found in containers inventory for %s", call.client->GetName());
        call.client->SendErrorMsg("Your ship was not found.  Ref: ServerError xxxxx");
        return nullptr;
    }

    // merge slaveID group into masterID group
    sRef->MergeModuleGroups(masterID->value(), slaveID->value());

    return sRef->GetLinkedWeapons();
}

PyResult DogmaIMBound::PeelAndLink(PyCallArgs& call, PyInt* shipID, PyInt* masterID, PyInt* slaveID) {
    //info = self.remoteDogmaLM.PeelAndLink(shipID, masterID, slaveID)
    if (is_log_enabled(SHIP__MESSAGE)) {
        sLog.Warning("DogmaIMBound::Handle_PeelAndLink()", "size=%lu", call.tuple->size());
        call.Dump(SHIP__MESSAGE);
    }

    /* args.shipID
     * args.masterID
     * args.slaveID
     */
    if (!IsPlayerItem(shipID->value()))
        return nullptr;
    SystemManager* pSysMgr(call.client->SystemMgr());
    ShipItemRef sRef(nullptr);
    if (call.client->IsDocked()) {
        sRef = pSysMgr->GetStationFromInventory(call.client->GetStationID())->GetShipFromInventory(shipID->value());
    } else {
        sRef = pSysMgr->GetShipFromInventory(shipID->value());
    }
    if (sRef.get() == nullptr) {
        _log(INV__ERROR, "ShipRef not found in containers inventory for %s", call.client->GetName());
        call.client->SendErrorMsg("Your ship was not found.  Ref: ServerError xxxxx");
        return nullptr;
    }

    // remove slaveID from existing group and add to masterID group
    sRef->PeelAndLink(masterID->value(), slaveID->value());
    return sRef->GetLinkedWeapons();
}

PyResult DogmaIMBound::Activate(PyCallArgs& call, PyInt* itemID, PyInt* effectID) {
    // dogmaLM.Activate(itemID, const.effectOnlineForStructures)
    // dogmaLM.Activate(itemID, const.effectAnchorDrop)
    // dogmaLM.Activate(itemID, const.effectAnchorLift)
    // dogmaLM.Activate(itemID, const.effectAnchorLiftForStructures)

    if (!call.client->IsInSpace()) {
        call.client->SendNotifyMsg("You can't do this while docked.");
        return PyStatic.NewZero();
    }

    SystemEntity* pSE = call.client->SystemMgr()->GetSE(itemID->value());
    if (pSE == nullptr) {
        sLog.Error("DogmaIMBound::Handle_Activate()", "%u is not a valid EntityID in this system.", itemID->value());
        return PyStatic.NewZero();
    }


    // anchor cargo and pos items
    // online pos items
    /*      this is deactivate call....
        22:06:59 W DogmaIMBound::Handle_Activate(): size=2
        22:06:59 [POS:Dump]   Call Arguments:
        22:06:59 [POS:Dump]      Tuple: 2 elements
        22:06:59 [POS:Dump]       [ 0]    Integer: 140000061
        22:06:59 [POS:Dump]       [ 1]    Integer: 1023     << deactivate

        anchorDrop =   649,     // effects.AnchorDrop
        anchorLift =   650,     // effects.AnchorLift
        onlineForStructures =   901,     // effects.StructureOnline
        anchorDropForStructures =   1022,     // effects.AnchorDrop
        anchorLiftForStructures =   1023,     // effects.AnchorLift
        anchorDropOrbital =   4769,     // effects.AnchorDrop
        anchorLiftOrbital =   4770,     // effects.AnchorLift
    */


    // determine if this pSE is pos or cont.
    //call (de)activate on pSE, pass effectID, send effect to clients (bubblecast) then set timers.
    if (pSE->IsPOSSE()) {
        /*  these two may be called in posMgr...
        if ((args.arg2 == anchorDropForStructures)
        or (args.arg2 == anchorDropOrbital)) {
            pSE->GetPOSSE()->SetAnchor(args.arg2);
        } else */ if (effectID->value() == anchorLiftForStructures) {
            pSE->GetPOSSE()->PullAnchor();
        }
        else if (pSE->IsPlatformSE() and effectID->value() == anchorDrop) {
            GPoint pos = pSE->GetPosition();
            pSE->GetPOSSE()->SetAnchor(call.client, pos);
        }
        else if (pSE->IsPlatformSE() and effectID->value() == anchorLift) {
            pSE->GetPOSSE()->PullAnchor();
        }
        else if (effectID->value() == onlineForStructures) {
            pSE->GetPOSSE()->Activate(effectID->value());
        }
        if (pSE->IsContainerSE() and effectID->value() == anchorDrop) {
            GPoint pos = pSE->GetPosition();
            pSE->GetContSE()->SetAnchor(call.client, pos);
        }
        else if (pSE->IsContainerSE() and effectID->value() == anchorLift) {
            pSE->GetContSE()->PullAnchor();
        }

    }

    return PyStatic.NewOne();
}

PyResult DogmaIMBound::Activate(PyCallArgs& call, PyInt* itemID, PyWString* effectName, std::optional <PyInt*> target, PyInt* repeat)
{
    // ret = self.GetDogmaLM().Activate(itemID, effectName, target, repeat)  - i cant find where this return is used but is "1" in packet logs
    Client* pClient(call.client);

    if (!pClient->IsInSpace()) {
        pClient->SendNotifyMsg("You can't do this while docked.");
        return PyStatic.NewZero();
    }
    /*
    * {'FullPath': u'UI/Messages', 'messageID': 260384, 'label': u'AnchorLocationUnsuitableBody'}(u'You cannot safely anchor {[item]typeID.nameWithArticle} within {[numeric]distance.distance} of large objects in space.', None, {u'{[numeric]distance.distance}': {'conditionalValues': [], 'variableType': 9, 'propertyName': 'distance', 'args': 256, 'kwargs': {}, 'variableName': 'distance'}, u'{[item]typeID.nameWithArticle}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'nameWithArticle', 'args': 0, 'kwargs': {}, 'variableName': 'typeID'}})
    * {'FullPath': u'UI/Messages', 'messageID': 260389, 'label': u'AnchorProximityUnsuitableBody'}(u'You cannot anchor an item of group {anchorGroupName} within {[numeric]distance.distance} of items of group {ballGroupName} in space.', None, {u'{ballGroupName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'ballGroupName'}, u'{anchorGroupName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'anchorGroupName'}, u'{[numeric]distance.distance}': {'conditionalValues': [], 'variableType': 9, 'propertyName': 'distance', 'args': 256, 'kwargs': {}, 'variableName': 'distance'}})
    * {'FullPath': u'UI/Messages', 'messageID': 260403, 'label': u'AnchoringObjectBody'}(u'Anchoring the selected object, it will take around {[numeric]delay, decimalPlaces=1} {[numeric]delay -> "second", "seconds"} to do so.', None, {u'{[numeric]delay -> "second", "seconds"}': {'conditionalValues': [u'second', u'seconds'], 'variableType': 9, 'propertyName': None, 'args': 320, 'kwargs': {}, 'variableName': 'delay'}, u'{[numeric]delay, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'delay'}})
    *
    */
    // activate ship module
    pClient->GetShip()->Activate(itemID->value(), effectName->content(), target.has_value () ? target.value ()->value() : 0, repeat->value());
    // are there any other cases to test for here?

    // returns 1 on success (throw error otherwise)
    return PyStatic.NewOne();
}

PyResult DogmaIMBound::Deactivate(PyCallArgs& call, PyInt* itemID, PyInt* effect) {
    Client* pClient(call.client);

    if (!pClient->IsInSpace()) {
        pClient->SendNotifyMsg("You can't do this while docked");
        return PyStatic.NewNone();
    }

    // if effect is integer, call is for pos or container
    call.Dump(POS__DUMP);
    SystemEntity* pSE = pClient->SystemMgr()->GetSE(itemID->value());
    if (pSE == nullptr) {
        sLog.Error("DogmaIMBound::Handle_Deactivate()", "%u is not a valid EntityID in this system.", itemID->value());
        return PyStatic.NewNone();
    }
    /*
     * 22:24:28 W DogmaIMBound::Handle_Deactivate(): size=2
     * 22:24:28 [POS:Dump]   Call Arguments:
     * 22:24:28 [POS:Dump]      Tuple: 2 elements
     * 22:24:28 [POS:Dump]       [ 0]    Integer: 140000061
     * 22:24:28 [POS:Dump]       [ 1]    Integer: 901
     */
     // determine if this pSE is pos or cont.
     //call activate on pSE, pass effectID, send effect to clients (bubblecast) then set timers.
    if (pSE->IsPOSSE()) {
        pSE->GetPOSSE()->Deactivate(effect->value());
    }
    else if (pSE->IsContainerSE()) {
        pSE->GetContSE()->Deactivate(effect->value());
    }
    else {
        ; // make error here
    }

    return PyStatic.NewNone();
}

PyResult DogmaIMBound::Deactivate(PyCallArgs& call, PyInt* itemID, PyWString* effectName)
{
    //  return self.statemanager.Deactivate(self.itemID, self.effectName)
    //  dogmaLM.Deactivate(itemID, const.effectOnlineForStructures)
    if (is_log_enabled(SHIP__MESSAGE)) {
        sLog.Warning("DogmaIMBound::Handle_Deactivate()", "size=%lu", call.tuple->size());
        call.Dump(SHIP__MESSAGE);
    }

    Client* pClient(call.client);

    if (!pClient->IsInSpace()) {
        pClient->SendNotifyMsg("You can't do this while docked");
        return PyStatic.NewNone();
    }

    pClient->GetShip()->Deactivate(itemID->value(), effectName->content());
    // are there any other cases to test for?

    // returns None()
    return PyStatic.NewNone();
}

PyResult DogmaIMBound::Overload(PyCallArgs& call, PyInt* itemID, PyInt* effectID) {
    /*
     * 23:52:45 L DogmaIMBound::Handle_Overload(): size=2
     * 23:52:45 [SvcCallDump]   Call Arguments:
     * 23:52:45 [SvcCallDump]       Tuple: 2 elements
     * 23:52:45 [SvcCallDump]         [ 0] Integer field: 140002542
     * 23:52:45 [SvcCallDump]         [ 1] Integer field: 3035
     */
    // self.GetDogmaLM().Overload(itemID, effectID)

    Client* pClient(call.client);
    //TODO:  need to verify pilot can OL modules
    // AttrRequiredThermoDynamicsSkill
    pClient->GetShip()->Overload(itemID->value());
    return nullptr;
}

// this one is called from Deactivate() when module is OL
PyResult DogmaIMBound::StopOverload(PyCallArgs& call, PyInt* itemID, PyInt* effectID)
{
    Client* pClient(call.client);
    //  cancel overload then deactivate module
    pClient->GetShip()->CancelOverloading(itemID->value());
    pClient->GetShip()->Deactivate(itemID->value(), sFxDataMgr.GetEffectName(effectID->value()));
    // returns none
    return PyStatic.NewNone();
}

PyResult DogmaIMBound::CancelOverloading(PyCallArgs& call, PyInt* itemID) {
    // self.dogmaLM.CancelOverloading(itemID)

    Client* pClient(call.client);
    pClient->GetShip()->CancelOverloading(itemID->value());
    return nullptr;
}

PyResult DogmaIMBound::OverloadRack(PyCallArgs& call, PyInt* itemID) {
    /* moduleIDs = self.GetDogmaLM().OverloadRack(itemID)
     *   moduleIDs is list of modules in rack.
     */
    Client* pClient(call.client);
    // not supported yet
    PyList* list = new PyList();
    return list;
}

PyResult DogmaIMBound::StopOverloadRack(PyCallArgs& call, PyInt* itemID) {
    /* moduleIDs = self.GetDogmaLM().StopOverloadRack(itemID)
     *   moduleIDs is list of modules in rack.
     */
    Client* pClient(call.client);
    // not supported yet
    PyList* list = new PyList();
    return list;
}

PyResult DogmaIMBound::InitiateModuleRepair(PyCallArgs& call, PyInt* itemID) {
    //  this is for repairing modules using nanite paste (button's ring turns white).  return bool.
    //  res = self.GetDogmaLM().InitiateModuleRepair(itemID)
    // see notes in ModuleManager::ModuleRepair()

    return call.client->GetShip()->ModuleRepair(itemID->value());
}

PyResult DogmaIMBound::StopModuleRepair(PyCallArgs& call, PyInt* itemID) {
    //  self.GetDogmaLM().StopModuleRepair(itemID)

    call.client->GetShip()->StopModuleRepair(itemID->value());

    // returns nothing
    return nullptr;
}

PyResult DogmaIMBound::ChangeDroneSettings(PyCallArgs& call, PyDict* settings) {
    /*
     * 21:59:29 L DogmaIMBound::Handle_ChangeDroneSettings(): size=1
     * 21:59:29 [SvcCall]   Call Arguments:
     * 22:04:44 [SvcCall]       Tuple: 1 elements
     * 22:04:44 [SvcCall]         [ 0] Dictionary: 3 entries
     * 22:04:44 [SvcCall]         [ 0]   [ 0] Key: Integer field: 1283 <-- AttrFightersAttackAndFollow
     * 22:04:44 [SvcCall]         [ 0]   [ 0] Value: Integer field: 1
     * 22:04:44 [SvcCall]         [ 0]   [ 1] Key: Integer field: 1275 <-- AttrDroneIsAgressive
     * 22:04:44 [SvcCall]         [ 0]   [ 1] Value: Integer field: 1
     * 22:04:44 [SvcCall]         [ 0]   [ 2] Key: Integer field: 1297 <-- AttrDroneFocusFire
     * 22:04:44 [SvcCall]         [ 0]   [ 2] Value: Integer field: 1
     */
    
    std::map<int16, int8> attribs;
    for (PyDict::const_iterator itr = settings->begin(); itr != settings->end(); ++itr)
        attribs[PyRep::IntegerValueU32(itr->first)] = PyRep::IntegerValue(itr->second);

    call.client->GetShipSE()->UpdateDrones(attribs);

    // returns nothing
    return nullptr;
}
