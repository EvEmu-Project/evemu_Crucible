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

/** @todo update this code to use throws for client msgs also */
/** @todo  many unfinished calls in this file.... */

#include "eve-server.h"

#include "EntityList.h"

#include "StaticDataMgr.h"
#include "inventory/InvBrokerService.h"
#include "inventory/InventoryBound.h"
#include "station/StationDataMgr.h"
#include "station/StationOffice.h"
#include "system/SystemManager.h"

InvBrokerService::InvBrokerService(EVEServiceManager &mgr) :
    BindableService("invbroker", mgr)
{
    this->Add("GetItemDescriptor", &InvBrokerService::GetItemDescriptor);
}

PyResult InvBrokerService::GetItemDescriptor(PyCallArgs &call) {
    return sDataMgr.CreateHeader();
}

BoundDispatcher* InvBrokerService::BindObject(Client* client, PyRep* bindParameters) {
    InvBroker_BindArgs args;
    //crap
    PyRep* tmp(bindParameters->Clone());
    if (!args.Decode(&tmp)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode bind args.", GetName().c_str());
        return nullptr;
    }

    _log(INV__BIND, "InvBrokerService bind request:");
    args.Dump(INV__BIND, "    ");

    return new InvBrokerBound(this->GetServiceManager(), *this, args.locationID, args.groupID);
}

void InvBrokerService::BoundReleased (InvBrokerBound* bound) {

}

InvBrokerBound::InvBrokerBound(EVEServiceManager& mgr, InvBrokerService& parent, uint32 locationID, uint32 groupID) :
    EVEBoundObject(mgr, parent),
    m_locationID(locationID),
    m_groupID(groupID)
{
    this->Add("GetContainerContents", &InvBrokerBound::GetContainerContents);
    this->Add("GetInventoryFromId", &InvBrokerBound::GetInventoryFromId);
    this->Add("GetInventory", &InvBrokerBound::GetInventory);
    this->Add("SetLabel", &InvBrokerBound::SetLabel);
    this->Add("TrashItems", &InvBrokerBound::TrashItems);
    this->Add("AssembleCargoContainer", &InvBrokerBound::AssembleCargoContainer);
    this->Add("BreakPlasticWrap", &InvBrokerBound::BreakPlasticWrap);
    this->Add("TakeOutTrash", &InvBrokerBound::TakeOutTrash);
    this->Add("SplitStack", &InvBrokerBound::SplitStack);
    this->Add("DeliverToCorpHangar", &InvBrokerBound::DeliverToCorpHangar);
    this->Add("DeliverToCorpMember", &InvBrokerBound::DeliverToCorpMember);
}

bool InvBrokerBound::CanClientCall(Client* client) {
    return true; // TODO: properly implement this
}

//is this completely right?
PyResult InvBrokerBound::GetContainerContents(PyCallArgs &call, PyInt* containerID, PyInt* locationID)
{
    InventoryItemRef item = sItemFactory.GetItemRefFromID(containerID->value());
    if (item.get() == nullptr) {
        _log(INV__ERROR, "GetContainerContents() - Unable to load inventory for itemID %u in locationID %u", containerID->value(), locationID->value());
        return nullptr;
    }
    /** @todo this will need lots-o-work for corp usage, and the List() function, as well.  */
    if (item->ownerID() == call.client->GetCharacterID()) {
        _log(INV__MESSAGE, "Handle_GetContainerContents() -  %s(%u) is owned by calling character %s(%u) ", \
                    item->name(), item->itemID(), call.client->GetName(), call.client->GetCharacterID());
    } else if ((item->ownerID() != call.client->GetCharacterID()) and sDataMgr.IsSolarSystem(locationID->value())) {
        _log(INV__WARNING, "Handle_GetContainerContents() -  %s(%u) is in space and not owned by calling character %s(%u) ", \
                    item->name(), item->itemID(), call.client->GetName(), call.client->GetCharacterID());
    } else {
        _log(INV__WARNING, "Handle_GetContainerContents() -  %s(%u) is not owned by calling character %s(%u) ", \
                    item->name(), item->itemID(), call.client->GetName(), call.client->GetCharacterID());
         throw UserError ("CantDoThatWithSomeoneElsesStuff");
    }

    return item->GetMyInventory()->List( flagNone );
}

//this is a view into the entire inventory item.  this CAN throw.  find and implement client error msgs here for corp usage
PyResult InvBrokerBound::GetInventoryFromId(PyCallArgs &call, PyInt* inventoryID, PyInt* passive) {
    /** @note this means "Get the Inventory of this itemID */
    /*
            if e.args[0] == 'CrpAccessDenied':
                self.CloseContainer(itemid)
    */
    _log(INV__DUMP, "InvBrokerBound::Handle_GetInventoryFromId() size=%li", call.tuple->size());
    call.Dump(INV__DUMP);

    sItemFactory.SetUsingClient( call.client );
    InventoryItemRef iRef(nullptr);
    // if item requested is office folder, we have to do shit a lil different, as it sends officeFolderID, instead of itemID
    if ((m_groupID == EVEDB::invGroups::Station) and (IsOfficeFolder(inventoryID->value()))) {
        uint32 officeID = stDataMgr.GetOfficeIDForCorp(m_locationID, call.client->GetCorporationID());
        iRef = sItemFactory.GetItemRefFromID( officeID );
    } else {
        iRef = sItemFactory.GetItemRefFromID(inventoryID->value());
    }
    sItemFactory.UnsetUsingClient();
    if (iRef.get() == nullptr) {
        _log(INV__ERROR, "GetInventoryFromId() - Unable to get inventoryItem for itemID %u", inventoryID->value());
        // send error to player?
        return nullptr;
    }

    // this can send locationID as named arg.  not sure if we need it, possible test cases (usually stationID)

    // use container's owner as ownerID
    uint32 ownerID = iRef->ownerID();

    /** @todo  test for container types and set flag accordingly
     * this is used to later call List().  flags set here are used by list to determine what contents to return
     *
     * NOTE: found this in client.  'LoadFitting' not coded yet, but will need to test for it once implemented
     * def LoadFitting(self, ownerID, fittingID):
     *   ...
        inv = self.invCache.GetInventoryFromId(const.containerHangar)
     */
    EVEItemFlags flag = flagNone;
    switch (iRef->categoryID()) {
        case EVEDB::invCategories::Owner: {
            switch (iRef->groupID()) {
                case EVEDB::invGroups::Character: {
                    flag = flagNone;
                    ownerID = call.client->GetCharacterID();
                } break;
                case EVEDB::invGroups::Corporation: {
                    flag = flagNone;
                    ownerID = call.client->GetCorporationID();
                } break;
                case EVEDB::invGroups::Alliance: {
                    flag = flagNone;
                    ownerID = call.client->GetAllianceID();
                } break;
                case EVEDB::invGroups::Faction: {
                    // not sure if this is used...
                    flag = flagNone;
                    ownerID = call.client->GetWarFactionID();
                } break;
            }
        } break;
        case EVEDB::invCategories::Ship: {
            // should we test for ships and hangar types here?   yes.
            if (iRef->HasAttribute(AttrHasCorporateHangars)) {
                flag = flagCargoHold;   // this is also corp hangar 1
            } else {
                flag = flagCargoHold;
            }
        } break;
        case EVEDB::invCategories::Structure: {
            switch(iRef->groupID()) {
                case EVEDB::invGroups::Control_Tower: {
                    flag = flagNone;
                } break;
                default: {
                    flag = flagHangar;
                } break;
            } break;
        } break;
        case EVEDB::invCategories::Orbitals: {
            switch(iRef->groupID()) {
                case EVEDB::invGroups::Orbital_Construction_Platform: {
                    // not sure what to do in this case...
                    flag = flagNone;
                } break;
                case EVEDB::invGroups::Orbital_Infrastructure: {
                    // this includes orbital command centers, which this may not be right for.
                    flag = flagHangar;
                    ownerID = call.client->GetCharacterID();
                } break;
            }
        } break;
        case EVEDB::invCategories::Station: {
            flag = flagHangar;
        } break;
        case EVEDB::invCategories::Deployable:
        case EVEDB::invCategories::StructureUpgrade: {
            flag = flagCargoHold;
        } break;
        case EVEDB::invCategories::Trading: {
            _log(INV__WARNING, "GetInventoryFromID called for Trading locationID %u using itemID %u", m_locationID, inventoryID->value());
            flag = flagNone;
        } break;
        case EVEDB::invCategories::Celestial: {
            switch (iRef->groupID ()) {
                case EVEDB::invGroups::Cargo_Container: {
                    flag = flagNone;
                    // only check distance if the item is in space and not in a station
                    if (sDataMgr.IsSolarSystem(iRef->locationID())) {
                        GVector direction(iRef->position(), call.client->GetShip()->position());
                        float maxDistance = 2500.0f;

                        if (iRef->HasAttribute (AttrMaxOperationalDistance) == true)
                            maxDistance = iRef->GetAttribute(AttrMaxOperationalDistance).get_float ();

                        if (direction.length () > maxDistance)
                            throw UserError ("NotCloseEnoughToOpen")
                                .AddAmount ("maxdist", maxDistance);
                    }
                    break;
                }
            }
        }
    }

    InventoryBound* ib = new InventoryBound(this->GetServiceManager(), *this, iRef, flag, ownerID, passive->value());

    return new PySubStruct(new PySubStream(ib->GetOID()));
}
void InvBrokerBound::BoundReleased (InventoryBound* bound) {
    // TODO: see how to properly keep track of these as they're not exactly your normal bound service
}

//this is a view into an inventory item using a specific flag.
PyResult InvBrokerBound::GetInventory(PyCallArgs &call, PyInt* containerID, std::optional <PyInt*> cOwnerID) {
    /** @note  this means "Get the Inventory containing this itemID */
    _log(INV__DUMP, "InvBrokerBound::Handle_GetInventory() size=%li", call.tuple->size());
    call.Dump(INV__DUMP);

    uint32 ownerID = cOwnerID.has_value() ? cOwnerID.value()->value() : 0;
    sItemFactory.SetUsingClient( call.client );
    InventoryItemRef item;
    if (m_groupID == EVEDB::invGroups::Station) {
        _log(INV__WARNING, "GetInventory called for station %u", m_locationID);
        //item = sEntityList.GetStationByID(m_locationID);
        item = sItemFactory./*GetStation*/GetItemRef(m_locationID);
    } else if (m_groupID == EVEDB::invGroups::Solar_System) {
        _log(INV__WARNING, "GetInventory called for solar system %u", m_locationID);
        item = sItemFactory./*GetSolarSystem*/GetItemRef(m_locationID);
    } else {
        _log(INV__WARNING, "GetInventory called for item %u (group: %u)", m_locationID, m_groupID);
        item = sItemFactory./*GetItemRefFromID*/GetItemRef(m_locationID);
    }
    sItemFactory.UnsetUsingClient();

    if (item.get() == nullptr) {
        codelog(INV__ERROR, "%s: Unable to load item %u for flag %u", call.client->GetName(), m_locationID, containerID->value());
        return nullptr;
    }

    EVEItemFlags flag = flagNone;
    switch(containerID->value()) {
        case Inv::Container::Wallet: { /*10001*/
            if (ownerID == 0)
                ownerID = call.client->GetCharacterID();
            flag = flagWallet;
        } break;
        case Inv::Container::Character: { /*10011*/
            if (ownerID == 0)
                ownerID = call.client->GetCharacterID();
            flag = flagSkill;
        } break;
        case Inv::Container::Hangar: { /*10004*/
            if (ownerID == 0)
                ownerID = call.client->GetCharacterID();
            flag = flagHangar;
        } break;
        case Inv::Container::CorpMarket: { /*10012*/   //this is for corp deliveries
            if (ownerID == 0)
                ownerID = call.client->GetCorporationID();
            flag = flagCorpMarket;
        } break;
        case Inv::Container::Offices: { /*10009*/
            if (ownerID == 0)
                ownerID = call.client->GetCorporationID();
            flag = flagOffice;
        } break;
        case Inv::Container::SolarSystem: { /*10003*/ // is this for flagProperty items?  (corp items in space)
            // not sure on this one
            if (ownerID == 0)
                ownerID = call.client->GetCorporationID();
            flag = flagProperty;
        } break;

        case Inv::Container::Global: { /*10002*/    // used in asset listings.  means "everywhere"
            // not sure how to code it yet...
            if (ownerID == 0)
                ownerID = call.client->GetCharacterID();
            flag = flagNone;
        } break;

        case Inv::Container::Factory: { /*10006*/
            // not sure on this one  (not coded in client)
            if (ownerID == 0)
                ownerID = call.client->GetCharacterID();
            flag = flagFactoryOperation;
        } break;
        //case Inv::Container::ScrapHeap:/*10005*/
        //case Inv::Container::Bank:/*10007*/
        //case Inv::Container::Recycler:/*10008*/
        //case Inv::Container::StationCharacters:/*10010*/
            //flag = flagNone;
            //break;
        // there is no 10005, 10006, or 10007 defined in client
        default:
            _log(INV__ERROR, "Unhandled container type %u for locationID %u", containerID->value(), m_locationID);
            return nullptr;
    }

    InventoryBound* ib = new InventoryBound(this->GetServiceManager(), *this, item, flag, ownerID, false);

    return new PySubStruct(new PySubStream(ib->GetOID()));
}

// this cannot throw, returns nothing.
PyResult InvBrokerBound::SetLabel(PyCallArgs &call, PyInt* itemID, PyRep* itemName) {
    sItemFactory.SetUsingClient( call.client );
    InventoryItemRef iRef = sItemFactory.GetItemRef(itemID->value());
    if (iRef.get() == nullptr) {
        codelog(INV__ERROR, "%s: Unable to load item %u", call.client->GetName(), itemID->value());
        sItemFactory.UnsetUsingClient();
        return nullptr;
    }

    /*
     * {'FullPath': u'UI/Messages', 'messageID': 258478, 'label': u'SetNameInvalidBody'}(u"You can't rename that type of object.", None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 258479, 'label': u'SetNameShipMustBePilotBody'}(u'You can only rename ships that you are currently piloting.', None, None)
     */

    bool error(false);
    /** @todo if owner is corp, make sure char has permissions to rename corp items  */
    if (IsPlayerCorp(iRef->ownerID())) {
        if (iRef->ownerID() != call.client->GetCorporationID()) {
            _log(INV__ERROR, "%u(%u) tried to rename CorpItem %s(%u) owned by %u.", \
                    call.client->GetName(), call.client->GetCharacterID(), iRef->name(), \
                    iRef->itemID(), iRef->ownerID());
            error = true;
        }
    } else if (IsCharacterID(iRef->ownerID())) {
        if (iRef->ownerID() != call.client->GetCharacterID()) {
            _log(INV__ERROR, "%u(%u) tried to rename PlayerItem %s(%u) owned by %u.", \
                    call.client->GetName(), call.client->GetCharacterID(), iRef->name(), \
                    iRef->itemID(), iRef->ownerID());
            error = true;
        }
    } else {
        // error here....
        error = true;
    }

    if (error) {
        call.client->SendErrorMsg("You are not allowed to rename that.");
    } else {
        iRef->Rename(PyRep::StringContent(itemName));
    }

    // Release the ItemFactory
    sItemFactory.UnsetUsingClient();

    //OnItemNameChange
    // need to also check pos rename code.
    //  this needs investigating to verify
    //  -- not sure this is sent from server

    return nullptr;
}

PyResult InvBrokerBound::TrashItems(PyCallArgs &call, PyList* itemIDs, PyInt* locationID) {

    std::vector <int32> items;

    PyList::const_iterator list_2_cur = itemIDs->begin();
    for (size_t list_2_index(0); list_2_cur != itemIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_TrashItems failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        items.push_back(t->value());
    }

    std::vector<int32>::const_iterator cur = items.begin(), end = items.end();
    for(; cur != end; ++cur) {
        InventoryItemRef item = sItemFactory.GetItemRef( *cur );
        if (item.get() == nullptr) {
            _log(INV__ERROR, "%s: Unable to load item %u to delete it. Skipping.", call.client->GetName(), *cur);
        } else if (call.client->GetCharacterID() != item->ownerID()) {
            _log(INV__ERROR, "%s: Tried to trash item %u which is not yours. Skipping.", call.client->GetName(), *cur);
        } else if (item->locationID() != locationID->value()) {
            _log(INV__ERROR, "%s: Item %u is not in location %u. Skipping.", call.client->GetName(), *cur, locationID->value());
        } else {
            item->Delete();
            // these below dont work...
            //item->SetFlag(flagJunkyardTrashed);
            //item->ChangeOwner(call.client->GetStationID(), false);
            //item->Move(call.client->GetStationID(), flagJunkyardTrashed);
        }
    }

    return nullptr;
}

/**     ***********************************************************************
 * @note   these do absolutely nothing at this time....
 */

PyResult InvBrokerBound::AssembleCargoContainer(PyCallArgs &call, PyInt* itemID, PyNone* none, PyFloat* zero) {
    /* invMgr.AssembleCargoContainer(invItem.itemID, None, 0.0)
     *
     * 14:37:46 [BindDump]   Call Arguments:
     * 14:37:46 [BindDump]       Tuple: 3 elements
     * 14:37:46 [BindDump]         [ 0] Integer field: 140000489
     * 14:37:46 [BindDump]         [ 1] (None)
     * 14:37:46 [BindDump]         [ 2] Real field: 0.000000
     *
     * 14:37:46 L InvBrokerBound::Handle_AssembleCargoContainer(): [00msize= 3
     * 14:37:46 [InvMsg]   Call Arguments:
     * 14:37:46 [InvMsg]       Tuple: 3 elements
     * 14:37:46 [InvMsg]         [ 0] Integer field: 140000489
     * 14:37:46 [InvMsg]         [ 1] (None)
     * 14:37:46 [InvMsg]         [ 2] Real field: 0.000000
     */

    sLog.Warning( "InvBrokerBound::Handle_AssembleCargoContainer()", "size=%lu", call.tuple->size());
    call.Dump(INV__DUMP);

    return nullptr;
}

PyResult InvBrokerBound::BreakPlasticWrap(PyCallArgs &call) {
    // ConfirmBreakCourierPackage   - this is for courier contracts
    sLog.Warning( "InvBrokerBound::Handle_BreakPlasticWrap()", "size=%lu", call.tuple->size());
    call.Dump(INV__DUMP);

    return nullptr;
}

PyResult InvBrokerBound::TakeOutTrash(PyCallArgs& call, PyList* itemIDs) {
    //self.invCache.GetInventory(const.containerHangar).TakeOutTrash([ invItem.itemID for invItem in invItems ])
    sLog.Warning( "InvBrokerBound::Handle_TakeOutTrash()", "size=%lu", call.tuple->size());
    call.Dump(INV__DUMP);

    return nullptr;
}

PyResult InvBrokerBound::SplitStack(PyCallArgs &call, PyInt* locationID, PyInt* itemID, PyInt* quantity, PyInt* ownerID) {
    //
    /*
    18:22:26 W InvBrokerBound::Handle_SplitStack(): size= 4
    18:22:26 [InvDump]   Call Arguments:
    18:22:26 [InvDump]      Tuple: 4 elements
    18:22:26 [InvDump]       [ 0]    Integer: 60014140          << locationID
    18:22:26 [InvDump]       [ 1]    Integer: 140024213         << itemID to split
    18:22:26 [InvDump]       [ 2]    Integer: 100               << qty to take
    18:22:26 [InvDump]       [ 3]    Integer: 98000001          << ownerID (corpID)
    */

    sLog.Warning( "InvBrokerBound::Handle_SplitStack()", "size=%lu", call.tuple->size());
    call.Dump(INV__DUMP);

    return nullptr;
}

PyResult InvBrokerBound::DeliverToCorpHangar(PyCallArgs &call, PyInt* officeID, PyInt* locationID, PyInt* itemsToDeliver, std::optional <PyInt*> quantity, PyInt* ownerID, PyInt* destinationFlag) {
    //
    /*
    18:11:51 W InvBrokerBound::Handle_DeliverToCorpHangar(): size= 6
    18:11:51 [InvDump]   Call Arguments:
    18:11:51 [InvDump]      Tuple: 6 elements
    18:11:51 [InvDump]       [ 0]    Integer: 100000000         << officeID
    18:11:51 [InvDump]       [ 1]    Integer: 60014140          << locationID
    18:11:51 [InvDump]       [ 2]   List: 1 elements            << items to deliver
    18:11:51 [InvDump]       [ 2]   [ 0]    Integer: 140024211  << itemID
    18:11:51 [InvDump]       [ 3]       None                    << u/k
    18:11:51 [InvDump]       [ 4]    Integer: 98000001          << ownerID (corpID)
    18:11:51 [InvDump]       [ 5]    Integer: 119               << destination flagID
    */

    sLog.Warning( "InvBrokerBound::Handle_DeliverToCorpHangar()", "size=%lu", call.tuple->size());
    call.Dump(INV__DUMP);

    return nullptr;
}

PyResult InvBrokerBound::DeliverToCorpMember(PyCallArgs &call, PyInt* corporationMemberID, PyInt* stationID, PyList* itemIDs, std::optional <PyInt*> quantity, PyInt* ownerID) {
    //
    /*
    18:49:06 W InvBrokerBound::Handle_DeliverToCorpMember(): size= 5
    18:49:06 [InvDump]   Call Arguments:
    18:49:06 [InvDump]      Tuple: 5 elements
    18:49:06 [InvDump]       [ 0]    Integer: 90000000          << member charID
    18:49:06 [InvDump]       [ 1]    Integer: 60014140          << locationID
    18:49:06 [InvDump]       [ 2]   List: 1 elements            << items to deliver
    18:49:06 [InvDump]       [ 2]   [ 0]    Integer: 140024205  << itemID
    18:49:06 [InvDump]       [ 3]       None                    << u/k
    18:49:06 [InvDump]       [ 4]    Integer: 98000001          << ownerID (corpID)
    */
    sLog.Warning( "InvBrokerBound::Handle_DeliverToCorpMember()", "size=%lu", call.tuple->size());
    call.Dump(INV__DUMP);

    return nullptr;
}
