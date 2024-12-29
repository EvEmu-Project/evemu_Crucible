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
    Author:     Zhur
    Rewrite:    Allan
*/

#include "eve-server.h"




#include "EVEServerConfig.h"
#include "npc/Drone.h"
#include "planet/CustomsOffice.h"
#include "planet/Moon.h"
#include "pos/Structure.h"
#include "ship/ShipService.h"
#include "system/Container.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/sov/SovereigntyDataMgr.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "services/ServiceManager.h"

ShipService::ShipService(EVEServiceManager& mgr) :
    BindableService("ship", mgr)
{
}

/** @todo do we need more data here?  */
BoundDispatcher *ShipService::BindObject(Client *client, PyRep* bindParameters) {
    /*
     * 23:08:44 [ClientMsg] ShipService bind request
     * 23:08:44 [ClientMsg]      Tuple: 2 elements
     * 23:08:44 [ClientMsg]       [ 0]    Integer: 60014137     <<-- locationID
     * 23:08:44 [ClientMsg]       [ 1]    Integer: 15           <<-- location's groupID
     */
    _log(CLIENT__MESSAGE, "ShipService bind request");

    // this service should be bound by locationID and ship, but we'll use shipID for now...
    uint32 shipID = client->GetShip()->itemID();
    auto it = this->m_instances.find (shipID);

    if (it != this->m_instances.end ())
        return it->second;

    ShipBound* bound = new ShipBound(this->GetServiceManager(), *this, client->GetShip().get());

    this->m_instances.insert_or_assign (shipID, bound);

    return bound;
}

void ShipService::BoundReleased (ShipBound* bound) {
    auto it = this->m_instances.find (bound->GetShipID());

    if (it == this->m_instances.end ())
        return;

    this->m_instances.erase (it);
}
ShipBound::ShipBound (EVEServiceManager& mgr, ShipService& parent, ShipItem* ship) :
    EVEBoundObject(mgr, parent),
    pShip(ship)
{
    this->Add("Board", &ShipBound::Board);
    this->Add("Eject", &ShipBound::Eject);
    this->Add("LeaveShip", &ShipBound::LeaveShip);
    this->Add("ActivateShip", &ShipBound::ActivateShip);
    this->Add("Undock", &ShipBound::Undock);
    this->Add("AssembleShip", static_cast <PyResult (ShipBound::*)(PyCallArgs&, PyInt*)> (&ShipBound::AssembleShip));
    this->Add("AssembleShip", static_cast <PyResult(ShipBound::*)(PyCallArgs&, PyList*)> (&ShipBound::AssembleShip));
    this->Add("Drop", &ShipBound::Drop);
    this->Add("Scoop", &ShipBound::Scoop);
    this->Add("ScoopDrone", &ShipBound::ScoopDrone);
    this->Add("ScoopToSMA", &ShipBound::ScoopToSMA);
    this->Add("LaunchFromContainer", &ShipBound::LaunchFromContainer);
    this->Add("Jettison", &ShipBound::Jettison);
    this->Add("ConfigureShip", &ShipBound::ConfigureShip);
    this->Add("GetShipConfiguration", &ShipBound::GetShipConfiguration);
    this->Add("SelfDestruct", &ShipBound::SelfDestruct);
    this->Add("BoardStoredShip", &ShipBound::BoardStoredShip);
    this->Add("StoreVessel", &ShipBound::StoreVessel);
}

/* only called in space */
PyResult ShipBound::Board(PyCallArgs &call, PyInt* newShipID, std::optional<PyInt*> oldShipID) {
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change currently active.");
        return nullptr;
    }

    Client* pClient = call.client;

    ShipSE* pShipSE =  pClient->GetShipSE();
    if (pShipSE == nullptr)
        throw CustomError ("Invalid Ship.  Ref: ServerError xxxxx");
    /** @todo  check for active cyno (when we implement it...) and other things that affect eject */
    if (pShipSE->isGlobal()) { /* close enough.  cyno (isGlobal() = true), so this will work */
        /* find proper error msg for this...im sure there is one  */
        throw CustomError ("You cannot eject current ship with an active Cyno Field.");
    }

    //  do we need this? yes....this needs more work in destiny to implement correctly
    if (pShipSE->DestinyMgr()->GetSpeed() > 20)
        throw CustomError ("You cannot eject current ship while moving faster than 20m/s. Ref: ServerError 05139.");

    SystemManager* pSystem = pClient->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return nullptr;
    }

    // this will segfault if newShipID is invalid or not in system inventory
    pShipSE = pSystem->GetSE(newShipID->value())->GetShipSE();

    if (pShipSE == nullptr) {
        _log(SHIP__ERROR, "Handle_Board() - Failed to get new ship %u for %s.", newShipID->value(), pClient->GetName());
        throw CustomError ("Something bad happened as you prepared to board the ship.  Ref: ServerError 25107.");
    }

    if (pShipSE->GetTypeID() == itemTypeCapsule) {
        codelog(ITEM__ERROR, "Empty Pod %u in space.  SystemID %u.", newShipID->value(), pSystem->GetID());
        throw CustomError ("You already have a pod.  These cannot be boarded manally.");
    }

    //CantBoardTargeted

    //  do we need this? yes....this needs more work in destiny to implement correctly
    if (pShipSE->DestinyMgr()->GetSpeed() > 20)
        throw CustomError ("You cannot board the ship while it's moving faster than 20m/s. Ref: ServerError 05139.");

    // should we eject player here and deny boarding new ship, or just leave char in current ship and return?
    if (!pShipSE->GetShipItemRef()->ValidateBoardShip(pClient->GetChar()))
        throw CustomError ("You do not have the skills to fly a %s.", pShipSE->GetName());

    float distance = pClient->GetShipSE()->GetPosition().distance(pShipSE->GetPosition());
    // fudge for radii ?
    if (distance > sConfig.world.shipBoardDistance)
        throw CustomError ("You are too far from %s to board it.<br>You must be within %u meters to board this ship.",\
                pShipSE->GetName(), sConfig.world.shipBoardDistance);

    pClient->Board(pShipSE);

    /* return error msg from this call, if applicable (not sure how yet), else nodeid and timestamp */
    // returns nodeID and timestamp

    // HACK: WE'RE RETURNING BACK THE SAME BOUND SERVICE, IN REALITY A NEW BOUND INSTANCE SHOULD BE CREATED FOR THIS SHIP IN SPECIFIC
    //       INSTEAD OF REUSING THIS ONE, THIS WOULD HELP KEEP INFORMATION IN/OUT OF MEMORY BASED ON THE BOUND SERVICES
    return this->GetOID();
}

/* only called in space */
PyResult ShipBound::Eject(PyCallArgs &call) {
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change currently active.");
        return nullptr;
    }

    //no arguments.
    Client* pClient = call.client;
    /** @todo create and implement "Weapon Flag"....
     *      Weapon Flag --  the 60-sec timer started upon any offensive weapon activation
     *   this will be in client's criminaltimer object
     *
     * if (pClient->CrimeMgr()->IsWeaponFlagActive())
     *  deny eject
     */
    //{'FullPath': u'UI/Messages', 'messageID': 256625, 'label': u'NoEjectingToSpaceInStationBody'}(u"You can't eject into space while you're docked. Try leaving your ship the usual way.", None, None)

    SystemEntity* pShipSE = pClient->GetShipSE();
    if (pShipSE == nullptr)
        throw CustomError ("Invalid Ship.  Ref: ServerError xxxxx");
    /** @todo  check for active cyno (when we implement it...) and other things that affect eject */
    if (pShipSE->isGlobal()) { /* close enough.  cyno (isGlobal() = true), so this will work */
        /* find proper error msg for this...im sure there is one  */
        throw CustomError ("You cannot eject with an active Cyno Field.");
    }

    //  do we need this? yes....this needs more work in destiny to implement correctly
    if (pShipSE->DestinyMgr()->GetSpeed() > 20)
        throw CustomError ("You cannot eject current ship while moving faster than 20m/s. Ref: ServerError 05139.");

    pClient->Eject();

    /* return error msg from this call, if applicable (not sure how yet), else nodeid and timestamp */
    // returns nodeID and timestamp
    // 
    // HACK: WE'RE RETURNING BACK THE SAME BOUND SERVICE, IN REALITY A NEW BOUND INSTANCE SHOULD BE CREATED FOR THIS SHIP IN SPECIFIC
    //       INSTEAD OF REUSING THIS ONE, THIS WOULD HELP KEEP INFORMATION IN/OUT OF MEMORY BASED ON THE BOUND SERVICES
    return this->GetOID();
}

// NOTE  LeaveShip and ActivateShip are working.  dont fuck with them
/* only called when docked. */
PyResult ShipBound::LeaveShip(PyCallArgs &call, PyInt* shipID)
{
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change currently active.");
        return nullptr;
    }

    //  sends itemID of ship to leave
    Client* pClient = call.client;
    uint32 podID = pClient->GetPodID();
    ShipItemRef podRef = pClient->SystemMgr()->GetShipFromInventory(podID);
    if (podRef.get() == nullptr)
        podRef = sItemFactory.GetShipRef(podID);

    //verify owner (not sure why pod doesnt have correct owner...)
    podRef->ChangeOwner(pClient->GetCharacterID(), false);
    //move capsule into the players hangar
    podRef->Move(pClient->GetStationID(), flagHangar, true);

    // capsuleID = shipsvc.LeaveShip(shipid)
    return new PyInt(podID);
}

/* only called when docked. */
PyResult ShipBound::ActivateShip(PyCallArgs &call, PyInt* newShipID, std::optional<PyInt*> oldShipID) {
    //self.instanceCache, self.instanceFlagQuantityCache, self.wbData = self.remoteShipMgr.ActivateShip(shipID, oldShipID)

    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change currently active.");
        return nullptr;
    }
    Client* pClient = call.client;
    ShipItemRef newShipRef = sItemFactory.GetShipRef(newShipID->value());
    if (newShipRef.get() == nullptr) {
        sLog.Error("ShipBound::Handle_ActivateShip()", "%s: Failed to get new ship %u.", pClient->GetName(), newShipID->value());
        throw CustomError ("Something bad happened as you prepared to board the ship.  Ref: ServerError 15173+1");
    }
    //ShipMustBeInPersonalHangar

    pClient->BoardShip(newShipRef);

    // response should return ship modules, loaded charges, and linked weapons
    PyTuple* rsp = new PyTuple(3);
        rsp->SetItem(0, newShipRef->GetShipState());    //dict of ship modules
        rsp->SetItem(1, newShipRef->GetChargeState());    //dict of flagID/subLocation{loc, flag, typeID}
        rsp->SetItem(2, newShipRef->GetLinkedWeapons()); // dict of linked modules
    if (is_log_enabled(CLIENT__INFO))
        rsp->Dump(CLIENT__INFO, "    ");
    return rsp;
}

PyResult ShipBound::Undock(PyCallArgs &call, PyInt* shipID, PyBool* ignoreContraband) {
    //ShipIllegalTypeUndock

    /*  we could have some fun with these....
     * (258696, `Undock Delayed`)
     * (258702, `Undock Prohibited`)
     * (258700, `Undock Delayed`)
     * (258703, `Officials have closed the undocking ramps in {system} due to heavy congestion. Please try again later.`)
     * (258697, `{system} Traffic Control is currently offline and unable to process your undocking request. Please try again in a moment.`)
     */

    Client* pClient = call.client;
    ShipItemRef pShip = pClient->GetShip();
    if (pShip.get() == nullptr) {
        sLog.Error("ShipBound::Handle_ActivateShip()", "%s: Failed to get ship item.", pClient->GetName());
        throw CustomError ("Something bad happened as you prepared to board the ship.  Ref: ServerError 15173");
    }

    // nowhere near implementing ignoreContraband yet....

    //  get vector of online modules as (k,v) pair,
    //    where key is slotID, value is moduleID
    if (call.byname.find("onlineModules") != call.byname.end()) {
        PyDict* onlineModules = call.byname["onlineModules"]->AsDict();
        if (is_log_enabled(MODULE__INFO)) {
            _log(MODULE__INFO, "Dumping 'onlineModules' List");
            onlineModules->Dump(MODULE__INFO, "   ");
        }
        PyDict::const_iterator cur = onlineModules->begin(), end = onlineModules->end();
        for (; cur != end; ++cur)
            pShip->AddModuleToOnlineVec(cur->second->AsInt()->value());
    }

    pClient->UndockFromStation();

    // returns nodeID and timestamp
    // HACK: WE'RE RETURNING BACK THE SAME BOUND SERVICE, IN REALITY A NEW BOUND INSTANCE SHOULD BE CREATED FOR THIS SHIP IN SPECIFIC
    //       INSTEAD OF REUSING THIS ONE, THIS WOULD HELP KEEP INFORMATION IN/OUT OF MEMORY BASED ON THE BOUND SERVICES
    return this->GetOID();
}

PyResult ShipBound::Drop(PyCallArgs &call, PyList* PyToDropList, std::optional <PyInt*> oOwnerID, PyBool* ignoreWarning)
{
    _log(SHIP__INFO, "ShipBound::Handle_Drop()");
    call.Dump(SHIP__INFO);

    if (sDataMgr.IsStation(call.client->GetLocationID())) {
        _log(SERVICE__ERROR, "%s: Trying to drop items when not in space!", call.client->GetName());
        return nullptr;
    }

   uint32 ownerID = oOwnerID.has_value () ? oOwnerID.value()->value() : 1;  // not sent for LaunchDrone() command  (not needed)
    //used for LaunchUpgradePlatformWarning
    // args.ignoreWarning

    bool dropped = false, shipDrop = false;

    Client* pClient = call.client;
    SystemManager* pSystem = pClient->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return nullptr;
    }

    uint8 qty = 0;
    uint32 itemID = 0;
    double radius = pShip->radius();

    InventoryItemRef iRef(nullptr);
    PyDict* dict = new PyDict();
    for (uint32 i = 0; i < PyToDropList->size(); ++i) {
        dropped = false;
        PyList* list = new PyList();
        GPoint location(pShip->position());
        location.MakeRandomPointOnSphereLayer(500,1500);
        qty = PyToDropList->items.at(i)->AsTuple()->items.at(1)->AsInt()->value();
        itemID = PyToDropList->items.at(i)->AsTuple()->items.at(0)->AsInt()->value();
        iRef = sItemFactory.GetItemRef(itemID);
        if (iRef.get() == nullptr) {
            sLog.Error("ShipBound::Handle_Drop()", "%s: Unable to find item %u to drop.", pClient->GetName(), itemID);
            continue;
        }

        switch (iRef->categoryID()) {
            case EVEDB::invCategories::Drone: {
                if (!sConfig.testing.EnableDrones) {
                    throw CustomError ("Drones are disabled.");
                }

                if (pClient->GetChar()->GetAttribute(AttrMaxActiveDrones).get_uint32() < 1) {
                    throw UserError ("NoDroneManagementAbilities")
                            .AddFormatValue ("typeID", new PyInt (iRef->typeID ()));
                    //{'FullPath': u'UI/Messages', 'messageID': 259203, 'label': u'NoDroneManagementAbilitiesBody'}(u'You cannot launch {[item]typeID.nameWithArticle} because you do not have the ability to control any drones.', None, {u'{[item]typeID.nameWithArticle}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'nameWithArticle', 'args': 0, 'kwargs': {}, 'variableName': 'typeID'}})
                }
                if (pClient->GetChar()->GetAttribute(AttrMaxActiveDrones).get_uint32() <= pClient->GetShipSE()->DroneCount()) {
                    throw UserError ("NoDroneManagementAbilitiesLeft")
                            .AddFormatValue ("item", new PyInt (iRef->typeID ()))
                            .AddFormatValue ("limit", new PyInt (pClient->GetChar ()->GetAttribute (AttrMaxActiveDrones).get_uint32()));
                    //{'FullPath': u'UI/Messages', 'messageID': 259140, 'label': u'NoDroneManagementAbilitiesLeftBody'}(u'You cannot launch {[item]item.name} because you are already controlling {[numeric]limit} drones, as much as you have skill to.', None, {u'{[numeric]limit}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'limit'}, u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}})
                }

                if (iRef->flag() != flagDroneBay) {
                    throw UserError ("DropItemNotInDroneBay")
                            .AddFormatValue ("item", new PyInt (iRef->typeID ()));
                    // {'FullPath': u'UI/Messages', 'messageID': 259680, 'label': u'DropItemNotInDroneBayBody'}(u'{[item]item.name} cannot be dropped because it is not in your drone bay.', None, {u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}})
                }

                // This item is a drone, so launch it into space:
                if (qty > 1) {
                    for (uint8 i = 0; i < qty; ++i) {
                        InventoryItemRef newItem = iRef->Split(1);
                        if (newItem.get() == nullptr) {
                            _log(INV__ERROR, "ShipBound::Handle_Drop() - Error splitting item %u. Skipping.", iRef->itemID());
                            continue;
                        }
                        if (newItem->quantity() > 1)
                            _log(INV__ERROR, "ShipBound::Handle_Drop() - Split item %u qty > 1 (%u).  Continuing.", newItem->itemID(), newItem->quantity());

                        if (pClient->GetShipSE()->LaunchDrone(newItem)) {
                            dropped = true;
                            shipDrop = true;
                            list->AddItem(new PyInt(newItem->itemID()));
                        } else
                            throw UserError ("MaxBandwithExceeded2")
                                    .AddTypeName ("droneNAme", newItem->typeID ())
                                    .AddAmount ("droneBandwithUsed", newItem->GetAttribute (AttrDroneBandwidthUsed).get_uint32())
                                    .AddAmount ("bandwidthLeft", pShip->GetAttribute (AttrDroneBandwidth).get_uint32 () - pShip->GetAttribute (AttrDroneBandwidthLoad).get_uint32());
                    }
                } else {
                    if (pClient->GetShipSE()->LaunchDrone(iRef)) {
                        dropped = true;
                        shipDrop = true;
                        list->AddItem(new PyInt(iRef->itemID()));
                    } else
                            throw UserError ("MaxBandwithExceeded2")
                                    .AddTypeName ("droneNAme", iRef->typeID ())
                                    .AddAmount ("droneBandwithUsed", iRef->GetAttribute (AttrDroneBandwidthUsed).get_uint32())
                                    .AddAmount ("bandwidthLeft", pShip->GetAttribute (AttrDroneBandwidth).get_uint32 () - pShip->GetAttribute (AttrDroneBandwidthLoad).get_uint32());
                }
            } break;
            case EVEDB::invCategories::Structure: {
                // test for existing tower
                if (iRef->groupID() == EVEDB::invGroups::Control_Tower) {
                    if (pClient->SystemMgr()->GetClosestMoonSE(location)->GetMoonSE()->HasTower()) {
                        pClient->SendErrorMsg("This Moon already has a Control Tower in orbit.  Aborting Drop.");
                        return nullptr;
                    }
                } else {
                    if (!pClient->SystemMgr()->GetClosestMoonSE(location)->GetMoonSE()->HasTower()) {
                        pClient->SendErrorMsg("You need an anchored Control Tower before launching modules.  Aborting Drop.");
                        return nullptr;
                    }
                }
                /** @todo implement these checks  (may be more i havent found yet) */
                //{'FullPath': u'UI/Messages', 'messageID': 259679, 'label': u'DropNeedsPlayerCorpBody'}(u'In order to launch {[item]item.name} you need to be a member of a independent corporation.', None, {u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}})
                //{'FullPath': u'UI/Messages', 'messageID': 256411, 'label': u'CantInHighSecSpaceBody'}(u'You cannot do that as CONCORD currently restricts the launching, anchoring and control of that type of structure within CONCORD-protected space to authorized agents of the empires.', None, None)

                DBSystemDynamicEntity entity = DBSystemDynamicEntity();
                entity.ownerID = ownerID;
                entity.factionID = pClient->GetWarFactionID();
                entity.allianceID = pClient->GetAllianceID();
                entity.corporationID = pClient->GetCorporationID();
                entity.itemID = iRef->itemID();
                entity.itemName = iRef->itemName();
                entity.typeID = iRef->typeID();
                entity.groupID = iRef->groupID();
                entity.categoryID = iRef->categoryID();

                // Move item from cargo bay to space: (and send OnItemChange packet)
                iRef->Move(pClient->GetLocationID(), flagNone, true);
                iRef->SetPosition(location + iRef->radius() + radius);
                iRef->ChangeOwner(entity.ownerID);

                entity.position = iRef->position();

                if (entity.groupID == EVEDB::invGroups::Orbital_Infrastructure)
                    entity.planetID = pSystem->GetClosestPlanetID(location);

                SystemEntity* pSE = DynamicEntityFactory::BuildEntity(*pSystem, entity);
                if (pSE == nullptr) {
                    //couldnt create entity.  move item back to orig location and continue
                    iRef->Donate(pClient->GetCharacterID(), pShip->itemID(), flagCargoHold);
                    continue;
                }

                // item was successfully created.  set singleton
                iRef->ChangeSingleton(true);

                dropped = true;
                shipDrop = true;
                pSE->GetPOSSE()->Drop(pClient->GetShipSE()->SysBubble());
                pSystem->AddEntity(pSE);
                list->AddItem(new PyInt(entity.itemID));
            } break;
            case EVEDB::invCategories::Deployable: {
                pClient->SendNotifyMsg("Launching Deployables isnt available yet.");
            } break;
            case EVEDB::invCategories::SovereigntyStructure: {
                //Code for spawning sovereignty structures

                //Check if system is in empire space
                SystemData sysData;
                sDataMgr.GetSystemData(pClient->GetSystemID(), sysData);
                if (sysData.factionID) {
                    pClient->SendErrorMsg("Launching sovereignty structures is forbidden in empire space.");
                    return nullptr;
                }

                //Check if client is part of an alliance
                if (pClient->GetAllianceID() == 0) {
                    pClient->SendErrorMsg("You must be part of an alliance to launch a sovereignty structure.");
                    return nullptr;
                }

                switch (iRef->groupID()) {
                    case EVEDB::invGroups::Sovereignty_Blockade_Units: {
                        // Make sure SBU is deployed in the same bubble as a gate
                        std::vector<uint16> gateBubbles;
                        for (auto cur: pSystem->GetOperationalStatics()) {
                            if (cur.second->IsGateSE())
                            {
                                gateBubbles.push_back(cur.second->SysBubble()->GetID());
                            }
                        }
                        if (!(std::find(gateBubbles.begin(), gateBubbles.end(),pClient->GetShipSE()->SysBubble()->GetID())!=gateBubbles.end()))
                        {
                            pClient->SendErrorMsg("Sovereignty blockade units must be deployed near a stargate.");
                            return nullptr;
                        }

                        // Check if this system is currently claimed
                        if (svDataMgr.GetSystemAllianceID(pClient->GetSystemID()) == 0 ) {
                            pClient->SendErrorMsg("You cannot launch a Sovereignty Blockade Unit in an unclaimed system.");
                            return nullptr;
                        }

                        // Check if this system is claimed by your alliance
                        if (svDataMgr.GetSystemAllianceID(pClient->GetSystemID()) == pClient->GetAllianceID()) {
                            pClient->SendErrorMsg("You cannot launch a Sovereignty Blockade Unit in a system claimed by your alliance");
                            return nullptr;
                        }

                        // Check if there is already an SBU on this stargate
                        SystemEntity* pSE = pSystem->GetClosestGateSE(pClient->GetShipSE()->GetPosition());
                        if (pSE->GetGateSE()->HasSBU()) {
                            pClient->SendErrorMsg("There is already a Sovereignty Blockade Unit on this stargate.  Aborting Drop.");
                            return nullptr;
                        }

                        // Check if there is already an unanchored SBU in the current bubble
                        for (auto cur: pSystem->GetOperationalStatics()) {
                            if (cur.second->IsSBUSE())
                            {
                                if (cur.second->SysBubble()->GetID() == pClient->GetShipSE()->SysBubble()->GetID())
                                {
                                    pClient->SendErrorMsg("There is already a Sovereignty Blockade Unit on this stargate.  Aborting Drop.");
                                    return nullptr;
                                }
                            }
                        }
                    } break;
                    //verify only one TCU or IHub is deployed in the system.
                    // todo:  check Structure.h  - these can be dropped and anchored, but not onlined if >1 in system
                    case EVEDB::invGroups::Territorial_Claim_Units: {
                        // Only one can be launched in a system
                        for (auto cur: pSystem->GetOperationalStatics()) {
                            if (cur.second->IsTCUSE())
                            {
                                pClient->SendErrorMsg("There is already a Territorial Claim Unit in this system.  Aborting Drop.");
                                return nullptr;
                            }
                        }

                        //This should never hit as there should always be a TCU in a claimed system
                        if (svDataMgr.GetSystemAllianceID(pClient->GetSystemID()) != 0 ) {
                            pClient->SendErrorMsg("This system has already been claimed. ");
                            return nullptr;
                        }
                    } break;
                    case EVEDB::invGroups::Infrastructure_Hubs: {
                        // Only one can be launched in a system
                        for (auto cur: pSystem->GetOperationalStatics()) {
                            if (cur.second->IsIHubSE())
                            {
                                pClient->SendErrorMsg("There is already an Infrastructure Hub this system.  Aborting Drop.");
                                return nullptr;
                            }
                        }

                        // Check if this system is not claimed by your alliance
                        if (svDataMgr.GetSystemAllianceID(pClient->GetSystemID()) != uint32(pClient->GetAllianceID())) {
                            pClient->SendErrorMsg("You cannot launch an Infrastructure Hub in a system not claimed by your alliance.");
                            return nullptr;
                        }
                    } break;
                }

                DBSystemDynamicEntity entity = DBSystemDynamicEntity();
                entity.ownerID = ownerID;
                entity.factionID = pClient->GetWarFactionID();
                entity.allianceID = pClient->GetAllianceID();
                entity.corporationID = pClient->GetCorporationID();
                entity.itemID = iRef->itemID();
                entity.itemName = iRef->itemName();
                entity.typeID = iRef->typeID();
                entity.groupID = iRef->groupID();
                entity.categoryID = iRef->categoryID();

                // Move item from cargo bay to space: (and send OnItemChange packet)
                iRef->Move(pClient->GetLocationID(), flagNone, true);
                iRef->SetPosition(location + iRef->radius() + radius);
                iRef->ChangeOwner(entity.ownerID);

                entity.position = iRef->position();

                SystemEntity* pSE = DynamicEntityFactory::BuildEntity(*pSystem, entity);
                if (pSE == nullptr) {
                    //couldnt create entity.  move item back to orig location and continue
                    iRef->Donate(pClient->GetCharacterID(), pShip->itemID(), flagCargoHold);
                    continue;
                }

                // item was successfully created.  set singleton
                iRef->ChangeSingleton(true);

                dropped = true;
                shipDrop = true;
                pSE->GetPOSSE()->Drop(pClient->GetShipSE()->SysBubble());
                pSystem->AddEntity(pSE);
                list->AddItem(new PyInt(entity.itemID));
            } break;
            case EVEDB::invCategories::Celestial: { //Outpost construction platforms
                if (iRef->groupID() == EVEDB::invGroups::Construction_Platform) {
                    // Get current planet to anchor platform on
                    uint32 planetID = pSystem->GetClosestPlanetID(pClient->GetShipSE()->GetPosition());

                    // Checks to see if we can deploy the platform
                    // Our alliance must have sovereignty in the system to deploy an outpost
                    if (svDataMgr.GetSystemAllianceID(pClient->GetSystemID()) != pClient->GetAllianceID() ) {
                        pClient->SendErrorMsg("You cannot launch Construction Platforms in a system which your alliance does not control. ");
                        return nullptr;
                    }

                    // Platforms must be deployed on a planet
                    for (auto cur: pSystem->GetOperationalStatics())
                        if (cur.second->IsPlatformSE())
                            if (cur.second->GetPOSSE()->GetPlanetID() ==  planetID) {
                                pClient->SendErrorMsg("There is already a platform deployed at this planet.");
                                return nullptr;
                            }

                    DBSystemDynamicEntity entity = DBSystemDynamicEntity();
                    entity.ownerID = ownerID;
                    entity.factionID = pClient->GetWarFactionID();
                    entity.allianceID = pClient->GetAllianceID();
                    entity.corporationID = pClient->GetCorporationID();
                    entity.itemID = iRef->itemID();
                    entity.itemName = iRef->itemName();
                    entity.typeID = iRef->typeID();
                    entity.groupID = iRef->groupID();
                    entity.categoryID = iRef->categoryID();

                    // Move item from cargo bay to space: (and send OnItemChange packet)
                    iRef->Move(pClient->GetLocationID(), flagNone, true);
                    iRef->SetPosition(location + iRef->radius() + radius);
                    iRef->ChangeOwner(entity.ownerID);

                    entity.position = iRef->position();

                    SystemEntity* pSE = DynamicEntityFactory::BuildEntity(*pSystem, entity);
                    if (pSE == nullptr) {
                        //couldnt create entity.  move item back to orig location and continue
                        iRef->Donate(pClient->GetCharacterID(), pShip->itemID(), flagCargoHold);
                        continue;
                    }

                    // item was successfully created.  set singleton
                    iRef->ChangeSingleton(true);

                    dropped = true;
                    shipDrop = true;
                    pSE->GetPOSSE()->Drop(pClient->GetShipSE()->SysBubble());
                    pSystem->AddEntity(pSE);
                    list->AddItem(new PyInt(entity.itemID));
                }
                if (iRef->groupID() == EVEDB::invGroups::Secure_Cargo_Container or
                    EVEDB::invGroups::Secure_Cargo_Container or
                    EVEDB::invGroups::Cargo_Container or
                    EVEDB::invGroups::Freight_Container or
                    EVEDB::invGroups::Audit_Log_Secure_Container or
                    EVEDB::invGroups::Mission_Container) 
                {
                    DBSystemDynamicEntity entity = DBSystemDynamicEntity();
                    entity.ownerID = ownerID;
                    entity.factionID = pClient->GetWarFactionID();
                    entity.allianceID = pClient->GetAllianceID();
                    entity.corporationID = pClient->GetCorporationID();
                    entity.itemID = iRef->itemID();
                    entity.itemName = iRef->itemName();
                    entity.typeID = iRef->typeID();
                    entity.groupID = iRef->groupID();
                    entity.categoryID = iRef->categoryID();

                    // Move item from cargo bay to space: (and send OnItemChange packet)
                    iRef->Move(pClient->GetLocationID(), flagNone, true);
                    iRef->SetPosition(location + iRef->radius() + radius);
                    iRef->ChangeOwner(entity.ownerID);

                    entity.position = iRef->position();

                    SystemEntity* pSE = DynamicEntityFactory::BuildEntity(*pSystem, entity);
                    if (pSE == nullptr) {
                        //couldnt create entity.  move item back to orig location and continue
                        iRef->Donate(pClient->GetCharacterID(), pShip->itemID(), flagCargoHold);
                        continue;
                    }

                    // item was successfully created.  set singleton
                    iRef->ChangeSingleton(true);

                    dropped = true;
                    shipDrop = true;
                    pSE->GetPOSSE()->Drop(pClient->GetShipSE()->SysBubble());
                    pSystem->AddEntity(pSE);
                    list->AddItem(new PyInt(entity.itemID));
                }
            } break;
            default: {
                _log(INV__ERROR, "ShipBound::Handle_Drop() - Item %s (cat %u) is neither drone, structure or deployable.", iRef->name(), iRef->categoryID());
            }
        }

    // returns dict of dropped items as {itemID, data} where data is list of itemIDs dropped (split stack if applicable)
    // however, on non-throw error, data is tuple of errID, errDetailsType, errDetails (unknown where these are defined)

        if (dropped) {
            dict->SetItem(new PyInt(iRef->itemID()), list);
        } else {
            PyTuple* err = new PyTuple(3);
            err->SetItem(0, new PyInt(1));
            err->SetItem(1, new PyString("unsure"));
            err->SetItem(2, new PyString("misc error"));
            dict->SetItem(new PyInt(iRef->itemID()), err);
        }
    }

    //  missing launch error throw msgs.... LaunchCPWarning, LaunchUpgradePlatformWarning
    //   - these are thrown before item is launched and will negate entire launch group until warning is approved, then entire group is reprocessed

    if (shipDrop)
        pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();

    return dict;
}

PyResult ShipBound::Scoop(PyCallArgs &call, PyInt* itemID) {
    Client* pClient(call.client);
    SystemManager* pSysMgr(pClient->SystemMgr());
    if (pSysMgr == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager.", pClient->GetName());
        return PyStatic.mtDict();
    }
    SystemEntity* pSE = pSysMgr->GetSE(itemID->value());
    if (pSE == nullptr) {
        _log(SERVICE__ERROR, "%s: Unable to find object %i to scoop.", pClient->GetName(), itemID->value());
        return PyStatic.mtDict();
        //{'FullPath': u'UI/Messages', 'messageID': 258825, 'label': u'ScoopObjectGoneBody'}(u'{target} is no longer there.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
    }

    //CannotScoopTargetedShip
    //CantScoopAnchoring
    //CantScoopOwnerNpc
    //CantScoopThat
    //ScoopObjectGone
    //ShpScoopSecureCC
    //CanOnlyScoopIfUnanchored

    // check to see if this object is anchored and if so, refuse to scoop it
    if (pSE->IsContainerSE())
        if (pSE->GetContSE()->IsAnchored())
            throw CustomError ("%s is anchored.  Cannot scoop.", pSE->GetName());

    // check drones for other pilots control
    if (pSE->IsDroneSE())
        if (pSE->GetDroneSE()->IsEnabled())
            if (pSE->GetDroneSE()->GetOwner() != pClient)
                throw CustomError ("%s is under another pilot's control.  Cannot scoop.", pSE->GetName());

    InventoryItemRef iRef = pSE->GetSelf();
    if (iRef.get() == nullptr) {
        codelog(CLIENT__ERROR, "ItemRef for %i not found.", itemID->value());
        return PyStatic.mtDict();
    }

    // Check cargo bay capacity:
    if (pClient->GetShip()->GetMyInventory()->ValidateAddItem(flagCargoHold, iRef)) {  // this will throw if it fails
        // We have enough Cargo bay capacity to hold the item being scooped,
        // so take ownership of it
        iRef->ChangeOwner(pClient->GetCharacterID(), true);
        // perform data cleanup for structures
        if (pSE->IsPOSSE())
            pSE->GetPOSSE()->Scoop();
        // perform data cleanup for drones
        if (pSE->IsDroneSE())
            pClient->GetShipSE()->ScoopDrone(pSE);
        // move it into the cargo bay:
        pClient->MoveItem(iRef->itemID(), pClient->GetShipID(), flagCargoHold);
        pSysMgr->RemoveEntity(pSE);
        // delete SE since item is no longer in space
        SafeDelete(pSE);
    }

    return PyStatic.mtDict();
}

PyResult ShipBound::ScoopDrone(PyCallArgs &call, PyList* itemIDs) {
    std::vector <int32> ints;

    PyList::const_iterator list_2_cur = itemIDs->begin();
    for (size_t list_2_index(0); list_2_cur != itemIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_SingleIntList failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        ints.push_back(t->value());
    }
    // per patch notes, if ship is too far to scoop, it will automagically travel closer till drone is within range, then scoop and stop

    Client* pClient(call.client);
    SystemEntity* pDroneSE(nullptr);
    InventoryItemRef iRef(nullptr);
    SystemManager* pSysMgr(pClient->SystemMgr());
    std::vector<int32>::const_iterator cur = ints.begin();
    for(; cur != ints.end(); ++cur) {
        pDroneSE = pSysMgr->GetSE(*cur);
        if (pDroneSE == nullptr) {
            _log(SERVICE__ERROR, "%s: Unable to find droneSE %i to scoop.", pClient->GetName(), *cur);
            continue;
        }

        iRef = pDroneSE->GetSelf();
        if (iRef.get() == nullptr) {
            _log(SERVICE__ERROR, "%s: Unable to find droneItem %i to scoop.", pClient->GetName(), *cur);
            continue;
        }

        //AttrDroneBaySlotsLeft??
        // Check to see that this is really a drone:
        pClient->GetShip()->VerifyHoldType(flagDroneBay, iRef, pClient);

        // check ownership/control
        if (pDroneSE->GetDroneSE()->IsEnabled())
            if (pDroneSE->GetDroneSE()->GetOwner() != pClient)
                throw CustomError ("The %s is under another pilot's control.  Cannot scoop.", pDroneSE->GetName());

        // Check drone bay capacity:
        if (pClient->GetShip()->GetMyInventory()->ValidateAddItem(flagDroneBay, iRef)) {  // this will throw if it fails
            // We have enough Drone bay capacity to hold the drone,
            // so take ownership of it and move it into the Drone bay:
            iRef->ChangeOwner(pClient->GetCharacterID(), true);
            pClient->MoveItem(iRef->itemID(), pClient->GetShipID(), flagDroneBay);
            pClient->GetShipSE()->ScoopDrone(pDroneSE);
            pSysMgr->RemoveEntity(pDroneSE);
            SafeDelete(pDroneSE);
        }
    }

    /** @todo complete error msgs here */
    // returns error on error else mtDict
    return PyStatic.mtDict();
}

PyResult ShipBound::Jettison(PyCallArgs &call, PyList* itemIDs) {
    std::vector <int32> ints;

    PyList::const_iterator list_2_cur = itemIDs->begin();
    for (size_t list_2_index(0); list_2_cur != itemIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_SingleIntList failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        ints.push_back(t->value());
    }
    Client* pClient(call.client);
    if (!pClient->IsInSpace()) {
        _log(SERVICE__ERROR, "%s: Trying to jettison items when not in space!", pClient->GetName());
        return nullptr;
    }

    SystemManager* pSysMgr(pClient->SystemMgr());
    //Get location of our ship
    GPoint location(pClient->GetShipSE()->GetPosition());

    InventoryItemRef cRef(nullptr), iRef(nullptr);
    CargoContainerRef jcRef(nullptr), ccRef(nullptr);
    StructureItemRef sRef(nullptr);

    /** @todo  deal with launching items for corp... they will use flagProperty */

    FactionData data = FactionData();
        data.allianceID = pClient->GetAllianceID();
        data.corporationID = pClient->GetCorporationID();
        data.factionID = pClient->GetWarFactionID();
        /** @todo  determine if this is char or corp here */
        data.ownerID = pClient->GetCharacterID();

    //args contains id's of items to jettison
    std::vector<int32>::iterator itr = ints.begin();
    // loop thru items to see if there is a container in this list.
    for (; itr != ints.end(); ++itr) {
        // running this list twice is fuckedup, but not sure of another way to determine if container is in jettison list.
        iRef = sItemFactory.GetItemRef(*itr);
        if (iRef.get() == nullptr)
            continue;
        switch (iRef->categoryID()) {
            case EVEDB::invCategories::Structure:
            case EVEDB::invCategories::SovereigntyStructure:
            case EVEDB::invCategories::StructureUpgrade: {
                sRef = sItemFactory.GetStructureRef(*itr);
                if (sRef.get() == nullptr)
                    throw CustomError ("Unable to spawn Structure item of type %u.", sRef->typeID());

                sRef->Move(pClient->GetLocationID(), flagNone, true);
                StructureSE* sSE = new StructureSE(sRef, this->GetServiceManager(), pSysMgr, data);
                location.MakeRandomPointOnSphere(1500.0 + sRef->type().radius());
                sSE->SetPosition(location);
                sRef->SaveItem();
                pSysMgr->AddEntity(sSE);
                pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();
                itr = ints.erase(itr);
            } break;
            case EVEDB::invCategories::Orbitals: {
                sRef = sItemFactory.GetStructureRef(*itr);
                if (sRef.get() == nullptr)
                    throw CustomError ("Unable to spawn Structure item of type %u.", sRef->typeID());

                sRef->Move(pClient->GetLocationID(), flagNone, true);
                CustomsSE* sSE = new CustomsSE(sRef, this->GetServiceManager(), pSysMgr, data);
                location.MakeRandomPointOnSphere(1500.0 + sRef->type().radius());
                sSE->SetPosition(location);
                sRef->SaveItem();
                pSysMgr->AddEntity(sSE);
                pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();
                itr = ints.erase(itr);
            } break;
            case EVEDB::invCategories::Deployable: {
                cRef = sItemFactory.GetItemRef(*itr);
                if (cRef.get() == nullptr)
                    throw CustomError ("Unable to spawn Deployable item of type %u.", cRef->typeID());

                cRef->Move(pClient->GetLocationID(), flagNone, true);
                //flagUnanchored: for some DUMB reason, this flag, 1023 yields a PyNone when notifications
                // are created inside InventoryItem::Move() from passing it into a PyInt() constructor...WTF?
                DeployableSE* dSE = new DeployableSE(cRef, this->GetServiceManager(), pSysMgr, data);
                location.MakeRandomPointOnSphere(1500.0 + cRef->type().radius());
                dSE->SetPosition(location);
                cRef->SaveItem();
                pSysMgr->AddEntity(dSE);
                pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();
                itr = ints.erase(itr);
            } break;
            /** @todo  Handle other cargo */

            // test for cargo container being launched
            case EVEDB::invCategories::Celestial: {
                switch (iRef->groupID()) {
                    case EVEDB::invGroups::Cargo_Container:
                    case EVEDB::invGroups::Audit_Log_Secure_Container:
                    case EVEDB::invGroups::Secure_Cargo_Container:
                    case EVEDB::invGroups::Freight_Container: {
                        /** @todo (allan)  check these for accuracy  */
                        /** @todo (allan)  *****  there are stipulations on placement of these items.  *****  */
                        ccRef = sItemFactory.GetCargoRef(*itr);
                        if (ccRef.get() == nullptr)
                            throw CustomError ("Unable to spawn item of type %u.", ccRef->typeID());

                        ccRef->Move(pClient->GetLocationID(), flagNone, true);
                        ContainerSE* cSE = new ContainerSE(ccRef, this->GetServiceManager(), pSysMgr, data);
                        cSE->Init();
                        location.MakeRandomPointOnSphere(500.0);
                        cSE->SetPosition(location);
                        ccRef->SaveItem();
                        pSysMgr->AddEntity(cSE);
                        pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();

                        // container found.  remove this item from list, then break out of here and use to contain all other non-pos items
                        ints.erase(itr);
                        itr = ints.end();
                    } break;
                    case EVEDB::invGroups::Construction_Platform:
                    case EVEDB::invGroups::Mobile_Sentry_Gun:
                    case EVEDB::invGroups::Global_Warp_Disruptor:
                    case EVEDB::invGroups::Station_Upgrade_Platform:
                    case EVEDB::invGroups::Station_Improvement_Platform:
                    case EVEDB::invGroups::Covert_Beacon:
                    case EVEDB::invGroups::Covert_Cynosural_Field_Generator: {
                        //these should need corp shit for use, i think.
                        sLog.Warning("Ship::Jettison", "%s: %s called to jettison.",pClient->GetName(), iRef->name());
                    } break;
                    case EVEDB::invGroups::Shipping_Crates: {
                        // not sure what this is, can it be jettisoned?
                        sLog.Error("Ship::Jettison", "%s: %s called to jettison.",pClient->GetName(), iRef->name());
                    } break;
                }
            }
        }
    }

    // container check complete, loop thru list for other items
    for (auto cur : ints) {
        iRef = sItemFactory.GetItemRef(cur);
        if (iRef.get() == nullptr)
            continue;

        // item can be jettisoned.  check if container was already created
        if ((ccRef.get() == nullptr) and (jcRef.get() == nullptr)) {
            if (!pClient->IsJetcanAvalible()) {
                throw UserError ("ShpJettisonPending")
                    .AddTimeShort ("eta", pClient->JetcanTime() * EvE::Time::Second);
            }
            // Spawn jetcan then continue loop
            location.MakeRandomPointOnSphere(500.0f);
            ItemData p_idata(
                            23,                         // 23 = cargo container
                            pClient->GetCharacterID(),  //owner is Character?  figure out how to test for corp owner
                            pClient->GetLocationID(),
                            flagNone,
                            "Jettisoned Cargo Container",
                            location);

            jcRef = sItemFactory.SpawnCargoContainer(p_idata);
            if (jcRef.get() == nullptr)
                throw CustomError ("Unable to spawn jetcan.");
            // create new container
            ContainerSE* cSE = new ContainerSE(jcRef, this->GetServiceManager(), pSysMgr, data);
            cSE->Init();

            jcRef->SetMySE(cSE);
            // set anchored to avoid deletion when empty
            jcRef->SetAnchor(true);
            pSysMgr->AddEntity(cSE);
            pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();
            pClient->StartJetcanTimer();
        }
        // check current can for capacity limits.
        //if over limit create new can?  reject remaining cargo?  delete?  crash?  run thru station naked?
        if (ccRef.get() != nullptr) {
            if (ccRef->GetMyInventory()->HasAvailableSpace(flagNone, iRef)) {
                pClient->MoveItem(cur, ccRef->itemID(), flagNone);
            } else {
                // extra step, try to move as much items as possible, this needs a new item creation tho
                float remainingCapacity = ccRef->GetMyInventory ()->GetRemainingCapacity (flagNone);
                int32 maximumAmountOfItems = (int32) floor(remainingCapacity / iRef->GetAttribute (AttrVolume).get_float ());

                ItemData newItem(iRef->typeID(), iRef->ownerID(), ccRef->itemID(), flagNone, maximumAmountOfItems);
                ccRef->AddItem(sItemFactory.SpawnItem(newItem));

                iRef->AlterQuantity(-maximumAmountOfItems, true);
                _log(ITEM__WARNING, "%s: CargoContainer %u is full.", pClient->GetName(), ccRef->itemID());
                throw UserError ("NotAllItemsWereMoved");
            }
        } else if (jcRef.get() != nullptr) {
            if (jcRef->GetMyInventory()->HasAvailableSpace(flagNone, iRef)) {
                pClient->MoveItem(cur, jcRef->itemID(), flagNone);
            } else {
                // extra step, try to move as much items as possible, this needs a new item creation tho
                float remainingCapacity = jcRef->GetMyInventory ()->GetRemainingCapacity (flagNone);
                int32 maximumAmountOfItems = (int32) floor(remainingCapacity / iRef->GetAttribute (AttrVolume).get_float ());

                ItemData newItem(iRef->typeID(), iRef->ownerID(), jcRef->itemID(), flagNone, maximumAmountOfItems);
                jcRef->AddItem(sItemFactory.SpawnItem(newItem));

                iRef->AlterQuantity(-maximumAmountOfItems, true);

                _log(ITEM__WARNING, "%s: Jetcan %u is full.", pClient->GetName(), jcRef->itemID());
                throw UserError ("NotAllItemsWereMoved");
            }
            //jetcan should spawn unanchored
            jcRef->SetAnchor(false);
            continue;
        } else {
            _log(ITEM__ERROR, "Jettison call for %s - no CC or Jcan.", pClient->GetName());
            throw CustomError ("Item container not found.", cRef->typeID());
        }
        continue;
    }

    // HACK: WE'RE RETURNING BACK THE SAME BOUND SERVICE, IN REALITY A NEW BOUND INSTANCE SHOULD BE CREATED FOR THIS SHIP IN SPECIFIC
    //       INSTEAD OF REUSING THIS ONE, THIS WOULD HELP KEEP INFORMATION IN/OUT OF MEMORY BASED ON THE BOUND SERVICES
    return this->GetOID();
}

PyResult ShipBound::AssembleShip(PyCallArgs& args, PyInt* shipID) {
    // this one has subSystems as a byname argument, handled by the AssembleShip call with a PyList for now
    PyList* list = new PyList();

    list->AddItem(shipID);

    return this->AssembleShip(args, list);
}

PyResult ShipBound::AssembleShip(PyCallArgs &call, PyList* itemIDs) {
    /* 13:05:41 [BindDump] NodeID: 888444 BindID: 129 calling AssembleShip in service manager 'ShipBound'
     * 13:05:41 [BindDump]   Call Arguments:
     * 13:05:41 [BindDump]       Tuple: 1 elements
     * 13:05:41 [BindDump]         [ 0] List: 5 elements
     * 13:05:41 [BindDump]         [ 0]   [ 0] Integer field: 140000073
     * 13:05:41 [BindDump]         [ 0]   [ 1] Integer field: 140000074
     * 13:05:41 [BindDump]         [ 0]   [ 2] Integer field: 140000075
     * 13:05:41 [BindDump]         [ 0]   [ 3] Integer field: 140000076
     * 13:05:41 [BindDump]         [ 0]   [ 4] Integer field: 140000077
     *
     *              [PyTuple 2 items]     << response to AssembleShip call
     *                [PyList 1 items]
     *                  [PyPackedRow 33 bytes]
     *                    ["itemID" => <1002333477860> [I8]]
     *                    ["typeID" => <24700> [I4]]
     *                    ["ownerID" => <1661059544> [I4]]
     *                    ["locationID" => <61000012> [I8]]
     *                    ["flagID" => <4> [I2]]
     *                    ["quantity" => <-1> [I4]]
     *                    ["groupID" => <419> [I2]]
     *                    ["categoryID" => <6> [I2]]
     *                    ["customInfo" => <empty string> [Str]]
     *                [PyDict 1 kvp]
     *                  [PyInt 10]        << flagdisconnect??
     *                  [PyInt 0]
     */

    call.Dump(COLLECT__CALL_DUMP);
    if (call.tuple->empty())
        return nullptr;

    bool t3Ship = false;
    std::vector<int32> itemIDList;
    PyList* subSystemList(nullptr);
    if (call.byname.find("subSystems") != call.byname.end()) {
        /*
         * 21:29:15 [Bound] ShipBound::AssembleShip()
         * 21:29:15 [CollectCallDump]   Call Arguments:
         * 21:29:15 [CollectCallDump]      Tuple: 1 elements
         * 21:29:15 [CollectCallDump]       [ 0]    Integer: 140023628
         * 21:29:15 [CollectCallDump]  Named Arguments:
         * 21:29:15 [CollectCallDump]   subSystems
         * 21:29:15 [CollectCallDump]       List: 5 elements
         * 21:29:15 [CollectCallDump]       [ 0]    Integer: 140023637
         * 21:29:15 [CollectCallDump]       [ 1]    Integer: 140023638
         * 21:29:15 [CollectCallDump]       [ 2]    Integer: 140023635
         * 21:29:15 [CollectCallDump]       [ 3]    Integer: 140023634
         * 21:29:15 [CollectCallDump]       [ 4]    Integer: 140023636
         *
         * 16:31:52 W AssembleShip: byname call is Invalid Type
         *
         */
        // this is a list of subSystems for t3 assembly
        t3Ship = true;
        itemIDList.push_back(PyRep::IntegerValueU32(call.tuple->GetItem(0)));
        subSystemList = call.byname.find("subSystems")->second->AsList();
    } else if (call.tuple->GetItem(0)->IsList()) {
        PyList::const_iterator list_2_cur = itemIDs->begin();
        for (size_t list_2_index(0); list_2_cur != itemIDs->end(); ++list_2_cur, ++list_2_index) {
            if (!(*list_2_cur)->IsInt()) {
                _log(XMLP__DECODE_ERROR, "Decode Call_AssembleShip failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
                return nullptr;
            }

            const PyInt* t = (*list_2_cur)->AsInt();
            itemIDList.push_back(t->value());
        }
    } else if (call.tuple->size() == 2) { // this one doesn't seem to be anywhere in the normal client code
        if (call.tuple->GetItem(0)->IsInt()
        and call.tuple->GetItem(1)->IsString()) {
            // This block is for how DNA calls AssembleShip
            // @TODO Ignoring name
            // Can't get xmlpktgen to pickup the change so.. lol
            //if (!argsNamed.Decode(&call.tuple)) {
            //    codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
            //    return nullptr;
            //}
            itemIDList.push_back(PyRep::IntegerValueU32(call.tuple->GetItem(0)));
        } else {
            sLog.Error("AssembleShip", "tuple size == 2 and ([0] != int and [1] != string) or some shit like that.");
        }
    } else {
        sLog.Error( "Handle_AssembleShip", "end of conditional" );
        return nullptr;
    }

    ShipItemRef ship(nullptr);
    for (auto cur : itemIDList) {
        ship = sItemFactory.GetShipRef(cur);

        if (ship.get() == nullptr) {
            _log(ITEM__ERROR, "Failed to load ship %i to assemble.", cur);
            continue;
        }

        //check if the ship is a stack
        if (ship->quantity() > 1) {
            // Split the stack into a new inventory item with quantity of one, cast to ShipItemRef then assembled:
            // original item stack will be left with qty-1 at original location
            ship = ShipItemRef::StaticCast(ship->Split(1, true));
            if (ship.get() == nullptr) {
                _log(ITEM__ERROR, "Failed to split stack to assemble ship %i.", cur);
                continue;
            }
        }

        if ( t3Ship ) {
            if (subSystemList == nullptr) {
                // send error here, then exit
                sLog.Error("AssembleShip", "subSystemList == nullptr for %s", call.client->GetName());
                return nullptr;
            }
            // Move the five specified subsystems to the newly assembled Tech 3 ship
            InventoryItemRef subSystemItem(nullptr);
            PyList::const_iterator itr = subSystemList->begin(), end = subSystemList->end();
            while (itr != end) {
                subSystemItem = sItemFactory.GetItemRef(PyRep::IntegerValueU32(*itr));
                if (subSystemItem.get() != nullptr)
                    subSystemItem->Move(ship->itemID(), (EVEItemFlags)(subSystemItem->GetAttribute(AttrSubSystemSlot).get_uint32()), true);
                ++itr;
            }
        }

        ship->ChangeSingleton(true, true);
    }
    return nullptr;
}

PyResult ShipBound::GetShipConfiguration(PyCallArgs &call)
{
    PyDict* dict = new PyDict();
    dict->SetItemString("allowFleetSMBUsage", new PyBool(call.client->GetShipSE()->GetFleetSMBUsage()));
    return dict;
}

PyResult ShipBound::ConfigureShip(PyCallArgs &call, PyDict* configuration)
{
    call.client->GetShipSE()->SetFleetSMBUsage(configuration->GetItemString("allowFleetSMBUsage")->AsBool());

    return nullptr;
}


/** ***********************************************************************
 * @note   these below are partially coded
 */


/** ***********************************************************************
 * @note   these do absolutely nothing at this time....
 */


PyResult ShipBound::LaunchFromContainer(PyCallArgs &call, PyInt* structureID, PyList* ids) {
    /*
     * def LaunchSMAContents(self, invItems):
     *        ids = []
     *        for invItem in invItems:
     *            structureID = invItem.locationID
     *            ids += [invItem.itemID] * invItem.stacksize
     *
     *  LaunchFromContainer(structureID, ids)
     */

    _log(SERVICE__CALL_DUMP, "ShipBound::Handle_LaunchFromContainer()");
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}


// ShipMaintenanceArray
PyResult ShipBound::ScoopToSMA(PyCallArgs &call, PyInt* objectID) {
    // no packet data

    _log(SERVICE__CALL_DUMP, "ShipBound::Handle_ScoopToSMA()");
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}


PyResult ShipBound::BoardStoredShip(PyCallArgs &call, PyInt* structureID, PyInt* shipID) {
    // no packet data

    //sm.StartService('sessionMgr').PerformSessionChange('board', ship.BoardStoredShip, structureID, shipID)
    _log(SERVICE__CALL_DUMP, "ShipBound::Handle_BoardStoredShip()");
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult ShipBound::StoreVessel(PyCallArgs &call, PyInt* destID) {
    // no packet data

    //sm.StartService('sessionMgr').PerformSessionChange('storeVessel', ship.StoreVessel, destID)
    _log(SERVICE__CALL_DUMP, "ShipBound::Handle_StoreVessel()");
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult ShipBound::SelfDestruct(PyCallArgs &call, PyInt* shipID) {
    /** @todo finish this later
     * 22:13:29 L ShipBound::Handle_SelfDestruct(): size=1
     * 22:13:29 [SvcCall]   Call Arguments:
     * 22:13:29 [SvcCall]       Tuple: 1 elements
     * 22:13:29 [SvcCall]         [ 0] Integer field: 140000378     <- ship id
     *
  _log(SERVICE__CALL_DUMP, "ShipBound::Handle_SelfDestruct()");
    call.Dump(SERVICE__CALL_DUMP);
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 60 bytes]
          [PyTuple 2 items]
            [PyInt 0]
            [PyTuple 2 items]
              [PyInt 1]
              [PyTuple 2 items]
                [PyString "SelfDestructTimer"]
                [PyDict 2 kvp]
                  [PyString "what"]
                  [PyTuple 2 items]
                    [PyInt 4]
                    [PyInt 24700]
                  [PyString "time"]
                  [PyString "2 Minutes"]
    *********  sends msg every 10 sec ************
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 70 bytes]
          [PyTuple 2 items]
            [PyInt 0]
            [PyTuple 2 items]
              [PyInt 1]
              [PyTuple 2 items]
                [PyString "SelfDestructTimer"]
                [PyDict 2 kvp]
                  [PyString "what"]
                  [PyTuple 2 items]
                    [PyInt 4]
                    [PyInt 24700]
                  [PyString "time"]
                  [PyString "1 Minute 49 Seconds"]
    ************  destruct immediate  ****************
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 47 bytes]
          [PyTuple 2 items]
            [PyInt 0]
            [PyTuple 2 items]
              [PyInt 1]
              [PyTuple 2 items]
                [PyString "SelfDestructImmediate"]
                [PyDict 1 kvp]
                  [PyString "what"]
                  [PyTuple 2 items]
                    [PyInt 4]
                    [PyInt 24700]
    ***********  not sure how to kill ship  *************
    Damage damage((static_cast<SystemEntity*>(who)),true);
    entity->Killed(damage);
    *************  cancel destruct  *****************
    [PyTuple 3 items]
      [PyInt 6]
      [PyInt 2]
      [PyTuple 1 items]
        [PySubStream 94 bytes]
          [PyObjectEx Normal]
            [PyTuple 3 items]
              [PyToken ccp_exceptions.UserError]
              [PyTuple 2 items]
                [PyString "SelfDestructAborted2"]
                [PyDict 1 kvp]
                  [PyString "when"]
                  [PyInt 83]
              [PyDict 2 kvp]
                [PyString "msg"]
                [PyString "SelfDestructAborted2"]
                [PyString "dict"]
                [PyDict 1 kvp]
                  [PyString "when"]
                  [PyInt 83]
                  //if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
        throw UserError ("SelfDestructAborted2");
*/

    /*{'messageKey': 'SelfDestructAborted2', 'dataID': 17879480, 'suppressable': False, 'bodyID': 258024, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2405}
     * {'messageKey': 'SelfDestructAbortedOther2', 'dataID': 17879483, 'suppressable': False, 'bodyID': 258025, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2406}
     * {'messageKey': 'SelfDestructCancelledExternal', 'dataID': 17876999, 'suppressable': False, 'bodyID': 257085, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3182}
     * {'messageKey': 'SelfDestructCancelledWarp', 'dataID': 17878652, 'suppressable': False, 'bodyID': 257707, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2564}
     * {'messageKey': 'SelfDestructImmediate', 'dataID': 17881593, 'suppressable': False, 'bodyID': 258829, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1533}
     * {'messageKey': 'SelfDestructImmediateOther', 'dataID': 17881596, 'suppressable': False, 'bodyID': 258830, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1534}
     * {'messageKey': 'SelfDestructInitiated', 'dataID': 17881805, 'suppressable': False, 'bodyID': 258902, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1535}
     * {'messageKey': 'SelfDestructInitiatedOther', 'dataID': 17881599, 'suppressable': False, 'bodyID': 258831, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1536}
     * {'messageKey': 'SelfDestructTimer', 'dataID': 17878655, 'suppressable': False, 'bodyID': 257708, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2563}
     * {'messageKey': 'SelfDestructTooEarly', 'dataID': 17881605, 'suppressable': False, 'bodyID': 258833, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1537}
     */
    /* return error msg from this call, if applicable, else nodeid and timestamp */
    // returns nodeID and timestamp
    // HACK: WE'RE RETURNING BACK THE SAME BOUND SERVICE, IN REALITY A NEW BOUND INSTANCE SHOULD BE CREATED FOR THIS SHIP IN SPECIFIC
    //       INSTEAD OF REUSING THIS ONE, THIS WOULD HELP KEEP INFORMATION IN/OUT OF MEMORY BASED ON THE BOUND SERVICES
    return this->GetOID();
}
