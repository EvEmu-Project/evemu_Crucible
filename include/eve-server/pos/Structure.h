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
    Author:        Aknor Jaden
*/

#ifndef __STRUCTURE__H__INCL__
#define __STRUCTURE__H__INCL__

#include "inventory/Inventory.h"
#include "system/SystemEntity.h"

// TODO: We may need to create StructureTypeData and StructureType classes just as Ship.h/Ship.cpp
// has in order to load up type data specific to structures.  For now, the generic ItemType class is used.

/**
 * InventoryItem which represents Structure.
 */
class Structure
: public InventoryItem,
  public InventoryEx
{
    friend class InventoryItem;    // to let it construct us
public:
    /**
     * Loads Structure from DB.
     *
     * @param[in] factory
     * @param[in] structureID ID of Structure to load.
     * @return Pointer to Structure object; NULL if failed.
     */
    static StructureRef Load(ItemFactory &factory, uint32 structureID);
    /**
     * Spawns new Structure.
     *
     * @param[in] factory
     * @param[in] data Item data for Structure.
     * @return Pointer to new Structure object; NULL if failed.
     */
    static StructureRef Spawn(ItemFactory &factory, ItemData &data);

    /*
     * Primary public interface:
     */
    void Delete();

    double GetCapacity(EVEItemFlags flag) const;
    /*
     * _ExecAdd validation interface:
     */
    static void ValidateAddItem(EVEItemFlags flag, InventoryItemRef item, Client *c);

    /*
     * Public fields:
     */
    const ItemType &    type() const { return static_cast<const ItemType &>(InventoryItem::type()); }

    /*
     * Primary public packet builders:
     */
    PyObject *StructureGetInfo();


protected:
    Structure(
        ItemFactory &_factory,
        uint32 _structureID,
        // InventoryItem stuff:
        const ItemType &_itemType,
        const ItemData &_data
    );

    /*
     * Member functions
     */
    using InventoryItem::_Load;

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 structureID,
        // InventoryItem stuff:
        const ItemType &type, const ItemData &data)
    {
        // check if it's a structure
        if( type.categoryID() != EVEDB::invCategories::Structure )
        {
            _log( ITEM__ERROR, "Trying to load %s as Structure.", type.category().name().c_str() );
            return RefPtr<_Ty>();
        }
        //// cast the type
        //const ItemType &itemType = static_cast<const ItemType &>( type );

        // no additional stuff

        return _Ty::template _LoadStructure<_Ty>( factory, structureID, type, data );
    }

    // Actual loading stuff:
    template<class _Ty>
    static RefPtr<_Ty> _LoadStructure(ItemFactory &factory, uint32 structureID,
        // InventoryItem stuff:
        const ItemType &itemType, const ItemData &data
    );

    bool _Load();

    static uint32 _Spawn(ItemFactory &factory,
        // InventoryItem stuff:
        ItemData &data
    );

    uint32 inventoryID() const { return itemID(); }
    PyRep *GetItem() const { return GetItemRow(); }

    void AddItem(InventoryItemRef item);
};


/**
 * DynamicSystemEntity which represents structure object in space
 */
class PyServiceMgr;
class InventoryItem;
class DestinyManager;
class SystemManager;
class ServiceDB;

class StructureEntity
: public DynamicSystemEntity
{
public:
    StructureEntity(
        StructureRef structure,
        SystemManager *system,
        PyServiceMgr &services,
        const GPoint &position);

    /*
     * Primary public interface:
     */
    StructureRef GetStructureObject() { return _structureRef; }
    DestinyManager * GetDestiny() { return m_destiny; }
    SystemManager * GetSystem() { return m_system; }

    /*
     * Public fields:
     */

    inline double x() const { return(GetPosition().x); }
    inline double y() const { return(GetPosition().y); }
    inline double z() const { return(GetPosition().z); }

    //SystemEntity interface:
    virtual EntityClass GetClass() const { return(ecStructureEntity); }
    virtual bool IsStructureEntity() const { return true; }
    virtual StructureEntity *CastToStructureEntity() { return(this); }
    virtual const StructureEntity *CastToStructureEntity() const { return(this); }
    virtual void Process();
    virtual void EncodeDestiny( Buffer& into ) const;
    virtual void TargetAdded(SystemEntity *who) {}
    virtual void TargetLost(SystemEntity *who) {}
    virtual void TargetedAdd(SystemEntity *who) {}
    virtual void TargetedLost(SystemEntity *who) {}
    virtual void TargetsCleared() {}
    virtual void QueueDestinyUpdate(PyTuple **du) {/* not required to consume */}
    virtual void QueueDestinyEvent(PyTuple **multiEvent) {/* not required to consume */}
    virtual uint32 GetCorporationID() const { return(1); }
    virtual uint32 GetAllianceID() const { return(0); }
    virtual void Killed(Damage &fatal_blow);
    virtual SystemManager *System() const { return(m_system); }

    void ForcedSetPosition(const GPoint &pt);

    virtual bool ApplyDamage(Damage &d);
    virtual void MakeDamageState(DoDestinyDamageState &into) const;

    void SendNotification(const PyAddress &dest, EVENotificationStream &noti, bool seq=true);
    void SendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq=true);

protected:
    /*
     * Member functions
     */
    void _ReduceDamage(Damage &d);
    void ApplyDamageModifiers(Damage &d, SystemEntity *target);

    /*
     * Member fields:
     */
    SystemManager *const m_system;    //we do not own this
    PyServiceMgr &m_services;    //we do not own this
    StructureRef _structureRef;   // We don't own this

    /* Used to calculate the damages on NPCs
     * I don't know why, npc->Set_shieldCharge does not work
     * calling npc->shieldCharge() return the complete shield
     * So we get the values on creation and use then instead.
    */
    double m_shieldCharge;
    double m_armorDamage;
    double m_hullDamage;
};

#endif /* !__STRUCTURE__H__INCL__ */


