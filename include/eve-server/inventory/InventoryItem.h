/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
*/
#ifndef EVE_INVENTORY_ITEM_H
#define EVE_INVENTORY_ITEM_H

#include "inventory/EVEAttributeMgr.h"
#include "inventory/ItemFactory.h"
#include "inventory/ItemType.h"

class PyRep;
class PyDict;
class PyObject;

class ServiceDB;

class ItemContainer;
class Rsp_CommonGetInfo_Entry;
class ItemRowset_Row;

/*
 * NOTE:
 * this object system should somehow be merged with the SystemEntity stuff
 * and class hierarchy built from it (Client, NPC, etc..) in the system manager...
 * however, the creation and destruction time logic is why it has not been done.
 */

//extern const uint32 SKILL_BASE_POINTS;
extern const EvilNumber EVIL_SKILL_BASE_POINTS;
extern const int32 ITEM_DB_SAVE_TIMER_EXPIRY;

/*
 * Simple container for raw item data.
 */
class ItemData {
public:
    // Full + default constructor:
    ItemData( const char *_name = "", uint32 _typeID = 0, uint32 _ownerID = 0, uint32 _locationID = 0,
        EVEItemFlags _flag = flagAutoFit, bool _contraband = false, bool _singleton = false, uint32 _quantity = 0,
        const GPoint &_position = GPoint(0, 0, 0), const char *_customInfo = "");

    // Usual-item friendly constructor:
    ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, uint32 _quantity,
        const char *_customInfo = "", bool _contraband = false);

    // Singleton friendly constructor:
    ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, const char *_name = "",
        const GPoint &_position = GPoint(0, 0, 0), const char *_customInfo = "", bool _contraband = false);

    // Content:
    std::string     name;
    uint32          typeID;
    uint32          ownerID;
    uint32          locationID;
    EVEItemFlags    flag;
    bool            contraband;
    bool            singleton;
    uint32          quantity;
    GPoint          position;
    std::string     customInfo;
};

/**
 * Class which maintains generic item.
 */
class InventoryItem
: public RefObject
{
public:
    /**
     * Loads item from DB.
     *
     * @param[in] factory
     * @param[in] itemID ID of item to load.
     * @return Pointer to InventoryItem object; NULL if failed.
     */
    static InventoryItemRef Load(ItemFactory &factory, uint32 itemID);
    /**
     * Spawns new item.
     *
     * @param[in] factory
     * @param[in] data Item data of item to spawn.
     * @return Pointer to InventoryItem object; NULL if failed.
     */
    static InventoryItemRef Spawn(ItemFactory &factory, ItemData &data);

    /*
     * Primary public interface:
     */
    void Rename(const char *to);
    void ChangeOwner(uint32 new_owner, bool notify=true);
    void Move(uint32 location, EVEItemFlags flag=flagAutoFit, bool notify=true);
    void MoveInto(Inventory &new_home, EVEItemFlags flag=flagAutoFit, bool notify=true);
    bool ChangeSingleton(bool singleton, bool notify=true);
    bool AlterQuantity(int32 qty_change, bool notify=true);
    bool SetQuantity(uint32 qty_new, bool notify=true);
    void Relocate(const GPoint &pos);
    void SetCustomInfo(const char *ci);
    ItemFactory *GetItemFactory() { return &m_factory; };


    /*
     * Helper routines:
     */
    virtual void Delete();  //remove the item from the DB.
    virtual InventoryItemRef Split(int32 qty_to_take, bool notify=true);
    virtual bool Merge(InventoryItemRef to_merge, uint32 qty=0, bool notify=true);

    void PutOnline() { SetOnline(true); }
    void PutOffline() { SetOnline(false); }

    /*
     * Primary public packet builders:
     */
    bool Populate(Rsp_CommonGetInfo_Entry &into);

    PyPackedRow* GetItemRow() const;
    void GetItemRow( PyPackedRow* into ) const;

    PyObject *ItemGetInfo();

    /*
     * Public Fields:
     */
    uint32                  itemID() const      { return m_itemID; }
    const std::string &     itemName() const    { return m_itemName; }
    const ItemType &        type() const        { return m_type; }
    uint32                  ownerID() const     { return m_ownerID; }
    uint32                  locationID() const  { return m_locationID; }
    EVEItemFlags            flag() const        { return m_flag; }
    bool                    contraband() const  { return m_contraband; }
    bool                    singleton() const   { return m_singleton; }
    uint32                  quantity() const    { return m_quantity; }
    const GPoint &          position() const    { return m_position; }
    const std::string &     customInfo() const  { return m_customInfo; }


    // helper type methods
    uint32                  typeID() const      { return type().id(); }
    const ItemGroup &       group() const       { return type().group(); }
    uint32                  groupID() const     { return type().groupID(); }
    const ItemCategory &    category() const    { return type().category(); }
    EVEItemCategories       categoryID() const  { return type().categoryID(); }


    uint32 GetSaveTimerExpiry() { return m_saveTimerExpiryTime; };
    void SetSaveTimerExpiry(uint32 saveTimerExpiry) { m_saveTimerExpiryTime = saveTimerExpiry; };
    bool IsSaveTimerEnabled() { return m_saveTimer.Enabled(); };
    void EnableSaveTimer() { m_saveTimer.Start( m_saveTimerExpiryTime * 1000, true ); };   // Ensure actual time is set in milliseconds
    void DisableSaveTimer() { return m_saveTimer.Disable(); };
    bool CheckSaveTimer(bool iReset = true) { return m_saveTimer.Check( iReset ); };

    /*
     * Attribute access:
     */
    //ItemAttributeMgr attributes;

    /************************************************************************/
    /* start experimental new attribute system ( not operational )          */
    /************************************************************************/
private:
    AttributeMap mAttributeMap;
public:
    bool SetAttribute(uint32 attributeID, int num, bool notify = true);
    bool SetAttribute(uint32 attributeID, uint32 num, bool notify = true);
    bool SetAttribute(uint32 attributeID, int64 num, bool notify = true);
    bool SetAttribute(uint32 attributeID, uint64 num, bool notify = true);
    bool SetAttribute(uint32 attributeID, double num, bool notify = true);
    bool SetAttribute(uint32 attributeID, EvilNumber num, bool notify = true);
    
    EvilNumber GetAttribute(uint32 attributeID);
    EvilNumber GetAttribute(const uint32 attributeID) const;

    /*
     * HasAttribute
     *
     * returns true if this item has the attribute 'attributeID', false if it does not have this attribute
     *
     * @note this function should be used very infrequently and only for specific reasons
     */
    bool HasAttribute(uint32 attributeID);

    /**
     * SaveAttributes
     *
     * save all the attributes from a InventoryItem.
     *
     * @note this should be incorporated into the normal save function and only save when things have changes.
     */
    bool SaveAttributes();
    /************************************************************************/
    /* end experimental new attribute system                                */
    /************************************************************************/

protected:
    InventoryItem(
        ItemFactory &_factory,
        uint32 _itemID,
        // InventoryItem stuff:
        const ItemType &_type,
        const ItemData &_data);
    virtual ~InventoryItem();

    /*
     * Internal helper routines:
     */
    // Template helper:
    template<class _Ty>
    static RefPtr<_Ty> Load(ItemFactory &factory, uint32 itemID)
    {
        // static load
        RefPtr<_Ty> i = _Ty::template _Load<_Ty>( factory, itemID );
        if( !i )
            return RefPtr<_Ty>();

        // dynamic load
        if( !i->_Load() )
            return RefPtr<_Ty>();

        return i;
    }

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _Load(ItemFactory &factory, uint32 itemID)
    {
        // pull the item info
        ItemData data;
        if( !factory.db().GetItem( itemID, data ) )
            return RefPtr<_Ty>();

        // obtain type
        const ItemType *type = factory.GetType( data.typeID );
        if( type == NULL )
            return RefPtr<_Ty>();

        return _Ty::template _LoadItem<_Ty>( factory, itemID, *type, data );
    }

    // Actual loading stuff:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 itemID,
        // InventoryItem stuff:
        const ItemType &type, const ItemData &data
    );

    virtual bool _Load();

    static uint32 _Spawn(ItemFactory &factory,
        // InventoryItem stuff:
        ItemData &data
    );

    void SaveItem();  //save the item to the DB.

    void SendItemChange(uint32 toID, std::map<int32, PyRep *> &changes) const;
    void SetOnline(bool newval);

    /*
     * Member variables
     */
    // our save timer and our default countdown value
    Timer m_saveTimer;
    uint32 m_saveTimerExpiryTime;

    // our factory
    ItemFactory &       m_factory;

    // our item data:
    const uint32        m_itemID;
    std::string         m_itemName;
    const ItemType &    m_type;
    uint32              m_ownerID;
    uint32              m_locationID; //where is this item located
    EVEItemFlags        m_flag;
    bool                m_contraband;
    bool                m_singleton;
    uint32              m_quantity;
    GPoint              m_position;
    std::string         m_customInfo;
};

#endif
