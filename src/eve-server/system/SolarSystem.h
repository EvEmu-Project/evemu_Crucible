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


#include "EVEServerConfig.h"
#include "system/Celestial.h"

/**
 * Data container for solarsystem data.
 */
class SolarSystemData {
public:
    SolarSystemData(
        const GPoint &_minPos = GPoint(0, 0, 0),
        const GPoint &_maxPos = GPoint(0, 0, 0),
        double _luminosity = 0.0,
        bool _border = false,
        bool _fringe = false,
        bool _corridor = false,
        bool _hub = false,
        bool _international = false,
        bool _regional = false,
        bool _constellation = false,
        double _security = 0.0,
        uint32 _factionID = 0,
        double _radius = 0.0,
        uint32 _sunTypeID = 0,
        const char *_securityClass = ""
    );

    // Data members:
    GPoint minPosition;
    GPoint maxPosition;
    double luminosity;

    // use bitfield to save some memory...
    bool border :1;
    bool fringe :1;
    bool corridor :1;
    bool hub :1;
    bool international :1;
    bool regional :1;
    bool constellation :1;

    double security;
    uint32 factionID;
    double radius;
    uint32 sunTypeID;
    std::string securityClass;
};

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
    const GPoint &      minPosition() const             { return m_minPosition; }
    const GPoint &      maxPosition() const             { return m_maxPosition; }
    double              luminosity() const              { return m_luminosity; }

    bool                border() const                  { return m_border; }
    bool                fringe() const                  { return m_fringe; }
    bool                corridor() const                { return m_corridor; }
    bool                hub() const                     { return m_hub; }
    bool                international() const           { return m_international; }
    bool                regional() const                { return m_regional; }
    bool                constellation() const           { return m_constellation; }

    double              security() const                { return m_security; }
    uint32              factionID() const               { return m_factionID; }
    double              radius() const                  { return m_radius; }
    const std::string & securityClass() const           { return m_securityClass; }

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
            return RefPtr<_Ty>();
        }

        // load celestial data
        CelestialObjectData cData = CelestialObjectData();
        if (!sItemFactory.db()->GetCelestialObject(solarSystemID, cData))
            return RefPtr<_Ty>();

        // load solar system data
        /** @todo is this data in static data?  if not, do we continue db hit? */
        SolarSystemData ssData = SolarSystemData();
        if( !sItemFactory.db()->GetSolarSystem( solarSystemID, ssData ) )
            return RefPtr<_Ty>();

        return SolarSystemRef( new SolarSystem(solarSystemID, type, data, cData, ssData ) );
    }

    /*
     * Data members:
     */
    bool m_border :1;
    bool m_fringe :1;
    bool m_corridor :1;
    bool m_hub :1;
    bool m_international :1;
    bool m_regional :1;
    bool m_constellation :1;

    uint32 m_factionID;

    double m_security;
    double m_radius;
    double m_luminosity;

    std::string m_securityClass;
    GPoint m_minPosition;
    GPoint m_maxPosition;
};

#endif /* !__SOLAR_SYSTEM__H__INCL__ */

