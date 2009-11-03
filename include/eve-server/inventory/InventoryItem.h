/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2008 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
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

extern const uint32 SKILL_BASE_POINTS;

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
    bool Populate(Rsp_CommonGetInfo_Entry &into) const;

    PyPackedRow* GetItemRow() const;
    void GetItemRow( PyPackedRow* into ) const;

    PyObject *ItemGetInfo() const;

    /*
     * Public Fields:
     */
    uint32                  itemID() const { return(m_itemID); }
    const std::string &     itemName() const { return(m_itemName); }
    const ItemType &        type() const { return(m_type); }
    uint32                  ownerID() const { return(m_ownerID); }
    uint32                  locationID() const { return(m_locationID); }
    EVEItemFlags            flag() const { return(m_flag); }
    bool                    contraband() const { return(m_contraband); }
    bool                    singleton() const { return(m_singleton); }
    uint32                  quantity() const { return(m_quantity); }
    const GPoint &          position() const { return(m_position); }
    const std::string &     customInfo() const { return(m_customInfo); }


    // helper type methods
    uint32                  typeID() const { return(type().id()); }
    const ItemGroup &       group() const { return(type().group()); }
    uint32                  groupID() const { return(type().groupID()); }
    const ItemCategory &    category() const { return(type().category()); }
    EVEItemCategories       categoryID() const { return(type().categoryID()); }

    /*
     * Attribute access:
     */
    ItemAttributeMgr attributes;

    /*
     * Redirections
     */
    #define ATTRFUNC(name, type) \
        inline type name() const { \
            return(attributes.name()); \
        } \
        inline void Set_##name(const type &value) { \
            attributes.Set_##name(value); \
        } \
        inline void Set_##name##_persist(const type &value) { \
            attributes.Set_##name##_persist(value); \
        } \
        inline void Clear_##name() { \
            attributes.Clear_##name(); \
        }
    #define ATTRI(ID, name, default_value, persistent) \
        ATTRFUNC(name, int)
    #define ATTRD(ID, name, default_value, persistent) \
        ATTRFUNC(name, double)
    #include "EVEAttributes.h"

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

    void SaveItem() const;  //save the item to the DB.

    void SendItemChange(uint32 toID, std::map<int32, PyRep *> &changes) const;
    void SetOnline(bool newval);

    /*
     * Member variables
     */
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
