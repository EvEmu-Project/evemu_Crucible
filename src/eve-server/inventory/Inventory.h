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
    Author:        Bloody.Rabbit
    Updates:    Allan
*/

#ifndef __INVENTORY__H__INCL__
#define __INVENTORY__H__INCL__


#include "inventory/InventoryDB.h"


class CRowSet;

/* this class is content management for items that can contain other items */
class Inventory
{
    friend class InventoryItem;
public:
    Inventory(InventoryItemRef iRef);
    virtual ~Inventory() noexcept                       { /* do nothing here*/ }

    void Reset();
    void Unload();  // used by stations and solar systems for item saving and unloading
    void AddItem(InventoryItemRef iRef);
    void RemoveItem(InventoryItemRef iRef);
    void DeleteContents();
    // returns map of mContents
    void GetInventoryMap(std::map<uint32, InventoryItemRef> &invMap);
    // this method also sorts in order - cargo, modules, charge, subsystems.
    void GetInventoryVec(std::vector<InventoryItemRef> &itemVec);
    void StackAll(EVEItemFlags flag, uint32 ownerID = 0);

    bool IsEmpty()                                      { return mContents.empty(); }
    bool LoadContents();
    // this checks for available space for iRef by flag
    bool ValidateAddItem(EVEItemFlags flag, InventoryItemRef iRef) const;// this will throw if it fails.
    // this checks for available space for iRef by flag
    bool HasAvailableSpace(EVEItemFlags flag, InventoryItemRef iRef) const;   //  this will not throw
    bool ContentsLoaded() const                         { return mContentsLoaded; }
    bool ContainsItem(uint32 itemID) const              { return mContents.find( itemID ) != mContents.end(); }
    // this checks all contents, not particular holds/hangars/items
    bool ContainsTypeQty(uint16 typeID, uint32 qty=0) const;
    bool ContainsTypeQtyByFlag(uint16 typeID, EVEItemFlags flag=flagNone, uint32 qty=0) const;
    bool ContainsTypeByFlag(uint16 typeID, EVEItemFlags flag=flagNone) const;

    float GetCapacity(EVEItemFlags flag) const;
    float GetStoredVolume(EVEItemFlags flag, bool combined=true) const;
    float GetCorpHangerCapyUsed() const;
    float GetRemainingCapacity(EVEItemFlags flag) const { return GetCapacity( flag ) - GetStoredVolume( flag ); }

    InventoryItemRef GetByID(uint32 id) const;
    InventoryItemRef GetByTypeFlag(uint32 typeID, EVEItemFlags flag) const;

    /* Inventory-by-Flag methods */
    /** @todo update to use m_usedVolumeByFlag container? */
    bool IsEmptyByFlag(EVEItemFlags flag) const;
    bool GetSingleItemByFlag(EVEItemFlags flag, InventoryItemRef &iRef) const;
    bool GetTypesByFlag(EVEItemFlags flag, std::map<uint16, InventoryItemRef> &items);
    // for characters, ALL skills are flagSkill. we are not keeping flagSkillInTraining separate here
    uint32 GetItemsByFlag(EVEItemFlags flag, std::vector<InventoryItemRef> &items) const;
    uint32 GetItemsByFlagRange(EVEItemFlags low_flag, EVEItemFlags high_flag, std::vector<InventoryItemRef> &items) const;
    uint32 GetItemsByFlagSet(std::set<EVEItemFlags> flags, std::vector<InventoryItemRef> &items) const;
    // for characters, ALL skills are flagSkill. we are not keeping flagSkillInTraining separate here
    InventoryItemRef FindFirstByFlag(EVEItemFlags flag) const;
    InventoryItemRef GetItemByTypeFlag(uint16 typeID, EVEItemFlags flag=flagNone);

    /* Primary packet builders */
    CRowSet* List(EVEItemFlags flag, uint32 ownerID = 0) const;

    /* for station shit */
    void GetInvForOwner(uint32 ownerID, std::vector<InventoryItemRef> &items);

    /* for debug command */
    void GetCargoList(std::multimap<uint8, InventoryItemRef> &cargoMap);     // returns map of cargoFlag:iRef from mContents

protected:
    bool GetItems(OwnerData od, std::vector< uint32 >& into);
    void List(CRowSet* into, EVEItemFlags flag, uint32 ownerID=0) const;

    InventoryDB m_db;
    InventoryItemRef m_self;

private:
    bool mContentsLoaded;

    uint32 m_myID;

    std::map<EVEItemFlags, double> m_itemsByFlag;

    std::vector<InventoryItemRef> SortVector(std::vector<InventoryItemRef> &itemVec);
    std::map<uint32, InventoryItemRef> mContents;        // itemID/ItemRef
    std::multimap<uint8, InventoryItemRef> m_contentsByFlag;  // flagID/ItemRef
};

#endif /* !__INVENTORY__H__INCL__ */

