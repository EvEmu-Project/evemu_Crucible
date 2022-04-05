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
    Updates:    Allan
*/

#ifndef __CONTAINER__H__INCL__
#define __CONTAINER__H__INCL__


#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "inventory/Inventory.h"
#include "inventory/InventoryItem.h"
#include "system/SystemEntity.h"
#include "pos/Structure.h"


/**
 * InventoryItem which represents cargo container.
 */
class CargoContainer
: public StructureItem
{
    friend class InventoryItem;    // to let it construct us
public:
    CargoContainer(uint32 _containerID, const ItemType &_containerType, const ItemData &_data);
    virtual ~CargoContainer();

    /**
     * Loads CargoContainer from DB.
     *
     * @param[in] containerID ID of container to load.
     * @return Pointer to CargoContainer object; NULL if failed.
     */
    static CargoContainerRef Load( uint32 containerID);
    /**
     * Spawns new CargoContainer.
     *
     * @param[in] data Item data for CargoContainer.
     * @return Pointer to new CargoContainer object; NULL if failed.
     */
    static CargoContainerRef Spawn( ItemData &data);
    static CargoContainerRef SpawnTemp( ItemData &data);

    /*
     * Primary public interface:
     */
    virtual void Delete();

    double GetCapacity(EVEItemFlags flag) const;
    /*
     * _ExecAdd validation interface:
     */
    void ValidateAddItem(EVEItemFlags flag, CargoContainerRef item) const;

    virtual void RemoveItem(InventoryItemRef iRef);

    // also used for jetcans to NOT delete when empty.
    bool IsAnchored()                                   { return m_isAnchored; }
    void SetAnchor(bool set=false)                      { m_isAnchored = set; }

    /*
     * Public fields:
     */
    const ItemType &type() const                        { return InventoryItem::type(); }

    /*
     * Primary public packet builders:
     */
    PyObject *CargoContainerGetInfo();

    virtual void MakeDamageState(DoDestinyDamageState &into) const;

    bool IsEmpty()                                      { return GetMyInventory()->IsEmpty(); }
    void SetMySE(SystemEntity* pSE)                     { mySE = pSE;}

protected:
    bool m_isAnchored;

    /*
     * Member functions:
     */
    using InventoryItem::_Load;
    virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 containerID, const ItemType &type, const ItemData &data) {
        if ( (type.groupID() != EVEDB::invGroups::Cargo_Container)
            && (type.groupID() != EVEDB::invGroups::Audit_Log_Secure_Container)
            && (type.groupID() != EVEDB::invGroups::Freight_Container)
            && (type.groupID() != EVEDB::invGroups::Secure_Cargo_Container)
            && (type.groupID() != EVEDB::invGroups::Spawn_Container) )
        {
            _log(ITEM__ERROR, "Trying to load %s as Container.", sDataMgr.GetCategoryName(type.categoryID()));
            if (sConfig.debug.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>(nullptr);
        }
        return CargoContainerRef( new CargoContainer(containerID, type, data ) );
    }

    static uint32 CreateItemID( ItemData &data);

    virtual PyRep* GetItem() const                      { return GetItemRow(); }

private:
    SystemEntity* mySE;
};


/**
 * ContainerEntity which represents container object in space
 */
class PyServiceMgr;
class Item;
class DestinyManager;
class SystemManager;
class ServiceDB;

class ContainerSE
: public StructureSE
{
public:
    ContainerSE(CargoContainerRef self, PyServiceMgr &services, SystemManager *system, const FactionData& data);
    virtual ~ContainerSE();

    /* Base */
    virtual bool                isGlobal()              { return m_global; }
    /* class type pointer querys. */
    virtual ContainerSE* GetContSE()                    { return this; }
    /* class type tests. */
    virtual bool IsContainerSE()                        { return true; }

    /* SystemEntity interface */
    virtual void Process();
    virtual void MakeDamageState(DoDestinyDamageState &into);
    // this uses targetMgr update to send to all interested parties
    virtual void SendDamageStateChanged();

    /* specific functions handled in this class. */
    void Init();
    void AnchorContainer();
    bool IsEmpty()                                      { return m_contRef->IsEmpty(); }
    bool IsAnchored()                                   { return m_contRef->IsAnchored(); }

    void SetGlobal(bool set=false)                      { m_global = set; }

protected:
    CargoContainerRef m_contRef;

    Timer m_deleteTimer;

    bool m_global;      // for making bubble centers global

    double m_shieldCharge;
    double m_armorDamage;
    double m_hullDamage;
};

/**
 * InventoryItem which represents wreck container.
 * Author:  Allan
 */
class WreckContainer
: public InventoryItem
{
    friend class InventoryItem;    // to let it construct us
public:
    // put factionID in item's CustomInfo field to allow salvage
    WreckContainer(uint32 _containerID, const ItemType &_containerType, const ItemData &_data);
    virtual ~WreckContainer();

    static WreckContainerRef Load( uint32 containerID);
    static WreckContainerRef Spawn( ItemData &data);

    virtual void Delete();
    virtual void RemoveItem(InventoryItemRef iRef);

    double GetCapacity(EVEItemFlags flag) const;
    void ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const;

    const ItemType &type() const                        { return InventoryItem::type(); }

    PyObject *WreckContainerGetInfo();

    bool IsEmpty()                                      { return pInventory->IsEmpty(); }
    void MakeSlimItemChange();
    void SetMySE(SystemEntity* pSE)                     { mySE = pSE;}
    void Salvaged()                                     { m_salvaged = true; }

protected:
    using InventoryItem::_Load;
    virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 containerID, const ItemType &type, const ItemData &data) {
        if (type.groupID() != EVEDB::invGroups::Wreck) {
            _log(ITEM__ERROR, "Trying to load %s as Wreck.", sDataMgr.GetCategoryName(type.categoryID()));
            if (sConfig.debug.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>(nullptr);
        }
        return WreckContainerRef( new WreckContainer(containerID, type, data ) );
    }

    static uint32 CreateItemID( ItemData &data );

private:
    SystemEntity* mySE;
    bool m_delete;
    bool m_salvaged;
};

/**
 * DynamicSystemEntity which represents wreck object in space
 * Author:  Allan
 */

class WreckSE
: public DynamicSystemEntity
{
public:
    // put factionID in item's CustomInfo field to allow salvage
    WreckSE(WreckContainerRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~WreckSE();

    /* class type pointer querys. */
    virtual WreckSE*            GetWreckSE()            { return this; }
    /* class type tests. */
    virtual bool                IsWreckSE()             { return true; }

    /* SystemEntity interface */
    virtual void                Process();
    virtual void                EncodeDestiny(Buffer& into);
    virtual PyDict*             MakeSlimItem();
    // this uses targetMgr update to send to all interested parties
    virtual void                SendDamageStateChanged();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Abandon();

    /* specific functions handled in this class. */
    void Salvaged()                                     { m_contRef->Salvaged(); }
    void SetLaunchedByID(uint32 launcherID=0)           { m_launchedByID = launcherID; }
    bool IsEmpty()                                      { return m_contRef->IsEmpty(); }

    /** @todo (allan) finish this */
    double GetOwnerBounty()                             { return 0; }

protected:
    WreckContainerRef m_contRef;

    Timer m_deleteTimer;

    uint32 m_launchedByID;

private:
    bool m_abandoned;

};

#endif /* !__CONTAINER__H__INCL__ */


