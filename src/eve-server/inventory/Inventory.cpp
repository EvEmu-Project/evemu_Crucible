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
    Author:     Bloody.Rabbit
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "PyCallable.h"
#include "character/Character.h"
#include "inventory/Inventory.h"
#include "inventory/ItemDB.h"
#include "inventory/ItemFactory.h"
#include "pos/Structure.h"
#include "ship/Ship.h"
#include "station/Station.h"
#include "station/StationDB.h"
#include "system/Container.h"
#include "system/SolarSystem.h"
#include "system/sov/SovereigntyDataMgr.h"

/*
 * Inventory
 */
Inventory::Inventory(InventoryItemRef iRef)
{
    mContentsLoaded = false;
    m_self = iRef;
    m_myID = iRef->itemID();
}

void Inventory::Reset()
{
    Unload();
    LoadContents();
}

void Inventory::Unload()
{
    if (!mContentsLoaded)
        return;

    //  save contents on the off-chance they have changed, but not on shutdown. (saved in ItemFactory::Close())
    Inventory* inv(nullptr);
    if (!sConsole.IsShutdown()) {
        std::vector<Inv::SaveData> items;
        items.clear();
        std::map<uint32, InventoryItemRef>::iterator itr = mContents.begin();
        while (itr != mContents.end()) {
            // test for item contents and unload as required
            inv = itr->second->GetMyInventory();
            if (inv != nullptr)
                inv->Unload();
            if (IsPlayerItem(itr->first)) {   // only save player items (except skills - saved in Character::SaveAll())
                if (itr->second->flag() == flagSkill) {
                    sItemFactory.RemoveItem(itr->first);
                    itr = mContents.erase(itr);
                    continue;
                }

                Inv::SaveData data = Inv::SaveData();
                    data.itemID = itr->first;
                    data.contraband = itr->second->contraband();
                    data.flag = itr->second->flag();
                    data.locationID = itr->second->locationID();
                    data.ownerID = itr->second->ownerID();
                    data.position = itr->second->position();
                    data.quantity = itr->second->quantity();
                    data.singleton = itr->second->isSingleton();
                    data.typeID = itr->second->typeID();
                    data.customInfo = itr->second->customInfo();
                items.push_back(data);
            }
            sItemFactory.RemoveItem(itr->first);
            itr = mContents.erase(itr);
        }

        ItemDB::SaveItems(items);
    }
    mContents.clear();
    m_contentsByFlag.clear();
    mContentsLoaded = false;
}

bool Inventory::GetItems(OwnerData od, std::vector< uint32 >& into ) {
    return m_db.GetItemContents(od, into);
}

bool Inventory::LoadContents() {
    if (IsAgent(m_myID))
        return true;
    double profileStartTime(GetTimeUSeconds());
    /* rewrote logic, optimized, and fixed "empty inventory" for new chars in existing systems  -allan 22.2.16 */
    Client* pClient(sItemFactory.GetUsingClient());

    // test for character creation (which throws errors) and station loading
    if (pClient != nullptr) {
        if (pClient->IsCharCreation())
            return true;
        if (sDataMgr.IsStation(m_myID)) {
            if (pClient->IsHangarLoaded(m_myID))
                return true;
            pClient->AddStationHangar(m_myID);
            mContentsLoaded = false;
        }
    }

    // check if the contents has already been loaded
    if (mContentsLoaded) {
        _log(INV__INFO, "Inventory::LoadContents() - inventory %u(%p) already loaded.", m_myID, this);
        return true;
    }

    OwnerData od = OwnerData();
        od.ownerID = 1;
        od.locID = m_myID;

    std::vector<uint32> items;
    if (pClient != nullptr) {
        if (pClient->IsValidSession())
            od.corpID = pClient->GetCorporationID();
        if (sDataMgr.IsStation(m_myID)) {
            if (!StationItemRef::StaticCast(m_self)->IsLoaded())
                StationDB::LoadOffices(od, items);
            if (IsPlayerCorp(od.corpID)) {
                /* this will load all non-NPC corp items in this station */
                od.ownerID = od.corpID;
                _log(INV__TRACE, "Inventory::LoadContents()::IsPlayerCorp() - Loading inventory %u(%p) with owner %u", m_myID, this , od.ownerID);
                GetItems(od, items);
            }
        } else if (IsOfficeID(m_myID)) {
            if (IsPlayerCorp(od.corpID)) {
                /* this will load corp hangars' inventory for this station */
                od.ownerID = od.corpID;
                _log(INV__TRACE, "Inventory::LoadContents() - Loading office inventory %u(%p) for corp %u in station %s",\
                            m_myID, this , od.ownerID, (pClient->IsValidSession() ? itoa(pClient->GetStationID()) : "(invalid)"));
                GetItems(od, items);
            } else {
                // make error for loading office and NOT a PC corp
                _log(INV__WARNING, "Inventory::LoadContents() - inventory of officeID %u using corpID %u. Continuing...", m_myID, od.corpID);
            }
        }
        if (pClient->IsValidSession()) {
            od.ownerID = pClient->GetCharacterID();
        } else {
            od.ownerID = pClient->GetCharID();
        }
    }

    _log(INV__TRACE, "Inventory::LoadContents() - Loading inventory of %s(%u) with owner %u", m_self->name(), m_myID, od.ownerID);
    if (!GetItems(od, items)) {
        _log(INV__ERROR, "Inventory::LoadContents() - Failed to get inventory items for %s(%u)", m_self->name(), m_myID);
        if ((pClient != nullptr) and sDataMgr.IsStation(m_myID))
            pClient->RemoveStationHangar(m_myID);
        return false;
    }

    for (auto cur : items) {
        if ((cur == od.ownerID) or (cur == od.locID) or (cur == m_myID))
            continue;
        InventoryItemRef iRef = sItemFactory.GetItemRef(cur);
        if (iRef.get() == nullptr) {
            _log(INV__WARNING, "Inventory::LoadContents() - Failed to load item %u contained in %u. Skipping.", cur, m_myID);
            continue;
        } else {
            AddItem(iRef);
            Client* pClient(sItemFactory.GetUsingClient());
        }
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::itemload, GetTimeUSeconds() - profileStartTime);

    return (mContentsLoaded = true);
}

void Inventory::AddItem(InventoryItemRef iRef) {
    //segfault check
    if (iRef.get() == nullptr)
        return;

    std::map<uint32, InventoryItemRef>::iterator itr = mContents.find(iRef->itemID());
    std::pair <std::_Rb_tree_iterator <std::pair <const uint32, InventoryItemRef > >, bool > test;
    if (itr == mContents.end())
        test = mContents.emplace(iRef->itemID(), iRef);

    if (test.second) {
        _log(INV__TRACE, "Inventory::AddItem() - Updated %s(%u) to contain (%u) %s(%u) in %s.", \
                m_self->name(), m_myID, iRef->quantity(), iRef->name(), iRef->itemID(), sDataMgr.GetFlagName(iRef->flag()));
    } else {
        _log(INV__TRACE, "Inventory::AddItem() - %s(%u) already contains %s(%u) in %s.", \
                m_self->name(), m_myID, iRef->name(), iRef->itemID(), sDataMgr.GetFlagName(iRef->flag()));
    }

    // need to find and remove skill in training flag here for proper skill search
    if (IsCharacterID(m_myID)) {
        if (iRef->categoryID() == EVEDB::invCategories::Skill) {
            m_contentsByFlag.emplace(flagSkill, iRef);
        } else {
            m_contentsByFlag.emplace(iRef->flag(), iRef);
        }
    } else {
        m_contentsByFlag.emplace(iRef->flag(), iRef);
    }

    // Apply iHub upgrades
    if (m_self->typeID() == EVEDB::invTypes::InfrastructureHub) {
        _log(SOV__DEBUG, "Applying system upgrade %s to system %s...", iRef->name(), m_self->name());

        // For now, all we need to do is mark the upgrade active,
        // but in the future upgrades for military and resource harvesting
        // will need to be 'activated' here
        iRef->SetFlag(EVEItemFlags::flagStructureActive, true);
    }
}

void Inventory::RemoveItem(InventoryItemRef iRef) {
    //segfault check
    if (iRef.get() == nullptr)
        return;

    std::map<uint32, InventoryItemRef>::iterator itr = mContents.find(iRef->itemID());
    if (itr != mContents.end()) {
        mContents.erase(itr);
        _log(INV__TRACE, "Inventory::RemoveItem(1) - Updated %s(%u) to no longer contain %s(%u) in %s.", \
                m_self->name(), m_myID, iRef->name(), iRef->itemID(), sDataMgr.GetFlagName(iRef->flag()));
    } else {
        _log(INV__WARNING,"Inventory::RemoveItem(1) - %s(%u) contents does not contain %s(%u) in %s.", \
                m_self->name(), m_myID, iRef->name(), iRef->itemID(), sDataMgr.GetFlagName(iRef->flag()));
    }

    /** @todo @note  this isnt working right, and im not sure why yet...  */
    auto range = m_contentsByFlag.equal_range(iRef->flag());
    for (auto cur = range.first; cur != range.second; ++cur) {
        if (cur->second == iRef) {
            m_contentsByFlag.erase(cur);
            _log(INV__TRACE, "Inventory::RemoveItem(2) - %s(%u) removed from %s(%u) flagMap at %s.", \
                    iRef->name(), iRef->itemID(), m_self->name(), m_myID, sDataMgr.GetFlagName(iRef->flag()));
            return;
        }
    }

    _log(INV__WARNING,"Inventory::RemoveItem(2) - %s(%u) flagMap does not contain %s(%u) in %s.", \
            m_self->name(), m_myID, iRef->name(), iRef->itemID(), sDataMgr.GetFlagName(iRef->flag()));
}

void Inventory::DeleteContents()
{
    if (!mContentsLoaded)
        return;
    InventoryItemRef iRef(nullptr);
    std::map<uint32, InventoryItemRef>::iterator itr = mContents.begin();
    while (itr != mContents.end()) {
        iRef = itr->second;
        ++itr;
        iRef->Delete();
    }

    mContents.clear();
    m_contentsByFlag.clear();
    mContentsLoaded = false;
}

CRowSet* Inventory::List(EVEItemFlags flag, uint32 ownerID/*0*/) const
{
    DBRowDescriptor* header = sDataMgr.CreateHeader();
    CRowSet* rowset = new CRowSet(&header);
    List(rowset, flag, ownerID);

    if (is_log_enabled(INV__LIST))
        rowset->Dump(INV__LIST, "    ");
    return rowset;
}

void Inventory::List(CRowSet* into, EVEItemFlags flag, uint32 ownerID) const {
    //there has to be a better way to build this...
    PyPackedRow* row(nullptr);
    // office hangars list ALL items.  client separates by division flag
    if (IsOfficeID(m_myID) or IsCharacterID(m_myID)) {
        for (auto cur : mContents) {
            row = into->NewRow();
            cur.second->GetItemRow(row);
        }
    } else if (m_self->categoryID() == EVEDB::invCategories::Ship) {
        bool space = sDataMgr.IsSolarSystem(m_self->locationID());
        for (auto cur : mContents) {
            // this also fills module/charges in fit window when docked.
            //  charges not sent like this in space (uses subLocation sent via shipInfo())
            if (space and IsFittingSlot(cur.second->flag()))
                if (cur.second->categoryID() == EVEDB::invCategories::Charge)
                    continue;
            row = into->NewRow();
            cur.second->GetItemRow(row);
        }
    } else {
        for (auto cur : mContents) {
            if (((ownerID == 0)        or (cur.second->ownerID() == ownerID))
            and ((flag == flagNone) or (cur.second->flag() == flag))) {
                row = into->NewRow();
                cur.second->GetItemRow(row);
            }
        }
    }
}

void Inventory::GetCargoList(std::multimap< uint8, InventoryItemRef >& cargoMap) {
    for (auto cur : mContents)
        cargoMap.emplace(cur.second->flag(), cur.second);
}

float Inventory::GetCorpHangerCapyUsed() const {
    float totalVolume(0.0f);
    for (auto cur : mContents)
        if (IsHangarFlag(cur.second->flag()))
            totalVolume += cur.second->quantity() * cur.second->GetAttribute(AttrVolume).get_float();
    return totalVolume;
}

void Inventory::GetInventoryVec(std::vector<InventoryItemRef> &itemVec) {
    std::vector<InventoryItemRef> itemVecTmp;
    itemVecTmp.clear();
    for (auto cur : mContents)
        itemVecTmp.push_back(cur.second);
    /* sorting method to put modules first, charges second, and cargo last
     *  this is needed to correctly fit modules BEFORE trying to load charges
     */
    itemVec = SortVector(itemVecTmp);
}

std::vector<InventoryItemRef> Inventory::SortVector(std::vector<InventoryItemRef> &itemVec)
{
    // my sort
    //15:53:09 [ItemTrace] Inventory::SortVector: 41 items sorted in 0.177us with 480 loops.  <<-- dunno when/where this was run
    //15:40:20 [ItemTrace] Inventory::SortVector() - 30 items sorted in 28.250us with 87 loops.i
    //22:09:28 [InvTrace] Inventory::SortVector() - 47 items sorted in 129.250us with 644 loops.


    // std::swap
    //12:57:36 [ItemTrace] Inventory::SortVector() - 21 items sorted in 16.000us with 60 loops.
    //13:00:55 [ItemTrace] Inventory::SortVector() - 15 items sorted in 20.750us with 28 loops.
    //13:01:20 [ItemTrace] Inventory::SortVector() - 19 items sorted in 46.000us with 90 loops.
    /* sorts a vector of items by category, with loaded modules first (in slot order), then loaded charges (in slot order), then cargo
     * if there is only one item, no sorting required...
     *  this should only be called by ships
     *   -allan
     */
    if (itemVec.size() < 2)
        return itemVec;

    uint16 count(0);
    double start(GetTimeUSeconds());

    //begin basic sort
    bool done(false);
    InventoryItemRef tmp(nullptr);

    while (!done) { //check if sorted
        done = true;  //assume sorted
        //iterate though list
        for (int i = 0, i2 = 1; (i < itemVec.size()) && (i2 < itemVec.size()); ++i, ++i2) {
            if ((IsModuleSlot(itemVec[i]->flag())) && (IsModuleSlot(itemVec[i2]->flag()))) {
                //check if each pair is sorted by category.  subsystems > charges > modules
                if (itemVec[i]->categoryID() > itemVec[i2]->categoryID()) {
                    //it's not, so flip the values
                    //std::swap(itemVec[i],itemVec[i2]);  // this is ~100x slower on dev server
                    tmp = itemVec[i];
                    itemVec[i] = itemVec[i2];
                    itemVec[i2] = tmp;
                    done = false;  //we weren't sorted, so now go back and check if we are
                }
            //check if each pair is sorted by flag.  cargo > module
            } else if ((IsCargoHoldFlag(itemVec[i]->flag())) && (IsModuleSlot(itemVec[i2]->flag()))) {
                //it's not, so flip the values
                tmp = itemVec[i];
                itemVec[i] = itemVec[i2];
                itemVec[i2] = tmp;
                done = false;  //we weren't sorted, so now go back and check if we are
            }
            ++count;
        }
    }

    if (sConfig.debug.IsTestServer)
        _log(INV__TRACE, "Inventory::SortVector() - %lu items sorted in %.3fus with %u loops.", itemVec.size(), (GetTimeUSeconds() - start), count);

    return itemVec;  //returns sorted list
}

InventoryItemRef Inventory::GetByID(uint32 id) const {
    std::map<uint32, InventoryItemRef>::const_iterator res = mContents.find(id);
    if (res != mContents.end())
        return res->second;

    return InventoryItemRef(nullptr);
}

// for stations only...can get expensive for stations that have many players loaded
//  maybe create an inventory map by owner in station?
void Inventory::GetInvForOwner(uint32 ownerID, std::vector< InventoryItemRef >& items)
{
    if (!IsOfficeID(m_myID) and !sDataMgr.IsStation(m_myID)) {
        _log(INV__ERROR, "GetInvForOwner called on non-station item %s(%u)", m_self->name(), m_myID);
        EvE::traceStack();
    }
    for (auto cur : mContents)
        if (cur.second->ownerID() == ownerID)
            items.push_back(cur.second);
}

InventoryItemRef Inventory::FindFirstByFlag(EVEItemFlags flag) const {
    for (auto cur : mContents)
        if (cur.second->flag() == flag)
            return cur.second;

        return InventoryItemRef(nullptr);
}

InventoryItemRef Inventory::GetByTypeFlag(uint16 typeID, EVEItemFlags flag) const {
    auto range = m_contentsByFlag.equal_range(flag);
    for ( auto itr = range.first; itr != range.second; ++itr )
        if (itr->second->typeID() == typeID)
            return itr->second;

    return InventoryItemRef(nullptr);
}

void Inventory::GetInventoryMap( std::map< uint32, InventoryItemRef >& invMap ) {
    for (auto cur : mContents)
        invMap.emplace(cur.first, cur.second);
}

uint32 Inventory::GetItemsByFlag(EVEItemFlags flag, std::vector<InventoryItemRef> &items) const {
    auto range = m_contentsByFlag.equal_range(flag);
    for ( auto itr = range.first; itr != range.second; ++itr )
            items.push_back(itr->second);
    return items.size();
}

bool Inventory::GetTypesByFlag(EVEItemFlags flag, std::map< uint16, InventoryItemRef >& items)
{
    auto range = m_contentsByFlag.equal_range(flag);
    for ( auto itr = range.first; itr != range.second; ++itr )
        items.emplace(itr->second->typeID(), itr->second);

    if (items.size() > 0)
        return true;
    return false;
}

InventoryItemRef Inventory::GetItemByTypeFlag(uint16 typeID, EVEItemFlags flag)
{
    std::vector<InventoryItemRef> items;
    if (GetItemsByFlag(flag, items) < 1)
        return InventoryItemRef(nullptr);

    for (auto cur : items)
        if (cur->typeID() == typeID )
            return cur;

    return InventoryItemRef(nullptr);
}

bool Inventory::GetSingleItemByFlag(EVEItemFlags flag, InventoryItemRef& iRef) const {
    auto range = m_contentsByFlag.equal_range(flag);
    for ( auto itr = range.first; itr != range.second; ++itr ) {
        iRef = itr->second;
        return true;
    }
    return false;
}

bool Inventory::IsEmptyByFlag(EVEItemFlags flag) const {
    return (m_contentsByFlag.find(flag) == m_contentsByFlag.end());
}

uint32 Inventory::GetItemsByFlagRange(EVEItemFlags lowflag, EVEItemFlags highflag, std::vector<InventoryItemRef> &items) const
{
    // i dont yet see a better way to do this one...
    uint32 count = 0;
    for (auto cur : mContents)
        if (cur.second->flag() >= lowflag && cur.second->flag() <= highflag) {
            items.push_back(cur.second);
            ++count;
        }
    return count;
}

uint32 Inventory::GetItemsByFlagSet(std::set<EVEItemFlags> flags, std::vector<InventoryItemRef> &items) const
{
    // i dont yet see a better way to do this one...
    uint32 count = 0;
    for (auto cur : mContents)
        if (flags.find(cur.second->flag()) != flags.end()) {
            items.push_back(cur.second);
            ++count;
        }
    return count;
}

bool Inventory::ContainsTypeQty(uint16 typeID, uint32 qty/*0*/) const
{
    uint32 count(0);
    for (auto cur : mContents) {
        if (cur.second->typeID() == typeID ) {
            if (cur.second->quantity() >= qty) {
                return true;
            } else {
                count += cur.second->quantity();
            }
        }
    }

    return (count >= qty);
}

bool Inventory::ContainsTypeQtyByFlag(uint16 typeID, EVEItemFlags flag, uint32 qty) const
{
    uint32 count(0);
    std::vector<InventoryItemRef> itemVec;
    if (GetItemsByFlag(flag, itemVec) < 1)
        return false;

    for (auto cur : itemVec) {
        if (cur->typeID() == typeID) {
            if (cur->quantity() >= qty) {
                return true;
            } else {
                count += cur->quantity();
            }
        }
    }

    return (count >= qty);
}

/**
 * This function does essentially the same as ContainsTypeQtyByFlag, but it's criteria is based on finding at least 1 stack that
 * follows >= rule (while ContainsTypeQtyByFlag calculates total amounts for all items in container).
 * @param typeID - TypeID to search for
 * @param flag - Location flag
 * @param qty - Required quantity
 * @return ItemID for located entity, 0 if we didn't find it.
 */
int Inventory::ContainsTypeStackQtyByFlag(uint16 typeID, EVEItemFlags flag, uint32 qty) const
{
    std::vector<InventoryItemRef> itemVec;
    if (GetItemsByFlag(flag, itemVec) < 1)
        return 0;

    for (auto cur : itemVec) {
        if (cur->typeID() == typeID && cur->quantity() >= qty) {
            return cur->itemID();
        }
    }

    return 0;
}

bool Inventory::ContainsTypeByFlag(uint16 typeID, EVEItemFlags flag) const
{
    std::vector<InventoryItemRef> itemVec;
    if (GetItemsByFlag(flag, itemVec) < 1)
        return false;

    for (auto cur : itemVec)
        if (cur->typeID() == typeID)
            return true;

    return false;
}


void Inventory::StackAll(EVEItemFlags locFlag, uint32 ownerID/*0*/)
{
    InventoryItemRef iRef(nullptr);
    std::vector<InventoryItemRef> delVec;
    std::map<uint16, InventoryItemRef> types;
    std::map<uint16, InventoryItemRef>::iterator tItr = types.end();

    auto range = m_contentsByFlag.equal_range(locFlag);
    for (auto itr = range.first; itr != range.second; ++itr) {
        iRef = itr->second;
        // check to avoid removing modules (and their charges) from ship
        if (IsModuleSlot(iRef->flag()))
            continue;
        // singletons dont stack
        if (iRef->isSingleton())
            continue;
        if ((ownerID == 0) or (ownerID == iRef->ownerID())) {
            tItr = types.find(iRef->typeID());
            if (tItr == types.end()) {
                // insert type into map for later comparison (existing stack to merge into)
                types.emplace(iRef->typeID(), iRef);
            } else {
                // found another stack of this type.
                delVec.push_back(iRef);
                // fake merge as calling II::Merge() will invalidate iterator
                tItr->second->SetQuantity(tItr->second->quantity() + iRef->quantity(), true, false);
            }
        }
    }

    for (auto cur : delVec)
        cur->Delete();
}

float Inventory::GetStoredVolume(EVEItemFlags flag, bool combined/*true*/) const
{
    float totalVolume(0.0f);
    if (IsHangarFlag(flag) and combined) {
        for (auto cur : mContents)
            if (IsHangarFlag(cur.second->flag()))
                totalVolume += cur.second->quantity() * cur.second->GetAttribute(AttrVolume).get_float();
    } else {
        auto range = m_contentsByFlag.equal_range(flag);
        for ( auto itr = range.first; itr != range.second; ++itr )
            totalVolume += itr->second->quantity() * itr->second->GetAttribute(AttrVolume).get_float();
            // This formula is a hybrid of both old and new ones...and it works \o/
    }
    return totalVolume;
}

bool Inventory::HasAvailableSpace(EVEItemFlags flag, InventoryItemRef iRef) const {
    float capacity(GetRemainingCapacity(flag));
    float volume(iRef->quantity() * iRef->GetAttribute(AttrVolume).get_float());

    if (is_log_enabled(INV__CAPY))
        _log(INV__CAPY, "Inventory::HasAvailableSpace() - Testing %s's %s available capy of %.2f to add %u %s at %.2f (%.3f each)", \
                m_self->name(), sDataMgr.GetFlagName(flag), capacity, iRef->quantity(), iRef->name(), \
                volume, iRef->GetAttribute(AttrVolume).get_float());

    // check capy for all units
    if (volume > capacity)
        return false;

    return true;
}

float Inventory::GetCapacity(EVEItemFlags flag) const {
    // added hangar capy for all hangar types
    // are we missing any hangar types here?  POS types?  yes...see next line
    /** @todo  finish these for POS */
    //   IsFlagCapacityLocationWide   item.groupID in (const.groupCorporateHangarArray, const.groupAssemblyArray, const.groupMobileLaboratory):

    switch (flag) {
        case flagOffice:
        case flagProperty:
        //case flagDelivery:
        case flagImpounded:
        case flagCorpMarket:                    return maxHangarCapy;
        case flagNone:
        case flagCargoHold:                     return m_self->GetAttribute(AttrCapacity).get_float();
        case flagDroneBay:                      return m_self->GetAttribute(AttrDroneCapacity).get_float();
        case flagShipHangar:                    return m_self->GetAttribute(AttrShipMaintenanceBayCapacity).get_float();
        case flagSecondaryStorage:              return m_self->GetAttribute(AttrSecondaryCapacity).get_float();
        case flagFuelBay:                       return m_self->GetAttribute(AttrFuelBayCapacity).get_float();
        case flagOreHold:                       return m_self->GetAttribute(AttrOreHoldCapacity).get_float();
        case flagGasHold:                       return m_self->GetAttribute(AttrGasHoldCapacity).get_float();
        case flagAmmoHold:                      return m_self->GetAttribute(AttrAmmoHoldCapacity).get_float();
        case flagShipHold:                      return m_self->GetAttribute(AttrShipHoldCapacity).get_float();
        case flagMineralHold:                   return m_self->GetAttribute(AttrMineralHoldCapacity).get_float();
        case flagSalvageHold:                   return m_self->GetAttribute(AttrSalvageHoldCapacity).get_float();
        case flagSmallShipHold:                 return m_self->GetAttribute(AttrSmallShipHoldCapacity).get_float();
        case flagMediumShipHold:                return m_self->GetAttribute(AttrMediumShipHoldCapacity).get_float();
        case flagLargeShipHold:                 return m_self->GetAttribute(AttrLargeShipHoldCapacity).get_float();
        case flagIndustrialShipHold:            return m_self->GetAttribute(AttrIndustrialShipHoldCapacity).get_float();
        // for PI
        case flagCommandCenterHold:             return m_self->GetAttribute(AttrCommandCenterHoldCapacity).get_float();
        case flagPlanetaryCommoditiesHold:      return m_self->GetAttribute(AttrPlanetaryCommoditiesHoldCapacity).get_float();
        // for pos battery/array
        case flagHiSlot0:                       return m_self->GetAttribute(AttrAmmoCapacity).get_float();
        // still not sure where/why this is used....
        case flagQuafeBay:                      return m_self->GetAttribute(AttrQuafeHoldCapacity).get_float();
        // these can be in station OR on ship....
        case flagCorpHangar2:
        case flagCorpHangar3:
        case flagCorpHangar4:
        case flagCorpHangar5:
        case flagCorpHangar6:
        case flagCorpHangar7: {
            if (sDataMgr.IsStation(m_myID))
                return maxHangarCapy;
            //for ship, this is TOTAL capy for all corp hangars (they share capy)
            if (m_self->HasAttribute(AttrHasCorporateHangars))
                return m_self->GetAttribute(AttrCorporateHangarCapacity).get_float();
        } break;
        case flagHangar: {
            if (sDataMgr.IsStation(m_myID))
                return maxHangarCapy;
            if (m_self->HasAttribute(AttrHasCorporateHangars))
                return m_self->GetAttribute(AttrCorporateHangarCapacity).get_float();
            //for cargo container, this is 27k5m3.
            return m_self->GetAttribute(AttrCapacity).get_float();
        } break;
    }

    _log(INV__WARNING, "Inventory::GetCapacity() - Unsupported flag %s(%u) called for %s(%u)", \
                sDataMgr.GetFlagName(flag), flag, m_self->name(), m_myID);
    return 0.0f;
}

// Validate an IHub Upgrade and trigger it's activation
bool Inventory::ValidateIHubUpgrade(InventoryItemRef iRef) const {
    // Get days since claim for strategic development index
    SovereigntyData sovData = svDataMgr.GetSovereigntyData(m_self->locationID());
    double daysSinceClaim = (GetFileTimeNow() - sovData.claimTime) / Win32Time_Day;

    // Get client who currently owns the item
    Client* pClient(sItemFactory.GetUsingClient());
    //Client* pClient = sEntityList.FindClientByCharID(iRef->ownerID());

    switch (iRef->typeID()) {
        case EVEDB::invTypes::UpgCynosuralNavigation:
            if (daysSinceClaim > 2.0f) {
                break;
            } else {
                _log(INV__WARNING, "Inventory::ValidateIHubUpgrade() - Upgrade %s requires a higher development index.", iRef->name());
                throw CustomError("%s requires a development index of 2", iRef->name());
            }
            break;
        case EVEDB::invTypes::UpgCynosuralSuppression:
        case EVEDB::invTypes::UpgAdvancedLogisticsNetwork:
            if (daysSinceClaim > 3.0f) {
                break;
            } else {
                _log(INV__WARNING, "Inventory::ValidateIHubUpgrade() - Upgrade %s requires a higher development index.", iRef->name());
                throw CustomError("%s requires a development index of 3", iRef->name());
            }
            break;
        default:
            _log(INV__WARNING, "Inventory::ValidateIHubUpgrade() - Upgrade %s is not supported currently.", iRef->name());
            throw CustomError("%s is not currently supported.", iRef->name());
    }
    if (m_self->GetMyInventory()->ContainsItem(iRef->itemID())) {
        _log(INV__WARNING, "Inventory::ValidateIHubUpgrade() - Upgrade %s is already installed.", iRef->name());
        throw CustomError("%s is already installed.", iRef->name());
    }

    // If we didn't hit anything above, it must be okay to insert item
    return true;
}

bool Inventory::ValidateAddItem(EVEItemFlags flag, InventoryItemRef iRef) const
{
    if (m_self->typeID() == EVEDB::invTypes::InfrastructureHub)
        if (iRef->categoryID() == EVEDB::invCategories::StructureUpgrade)
            return ValidateIHubUpgrade(iRef);

    // i dont think we need to check shit in stations...yet
    if (m_self->categoryID() == EVEDB::invCategories::Station)
        return true;

    Client* pClient(sItemFactory.GetUsingClient());

    // check that we're close enough if a container in space
    if (m_self->groupID() == EVEDB::invGroups::Cargo_Container && sDataMgr.IsSolarSystem(m_self->locationID())) {
        GVector direction (m_self->position(), pClient->GetShip()->position());
        float maxDistance = 2500.0f;

        if (m_self->HasAttribute (AttrMaxOperationalDistance) == true)
            maxDistance = m_self->GetAttribute (AttrMaxOperationalDistance).get_float ();

        if (direction.length() > maxDistance)
            throw UserError ("NotCloseEnoughToAdd")
                    .AddAmount ("maxdist", maxDistance);
    }

    // check if where the item is coming from was a cargo container
    InventoryItemRef cRef = sItemFactory.GetItemRef(iRef->locationID());
    if (cRef->groupID() == EVEDB::invGroups::Cargo_Container && sDataMgr.IsSolarSystem(cRef->locationID())) {
        GVector direction (cRef->position(), pClient->GetShip()->position());
        float maxDistance(2500.0f);
        if (cRef->HasAttribute (AttrMaxOperationalDistance) == true)
            maxDistance = cRef->GetAttribute(AttrMaxOperationalDistance).get_float ();

        if (direction.length() > maxDistance)
            throw UserError("NotCloseEnoughToLoot")
                .AddAmount("maxdist", maxDistance);
    }
    // can this be coded to check weapon capy?   im sure it can. just a flag, right?

    float capacity = GetRemainingCapacity(flag);
    float volume = iRef->GetAttribute(AttrVolume).get_float();
    float totalVolume = iRef->quantity() * volume;

    _log(INV__CAPY, "Inventory::ValidateAddItem() - Testing %s's %s available capy of %.2f to add %i %s at %.2f (%.3f each)",
         m_self->name(), sDataMgr.GetFlagName(flag), capacity, iRef->quantity(), iRef->name(), totalVolume, volume);

    /** modify checks for splitting items in same container or moving items between a container's corp hangars
     * flag and iRef->flag() will be same (or same type).
     * add requested move volume to container's available capy before other checks
     *   this will show item being removed from container to allow subsequent additions
     */
    if (m_self->itemID() == iRef->locationID())
        if ((flag == iRef->flag())
        or  (IsHangarFlag(flag) and IsHangarFlag(iRef->flag()))) {
            // possible item split.  will have to molest the shit outta this one to verify nullablity of exploits
            capacity += totalVolume;
            _log(INV__CAPY, "Inventory::ValidateAddItem() - flag() %s (%u) == iRef->flag() %s (%u) - test capacity changed to %.2f",
                    sDataMgr.GetFlagName(flag), flag, sDataMgr.GetFlagName(iRef->flag()), iRef->flag(), capacity);
    }

    // check capy for single unit
    if (capacity < volume) { // smallest volume is 0.0025
        if (pClient != nullptr) {
            std::map<std::string, PyRep *> args;
            args["volume"] = new PyFloat(volume);
            sItemFactory.UnsetUsingClient();
            if (IsCargoHoldFlag(flag))
                throw UserError ("NotEnoughCargoSpace")
                        .AddAmount ("volume", volume)
                        .AddAmount ("available", capacity);
            else if (flag == flagShipHangar)
                throw UserError ("NotEnoughCargoSpaceFor1Unit")
                        .AddAmount ("volume", volume)
                        .AddFormatValue ("type", new PyInt (iRef->itemID ()))
                        .AddAmount ("required", volume)
                        .AddAmount ("free", capacity);
            else if (IsSpecialHoldFlag(flag))
                throw UserError ("NotEnoughSpecialBaySpaceOverload")
                        .AddAmount ("volume", volume)
                        .AddFormatValue ("item", new PyInt (iRef->itemID ()))
                        .AddAmount ("maximum", GetCapacity (flag))
                        .AddAmount ("user", GetStoredVolume (flag));
            else if (IsModuleSlot(flag))
                throw UserError ("NotEnoughChargeSpace")
                        .AddAmount ("volume", volume)
                        .AddAmount ("capacity", GetCapacity (flag));
            else if (IsHangarFlag(flag))
                throw UserError ("NotEnoughSpaceOverload")
                        .AddAmount ("volume", volume)
                        .AddLocationName ("item", iRef->itemID ())
                        .AddAmount ("maximum", GetCapacity (flag))
                        .AddAmount ("user", GetStoredVolume (flag));
            else if (flag == flagDroneBay)
                throw UserError ("NotEnoughDroneBaySpaceOverload")
                        .AddAmount ("volume", volume)
                        .AddLocationName ("item", iRef->itemID ())
                        .AddAmount ("maximum", GetCapacity (flag))
                        .AddAmount ("used", GetStoredVolume (flag));
            else
                throw UserError ("NoSpaceForThatOverload")
                        .AddAmount ("volume", volume)
                        .AddFormatValue ("item", new PyInt (iRef->itemID ()))
                        .AddAmount ("maximum", GetCapacity (flag))
                        .AddAmount ("used", GetStoredVolume (flag));
        }
        return false;
    }

    // check capy for all units
    if (totalVolume > capacity) {
        if (IsSpecialHoldFlag(flag))
            throw UserError ("NotEnoughSpecialBaySpace")
                    .AddAmount ("volume", totalVolume)
                    .AddAmount ("available", capacity);
        else if (flag == flagDroneBay)
            throw UserError ("NotEnoughDroneBaySpace")
                    .AddAmount ("volume", totalVolume)
                    .AddAmount ("available", capacity);
        else if (IsHangarFlag(flag))
            throw UserError ("NotEnoughCargoSpaceOverload")
                    .AddAmount ("volume", totalVolume)
                    .AddLocationName ("item", iRef->itemID ())
                    .AddAmount ("maximum", GetCapacity (flag))
                    .AddAmount ("used", GetStoredVolume (flag));
        else if (IsCargoHoldFlag(flag))
            throw UserError ("NotEnoughCargoSpace")
                    .AddAmount ("volume", totalVolume)
                    .AddAmount ("available", capacity);
        else
            throw UserError ("NoSpaceForThat")
                    .AddAmount ("volume", totalVolume)
                    .AddFormatValue ("itemTypeName", new PyInt (iRef->itemID ()))
                    .AddAmount ("itemVolume", totalVolume)
                    .AddAmount ("volumeAvailable", capacity);
        return false;
    }

    return true;
}

// multimerge - NotEnoughCargoSpace', 'NotEnoughCargoSpaceOverload', 'NotEnoughDroneBaySpace', 'NotEnoughDroneBaySpaceOverload',
// 'NoSpaceForThat', 'NoSpaceForThatOverload', 'NotEnoughChargeSpace'):

// add - NotEnoughCargoSpace', 'NotEnoughCargoSpaceOverload', 'NotEnoughDroneBaySpace', 'NotEnoughDroneBaySpaceOverload',
// 'NoSpaceForThat', 'NoSpaceForThatOverload', 'NotEnoughChargeSpace', 'NotEnoughSpecialBaySpace', 'NotEnoughSpecialBaySpaceOverload',
//  'NotEnoughSpace'):

//{'FullPath': u'UI/Messages', 'messageID': 259240, 'label': u'NotEnoughSpaceBody'}(u'{[numeric]volume} {[numeric]volume -> "unit", "units"} of space would be required to complete this operation. Destination container only has {[numeric]available} {[numeric]available-> "unit", "units"} available.', None, {u'{[numeric]volume}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'volume'}, u'{[numeric]volume -> "unit", "units"}': {'conditionalValues': [u'unit', u'units'], 'variableType': 9, 'propertyName': None, 'args': 320, 'kwargs': {}, 'variableName': 'volume'}, u'{[numeric]available-> "unit", "units"}': {'conditionalValues': [u'unit', u'units'], 'variableType': 9, 'propertyName': None, 'args': 320, 'kwargs': {}, 'variableName': 'available'}, u'{[numeric]available}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'available'}})

//{'FullPath': u'UI/Messages', 'messageID': 259447, 'label': u'ItemMoveGoesThroughFullCargoHoldBody'}(u'You fail to move the {[item]itemType.name} as it gets invisibly moved to your cargo hold before it gets moved elsewhere and your cargo hold is full.', None, {u'{[item]itemType.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'itemType'}})

//{'FullPath': u'UI/Messages', 'messageID': 259195, 'label': u'NotPermittedToAddCharBody'}(u'You cannot put items into that container because it belongs to {[character]owner.name} and you are not in their fleet, their corporation or held in the highest personal standing by them.', None, {u'{[character]owner.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'owner'}})

//{'FullPath': u'UI/Messages', 'messageID': 259161, 'label': u'NotEnoughCargoSpaceBody'}(u'{[numeric]volume} {[numeric]volume -> "cargo unit", "cargo units"} would be required to complete this operation. Destination container only has {[numeric]available} {[numeric]available -> "unit", "units"} available.', None, {u'{[numeric]volume}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'volume'}, u'{[numeric]volume -> "cargo unit", "cargo units"}': {'conditionalValues': [u'cargo unit', u'cargo units'], 'variableType': 9, 'propertyName': None, 'args': 320, 'kwargs': {}, 'variableName': 'volume'}, u'{[numeric]available}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'available'}, u'{[numeric]available -> "unit", "units"}': {'conditionalValues': [u'unit', u'units'], 'variableType': 9, 'propertyName': None, 'args': 320, 'kwargs': {}, 'variableName': 'available'}})

//{'FullPath': u'UI/Messages', 'messageID': 257335, 'label': u'NotEnoughSpecialBaySpaceBody'}(u'{volume} units would be required to complete this operation. Destination bay only has {available} units available.', None, {u'{available}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'available'}, u'{volume}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'volume'}})
//{'FullPath': u'UI/Messages', 'messageID': 257337, 'label': u'NotEnoughSpecialBaySpaceOverloadBody'}(u'That cargo bay is overloaded and cannot be made to fit {item}. It is currently only capable of fitting {maximum} units and it is currently jammed full with {used}units.', None, {u'{maximum}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'maximum'}, u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{used}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'used'}})
//{'FullPath': u'UI/Messages', 'messageID': 257338, 'label': u'CannotStoreDestinationRestrictedBody'}(u'You cannot place a {item} into that location in your {[item]ship.name}. The bay you are trying to access is specialized and can only handle certain types of items.', None, {u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{[item]ship.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'ship'}})
//{'FullPath': u'UI/Messages', 'messageID': 259154, 'label': u'NoSpaceForThatBody'}(u"You can't add the {[item]itemTypeName.name} as there simply isn't enough room for it to fit. It takes up {[numeric]itemVolume, decimalPlaces=2} units of volume, and there are only {[numeric]volumeAvailable} left.", None, {u'{[item]itemTypeName.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'itemTypeName'}, u'{[numeric]volumeAvailable}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'volumeAvailable'}, u'{[numeric]itemVolume, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'itemVolume'}})
//{'FullPath': u'UI/Messages', 'messageID': 259141, 'label': u'NotEnoughCargoSpaceFor1UnitBody'}(u'One unit of {[item]type.name} would take {[numeric]required, decimalPlaces=2} units of space. This container only has {[numeric]free, decimalPlaces=2} units free.', None, {u'{[numeric]free, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'free'}, u'{[item]type.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'type'}, u'{[numeric]required, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'required'}})
//{'FullPath': u'UI/Messages', 'messageID': 259208, 'label': u'NotEnoughCargoSpaceOverloadBody'}(u'The cargo bay is overloaded and cannot be made to fit {item}. It is currently only capable of fitting {maximum} units and it is currently jammed full with {used} units.', None, {u'{maximum}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'maximum'}, u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{used}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'used'}})
//{'FullPath': u'UI/Messages', 'messageID': 259191, 'label': u'NotEnoughDroneBaySpaceBody'}(u'{[numeric]volume} units would be required to complete this operation. Destination container only has {[numeric]available} units available.', None, {u'{[numeric]volume}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'volume'}, u'{[numeric]available}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'available'}})
//{'FullPath': u'UI/Messages', 'messageID': 259210, 'label': u'NotEnoughDroneBaySpaceOverloadBody'}(u'The drone bay is overloaded and cannot be made to fit {item}. It is currently only capable of fitting {maximum} units and it is currently jammed full with {used} units.', None, {u'{maximum}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'maximum'}, u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{used}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'used'}})
//{'FullPath': u'UI/Messages', 'messageID': 259213, 'label': u'NotEnoughSpaceOverloadBody'}(u'The storage area is overloaded and cannot be made to fit any {item}. It is currently only capable of fitting {maximum} units and it is currently jammed full with {used} units.', None, {u'{maximum}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'maximum'}, u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{used}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'used'}})
//{'FullPath': u'UI/Messages', 'messageID': 259183, 'label': u'NoSpaceForThatOverloadBody'}(u"You can't add the {[item]item.name} as there simply isn't enough room for it to fit. The container is currently only capable of fitting {[numeric]maximum} units and it is currently jammed full with {[numeric]used} units.", None, {u'{[numeric]used}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'used'}, u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{[numeric]maximum}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'maximum'}})
