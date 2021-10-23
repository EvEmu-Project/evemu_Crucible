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
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "packets/CorporationPkts.h"
#include "station/Station.h"
#include "station/StationOffice.h"
#include "station/StationDataMgr.h"
#include "system/Container.h"
#include "system/DestinyManager.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"

/*
 * StationType
 */
StationType::StationType(uint16 _id, const Inv::TypeData& _data)
: ItemType(_id, _data)
{
    // consistency check
    assert(_data.id == _id);
    assert(_data.groupID == EVEDB::invGroups::Station);
}

StationType *StationType::Load(uint16 stationTypeID)
{
    return ItemType::Load<StationType>(stationTypeID);
}

/*
 * Station Item
 */
StationItem::StationItem(uint32 stationID, const StationType& type, const ItemData& data, const CelestialObjectData& cData)
: CelestialObject(stationID, type, data, cData),
m_officePyData(nullptr),
m_stationType(type),
m_stationID(stationID),
m_loaded(false)
{
    pInventory = new Inventory(InventoryItemRef(this));

    _log(ITEM__TRACE, "Created Station for item %s (%u).", name(), itemID());
}

StationItem::~StationItem()
{
    if (pInventory != nullptr)
        pInventory->Unload();
    SafeDelete(pInventory);

    m_officeMap.clear();
    m_guestList.clear();
    PySafeDecRef(m_officePyData);
}

StationItemRef StationItem::Load( uint32 stationID)
{
    return InventoryItem::Load<StationItem>(stationID);
}

StationItemRef StationItem::Spawn( ItemData &data) {
    if (data.typeID == EVEDB::invGroups::Station) {
        StationType* iType = StationType::Load(data.typeID);
        uint32 itemID = StationItem::CreateItemID(data);
        if (itemID == 0)
            return StationItemRef(nullptr);
        StationItemRef stationRef = StationItem::Load(itemID);
        if (stationRef.get() == nullptr)
            return StationItemRef(nullptr);

        stationRef->SetAttribute(AttrShieldCharge,  stationRef->GetAttribute(AttrShieldCapacity), false);     // Shield Charge
        stationRef->SetAttribute(AttrArmorDamage,   EvilZero, false);                                         // Armor Damage
        stationRef->SetAttribute(AttrMass,          iType->mass(), false);           // Mass
        stationRef->SetAttribute(AttrRadius,        iType->radius(), false);       // Radius
        stationRef->SetAttribute(AttrVolume,        iType->volume(), false);       // Volume
        stationRef->SetAttribute(AttrCapacity,      iType->capacity(), false);   // Capacity

        return stationRef;
    } else if (data.typeID == EVEDB::invGroups::Station_Services) {
        // this should never hit...throw error
        codelog(INV__ERROR, "II::Spawn called for unhandled item type %u in locID: %u.", data.typeID, data.locationID);
        return StationItemRef(nullptr);
    }
    return StationItemRef(nullptr);
}

bool StationItem::_Load() {
    if (!pInventory->LoadContents())
        return false;

    m_officeMap.clear();
    m_guestList.clear();

    stDataMgr.GetStationData(m_stationID, m_data);
    stDataMgr.LoadOffices(m_stationID, m_officeMap);
    m_officePyData = StationDB::GetOffices(m_stationID);

    if (m_data.officeRentalFee < 10000)
        m_data.officeRentalFee = 10000;

    m_loaded = CelestialObject::_Load();

    return m_loaded;
}

uint32 StationItem::CreateItemID( ItemData &data) {
    return InventoryItem::CreateItemID(data);
}

uint32 StationItem::GetOfficeID(uint32 corpID)
{
    if (!IsPlayerCorp(corpID))
        return 0;
    for (auto cur : m_officeMap)
        if (cur.second.corporationID = corpID)
            return cur.first;
    return 0;
}

void StationItem::AddLoadedOffice(uint32 officeID)
{
    if (!IsOfficeID(officeID))
        return;
    m_officeLoaded.emplace(officeID, true);
}

bool StationItem::IsOfficeLoaded(uint32 officeID)
{
    if (!IsOfficeID(officeID))
        return false;
    std::map<uint32, bool>::const_iterator itr = m_officeLoaded.find(officeID);
    if (itr != m_officeLoaded.end())
        return itr->second;
    return false;
}

void StationItem::RemoveLoadedOffice(uint32 officeID)
{
    if (!IsOfficeID(officeID))
        return;
    m_officeLoaded.erase(officeID);
}

void StationItem::LoadStationOffice(uint32 corpID)
{
    if (!IsPlayerCorp(corpID))
        return;
    uint32 officeID = GetOfficeID(corpID);
    if (officeID == 0)
        return;
    if (IsOfficeLoaded(officeID))
        return;
    _log(CORP__TRACE, "StationItem::LoadStationOffice() is loading corp office %u in stationID %u", officeID, m_stationID);
    StationOfficeRef oRef = sItemFactory.GetOffice(officeID);
    if (oRef->GetMyInventory() == nullptr) {   // not sure why this would be null, but i *may* have seen errors from it
        _log(ITEM__ERROR, "StationItem::LoadStationOffice() - GetMyInventory() for corp office %u in stationID %u is NULL.", officeID, m_stationID);
        return;
    }
    oRef->SetLoaded(oRef->GetMyInventory()->LoadContents());
    m_officeLoaded.emplace(officeID, true);
}

void StationItem::ImpoundOffice(uint32 officeID)
{
    std::map<uint32, OfficeData>::iterator itr = m_officeMap.find(officeID);
    if (itr != m_officeMap.end())
        itr->second.lockDown = true;
}

void StationItem::RecoverOffice(uint32 officeID)
{
    std::map<uint32, OfficeData>::iterator itr = m_officeMap.find(officeID);
    if (itr != m_officeMap.end())
        itr->second.lockDown = false;
}


void StationItem::RentOffice(OfficeData& odata)
{
    odata.typeID = typeID();    // change from officeTypeID to stationTypeID
    odata.folderID = m_stationID + STATION_OFFICE_OFFSET;
    odata.stationID = m_stationID;

    // create new office item
    std::string name = odata.ticker;
    name += "'s Office";
    ItemData idata(27, m_data.corporationID, m_stationID, flagOffice, name.c_str());
    StationOfficeRef oRef = sItemFactory.SpawnOffice(idata, odata);
    if (oRef.get() == nullptr)
        return;  // make error here?

    // make and send notifications
    OnOfficeRentalChanged oorc;
        oorc.ownerID = odata.corporationID;
        oorc.officeID = odata.officeID;
        oorc.officeFolderID = odata.folderID;
    PyTuple* payload = oorc.Encode();
    for (auto cur : m_guestList) {
        PyIncRef(payload);
        cur.second->SendNotification("OnOfficeRentalChanged", "stationid", &payload, false);
    }
    PyDecRef( payload );

    oRef->ChangeOwner(odata.corporationID, true);

    // update data
    stDataMgr.AddOffice(m_stationID, odata);
    m_officeMap.emplace(odata.officeID, odata);
    m_officeLoaded.emplace(odata.officeID, true);
    PySafeDecRef(m_officePyData);
    m_officePyData = StationDB::GetOffices(m_stationID);
}

void StationItem::SendBill()
{
    // do we need this here?
}

void StationItem::AddGuest(Client* pClient)
{
    m_guestList.emplace(pClient->GetCharacterID(), pClient);
}

void StationItem::GetGuestList(std::vector< Client* >& cVec)
{
    for (auto cur : m_guestList)
        cVec.push_back(cur.second);
}

void StationItem::RemoveGuest(Client* pClient)
{
    m_guestList.erase(pClient->GetCharacterID());
}

void StationItem::GetRefineData(uint32& stationCorpID, float& staEfficiency, float& tax)
{
    stationCorpID = m_data.corporationID;
    staEfficiency = m_data.reprocessingEfficiency;
    tax = m_data.reprocessingStationsTake;
}

bool StationItem::HasShip(Client* pClient)
{
    std::vector< InventoryItemRef > items;
    pInventory->GetInvForOwner(pClient->GetCharacterID(), items);

    for (auto cur : items)
        if (cur->categoryID() == EVEDB::invCategories::Ship)
            if (cur->typeID() != EVEDB::invTypes::Capsule)
                return true;
    return false;
}

ShipItemRef StationItem::GetShipFromInventory(uint32 shipID)
{
    return ShipItemRef::StaticCast(pInventory->GetByID(shipID));
}

CargoContainerRef StationItem::GetContainerFromInventory(uint32 contID)
{
    return CargoContainerRef::StaticCast(pInventory->GetByID(contID));
}


/*
 * Station Entity
 */
StationSE::StationSE(StationItemRef station, PyServiceMgr &services, SystemManager* system)
: StaticSystemEntity(station, services, system)
{
    // Create default dynamic attributes in the AttributeMap:
    station->SetAttribute(AttrOnline,             EvilOne, false);
    station->SetAttribute(AttrCapacity,           STATION_HANGAR_MAX_CAPACITY, false);
    station->SetAttribute(AttrInertia,            EvilOne, false);
    station->SetAttribute(AttrDamage,             EvilZero, false);
    station->SetAttribute(AttrShieldCapacity,     20000000.0, false);
    station->SetAttribute(AttrShieldCharge,       station->GetAttribute(AttrShieldCapacity), false);
    station->SetAttribute(AttrArmorHP,            station->GetAttribute(AttrArmorHP), false);
    station->SetAttribute(AttrArmorUniformity,    station->GetAttribute(AttrArmorUniformity), false);
    station->SetAttribute(AttrArmorDamage,        EvilZero, false);
    station->SetAttribute(AttrMass,               station->type().mass(), false);
    station->SetAttribute(AttrRadius,             station->type().radius(), false);
    station->SetAttribute(AttrVolume,             station->type().volume(), false);
}

void StationSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head = BallHeader();
    head.entityID = m_self->itemID();
        head.mode = Ball::Mode::RIGID;
        head.radius = GetRadius();
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.flags = /*Ball::Flag::HasMiniBalls |*/ Ball::Flag::IsGlobal | Ball::Flag::IsMassive;
    into.Append( head );
    RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

/** @todo miniballs is broken and needs work...
 *  dont know what's wrong at this point, but client freaks out and ignores ANY ball data (in SetState) after this.
 * this causes BallNotInPark error with multiple stations, or ANY data sent AFTER first StationBall
    MiniBall miniball;
        miniball.x = -7701.181;
        miniball.y = 8060.06;
        miniball.z = 27878.900;
        miniball.radius = 1639.241;
    into.Append( miniball );
 */
    _log(SE__DESTINY, "StationSE::EncodeDestiny(): %s - id:%li, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict *StationSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for StationSE %s(%u)", m_self->name(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("groupID",          new PyInt(m_self->groupID()));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        slim->SetItemString("corpID",           IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
        slim->SetItemString("allianceID",       IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
        slim->SetItemString("warFactionID",     IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",          new PyInt(m_ownerID));
        slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("incapacitated",    new PyInt(0));
        slim->SetItemString("online",           PyStatic.NewOne());
    return slim;
}

void StationSE::UnloadStation()
{
    m_self->GetMyInventory()->Unload();
}

/*
static const int num_hack_sentry_locs = 8;
GPoint hack_sentry_locs[num_hack_sentry_locs] = {
    GPoint(35000.0f, 35000.0f, 35000.0f),
    GPoint(35000.0f, 35000.0f, -35000.0f),
    GPoint(35000.0f, -35000.0f, 35000.0f),
    GPoint(35000.0f, -35000.0f, -35000.0f),
    GPoint(-35000.0f, 35000.0f, 35000.0f),
    GPoint(-35000.0f, 35000.0f, -35000.0f),
    GPoint(-35000.0f, -35000.0f, 35000.0f),
    GPoint(-35000.0f, -35000.0f, -35000.0f)
};
*/
