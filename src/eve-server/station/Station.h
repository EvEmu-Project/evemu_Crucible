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

#ifndef __STATION__H__INCL__
#define __STATION__H__INCL__

#include "StaticDataMgr.h"
#include "inventory/ItemType.h"
#include "system/Celestial.h"

/**
 * Type of station.
 */
class StationType
: public ItemType
{
    friend class ItemType; // to let it construct us
public:

    static StationType *Load(uint16 stationTypeID);

protected:
    StationType(uint16 _id, const Inv::TypeData &_data);

    /*
     * Member functions:
     */
    using ItemType::_Load;

    // Template loader:
    template<class _Ty>
    static _Ty *_LoadType(uint16 stationTypeID, const Inv::TypeData &data)
    {
        if (data.groupID != EVEDB::invGroups::Station) {
            _log(ITEM__ERROR, "Trying to load %s as StationType.", sDataMgr.GetGroupName(data.groupID));
            if (sConfig.debug.StackTrace)
                EvE::traceStack();
            return nullptr;
        }

        return new StationType(stationTypeID, data);
    }
};

/**
 * CelestialObject which represents station.
 */
class StationItem
: public CelestialObject
{
    friend class InventoryItem; // to let it construct us
    friend class CelestialObject; // to let it construct us

public:
    /**
     * Loads station.
     *
     * @param[in] factory
     * @param[in] stationID ID of station to load.
     * @return Pointer to new Station object; NULL if fails.
     */
    static StationItemRef Load( uint32 stationID);
    static StationItemRef Spawn( ItemData &data);

    StationType* GetStationType() { return &m_stationType; }
    ShipItemRef GetShipFromInventory(uint32 shipID);
    CargoContainerRef GetContainerFromInventory(uint32 contID);

    // station methods here for offices, reprocessing, and docking.
    PyRep* GetOffices()                                 { PyIncRef(m_officePyData); return m_officePyData; }  // cached officeData for client call
    int8 GetAvalibleOfficeCount()                       { return maxRentableOffices - m_officeMap.size(); }
    int32 GetOfficeRentalFee()                          { return m_data.officeRentalFee; }
    uint32 GetOwnerID()                                 { return m_data.corporationID; }
    uint32 GetID()                                      { return m_data.stationID; }
    void RentOffice(OfficeData& odata);
    uint32 GetOfficeID(uint32 corpID);

    void LoadStationOffice(uint32 corpID);
    void AddLoadedOffice(uint32 officeID);
    void RemoveLoadedOffice(uint32 officeID);

    bool IsLoaded()                                     { return m_loaded; }
    bool IsOfficeLoaded(uint32 officeID);

    void GetGuestList(std::vector<Client*>& cVec);
    void AddGuest(Client* pClient);
    void RemoveGuest(Client* pClient);

    void GetRefineData(uint32& stationCorpID, float& staEfficiency, float& tax);

    // does client have a ship in this station?
    bool HasShip(Client* pClient);

    // will need methods/table for updated station data

protected:
    StationItem(uint32 stationID, const StationType &type, const ItemData &data, const CelestialObjectData &cData);
    virtual ~StationItem();
    /*
     * Member functions:
     */
    using InventoryItem::_Load;
    virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 stationID, const ItemType &type, const ItemData &data)
    {
        if (type.groupID() != EVEDB::invGroups::Station) {
            _log(ITEM__ERROR, "Trying to load %s as Station.", sDataMgr.GetGroupName(type.groupID()));
            if (sConfig.debug.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>(nullptr);
        }
        // cast the type
        const StationType &stType = static_cast<const StationType &>( type );

        // load celestial data
        CelestialObjectData cData = CelestialObjectData();
        if (!SystemDB::GetCelestialObjectData(stationID, cData))
            return RefPtr<_Ty>(nullptr);

        return StationItemRef(new StationItem(stationID, stType, data, cData));
    }

    static uint32 CreateItemID( ItemData &data);

    // internal office methods
    void SendBill();
    void ImpoundOffice(uint32 officeID);
    void RecoverOffice(uint32 officeID);

private:
    PyRep*                                              m_officePyData;
    StationType                                         m_stationType;
    StationData                                         m_data;

    bool                                                m_loaded;  // are offices loaded?
    uint32                                              m_stationID;

    std::map<uint32, Client*>                           m_guestList; // charID/Client*

    std::map<uint32, OfficeData>                        m_officeMap;   // officeID/data
    std::map<uint32, bool>                              m_officeLoaded;

};


/**
 * StaticSystemEntity which represents Station object in space
 */
class EVEServiceManager;
class SystemManager;

class StationSE
: public StaticSystemEntity
{
public:
    StationSE(StationItemRef station, EVEServiceManager &services, SystemManager* system);
    virtual ~StationSE()                                { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual StationSE* GetStationSE()                   { return this; }
    /* Static */
    virtual bool IsStationSE()                          { return true; }

    /* virtual functions to be overridden in derived classes */
    //virtual void Process();
    virtual PyDict* MakeSlimItem();
    virtual void EncodeDestiny( Buffer& into );

    /* specific functions handled here. */
    void UnloadStation();

};

#endif /* !__STATION__H__INCL__ */


