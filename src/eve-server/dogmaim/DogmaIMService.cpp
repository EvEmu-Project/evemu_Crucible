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
#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "cache/ObjCacheService.h"
#include "dogmaim/DogmaIMService.h"
#include "pos/Tower.h"
#include "ship/modules/GenericModule.h"
#include "system/Container.h"
#include "system/SystemManager.h"
#include "station/Station.h"

/** this is either DogmaLM (Location Manager) or DogmaIM (Instance Manager) for bound objects.
 * it depends on the object, location, and calling function
 *    i see no reason to change it at this point.
 */
class DogmaIMBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(DogmaIMBound)

    DogmaIMBound(PyServiceMgr* mgr, uint32 locationID, uint32 groupID)
    : PyBoundObject(mgr),
    m_dispatch(new Dispatcher(this)),
    m_locationID(locationID),
    m_groupID(groupID)
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "DogmaIMBound";

        PyCallable_REG_CALL(DogmaIMBound, ChangeDroneSettings);
        PyCallable_REG_CALL(DogmaIMBound, LinkWeapons);
        PyCallable_REG_CALL(DogmaIMBound, LinkAllWeapons);
        PyCallable_REG_CALL(DogmaIMBound, UnlinkModule);
        PyCallable_REG_CALL(DogmaIMBound, UnlinkAllModules);
        PyCallable_REG_CALL(DogmaIMBound, OverloadRack);
        PyCallable_REG_CALL(DogmaIMBound, StopOverloadRack);
        PyCallable_REG_CALL(DogmaIMBound, CharGetInfo);
        PyCallable_REG_CALL(DogmaIMBound, ItemGetInfo);
        PyCallable_REG_CALL(DogmaIMBound, GetAllInfo);
        PyCallable_REG_CALL(DogmaIMBound, DestroyWeaponBank);
        PyCallable_REG_CALL(DogmaIMBound, GetCharacterBaseAttributes);
        PyCallable_REG_CALL(DogmaIMBound, Activate);
        PyCallable_REG_CALL(DogmaIMBound, Deactivate);
        PyCallable_REG_CALL(DogmaIMBound, Overload);
        PyCallable_REG_CALL(DogmaIMBound, StopOverload);
        PyCallable_REG_CALL(DogmaIMBound, CancelOverloading);
        PyCallable_REG_CALL(DogmaIMBound, SetModuleOnline);
        PyCallable_REG_CALL(DogmaIMBound, TakeModuleOffline);
        PyCallable_REG_CALL(DogmaIMBound, LoadAmmoToBank);
        PyCallable_REG_CALL(DogmaIMBound, LoadAmmoToModules);
        PyCallable_REG_CALL(DogmaIMBound, GetTargets);
        PyCallable_REG_CALL(DogmaIMBound, GetTargeters);
        PyCallable_REG_CALL(DogmaIMBound, AddTarget);       //AddTargetOBO
        PyCallable_REG_CALL(DogmaIMBound, RemoveTarget);
        PyCallable_REG_CALL(DogmaIMBound, ClearTargets);
        PyCallable_REG_CALL(DogmaIMBound, InitiateModuleRepair);
        PyCallable_REG_CALL(DogmaIMBound, StopModuleRepair);
        PyCallable_REG_CALL(DogmaIMBound, MergeModuleGroups);
        PyCallable_REG_CALL(DogmaIMBound, PeelAndLink);
    }
    virtual ~DogmaIMBound() {delete m_dispatch;}
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(ChangeDroneSettings);
    PyCallable_DECL_CALL(LinkWeapons);
    PyCallable_DECL_CALL(LinkAllWeapons);
    PyCallable_DECL_CALL(UnlinkModule);
    PyCallable_DECL_CALL(UnlinkAllModules);
    PyCallable_DECL_CALL(OverloadRack);
    PyCallable_DECL_CALL(StopOverloadRack);
    PyCallable_DECL_CALL(CharGetInfo);
    PyCallable_DECL_CALL(ItemGetInfo);
    PyCallable_DECL_CALL(GetAllInfo);
    PyCallable_DECL_CALL(DestroyWeaponBank);
    PyCallable_DECL_CALL(GetCharacterBaseAttributes);
    PyCallable_DECL_CALL(Activate);
    PyCallable_DECL_CALL(Deactivate);
    PyCallable_DECL_CALL(Overload);
    PyCallable_DECL_CALL(StopOverload);
    PyCallable_DECL_CALL(CancelOverloading);
    PyCallable_DECL_CALL(SetModuleOnline);
    PyCallable_DECL_CALL(TakeModuleOffline);
    PyCallable_DECL_CALL(LoadAmmoToBank);
    PyCallable_DECL_CALL(LoadAmmoToModules);
    PyCallable_DECL_CALL(GetTargets);
    PyCallable_DECL_CALL(GetTargeters);
    PyCallable_DECL_CALL(AddTarget);
    PyCallable_DECL_CALL(RemoveTarget);
    PyCallable_DECL_CALL(ClearTargets);
    PyCallable_DECL_CALL(InitiateModuleRepair);
    PyCallable_DECL_CALL(StopModuleRepair);
    PyCallable_DECL_CALL(MergeModuleGroups);
    PyCallable_DECL_CALL(PeelAndLink);

    /*  OBO == ??  (pos targeting)
     * flag, targetList = self.GetDogmaLM().AddTargetOBO(sid, tid) (structureID, targetID)
     * self.GetDogmaLM().RemoveTargetOBO(sid, tid)  (structureID, targetID)
    */
protected:
    Dispatcher* const m_dispatch;

    uint32 m_locationID;
    uint32 m_groupID;
};

PyCallable_Make_InnerDispatcher(DogmaIMService)


DogmaIMService::DogmaIMService(PyServiceMgr* mgr)
: PyService(mgr, "dogmaIM"),  // IM = Instance Manager, also LM = Location Manager
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(DogmaIMService, GetAttributeTypes);
}

DogmaIMService::~DogmaIMService() {
    delete m_dispatch;
}

PyResult DogmaIMService::Handle_GetAttributeTypes(PyCallArgs& call) {
    PyString* str = new PyString("dogmaIM.attributesByName" );
    PyRep* result = m_manager->cache_service->GetCacheHint( str );
    PyDecRef( str );
    return result;
}

PyBoundObject* DogmaIMService::CreateBoundObject(Client *pClient, const PyRep* bind_args) {
    DogmaLM_BindArgs args;
    //crap
    PyRep* tmp(bind_args->Clone());
    if (!args.Decode(&tmp)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode bind args.", GetName());
        return nullptr;
    }

    return new DogmaIMBound(m_manager, args.locationID, args.groupID);
}

PyResult DogmaIMBound::Handle_CharGetInfo(PyCallArgs& call) {
    return call.client->GetChar()->GetCharInfo();
}

PyResult DogmaIMBound::Handle_ClearTargets(PyCallArgs& call) {
    call.client->GetShipSE()->TargetMgr()->ClearTargets();
    //call.client->GetShipSE()->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::ClientReq);
    return nullptr;
}

PyResult DogmaIMBound::Handle_GetTargets(PyCallArgs& call) {
    return call.client->GetShipSE()->TargetMgr()->GetTargets();
}

PyResult DogmaIMBound::Handle_GetTargeters(PyCallArgs& call) {
    return call.client->GetShipSE()->TargetMgr()->GetTargeters();
}

PyResult DogmaIMBound::Handle_GetCharacterBaseAttributes(PyCallArgs& call)
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


PyResult DogmaIMBound::Handle_ItemGetInfo(PyCallArgs& call) {
    // called when item 'row' info not in shipState data from GetAllInfo() return
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    InventoryItemRef itemRef = sItemFactory.GetItemRef(args.arg);
    if (itemRef.get() == nullptr ) {
        _log(INV__ERROR, "Unable to load item %u", args.arg);
        return PyStatic.NewNone();
    }

    PyObject* obj = itemRef->ItemGetInfo();
    if (is_log_enabled(ITEM__DEBUG))
        obj->Dump(ITEM__DEBUG, "    ");
    return obj;
}

PyResult DogmaIMBound::Handle_SetModuleOnline(PyCallArgs& call) {
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

	Call_TwoIntegerArgs args; //locationID, moduleID

	if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

	pClient->GetShip()->Online(args.arg2);

    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
        tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult DogmaIMBound::Handle_TakeModuleOffline(PyCallArgs& call) {
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

	Call_TwoIntegerArgs args; //locationID, moduleID
	if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

	pClient->GetShip()->Offline(args.arg2);

    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
        tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult DogmaIMBound::Handle_LoadAmmoToModules(PyCallArgs& call) {
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
    Call_Dogma_LoadAmmoToModules args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (args.moduleIDs.empty())
        return nullptr;
    if (args.moduleIDs.size() > 1) {
        sLog.Error("DogmaIMBound::Handle_LoadAmmoToModules()", "args.moduleIDs.size = %lu.", args.moduleIDs.size() );
        call.Dump(MODULE__WARNING);
    }

    // Get Reference to Ship and Charge
    ShipItemRef sRef = call.client->GetShip();
    GenericModule* pMod = sRef->GetModule(sItemFactory.GetItemRef(args.moduleIDs[0])->flag());
    if (pMod == nullptr)
        throw UserError ("ModuleNoLongerPresentForCharges");

    InventoryItemRef cRef = sItemFactory.GetItemRef(args.itemID);
    sRef->LoadCharge(cRef, pMod->flag());

    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
        tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult DogmaIMBound::Handle_LoadAmmoToBank(PyCallArgs& call) {
  /*   NOTE:  this to load ALL modules in weapon bank, if possible.
   * self.remoteDogmaLM.LoadAmmoToBank(shipID, masterID, chargeTypeID,  itemIDs,  chargeLocationID,   qty)
   *                                    ship,   module,  charge type, charge item, charge location, stack qty (usually none - havent found otherwise)
   *   *******    UPDATED VAR NAMES TO MATCH CLIENT CODE  -allan 26Jul14  *************
   */
  _log(MODULE__TRACE, "DogmaIMBound::Handle_LoadAmmoToBank()");
  call.Dump(MODULE__TRACE);
	Call_Dogma_LoadAmmoToBank args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    /*
    args.shipID
    args.masterID
    args.chargeTypeID
    args.itemIDs
    args.chargeLocationID
    args.qty
    */
    if (args.itemIDs.empty())
        return nullptr;

    // Get Reference to Ship, Module, and Charge
    ShipItemRef sRef = call.client->GetShip();
    if (sRef->itemID() != args.shipID)
        sLog.Error("DogmaIMBound::Handle_LoadAmmoToBank()", "passed shipID %u != current shipID %u.", args.shipID, sRef->itemID() );

    // if shipID passed in call isnt active ship (from client->GetShip()), would this work right?
    GenericModule* pMod = sRef->GetModule(sItemFactory.GetItemRef(args.masterID)->flag());
    if (pMod == nullptr)
        throw UserError ("ModuleNoLongerPresentForCharges");

    // figure out how to use args.qty for loading less-than-full charges
    //  LoadCharge() can be easily updated to do this.
    if (pMod->IsLinked()) {
        sRef->LoadLinkedWeapons(pMod, args.itemIDs);
    } else {
        sRef->LoadCharge(sItemFactory.GetItemRef(args.itemIDs.at(0)), pMod->flag());
    }

    // not sure why im not using this, as call is to load bank...
    //sRef->LoadChargesToBank(pMod->flag(), args.itemIDs);

    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
        tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult DogmaIMBound::Handle_AddTarget(PyCallArgs& call) {
   // flag, targetList = self.GetDogmaLM().AddTargetOBO(sid, tid)
    // flag, targetList = self.GetDogmaLM().AddTarget(tid)
    // as a note, targetList isnt used once call returns to client
    // throws here void returns.  client will raise throw (and ignore return)
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }
    // calling client tests
    Client* pClient(call.client);
    if (pClient->IsJump())
        throw UserError ("CantTargetWhileJumping");

    if (pClient->GetShipID() == args.arg)
        throw UserError ("DeniedTargetSelf");

    /** @todo pClient->IsUncloak() incomplete */
    if (pClient->IsUncloak())
        throw UserError ("DeinedTargetAfterCloak");

    if (pClient->IsDocked()) {
        pClient->SendNotifyMsg("You can't do this while docked");
        Rsp_Dogma_AddTarget rsp;
        rsp.flag = false;
        rsp.targetList.push_back(args.arg);
        return rsp.Encode();
    }
    // caller ship tests
    ShipSE* mySE = pClient->GetShipSE();
    if ( mySE->TargetMgr() == nullptr)
        throw UserError ("DeniedTargetingAttemptFailed")
                .AddFormatValue ("target", new PyInt (args.arg));
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
                .AddFormatValue ("target", new PyInt (args.arg));
    }
    if (mySE->SysBubble()->HasTower()) {
        TowerSE* ptSE = mySE->SysBubble()->GetTowerSE();
        if (ptSE->HasForceField() && mySE->GetPosition().distance(ptSE->GetPosition()) < ptSE->GetSOI())
                throw UserError ("DeniedTargetingInsideField")
                        .AddFormatValue ("target", new PyInt (args.arg));
    }

    // caller destiny tests
    DestinyManager* pMyDestiny = mySE->DestinyMgr();
    if (pMyDestiny == nullptr) {
        _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
        throw UserError ("DeniedTargetingAttemptFailed")
                .AddFormatValue ("target", new PyInt (args.arg));
    }
    if (pMyDestiny->IsCloaked())
        throw UserError ("CantTargetWhileCloaked");
       // throw UserError ("DeniedTargetingCloaked");

    // verify caller sysMgr
    SystemManager* pSysMgr = pClient->SystemMgr();
    if (pSysMgr == nullptr) {
        _log(PLAYER__WARNING, "Unable to find system manager for '%s'", pClient->GetName());
        throw UserError ("DeniedTargetingAttemptFailed")
                .AddFormatValue ("target", new PyInt (args.arg));
    }

    // target tests
    SystemEntity* tSE = pSysMgr->GetSE(args.arg);
    if (tSE == nullptr) {
        _log(INV__WARNING, "Unable to find entity %u in system %u from '%s'", args.arg, pSysMgr->GetID(), pClient->GetName());
        throw UserError ("DeniedTargetingAttemptFailed")
                .AddFormatValue ("target", new PyInt (args.arg));
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
                .AddFormatValue ("target", new PyInt (args.arg));
    }
    if (tSE->IsPOSSE())
        if (tSE->GetPOSSE()->IsReinforced())
            throw UserError ("DeniedTargetReinforcedStructure")
                    .AddFormatValue ("target", new PyInt (args.arg));
    if (tSE->SysBubble()->HasTower()) {
        TowerSE* ptSE = tSE->SysBubble()->GetTowerSE();
        if (ptSE->HasForceField() && tSE->GetPosition().distance(ptSE->GetPosition()) < ptSE->GetSOI())
                throw UserError ("DeniedTargetForceField")
                        .AddFormatValue ("target", new PyInt (args.arg))
                        .AddFormatValue ("range", new PyInt (ptSE->GetSOI ()))
                        .AddFormatValue ("item", new PyInt (ptSE->GetID ()));
    }

    if (!mySE->TargetMgr()->StartTargeting( tSE, pClient->GetShip())) {
        _log(TARGET__WARNING, "AddTarget() - TargMgr.StartTargeting() failed.");
        throw UserError ("DeniedTargetingAttemptFailed")
                .AddFormatValue ("target", new PyInt (args.arg));
    }

    Rsp_Dogma_AddTarget rsp;
    rsp.flag = true;    // false = immediate target lock in client, true = wait for OnTarget::add from server for lock
    rsp.targetList.push_back(args.arg); // not used in client
    return rsp.Encode();
}

PyResult DogmaIMBound::Handle_RemoveTarget(PyCallArgs& call) {
    Client* pClient(call.client);

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    SystemManager* pSysMgr = pClient->SystemMgr();
    if (pSysMgr == nullptr) {
        _log(SERVICE__ERROR, "Unable to find system manager for '%s'", pClient->GetName());
        return nullptr;
    }
    SystemEntity* pTSE = pSysMgr->GetSE(args.arg);
    if (pTSE == nullptr) {
        _log(SERVICE__ERROR, "Unable to find entity %u in system %u for '%s'", args.arg, pSysMgr->GetID(), pClient->GetName());
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


PyResult DogmaIMBound::Handle_GetAllInfo(PyCallArgs& call)
{
    // added more return data and updated logic (almost complete and mostly accurate) -allan 26Mar16
    // completed.  -allan 7Jan19
    // Start the Code
    Client* pClient(call.client);

    Call_TwoBoolArgs args; // arg1: getCharInfo, arg2: getShipInfo
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

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
    if (args.arg2) {
        rsp->SetItemString("locationInfo", new PyDict());
    } else {
        rsp->SetItemString("locationInfo", PyStatic.NewNone());
    }

    // Set "shipModifiedCharAttribs" in the Dictionary
    /** @todo  havent found a populated item in packet logs */
    rsp->SetItemString("shipModifiedCharAttribs", PyStatic.NewNone());

    // Set "charInfo" in the Dictionary  -fixed 24Mar16
    sItemFactory.SetUsingClient(pClient);
    if (args.arg1) {
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
    if (args.arg2) {
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

PyResult DogmaIMBound::Handle_LinkWeapons(PyCallArgs& call) {
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

    Call_Dogma_LinkWeapons args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    /* args.shipID
     * args.masterID
     * args.slaveID
     */
    if (!IsPlayerItem(args.shipID))
        return nullptr;

    SystemManager* pSysMgr(call.client->SystemMgr());
    ShipItemRef sRef(nullptr);
    if (call.client->IsDocked()) {
        sRef = pSysMgr->GetStationFromInventory(call.client->GetStationID())->GetShipFromInventory(args.shipID);
    } else {
        sRef = pSysMgr->GetShipFromInventory(args.shipID);
    }
    if (sRef.get() == nullptr) {
        _log(INV__ERROR, "ShipRef not found in containers inventory for %s", call.client->GetName());
        call.client->SendErrorMsg("Your ship was not found.  Ref: ServerError xxxxx");
        return nullptr;
    }
    sRef->LinkWeapon(args.masterID, args.slaveID);
    return sRef->GetLinkedWeapons();
}

PyResult DogmaIMBound::Handle_LinkAllWeapons(PyCallArgs& call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (!IsPlayerItem(arg.arg))
        return nullptr;

    SystemManager* pSysMgr(call.client->SystemMgr());
    ShipItemRef sRef(nullptr);
    if (call.client->IsDocked()) {
        sRef = pSysMgr->GetStationFromInventory(call.client->GetStationID())->GetShipFromInventory(arg.arg);
    } else {
        sRef = pSysMgr->GetShipFromInventory(arg.arg);
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

PyResult DogmaIMBound::Handle_DestroyWeaponBank(PyCallArgs& call) {
    //self.remoteDogmaLM.DestroyWeaponBank(shipID, itemID)
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (!IsPlayerItem(args.arg1) or !IsPlayerItem(args.arg2))
        return nullptr;

    SystemManager* pSysMgr(call.client->SystemMgr());
    ShipItemRef sRef(nullptr);
    if (call.client->IsDocked()) {
        sRef = pSysMgr->GetStationFromInventory(call.client->GetStationID())->GetShipFromInventory(args.arg1);
    } else {
        sRef = pSysMgr->GetShipFromInventory(args.arg1);
    }
    if (sRef.get() == nullptr) {
        _log(INV__ERROR, "ShipRef not found in containers inventory for %s", call.client->GetName());
        call.client->SendErrorMsg("Your ship was not found.  Ref: ServerError xxxxx");
        return nullptr;
    }
    sRef->UnlinkGroup(args.arg2);
    return nullptr;
}

PyResult DogmaIMBound::Handle_UnlinkAllModules(PyCallArgs& call) {
    //info = self.remoteDogmaLM.UnlinkAllModules(shipID)
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (!IsPlayerItem(arg.arg))
        return nullptr;

    SystemManager* pSysMgr(call.client->SystemMgr());
    ShipItemRef sRef(nullptr);
    if (call.client->IsDocked()) {
        sRef = pSysMgr->GetStationFromInventory(call.client->GetStationID())->GetShipFromInventory(arg.arg);
    } else {
        sRef = pSysMgr->GetShipFromInventory(arg.arg);
    }
    if (sRef.get() == nullptr) {
        _log(INV__ERROR, "ShipRef not found in containers inventory for %s", call.client->GetName());
        call.client->SendErrorMsg("Your ship was not found.  Ref: ServerError xxxxx");
        return nullptr;
    }
    sRef->UnlinkAllWeapons();
    return sRef->GetLinkedWeapons();
}

PyResult DogmaIMBound::Handle_UnlinkModule(PyCallArgs& call) {
    // slaveID = self.remoteDogmaLM.UnlinkModule(shipID, moduleID)
    if (is_log_enabled(SHIP__MESSAGE)) {
        sLog.Warning("DogmaIMBound::Handle_UnlinkModule()", "size=%lu", call.tuple->size());
        call.Dump(SHIP__MESSAGE);
    }

    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewZero();
    }

    if (!IsPlayerItem(args.arg1) or !IsPlayerItem(args.arg2))
        return PyStatic.NewZero();

    SystemManager* pSysMgr(call.client->SystemMgr());
    ShipItemRef sRef(nullptr);
    if (call.client->IsDocked()) {
        sRef = pSysMgr->GetStationFromInventory(call.client->GetStationID())->GetShipFromInventory(args.arg1);
    } else {
        sRef = pSysMgr->GetShipFromInventory(args.arg1);
    }
    if (sRef.get() == nullptr) {
        _log(INV__ERROR, "ShipRef not found in containers inventory for %s", call.client->GetName());
        call.client->SendErrorMsg("Your ship was not found.  Ref: ServerError xxxxx");
        return PyStatic.NewZero();
    }

    return new PyInt(sRef->UnlinkWeapon(args.arg2));
}

PyResult DogmaIMBound::Handle_MergeModuleGroups(PyCallArgs& call) {
    //info = self.remoteDogmaLM.MergeModuleGroups(shipID, masterID, slaveID)
    if (is_log_enabled(SHIP__MESSAGE)) {
        sLog.Warning("DogmaIMBound::Handle_MergeModuleGroups()", "size=%lu", call.tuple->size());
        call.Dump(SHIP__MESSAGE);
    }

    Call_Dogma_LinkWeapons args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    /* args.shipID
     * args.masterID
     * args.slaveID
     */
    if (!IsPlayerItem(args.shipID))
        return nullptr;
    SystemManager* pSysMgr(call.client->SystemMgr());
    ShipItemRef sRef(nullptr);
    if (call.client->IsDocked()) {
        sRef = pSysMgr->GetStationFromInventory(call.client->GetStationID())->GetShipFromInventory(args.shipID);
    } else {
        sRef = pSysMgr->GetShipFromInventory(args.shipID);
    }
    if (sRef.get() == nullptr) {
        _log(INV__ERROR, "ShipRef not found in containers inventory for %s", call.client->GetName());
        call.client->SendErrorMsg("Your ship was not found.  Ref: ServerError xxxxx");
        return nullptr;
    }

    // merge slaveID group into masterID group
    sRef->MergeModuleGroups(args.masterID, args.slaveID);

    return sRef->GetLinkedWeapons();
}

PyResult DogmaIMBound::Handle_PeelAndLink(PyCallArgs& call) {
    //info = self.remoteDogmaLM.PeelAndLink(shipID, masterID, slaveID)
    if (is_log_enabled(SHIP__MESSAGE)) {
        sLog.Warning("DogmaIMBound::Handle_PeelAndLink()", "size=%lu", call.tuple->size());
        call.Dump(SHIP__MESSAGE);
    }

    Call_Dogma_LinkWeapons args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    /* args.shipID
     * args.masterID
     * args.slaveID
     */
    if (!IsPlayerItem(args.shipID))
        return nullptr;
    SystemManager* pSysMgr(call.client->SystemMgr());
    ShipItemRef sRef(nullptr);
    if (call.client->IsDocked()) {
        sRef = pSysMgr->GetStationFromInventory(call.client->GetStationID())->GetShipFromInventory(args.shipID);
    } else {
        sRef = pSysMgr->GetShipFromInventory(args.shipID);
    }
    if (sRef.get() == nullptr) {
        _log(INV__ERROR, "ShipRef not found in containers inventory for %s", call.client->GetName());
        call.client->SendErrorMsg("Your ship was not found.  Ref: ServerError xxxxx");
        return nullptr;
    }

    // remove slaveID from existing group and add to masterID group
    sRef->PeelAndLink(args.masterID, args.slaveID);
    return sRef->GetLinkedWeapons();
}

PyResult DogmaIMBound::Handle_Activate(PyCallArgs& call)
{
    // ret = self.GetDogmaLM().Activate(itemID, effectName, target, repeat)  - i cant find where this return is used but is "1" in packet logs
    // dogmaLM.Activate(itemID, const.effectOnlineForStructures)
    // dogmaLM.Activate(itemID, const.effectAnchorDrop)
    // dogmaLM.Activate(itemID, const.effectAnchorLift)
    // dogmaLM.Activate(itemID, const.effectAnchorLiftForStructures)
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
    if (call.tuple->size() == 2) {
        call.Dump(POS__DUMP);
        // anchor cargo and pos items
        // online pos items
        Call_TwoIntegerArgs args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
            return PyStatic.NewZero();
        }
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

        SystemEntity* pSE = pClient->SystemMgr()->GetSE(args.arg1);
        if (pSE == nullptr) {
            sLog.Error("DogmaIMBound::Handle_Activate()", "%u is not a valid EntityID in this system.", args.arg1);
            return PyStatic.NewZero();
        }
        // determine if this pSE is pos or cont.
        //call (de)activate on pSE, pass effectID, send effect to clients (bubblecast) then set timers.
        if (pSE->IsPOSSE()) {
            /*  these two may be called in posMgr...
            if ((args.arg2 == anchorDropForStructures)
            or (args.arg2 == anchorDropOrbital)) {
                pSE->GetPOSSE()->SetAnchor(args.arg2);
            } else */ if (args.arg2 == anchorLiftForStructures) {
                pSE->GetPOSSE()->PullAnchor();
            } else if (pSE->IsPlatformSE() and args.arg2 == anchorDrop) {
                GPoint pos = pSE->GetPosition();
                pSE->GetPOSSE()->SetAnchor(call.client, pos);
            } else if (pSE->IsPlatformSE() and args.arg2 == anchorLift) {
                pSE->GetPOSSE()->PullAnchor();
            } else if (args.arg2 == onlineForStructures) {
                pSE->GetPOSSE()->Activate(args.arg2);
            }
        } else if (pSE->IsContainerSE()) {
            // not sure what calls are for containers yet
            pSE->GetContSE()->Activate(args.arg2);
        } else {
            ; // make error here
        }
    } else if (call.tuple->size() == 4) {
        // activate ship module
        Call_Dogma_Activate args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
            return PyStatic.NewZero();
        }

        pClient->GetShip()->Activate(args.itemID, args.effectName, args.target, args.repeat);
    }
    // are there any other cases to test for here?

    // returns 1 on success (throw error otherwise)
    return PyStatic.NewOne();
}


PyResult DogmaIMBound::Handle_Deactivate(PyCallArgs& call)
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

    if (call.tuple->items.at(1)->IsInt()) {
        // if effect is integer, call is for pos or container
        call.Dump(POS__DUMP);
        Call_TwoIntegerArgs args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
            return PyStatic.NewNone();
        }
        SystemEntity* pSE = pClient->SystemMgr()->GetSE(args.arg1);
        if (pSE == nullptr) {
            sLog.Error("DogmaIMBound::Handle_Deactivate()", "%u is not a valid EntityID in this system.", args.arg1);
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
            pSE->GetPOSSE()->Deactivate(args.arg2);
        } else if (pSE->IsContainerSE()) {
            pSE->GetContSE()->Deactivate(args.arg2);
        } else {
            ; // make error here
        }
    } else if (call.tuple->items.at(1)->IsWString()) {
        //if effect is wide string, then call is for module, including calls to online/offline (rclick module in HUD)
        Call_Dogma_Deactivate args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
            return PyStatic.NewNone();
        }

        pClient->GetShip()->Deactivate(args.itemID, args.effectName);
    }
    // are there any other cases to test for?

    // returns None()
    return PyStatic.NewNone();
}

PyResult DogmaIMBound::Handle_Overload(PyCallArgs& call) {
    /*
     * 23:52:45 L DogmaIMBound::Handle_Overload(): size=2
     * 23:52:45 [SvcCallDump]   Call Arguments:
     * 23:52:45 [SvcCallDump]       Tuple: 2 elements
     * 23:52:45 [SvcCallDump]         [ 0] Integer field: 140002542
     * 23:52:45 [SvcCallDump]         [ 1] Integer field: 3035
     */
    // self.GetDogmaLM().Overload(itemID, effectID)

    Client* pClient(call.client);
    Call_TwoIntegerArgs args;   //itemID, effectID
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    //TODO:  need to verify pilot can OL modules
    // AttrRequiredThermoDynamicsSkill
    pClient->GetShip()->Overload(args.arg1);
    return nullptr;
}

// this one is called from Deactivate() when module is OL
PyResult DogmaIMBound::Handle_StopOverload(PyCallArgs& call)
{
    Client* pClient(call.client);
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    //  cancel overload then deactivate module
    pClient->GetShip()->CancelOverloading(args.arg1);
    pClient->GetShip()->Deactivate(args.arg1, sFxDataMgr.GetEffectName(args.arg2));
    // returns none
    return PyStatic.NewNone();
}

PyResult DogmaIMBound::Handle_CancelOverloading(PyCallArgs& call) {
    // self.dogmaLM.CancelOverloading(itemID)

    Client* pClient(call.client);
    Call_SingleIntegerArg args;   //itemID
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    pClient->GetShip()->CancelOverloading(args.arg);
    return nullptr;
}

PyResult DogmaIMBound::Handle_OverloadRack(PyCallArgs& call) {
    /* moduleIDs = self.GetDogmaLM().OverloadRack(itemID)
     *   moduleIDs is list of modules in rack.
     */
    Client* pClient(call.client);
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return new PyList();
    }

    // not supported yet
    PyList* list = new PyList();
    return list;
}

PyResult DogmaIMBound::Handle_StopOverloadRack(PyCallArgs& call) {
    /* moduleIDs = self.GetDogmaLM().StopOverloadRack(itemID)
     *   moduleIDs is list of modules in rack.
     */
    Client* pClient(call.client);
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return new PyList();
    }

    // not supported yet
    PyList* list = new PyList();
    return list;
}

PyResult DogmaIMBound::Handle_InitiateModuleRepair(PyCallArgs& call) {
    //  this is for repairing modules using nanite paste (button's ring turns white).  return bool.
    //  res = self.GetDogmaLM().InitiateModuleRepair(itemID)
    // see notes in ModuleManager::ModuleRepair()

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewFalse();
    }

    return call.client->GetShip()->ModuleRepair(args.arg);
}

PyResult DogmaIMBound::Handle_StopModuleRepair(PyCallArgs& call) {
    //  self.GetDogmaLM().StopModuleRepair(itemID)

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    call.client->GetShip()->StopModuleRepair(args.arg);

    // returns nothing
    return nullptr;
}

PyResult DogmaIMBound::Handle_ChangeDroneSettings(PyCallArgs& call) {
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

    if (!call.tuple->GetItem(0)->IsDict()) {
        codelog(DRONE__ERROR, "Tuple Item is wrong type: %s.  Expected PyDict.", call.tuple->GetItem(0)->TypeString());
        return nullptr;
    }

    PyDict* dict = call.tuple->GetItem(0)->AsDict();

    std::map<int16, int8> attribs;
    for (PyDict::const_iterator itr = dict->begin(); itr != dict->end(); ++itr)
        attribs[PyRep::IntegerValueU32(itr->first)] = PyRep::IntegerValue(itr->second);

    call.client->GetShipSE()->UpdateDrones(attribs);

    // returns nothing
    return nullptr;
}
