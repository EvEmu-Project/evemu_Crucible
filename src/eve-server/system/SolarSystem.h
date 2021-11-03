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

#ifndef __SOLAR_SYSTEM__H__INCL__
#define __SOLAR_SYSTEM__H__INCL__


//#include "EVEServerConfig.h"
#include "system/Celestial.h"

struct SolarSystemData;

/**
 * CelestialObject which represents solar system.
 */
class SolarSystem
: public CelestialObject
{
    friend class InventoryItem; // to let it construct us
    friend class CelestialObject; // to let it construct us
public:
    /**
     * Loads solar system from DB.
     *
     * @param[in] factory
     * @param[in] solarSystemID ID of solar system to load.
     * @return Pointer to new solar system object; NULL if failed.
     */
    static SolarSystemRef Load( uint32 solarSystemID);

    /*
     * Public Fields:
     */
    const GPoint &      minPosition() const             { return m_data.minPosition; }
    const GPoint &      maxPosition() const             { return m_data.maxPosition; }
    double              luminosity() const              { return m_data.luminosity; }

    bool                border() const                  { return m_data.border; }
    bool                fringe() const                  { return m_data.fringe; }
    bool                corridor() const                { return m_data.corridor; }
    bool                hub() const                     { return m_data.hub; }
    bool                international() const           { return m_data.international; }
    bool                regional() const                { return m_data.regional; }
    bool                constellation() const           { return m_data.constellation; }

    double              security() const                { return m_security; }
    uint32              factionID() const               { return m_data.factionID; }
    double              radius() const                  { return m_radius; }
    const std::string & securityClass() const           { return m_data.securityClass; }

    // Solar System Inventory Functions:
    void AddItemToInventory(InventoryItemRef iRef);
    void RemoveItemFromInventory(InventoryItemRef iRef);

protected:
    SolarSystem(
        uint32 _solarSystemID,
        // InventoryItem stuff:
        const ItemType &_type,
        const ItemData &_data,
        // CelestialObject stuff:
        const CelestialObjectData &_cData,
        // SolarSystem stuff:
        const SolarSystemData &_ssData
    );
    virtual ~SolarSystem();

    /*
     * Member functions:
     */
    using InventoryItem::_Load;
    virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 solarSystemID, const ItemType &type, const ItemData &data) {
        if (type.groupID() != EVEDB::invGroups::Solar_System) {
            _log(ITEM__ERROR, "Trying to load %s as SolarSystem.", sDataMgr.GetCategoryName(type.categoryID()));
            if (sConfig.debug.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>(nullptr);
        }

        // load celestial data
        CelestialObjectData cData = CelestialObjectData();
        if (!SystemDB::GetCelestialObjectData(solarSystemID, cData))
            return RefPtr<_Ty>(nullptr);

        // load solar system data
        /** @todo is this data in static data?  if not, do we continue db hit? */
        SolarSystemData ssData = SolarSystemData();
        if (!sDataMgr.GetSolarSystemData(solarSystemID, ssData))
            return RefPtr<_Ty>(nullptr);

        return SolarSystemRef(new SolarSystem(solarSystemID, type, data, cData, ssData));
    }

private:
    SolarSystemData m_data;
};

#endif /* !__SOLAR_SYSTEM__H__INCL__ */

