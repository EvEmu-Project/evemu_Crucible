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
    Author:        Reve
    Rewrite:    Allan
*/

#include "eve-server.h"
#include "packets/Repair.h"



#include "Client.h"
#include "inventory/Inventory.h"
#include "ship/ShipDB.h"
#include "station/RepairService.h"
#include "station/Station.h"
#include "system/SystemManager.h"
#include "services/ServiceManager.h"


RepairService::RepairService(EVEServiceManager& mgr) :
    BindableService ("repairSvc", mgr)
{
    this->Add("UnasembleItems", &RepairService::UnasembleItems);
}

BoundDispatcher* RepairService::BindObject(Client* client, PyRep* bindParameters) {
    uint32 locationID = bindParameters->AsInt ()->value ();
    auto it = this->m_instances.find (locationID);

    if (it != this->m_instances.end ())
        return it->second;

    RepairServiceBound* bound = new RepairServiceBound (this->GetServiceManager(), *this, locationID);
    this->m_instances.insert_or_assign (locationID, bound);

    return bound;
}

void RepairService::BoundReleased (RepairServiceBound* bound) {
    auto it = this->m_instances.find (bound->GetLocationID());

    if (it == this->m_instances.end ())
        return;

    this->m_instances.erase (it);
}

void RepairService::GetDamageReports(uint32 itemID, Inventory* pInv, PyList* list) {
    std::vector<InventoryItemRef> itemRefVec;
    InventoryItemRef iRef = pInv->GetByID(itemID);
    if (iRef.get() == nullptr) {
        iRef = sItemFactory.GetItemRef(itemID);
        if (iRef.get() == nullptr)
            return;
    }
    itemRefVec.push_back(iRef);
    if (iRef->IsShipItem()) {
        if (iRef->GetShipItem()->IsActive()) {
            iRef->GetShipItem()->GetModuleRefVec(itemRefVec);
        }
        else {
            iRef->GetShipItem()->GetModuleItemVec(itemRefVec);
        }
    }

    for (auto cur : itemRefVec) {
        RepairItemData rid;
        rid.itemID = cur->itemID();
        rid.typeID = cur->typeID();
        rid.groupID = cur->groupID();
        rid.damage = cur->GetAttribute(AttrDamage).get_int();
        rid.maxHealth = cur->GetAttribute(AttrHP).get_int();
        // not sure how to find this data
        rid.repairable = 1;
        if (cur->IsShipItem()) {    // have to check for drone here, also
            rid.damage += cur->GetAttribute(AttrArmorDamage).get_int();
            rid.maxHealth += cur->GetAttribute(AttrArmorHP).get_int();
            // ship is (basePrice)*7.5e-10
            rid.costToRepairOneUnitOfDamage = (cur->type().basePrice() * sConfig.rates.ShipRepairModifier);
        }
        else {
            // modules are (basePrice)*1.25e-6
            rid.costToRepairOneUnitOfDamage = (cur->type().basePrice() * sConfig.rates.ModuleRepairModifier);
        }

        list->AddItem(rid.Encode());
    }
}

PyResult RepairService::UnasembleItems(PyCallArgs& call, PyDict* validIDsByStationID, PyList* skipChecks) {
    // sm.RemoteSvc('repairSvc').UnasembleItems(dict(validIDsByStationID), skipChecks)

    /*
     * 15:17:44 [PhysicsInfo]  Call_UnasembleItems
     * 15:17:44 [PhysicsInfo]  dict:
     * 15:17:44 [PhysicsInfo]      Dictionary: 2 entries
     * 15:17:44 [PhysicsInfo]        [ 0] Key: Integer field: 60004591     <-- stationID
     * 15:17:44 [PhysicsInfo]        [ 0] Value: List: 3 elements          <-- list of 2 element tuples
     * 15:17:44 [PhysicsInfo]        [ 0] Value:   [ 0] Tuple: 2 elements  <-- tuple of itemID/stationID
     * 15:17:44 [PhysicsInfo]        [ 0] Value:   [ 0]   [ 0] Integer field: 140006472
     * 15:17:44 [PhysicsInfo]        [ 0] Value:   [ 0]   [ 1] Integer field: 60004591
     * 15:17:44 [PhysicsInfo]        [ 0] Value:   [ 1] Tuple: 2 elements  <-- tuple of itemID/stationID
     * 15:17:44 [PhysicsInfo]        [ 0] Value:   [ 1]   [ 0] Integer field: 140006477
     * 15:17:44 [PhysicsInfo]        [ 0] Value:   [ 1]   [ 1] Integer field: 60004591
     * 15:17:44 [PhysicsInfo]        [ 0] Value:   [ 2] Tuple: 2 elements  <-- tuple of itemID/stationID
     * 15:17:44 [PhysicsInfo]        [ 0] Value:   [ 2]   [ 0] Integer field: 140006476
     * 15:17:44 [PhysicsInfo]        [ 0] Value:   [ 2]   [ 1] Integer field: 60004591
     * 15:17:44 [PhysicsInfo]        [ 1] Key: Integer field: 60014137     <-- stationID
     * 15:17:44 [PhysicsInfo]        [ 1] Value: List: 1 elements          <-- list of 2 element tuples
     * 15:17:44 [PhysicsInfo]        [ 1] Value:   [ 0] Tuple: 2 elements  <-- tuple of itemID/stationID
     * 15:17:44 [PhysicsInfo]        [ 1] Value:   [ 0]   [ 0] Integer field: 140000028
     * 15:17:44 [PhysicsInfo]        [ 1] Value:   [ 0]   [ 1] Integer field: 60014137
     * 15:17:44 [PhysicsInfo]  list:
     * 15:17:44 [PhysicsInfo]      List: Empty                             <-- skipChecks, not sure what this is for
     */

    PyList* pList(nullptr);
    PyTuple* tuple(nullptr);
    InventoryItemRef iRef(nullptr);
    uint32 itemID(0); //,locationID = 0,  itemLoc = 0;

    if (skipChecks->size() > 0)
        ;  // skipChecks is populated....do something constructive here

    /** @todo  may have to switch to station inventory to get item to check if this is container, and remove items BEFORE repacking!!  */
    for (PyDict::const_iterator dictItr = validIDsByStationID->begin(); dictItr != validIDsByStationID->end(); ++dictItr) {
        // Dictionary key is LocationID, value is tuple of itemID/item locationID
        //locationID = dictItr->first->AsInt()->value();
        pList = dictItr->second->AsList();
        if (pList != nullptr) {
            // Iterate through list.
            for (PyList::const_iterator listItr = pList->begin(); listItr != pList->end(); ++listItr) {
                // List is tuples of itemID, LocationID pairs.
                tuple = (*listItr)->AsTuple();
                if (tuple != nullptr) {
                    // Get the itemID.
                    itemID = PyRep::IntegerValue(tuple->GetItem(0));
                    //itemLoc = PyRep::IntegerValue(tuple->GetItem(1));
                    iRef = sItemFactory.GetItemRef(itemID);
                    if (iRef.get() != nullptr) {
                        // Add type exceptions here.
                        if (iRef->categoryID() == EVEDB::invCategories::Blueprint
                            or iRef->categoryID() == EVEDB::invCategories::Skill
                            or iRef->categoryID() == EVEDB::invCategories::Material) {
                            // Item cannot be repackaged once used!
                            continue;
                        }
                        // if this is a ship, remove all modules and empty cargo holds before repacking
                        //  maybe we should instruct player that ship is fitted/has cargo, and let them do this
                        if (iRef->IsShipItem()) {
                            iRef->GetShipItem()->EmptyCargo();
                            iRef->GetShipItem()->StripFitting();
                            // check insurance and delete if applicable
                            ShipDB::DeleteInsuranceByShipID(itemID);
                        }

                        iRef->ChangeSingleton(false, true);
                    }
                }
            }
        }
    }

    // MultipleItemsFailedToRepackage

    return PyStatic.NewNone();
}

RepairServiceBound::RepairServiceBound(EVEServiceManager& mgr, RepairService& parent, uint32 locationID) :
    EVEBoundObject(mgr, parent),
    m_locationID(locationID)
{
    this->Add("DamageModules", &RepairServiceBound::DamageModules);
    this->Add("RepairItems", &RepairServiceBound::RepairItems);
    this->Add("GetDamageReports", &RepairServiceBound::GetDamageReports);
}

PyResult RepairServiceBound::DamageModules(PyCallArgs &call, PyList* itemIDAndAmountOfDamage) {
    /*    itemIDAndAmountOfDamageList.append((item.itemID, amount)) <-- amount is % of damage
     *    self.repairSvc.DamageModules(itemIDAndAmountOfDamageList)
     */

    _log(PHYSICS__INFO, "RepairSvcBound::Handle_DamageModules() size=%lli", call.tuple->size());
    call.Dump(PHYSICS__INFO);

    return PyStatic.NewNone();
}

PyResult RepairServiceBound::RepairItems(PyCallArgs &call, PyList* itemIDs, PyFloat* iskAmount) {
    //  self.repairSvc.RepairItems(itemIDs, amount['qty'])
    /*
     * 00:18:28 W RepairSvcBound::Handle_RepairItems(): size= 2
     * 00:18:28 [PhysicsInfo]   Call Arguments:
     * 00:18:28 [PhysicsInfo]       Tuple: 2 elements
     * 00:18:28 [PhysicsInfo]         [ 0] List: 1 elements                 <-- list of itemIDs to repair
     * 00:18:28 [PhysicsInfo]         [ 0]   [ 0] Integer field: 140005905  <-- itemID
     * 00:18:28 [PhysicsInfo]         [ 1] Real field: 112500.000000        <-- isk amount to spend on repairs.
     */
    if (iskAmount->value() < 0.01)
        return nullptr;

    /* itemIDs is list of itemIDs to repair.
     * iskAmount is how much to spend on repairs.
     *  - this needs to be checked against total repair amount, and use fraction to reduce damage for all items in list
     */

    InventoryItemRef iRef(nullptr);
    double cost(0), total(0);
    uint32 damage(0);
    std::vector<InventoryItemRef> itemRefVec;
    PyList::const_iterator itr = itemIDs->begin(), end = itemIDs->end();
    for (; itr != end; ++itr) {
        iRef = sItemFactory.GetItemRef(PyRep::IntegerValueU32(*itr));
        if (iRef.get() == nullptr)
            continue;

        cost = 0;
        damage = iRef->GetAttribute(AttrDamage).get_uint32();
        itemRefVec.push_back(iRef);
        if (iRef->IsShipItem()) {
            damage += iRef->GetAttribute(AttrArmorDamage).get_uint32();
            // ship is (basePrice)*7.5e-10
            cost = (iRef->type().basePrice() * sConfig.rates.ShipRepairModifier);
        } else {
            // modules are (basePrice)*1.25e-6
            cost = (iRef->type().basePrice() * sConfig.rates.ModuleRepairModifier);
        }
        total += damage * cost;
    }

    float fraction = 1.0;
    if (iskAmount->value() < total)
        fraction = total / iskAmount->value();

    /** @todo  entire repair be based on items damage vs mats list at market value
     * get bp required mats for building.
     * from there, determine % of that type/amt that *may* be required for repairing.
     * check player hangar for amounts, and if present, subtract market value from cost estimate.
     * 5-10% of total cost as min charge for use of facilities
     */

    /** more shit to check for and add as required...
     * NotEnoughRepairMaterialToFinishAllRepairs
     * NotEnoughRepairMaterialToFinishAllRepairsBody'}(u'There is not enough unallocated {typeName} left to initiate repairs on this module.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
     *
     *{'messageKey': 'RepairModuleNotDamaged', 'dataID': 17882542, 'suppressable': False, 'bodyID': 259175, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1492}
     * {'messageKey': 'RepairNoMoneyForRepair', 'dataID': 17882601, 'suppressable': False, 'bodyID': 259197, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259196, 'messageID': 1493}
     */

    if (fraction > 1.0)
        fraction = 1;
    EvilNumber toRepair = EvilZero, curDamage = EvilZero;
    for (auto cur : itemRefVec) {
        if (cur->IsShipItem()) {
            if (fraction == 1) {
                cur->SetAttribute(AttrDamage, EvilZero);
                cur->SetAttribute(AttrArmorDamage, EvilZero);
                continue;
            }

            uint32 cHull =  cur->GetAttribute(AttrDamage).get_uint32();
            uint32 cArmor =  cur->GetAttribute(AttrArmorDamage).get_uint32();
            curDamage = cHull + cArmor;
            toRepair = curDamage * fraction;
            // this will repair hull first, then armor
            if (toRepair > cHull) {
                toRepair -= cHull;
                cur->SetAttribute(AttrDamage, EvilZero);
                if (toRepair >= cArmor) {
                    cur->SetAttribute(AttrArmorDamage, EvilZero);
                } else {
                    toRepair = cArmor - toRepair;
                    cur->SetAttribute(AttrArmorDamage, toRepair);
                }
            } else {
                cur->SetAttribute(AttrDamage, toRepair);
            }
        } else {
            if (fraction == 1) {
                cur->SetAttribute(AttrDamage, EvilZero);
                continue;
            }
            if ((cur->GetAttribute(AttrDamage).get_float() / cur->GetAttribute(AttrHP).get_float()) > fraction) {
                toRepair = cur->GetAttribute(AttrHP) * fraction;
            } else {
                toRepair = EvilZero;
            }
            if (toRepair < EvilZero)
                toRepair = EvilZero;
            cur->SetAttribute(AttrDamage, toRepair);
        }
    }

    return PyStatic.NewNone();
}

PyResult RepairServiceBound::GetDamageReports(PyCallArgs &call, PyList* itemIDs) {
    /*
     * 20:39:30 W RepairSvcBound::Handle_GetDamageReports(): size= 1
     * 20:39:30 [SvcCallDump]   Call Arguments:
     * 20:39:30 [SvcCallDump]       Tuple: 1 elements
     * 20:39:30 [SvcCallDump]         [ 0] List: 1 elements
     * 20:39:30 [SvcCallDump]         [ 0]   [ 0] Integer field: 140012041
     */
    std::vector <int32> ints;

    // TODO: update this when the type changes are in place so these things are easier to work with
    PyList::const_iterator list_2_cur = itemIDs->begin();
    for (size_t list_2_index(0); list_2_cur != itemIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_SingleIntList failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        ints.push_back(t->value());
    }

    PyDict* dict = new PyDict();
    Client* pClient = call.client;
    StationItemRef sRef = pClient->SystemMgr()->GetStationFromInventory(m_locationID);
    Inventory* pInv = sRef->GetMyInventory();
    float standing = 0.0f;
    // standing system isnt complete, but this is the correct data methods for station standing checks
    if (IsNPCCorp(sRef->ownerID())) {
        standing = pClient->GetChar()->GetNPCCorpStanding(sRef->ownerID(), pClient->GetCharacterID());
    } else {
        standing = pClient->GetChar()->GetStandingModified(sRef->ownerID(), pClient->GetCharacterID());
    }

    for (auto cur : ints) {
        RepairListRsp rlr;
            rlr.discount       = "0%";  // not sure....seen 0% and 100% in packets
            rlr.serviceCharge  = "0%";  // not sure....seen 0% in packets
            rlr.playerStanding = standing;
            rlr.lines = new PyList();
            RepairService::GetDamageReports(cur, pInv, rlr.lines);
        dict->SetItem(new PyInt(cur), rlr.Encode());
    }

    return dict;
}
