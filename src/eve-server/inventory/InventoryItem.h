/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2021 The EVEmu Team
 *    For the latest information visit https://evemu.dev
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:     Zhur
 *    Rewrite:    Allan
 */
#ifndef EVE_INVENTORY_ITEM_H
#define EVE_INVENTORY_ITEM_H


#include "POD_containers.h"
#include "inventory/AttributeMap.h"
//#include "inventory/Inventory.h"
//#include "inventory/InventoryDB.h"
#include "inventory/ItemDB.h"
#include "inventory/ItemType.h"
#include "inventory/ItemFactory.h"

struct fxData;

class Client;
class Inventory;
class ModuleItem;
class ShipItem;

/*
 * NOTE:
 * this object system should somehow be merged with the SystemEntity stuff
 * and class hierarchy built from it (Client, NPC, etc..) in the system manager...
 * however, the creation and destruction time logic is why it has not been done.
 *
 *  NOTE:
 * this system cannot and should not be merged with SE class, as not all Items are SE.
 * doing so would cause unnecessary SEs to be created for items that are NOT SE.
 * keeping this as a separate class is cleaner and better for creation and destruction
 * for both base and derived classes.       -allan 23.2.16
 */

/*
 * Class which maintains generic Inventory item.
 */
class InventoryItem
: public RefObject
{
public:
    InventoryItem(uint32 _itemID, const ItemType &_type, const ItemData &_data);

    /* begin rewrite and update */
    // copy c'tor
    InventoryItem(const InventoryItem& oth);
    // move c'tor
    InventoryItem(InventoryItem&& oth) noexcept;
    // copy assignment
    InventoryItem& operator= (const InventoryItem& oth) =delete;
    // move assignment
    InventoryItem& operator= (InventoryItem&& oth) =delete;

    virtual ~InventoryItem() noexcept;

    /* class type pointer querys. */
    virtual ShipItem*       GetShipItem()               { return nullptr; }
    virtual ModuleItem*     GetModuleItem()             { return nullptr; }
    /* class type tests. */
    virtual bool            IsShipItem()                { return false; }
    virtual bool            IsModuleItem()              { return false; }

    virtual void            Rename(std::string name);

    /* generic access functions handled here */
    Inventory*              GetMyInventory()            { return pInventory; }

    /* common functions for all entities handled here */
    /* public data queries  */
    bool                    contraband() const          { return m_data.contraband; }
    bool                    isSingleton() const         { return m_data.singleton != 0; }
    int32                   quantity() const            { return m_data.quantity; }
    uint32                  itemID() const              { return m_itemID; }
    uint32                  ownerID() const             { return m_data.ownerID; }
    uint32                  locationID() const          { return m_data.locationID; }
    EVEItemFlags            flag() const                { return m_data.flag; }
    const GPoint &          position() const            { return m_data.position; }
    const ItemType &        type() const                { return m_type; }
    const std::string &     itemName() const            { return m_data.name; }
    const std::string &     customInfo() const          { return m_data.customInfo; }

    const char*             name()                      { return m_data.name.c_str(); }

    /* public type queries  */
    uint16                  typeID() const              { return m_type.id(); }
    uint16                  groupID() const             { return m_type.groupID(); }
    double                  radius() const              { return (HasAttribute(AttrRadius) ? GetAttribute(AttrRadius).get_double() : 1.0); }
    uint8                   categoryID() const          { return m_type.categoryID(); }
    bool                    isGlobal() const            { return (HasAttribute(AttrIsGlobal) ? GetAttribute(AttrIsGlobal).get_bool() : false); }
    bool                    IsOnline()                  { return GetAttribute(AttrOnline).get_bool(); }

    /* public-access generic functions handled in base class. */
    void                    SetPosition(const GPoint& pos);     // change coords of item
    void                    SetCustomInfo(const char *ci);
    void                    ChangeOwner(uint32 new_owner, bool notify=false);
    // remove item from old location, add to new location and (optionally) notify client of changes
    // will bcast to corp for item update (incomplete)
    void                    Move(uint32 new_location=locTemp, EVEItemFlags flag=flagNone, bool notify=false);
    // same as Move() but xfer ownership also
    // will bcast to corp for item update (incomplete)
    void                    Donate(uint32 new_owner=ownerSystem, uint32 new_location=locTemp, EVEItemFlags new_flag=flagNone, bool notify=true);
    void                    SendItemChange(uint32 toID, std::map< int32, PyRep* >& changes);
    // this is for stacking recovered probes, mined ore, and salvage in ship's cargo
    void                    MergeTypesInCargo(ShipItem* pShip, EVEItemFlags flag=flagNone);  // will test for existing types
    bool                    ChangeSingleton(bool singleton, bool notify=false);
    // this also updates volume of item
    bool                    AlterQuantity(int32 qty, bool notify=false);  // make sure to use proper sign
    bool                    SetQuantity(int32 qty, bool notify=false, bool deleteOnZero=true);
    // sets new flag, if different, saves update to db, and (optionally) notifies client of change
    bool                    SetFlag(EVEItemFlags flag, bool notify=false);
    // sets owner for player-owned npc types (drone, missile, etc)
    void                    SetOwner(uint32 ownerID)    { m_data.ownerID = ownerID; }

    /* public-access data functions handled in base class. */
    void                    SaveItem();  //save the item to the DB.
    void                    UpdateLocation();   // save item's location, owner, flag
    void                   UpdateLocation(uint32 locID) { m_data.locationID = locID; }  // change item's locationID without saving

    /* virtual functions default to base class and overridden as needed */
    virtual void            Delete();  //totally removes item from game and deletes from the DB.
    // makes new stack of 'qty', then returns ref of new stack and update client for qty change
    // notify will update client for the new stack.
    // silent will not call Move() on new stack (which is loc=0).
    // failure will return nullptr
    virtual InventoryItemRef Split(int32 qty=0, bool notify=true, bool silent=false);
    // combines stacks of identical typeIDs
    virtual bool            Merge(InventoryItemRef to_merge, int32 qty=0, bool notify=true);
    // same as Move() but doesnt remove item from previous location
    // used for moving charges to/from ship without calling Remove()
    virtual void            Relocate(uint32 locID=0, EVEItemFlags flag=flagNone);

    // add itemRef to our inventory.
    // does not move item
    virtual void            AddItem(InventoryItemRef iRef);
    // remove itemRef from our inventory.
    // does not move item
    virtual void            RemoveItem(InventoryItemRef iRef);

    /* specific functions handled here */
    /* returns uID for new item.  saves item data to db */
    static uint32           CreateItemID( ItemData &data);
    /* returns uID for temp item, without saving to db */
    static uint32           CreateTempItemID( ItemData &data);
    /* loads attributes for this item */
    //bool LoadAttributes();
    double                  GetPackagedVolume();

    /* specific functions for ShipItem, virtual here to allow generic class access */
    virtual void            SetPlayer(Client* pClient)  { /* do nothing here */ }
    virtual bool            HasPilot()                  { return false; }
    virtual Client*         GetPilot()                  { return nullptr; }

    /* specific function from real item to virtual item for PI commodity xfer */
    void                    ToVirtual(uint32 locationID);    // this deletes item without updating client with 'this is deleted' data

    /*********************************************************************************************
     *
     * TEMPLATED LOADING INVOKATION EXPLANATION:        (written by allan)
     * Item types and classes (and their children) have special loading.
     *   Every such type has following methods: (with ShipItem being the exception)
     *
     *  static Load( <identifier>):
     *    Merges static and virtual loading trees.
     *    Call to static _Load()
     *    Call object's virtual _Load()
     *
     *  static _Load( <identifier>[, <data-argument>, ...]):
     *    retreives specific item data from db for created items, or creates default data for new items.
     *    retrieves type info from static data
     *    Call static _Ty::LoadItem()
     *
     *  static _Ty::LoadItem():
     *    creates/retrieves additional item data as required
     *    call the item constructor to create object
     *
     *  virtual _Load():
     *    Performs post-construction loading (container contents) as needed,
     *    Call InventoryItem::_Load() to load the item's attributes
     *    Add the created item to it's location's inventory.
     *
     *********************************************************************************************/

    /*  Item Creating and Loading methods */
    /* calls _Ty::Load<_Ty>.  */
    static InventoryItemRef Load( uint32 itemID);
    /* creates new Item and calls item::_Load() */
    /* does not save to db.  does not add item to ItemFactory */
    static InventoryItemRef SpawnItem( uint32 itemID, const ItemData &data);
    /* Spawns new Item and saves to db, except missile, sentry, forcefields */
    static InventoryItemRef Spawn( ItemData &data);
    /* Spawns new temp Item.  not saved to db */
    static InventoryItemRef SpawnTemp( ItemData &data);     // incomplete.  returns generic SpawnItem()

    /* base class _Load() only loads attributes */
    virtual bool _Load();

protected:
    /* template helper, calls template loader (below) then class loader */
    template<class _Ty>
    static RefPtr<_Ty> Load( uint32 itemID)
    {
        // static load
        RefPtr<_Ty> i = _Ty::template _Load<_Ty>( itemID );
        if (!i)
            return RefPtr<_Ty>(nullptr);

        // virtual load (load attributes)
        if (!i->_Load())
            return RefPtr<_Ty>(nullptr);

        return i;
    }

    /* template loader, calls class _LoadItem */
    template<class _Ty>
    static RefPtr<_Ty> _Load( uint32 itemID)
    {
        // pull the specific item info from db
        ItemData data;
        if (!ItemDB::GetItemData(itemID, data))
            return RefPtr<_Ty>(nullptr);

        // obtain type
        const ItemType *type = sItemFactory.GetType( data.typeID );
        if ( type == nullptr )
            return RefPtr<_Ty>(nullptr);

        return _Ty::template _LoadItem<_Ty>( itemID, *type, data );
    }

    /* template class _LoadItem.  defined in derived class. calls class c'tor */
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 itemID, const ItemType &type, const ItemData &data);


public:
    /* Primary public packet builders  */
    PyRep*                  GetItem() const             { return GetItemRow(); }

    void                    GetItemRow( PyPackedRow* into ) const;
    void                    GetItemStatusRow( PyPackedRow* into ) const;
    void                    GetChargeStatusRow( uint32 shipID, PyPackedRow* into ) const;

    bool                    Populate(Rsp_CommonGetInfo_Entry &into);

    PyList*                 GetItemInfo() const;
    PyObject*               ItemGetInfo();
    PyPackedRow*            GetItemRow() const;
    PyPackedRow*            GetItemStatusRow() const;
    PyPackedRow*            GetChargeStatusRow(uint32 shipID) const;

protected:
    Inventory* pInventory;
    uint32 m_itemID;

private:
    bool m_delete;
    ItemData m_data;
    ItemType m_type;

/* new effects processing system */
public:
    /*  this checks this item's required skills against callers' current skills.
     *  returns true if all pass */
    bool SkillCheck(InventoryItemRef refItem);

    // this clears m_modifiers
    void ClearModifiers();
    void AddModifier(fxData &data);
    void RemoveModifier(fxData &data);
    // this deletes all attributes, reloads default attribs from itemType and
    void ResetAttributes();   //  when called at the wrong time, this will really fuck up ship attributes.  ;)

    //  if itemType requires skill(skillID) return true else return false
    bool HasReqSkill(const uint16 skillID)              { return m_type.HasReqSkill(skillID); }

    // gotta make this public for now...
    std::multimap<int8, fxData> m_modifiers;     // k,v of math, data<math, src, targLoc, targAttr, srcAttr, grpID, typeID>, ordered by key (mathMethod)


/*  new attribute system */
    AttributeMap*           GetAttributeMap()           { return pAttributeMap; }
    int64                   GetTimeStamp()              { return m_timestamp; }

    void SetAttribute(uint16 attrID, int num, bool notify=true);
    void SetAttribute(uint16 attrID, uint32 num, bool notify=true);
    void SetAttribute(uint16 attrID, int64 num, bool notify=true);
    void SetAttribute(uint16 attrID, float num, bool notify=true);
    void SetAttribute(uint16 attrID, double num, bool notify=true);
    void SetAttribute(uint16 attrID, EvilNumber num, bool notify=true);
    void MultiplyAttribute(uint16 attrID, EvilNumber num, bool notify=false);
    bool HasAttribute(const uint16 attrID) const                       { return pAttributeMap->HasAttribute(attrID); }
    bool HasAttribute(const uint16 attrID, EvilNumber &value) const    { return pAttributeMap->HasAttribute(attrID, value); }
    bool SaveAttributes()                                              { return pAttributeMap->SaveAttributes(); }
    void ResetAttribute(uint16 attrID, bool notify=false)              { pAttributeMap->ResetAttribute(attrID, notify); }
    void DeleteAttribute(uint16 attrID)                                { pAttributeMap->DeleteAttribute(attrID); }

    EvilNumber GetAttribute(const uint16 attrID) const                 { return pAttributeMap->GetAttribute(attrID); }
    EvilNumber GetDefaultAttribute(const uint16 attrID) const          { return m_type.GetAttribute(attrID); }

protected:
    AttributeMap* pAttributeMap;

private:
    // fx timestamp
    int64   m_timestamp;        // not implemented yet

};

#endif
