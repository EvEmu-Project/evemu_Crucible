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
    Author:     Zhur
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "EVEServerConfig.h"
#include "character/Character.h"
#include "exploration/Probes.h"
#include "inventory/InventoryDB.h"
#include "inventory/ItemFactory.h"
#include "inventory/ItemType.h"
#include "manufacturing/Blueprint.h"
#include "pos/Structure.h"
#include "ship/Missile.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleItem.h"
#include "station/Station.h"
#include "station/StationOffice.h"
#include "system/Asteroid.h"
#include "system/Container.h"
#include "system/SolarSystem.h"
#include "system/SystemManager.h"

ItemFactory::ItemFactory()
:m_pClient(nullptr),
m_nextTempID(0),
m_nextNPCID(0),
m_nextDroneID(0),
m_nextMissileID(0),
m_db(nullptr)
{
}

ItemFactory::~ItemFactory()
{
    SafeDelete(m_db);
}

int ItemFactory::Initialize()
{
    ManagerDB::DeleteSpawnedRats(); // takes ~31.2s to run on main, 0.005s on dev

    if (sConfig.debug.DeleteTrackingCans)
        InventoryDB::DeleteTrackingCans();

    m_items.clear();

    // Initialize ID Authority variables:
    m_nextTempID = TEMP_ENTITY_ID;
    m_nextNPCID = NPC_ID;
    m_nextMissileID = MISSILE_ID;
    m_nextDroneID = DRONE_ID;

    m_db = new InventoryDB();

    sLog.Blue("      ItemFactory", "Item Factory Initialized.");
    return 1;
}

void ItemFactory::Close()
{
    sLog.Warning("      ItemFactory", "%u Items, %u Types still in list", \
                m_items.size(), m_types.size());
    // types
    for (auto cur : m_types)
        SafeDelete(cur.second);
    m_types.clear();
    // items
    //for (auto cur : m_items)
    //    delete(cur.second.get());
    m_items.clear();
    // Set Client pointer to NULL
    m_pClient = nullptr;
}

void ItemFactory::SaveItems() {
    if (sConfig.debug.DeleteTrackingCans)
        InventoryDB::DeleteTrackingCans();
    uint32 count(0);
    double startTime = GetTimeMSeconds();
    std::vector<Inv::SaveData> items;
    items.clear();
    for (auto cur : m_items) {
        if (IsPlayerItem(cur.first)) { // this is a hack for now.  will eventually move to static/dynamic item maps
            cur.second->SaveAttributes();
            Inv::SaveData data = Inv::SaveData();
                data.itemID = cur.first;
                data.contraband = cur.second->contraband();
                data.flag = cur.second->flag();
                data.locationID = cur.second->locationID();
                data.ownerID = cur.second->ownerID();
                data.position = cur.second->position();
                data.quantity = cur.second->quantity();
                data.singleton = cur.second->isSingleton();
                data.typeID = cur.second->typeID();
                data.customInfo = cur.second->customInfo();
            items.push_back(data);
            ++count;
        }
    }
    ItemDB::SaveItems(items);
    sLog.Warning("        SaveItems", "Saved %u Dynamic Items in %.3fms.", count, (GetTimeMSeconds() -startTime));
}

void ItemFactory::AddItem(InventoryItemRef iRef)
{
    if (IsTempItem(iRef->itemID()))
        return;

    if (iRef->itemID() < minAgent) {
        sLog.Warning("ItemFactory::AddItem()", "Trying to Add invalid UID %u for %s", iRef->itemID(), iRef->name());
        return;
    }
    m_items.emplace(iRef->itemID(), iRef);
}

void ItemFactory::RemoveItem(uint32 itemID)
{
    m_items.erase(itemID);
}

uint32 ItemFactory::GetNextTempID()
{
    if (m_nextTempID < PLANET_PIN_ID) {
        ++m_nextTempID;
    } else {
        m_nextTempID = TEMP_ENTITY_ID;
    }

    return m_nextTempID;
}

uint32 ItemFactory::GetNextNPCID()
{
    return ++m_nextNPCID;
}

uint32 ItemFactory::GetNextDroneID() {
    return ++m_nextDroneID;
}

uint32 ItemFactory::GetNextMissileID()
{
    return ++m_nextMissileID;
}

Inventory* ItemFactory::GetInventoryFromId(uint32 itemID, bool load /*true*/) {
    // do we need to check trade containers here?
    if (!IsValidLocationID(itemID))
        return nullptr;

    InventoryItemRef iRef(nullptr);
    std::map<uint32, InventoryItemRef>::iterator itr = m_items.find(itemID);
    if (itr != m_items.end()) {
        iRef = itr->second;
    } else if (load) {
        iRef = GetItem(itemID);
    }

    if (iRef.get() == nullptr)
        return nullptr;

    return iRef->GetMyInventory();
}

InventoryItemRef ItemFactory::GetInventoryItemFromID(uint32 itemID, bool load /*true*/) {
    InventoryItemRef iRef(nullptr);
    std::map<uint32, InventoryItemRef>::iterator itr = m_items.find(itemID);
    if (itr != m_items.end()) {
        iRef = itr->second;
    } else if (load) {
        iRef = GetItem(itemID);
    }

    return iRef;
}

InventoryItemRef ItemFactory::GetItemContainer(uint32 itemID, bool load/*true*/)
{
    InventoryItemRef iRef(nullptr);
    std::map<uint32, InventoryItemRef>::iterator itr = m_items.find(itemID);
    if (itr != m_items.end()) {
        iRef = itr->second;
        itr = m_items.find(iRef->locationID());
        iRef = itr->second;
    } else if (load) {
        iRef = GetItem(itemID);
        itr = m_items.find(itemID);
        if (itr != m_items.end()) {
            iRef = itr->second;
            itr = m_items.find(iRef->locationID());
            iRef = itr->second;
        }
    }

    if (iRef.get() == nullptr)
        return InventoryItemRef(nullptr);

    return iRef;
}

Inventory* ItemFactory::GetItemContainerInventory(uint32 itemID, bool load/*true*/)
{
    InventoryItemRef iRef(nullptr);
    std::map<uint32, InventoryItemRef>::iterator itr = m_items.find(itemID);
    if (itr != m_items.end()) {
        iRef = itr->second;
        itr = m_items.find(iRef->locationID());
        iRef = itr->second;
    } else if (load) {
        iRef = GetItem(itemID);
        itr = m_items.find(itemID);
        if (itr != m_items.end()) {
            iRef = itr->second;
            itr = m_items.find(iRef->locationID());
            iRef = itr->second;
        }
    }

    if (iRef.get() == nullptr)
        return nullptr;

    return iRef->GetMyInventory();
}

template<class _Ty>
const _Ty* ItemFactory::_GetType(uint16 typeID) {
    std::map<uint16, ItemType*>::iterator itr = m_types.find(typeID);
    if (itr == m_types.end()) {
        _Ty* type = _Ty::Load(typeID);
        if (type == nullptr)
            return nullptr;

        // insert into cache
        itr = m_types.insert(std::make_pair(typeID, type)).first;
    }
    return static_cast<const _Ty *>(itr->second);
}

const ItemType* ItemFactory::GetType(uint16 typeID) {
    return _GetType<ItemType>(typeID);
}

const BlueprintType* ItemFactory::GetBlueprintType(uint16 blueprintTypeID) {
    return _GetType<BlueprintType>(blueprintTypeID);
}

const CharacterType* ItemFactory::GetCharacterType(uint16 characterTypeID) {
    return _GetType<CharacterType>(characterTypeID);
}

const CharacterType* ItemFactory::GetCharacterTypeByBloodline(uint16 bloodlineID) {
    // Unfortunately, we have it indexed by typeID, so we must get it ...
    uint16 characterTypeID;
    if (!InventoryDB::GetCharacterTypeByBloodline(bloodlineID, characterTypeID))
        return nullptr;
    return GetCharacterType(characterTypeID);
}
const StationType* ItemFactory::GetStationType(uint16 stationTypeID) {
    return _GetType<StationType>(stationTypeID);
}

template<class _Ty>
RefPtr<_Ty> ItemFactory::_GetItem(uint32 itemID)
{
    std::map<uint32, InventoryItemRef>::iterator itr = m_items.find(itemID);
    if (itr == m_items.end()) {
        if (itemID < minAgent) {
            _log(ITEM__WARNING, "ItemFactory::_GetItem() called on invalid Item %u", itemID);
            //if (sConfig.debug.StackTrace)
            //    EvE::traceStack();
            return RefPtr<_Ty>();
        }

        // load the item
        RefPtr<_Ty> item = _Ty::Load(itemID);
        if (!item)
            return RefPtr<_Ty>();

        //we keep the original ref.
        itr = m_items.insert(std::make_pair(itemID, item)).first;
    }
    // return to the user.
    return RefPtr<_Ty>::StaticCast(itr->second);
}

InventoryItemRef ItemFactory::GetItem(uint32 itemID)
{
    return _GetItem<InventoryItem>(itemID);
}

BlueprintRef ItemFactory::GetBlueprint(uint32 blueprintID)
{
    return _GetItem<Blueprint>(blueprintID);
}

CharacterRef ItemFactory::GetCharacter(uint32 characterID)
{
    return _GetItem<Character>(characterID);
}

ShipItemRef ItemFactory::GetShip(uint32 shipID)
{
    return _GetItem<ShipItem>(shipID);
}

CelestialObjectRef ItemFactory::GetCelestialObject(uint32 celestialID)
{
    return _GetItem<CelestialObject>(celestialID);
}

SolarSystemRef ItemFactory::GetSolarSystem(uint32 solarSystemID)
{
    return _GetItem<SolarSystem>(solarSystemID);
}

StationItemRef ItemFactory::GetStationItem(uint32 stationID)
{
    return _GetItem<StationItem>(stationID);
}

SkillRef ItemFactory::GetSkill(uint32 skillID)
{
    return _GetItem<Skill>(skillID);
}

AsteroidItemRef ItemFactory::GetAsteroid(uint32 asteroidID)
{
    return _GetItem<AsteroidItem>(asteroidID);
}

StationOfficeRef ItemFactory::GetOffice(uint32 officeID)
{
    return _GetItem<StationOffice>(officeID);
}

StructureItemRef ItemFactory::GetStructure(uint32 structureID)
{
    return _GetItem<StructureItem>(structureID);
}

CargoContainerRef ItemFactory::GetCargoContainer(uint32 containerID)
{
    return _GetItem<CargoContainer>(containerID);
}

WreckContainerRef ItemFactory::GetWreckContainer(uint32 containerID)
{
    return _GetItem<WreckContainer>(containerID);
}

ModuleItemRef ItemFactory::GetModuleItem(uint32 moduleID)
{
    return _GetItem<ModuleItem>(moduleID);
}

ProbeItemRef ItemFactory::GetProbeItem(uint32 probeID) {
    return _GetItem<ProbeItem>(probeID);
}



InventoryItemRef ItemFactory::SpawnItem(ItemData &data) {
    InventoryItemRef iRef = InventoryItem::Spawn(data);
    if (iRef.get() != nullptr)
        AddItem(iRef);

    return iRef;
}

InventoryItemRef ItemFactory::SpawnTempItem(ItemData &data) {
    InventoryItemRef iRef = InventoryItem::SpawnTemp(data);
    return iRef;
}

CharacterRef ItemFactory::SpawnCharacter(CharacterData &charData, CorpData &corpData) {
    CharacterRef iRef = Character::Spawn(charData, corpData);
    return iRef;
}

ShipItemRef ItemFactory::SpawnShip(ItemData &data) {
    ShipItemRef iRef = ShipItem::Spawn(data);
    if (iRef.get() != nullptr)
        AddItem(iRef);

    return iRef;
}

SkillRef ItemFactory::SpawnSkill(ItemData &data)
{
    SkillRef iRef = Skill::Spawn(data);
    if (iRef.get() != nullptr)
        AddItem(iRef);

    return iRef;
}

StructureItemRef ItemFactory::SpawnStructure(ItemData &data)
{
    StructureItemRef iRef = StructureItem::Spawn(data);
    if (iRef.get() != nullptr)
        AddItem(iRef);

    return iRef;
}

AsteroidItemRef ItemFactory::SpawnAsteroid(ItemData &idata, AsteroidData& adata)
{
    AsteroidItemRef iRef = AsteroidItem::Spawn(idata, adata);
    if (iRef.get() != nullptr)
        AddItem(iRef);

    return iRef;
}

StationItemRef ItemFactory::SpawnOutpost(ItemData &idata)
{
    StationItemRef iRef = StationItem::Spawn(idata);
    if (iRef.get() != nullptr)
        AddItem(iRef);

    return iRef;
}

StationOfficeRef ItemFactory::SpawnOffice(ItemData &idata, OfficeData& odata)
{
    StationOfficeRef iRef = StationOffice::Spawn(idata, odata);
    if (iRef.get() != nullptr)
        AddItem(iRef);

    return iRef;
}

CargoContainerRef ItemFactory::SpawnCargoContainer(ItemData &data)
{
    CargoContainerRef iRef = CargoContainer::Spawn(data);
    if (iRef.get() != nullptr)
        AddItem(iRef);

    return iRef;
}

WreckContainerRef ItemFactory::SpawnWreckContainer(ItemData &data)
{
    WreckContainerRef iRef = WreckContainer::Spawn(data);
    if (iRef.get() != nullptr)
        AddItem(iRef);

    return iRef;
}

ModuleItemRef ItemFactory::SpawnModule(ItemData& data)
{
    ModuleItemRef iRef = ModuleItem::Spawn(data);
    if (iRef.get() != nullptr)
        AddItem(iRef);

    return iRef;
}

ProbeItemRef ItemFactory::SpawnProbe(ItemData& data)
{
    ProbeItemRef iRef = ProbeItem::Spawn(data);
    if (iRef.get() != nullptr)
        AddItem(iRef);

    return iRef;
}
