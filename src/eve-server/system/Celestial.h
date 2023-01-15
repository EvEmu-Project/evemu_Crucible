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
*/

#ifndef __CELESTIAL__H__INCL__
#define __CELESTIAL__H__INCL__

#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "inventory/InventoryItem.h"
#include "system/SystemEntity.h"

/**
 * InventoryItem for generic celestial object.
 */
class CelestialObject
: public InventoryItem
{
    friend class InventoryItem; // to let it construct us
public:
    CelestialObject(uint32 _celestialID, const ItemType &_type, const ItemData &_data);
    CelestialObject(uint32 _celestialID, const ItemType &_type, const ItemData &_data, const CelestialObjectData &_cData);
    virtual ~CelestialObject()                          { /* do nothing here */ }

    static CelestialObjectRef Load( uint32 celestialID);
    static CelestialObjectRef Spawn( ItemData &data);

    void Delete();

    double      radius() const                          { return m_radius; }
    double      security() const                        { return m_security; }
    uint8       celestialIndex() const                  { return m_celestialIndex; }
    uint8       orbitIndex() const                      { return m_orbitIndex; }

protected:
    using InventoryItem::_Load;
    //virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 celestialID, const ItemType &type, const ItemData &data)
    {
        if (type.categoryID() != EVEDB::invCategories::Celestial)  {
            _log(ITEM__ERROR, "Trying to load %s as Celestial.", sDataMgr.GetCategoryName(type.categoryID()));
            if (sConfig.debug.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>(nullptr);
        }

        CelestialObjectData cData = CelestialObjectData();
        if (!SystemDB::GetCelestialObjectData(celestialID, cData))
            return RefPtr<_Ty>(nullptr);

        return CelestialObjectRef(new CelestialObject(celestialID, type, data, cData));
    }

    static uint32 CreateItemID( ItemData &data);

    /* these have to be public for inventorydb to load into them. */
    double m_radius;
    double m_security;
    uint8 m_celestialIndex;
    uint8 m_orbitIndex;
};


/**
 * ItemSystemEntity which represents celestial object in space
 */
class EVEServiceManager;

class CelestialSE : public ItemSystemEntity {
public:
    CelestialSE(InventoryItemRef self, EVEServiceManager& services, SystemManager* system);
    virtual ~CelestialSE()                              { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual const CelestialSE*  GetCelestialSE()        { return this; }
    /* class type tests. */
    virtual bool                IsCelestialSE()         { return true; }

    /* SystemEntity interface */
    virtual void                MakeDamageState(DoDestinyDamageState &into);

};

class AnomalySE : public CelestialSE {
public:
    AnomalySE(CelestialObjectRef self, EVEServiceManager &services, SystemManager* system);
    virtual ~AnomalySE()                                { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual AnomalySE*          GetAnomalySE()          { return this; }
    /* class type tests. */
    /* Base */
    virtual bool                IsAnomalySE()           { return true; }

    /* SystemEntity interface */
    virtual void                EncodeDestiny( Buffer& into );

    virtual PyDict*             MakeSlimItem();
};

class WormholeSE : public CelestialSE {
public:
    WormholeSE(CelestialObjectRef self, EVEServiceManager& services, SystemManager* system);
    virtual ~WormholeSE()                               { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual WormholeSE*         GetWormholeSE()        { return this; }
    /* class type tests. */
    /* Base */
    virtual bool                IsWormholeSE()          { return true; }
    //virtual bool                GetDynamicSE()          { return this; }

    /* SystemEntity interface */
    virtual void                EncodeDestiny( Buffer& into );

    virtual PyDict*             MakeSlimItem();

private:
    int8    m_wormholeAge;
    float   m_wormholeSize;
    int64   m_expiryDate;
    uint16  m_count;
    uint16  m_dunSpawnID;
    uint16  m_nebulaType;
};

namespace WormHole {
    namespace Class {
        enum {
            Unknown1    = 0,
            Unknown2    = 1,
            Unknown3    = 2,
            Unknown4    = 3,
            Dangerous1  = 4,
            Dangerous2  = 5,
            Deadly      = 6,
            HiSec       = 7,
            LoSec       = 8,
            NullSec     = 9
        };
    }

    namespace Age {
        enum {
            New = 0,
            Adolescent = 1,
            Decaying = 2,
            Closing = 3
        };
    }

    namespace Size {
        // these are fuzzy logic
        enum {
            Full = 10,
            Reduced = 5,
            Disrupted = 1
        };
    }
}

#endif /* !__CELESTIAL__H__INCL__ */


