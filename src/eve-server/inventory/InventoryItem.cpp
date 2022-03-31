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
    Updates:     Allan
*/

#include "eve-server.h"

#include "../../eve-common/EVE_Mail.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "EntityList.h"
#include "character/Skill.h"
#include "effects/EffectsProcessor.h"
#include "exploration/Probes.h"
#include "manufacturing/Blueprint.h"
#include "pos/Structure.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleItem.h"
#include "station/Station.h"
#include "station/StationOffice.h"
#include "system/Asteroid.h"
#include "system/Celestial.h"
#include "system/Container.h"

// Pursche: This change was made to get rid of a circular inclusion between fxData and InventoryItem, this concept is called pImpl and helps us forward declare more stuff.
struct ModifierContainer : public ModifierContainerBase
{
    std::multimap<int8, fxData> modifiers;     // k,v of math, data<math, src, targLoc, targAttr, srcAttr, grpID, typeID>, ordered by key (mathMethod)
};

/*
 * InventoryItem
 */
InventoryItem::InventoryItem(uint32 _itemID, const ItemType& _type, const ItemData& _data)
: RefObject(0),
pAttributeMap(new AttributeMap(*this)),
pInventory(nullptr),      // this is created/destroyed in derived classes as needed.
m_data(_data),
m_type(_type),
m_itemID(_itemID),
m_timestamp(0),  // placeholder for fx timestamp, once implemented
m_delete(false)
{
    // assert for data consistency
    assert(_data.typeID == _type.id());

    ModifierContainer* modifierContainer = new ModifierContainer();
    m_modifierContainer = modifierContainer;
    modifierContainer->modifiers.clear();

    _log(ITEM__TRACE, "II::C'tor - Created Generic Item %p for item %s (%u).", this, m_data.name.c_str(), m_itemID);
}

// copy c'tor
InventoryItem::InventoryItem(const InventoryItem& oth)
: RefObject(0),
pAttributeMap(oth.pAttributeMap),
pInventory(oth.pInventory),
m_itemID(oth.m_itemID),
m_data(oth.m_data),
m_type(oth.m_type),
m_timestamp(oth.m_timestamp),
m_delete(false)
{
    sLog.Error("InventoryItem()", "InventoryItem copy c'tor called.");
    EvE::traceStack();
    assert(0);
}

// move c'tor
InventoryItem::InventoryItem(InventoryItem&& oth) noexcept
: RefObject(0),
pAttributeMap(oth.pAttributeMap),
pInventory(oth.pInventory),
m_itemID(oth.m_itemID),
m_data(oth.m_data),
m_type(oth.m_type),
m_timestamp(oth.m_timestamp),
m_delete(false)
{
    sLog.Error("InventoryItem()", "InventoryItem move c'tor called.");
    EvE::traceStack();
    assert(0);
}

// copy assignment =delete
/*
InventoryItem& InventoryItem::operator= ( const InventoryItem& oth )
{
    sLog.Error("InventoryItem()", "InventoryItem copy assign called.");
    EvE::traceStack();
    assert(0);

    oth.GetAttributeMap()->CopyAttributes(pAttributeMap->mAttributes);

    m_itemID = oth.itemID();
    m_type = oth.type();
} */

// move assignment =delete
/*
InventoryItem& InventoryItem::operator= ( InventoryItem&& oth ) noexcept
{
    sLog.Error("InventoryItem()", "InventoryItem move assign called.");
    EvE::traceStack();
    assert(0);

    oth.GetAttributeMap()->CopyAttributes(pAttributeMap->mAttributes);

    m_itemID = oth.itemID();
    m_type = oth.type();
} */


InventoryItem::~InventoryItem() noexcept
{
    SafeDelete(pAttributeMap);
}

InventoryItemRef InventoryItem::Load(uint32 itemID)
{
    return InventoryItem::Load<InventoryItem>(itemID);
}

InventoryItemRef InventoryItem::SpawnItem(uint32 itemID, const ItemData &data)
{
    if (data.quantity == 0)
        return InventoryItemRef(nullptr);
    const ItemType *iType = sItemFactory.GetType(data.typeID);
    if (iType == nullptr)
        return InventoryItemRef(nullptr);
    InventoryItemRef iRef = InventoryItemRef(new InventoryItem(itemID, *iType, data));
    if (iRef.get() != nullptr)
        iRef->_Load();

    return iRef;
}

uint32 InventoryItem::CreateItemID(ItemData &data) {
    // obtain type of new item
    const ItemType *iType = sItemFactory.GetType(data.typeID);
    if (iType == nullptr) {
        codelog(ITEM__ERROR, "II::CreateItemID() - Invalid type returned for typeID %u", data.typeID);
        return 0;
    }
    // fix the name (if empty)
    if (data.name.empty())
        data.name = iType->name();

    if (data.locationID == 0)
        if (is_log_enabled(ITEM__MESSAGE)) {
            _log(ITEM__MESSAGE, "II::CreateItemID() - LocationID = 0 for item");
            EvE::traceStack();
        }

    // insert new entry into DB
    return ItemDB::NewItem(data);
}

/* This Spawn function is meant for in-memory only items created from the following categories...
 *  EVEDB::invCategories::Entity (for npcs)
 *  EVEDB::invCategories::Charge (for launched missiles only)
 *  EVEDB::invCategories::Container (for Position Tracking only)
 *  all dungeon/anomaly items
 *
 * these items meant to never be saved to database
 * and be thrown away on server shutdown.
 * Updated 29May15 -Allan
 */
uint32 InventoryItem::CreateTempItemID(ItemData &data) {
    // obtain type of new item
    // this also checks that the type is valid
    const ItemType *iType = sItemFactory.GetType(data.typeID);
    if (iType == nullptr) {
        codelog(ITEM__ERROR, "II::CreateTempItemID() - Invalid ItemType returned for typeID %u", data.typeID);
        return 0;
    }

    // fix the name if empty
    if (data.name.empty())
        data.name = iType->name();

    // Get a new Entity ID from ItemFactory's ID Authority:
    // may need more testing to verify that ONLY NPC's and jetcan markers use this method
    if (iType->categoryID() == EVEDB::invCategories::Entity)
        return sItemFactory.GetNextNPCID();

    if (data.flag == EVEItemFlags::flagMissile)
        return sItemFactory.GetNextMissileID();

    return sItemFactory.GetNextTempID();
}

bool InventoryItem::_Load() {
    if (!pAttributeMap->Load()) {
        _log(ITEM__WARNING, "II::_Load() - Failed to load attribute map for %s(%u).", m_data.name.c_str(), m_itemID);
        return false;
    }

    return true;
}

template<class _Ty>
RefPtr<_Ty> InventoryItem::_LoadItem(uint32 itemID, const ItemType &type, const ItemData &data) {
    switch(type.categoryID() ) {
        case EVEDB::invCategories::_System:
        case EVEDB::invCategories::Material:    // includes minerals
        case EVEDB::invCategories::Trading:
        case EVEDB::invCategories::Bonus:
        case EVEDB::invCategories::PlanetaryInteraction:
        case EVEDB::invCategories::PlanetaryResources:
        case EVEDB::invCategories::PlanetaryCommodities:
        case EVEDB::invCategories::Commodity:
        case EVEDB::invCategories::Accessories: { // this is for bookmark vouchers
            // Generic item, create one
            return InventoryItemRef(new InventoryItem(itemID, type, data ));
        } break;
        case EVEDB::invCategories::Deployable: // these may need their own class.  not sure yet
        case EVEDB::invCategories::Drone:
        case EVEDB::invCategories::Implant:
        case EVEDB::invCategories::Reaction: {
            // create Generic item for now
            return InventoryItemRef(new InventoryItem(itemID, type, data ));
        } break;
        case EVEDB::invCategories::Module:
        case EVEDB::invCategories::Subsystem: {
            return ModuleItem::_LoadItem<ModuleItem>(itemID, type, data);
        } break;
        case EVEDB::invCategories::Owner: {
            return Character::_LoadItem<Character>(itemID, type, data);
        } break;
        case EVEDB::invCategories::Skill: {
            return Skill::_LoadItem<Skill>(itemID, type, data);
        } break;
        case EVEDB::invCategories::Blueprint: {
            return Blueprint::_LoadItem<Blueprint>(itemID, type, data);
        } break;
        case EVEDB::invCategories::Asteroid: {
            if (IsAsteroidID(itemID))
                return AsteroidItem::_LoadItem<AsteroidItem>(itemID, type, data);
            // mined ore.  create default item
            return InventoryItemRef(new InventoryItem(itemID, type, data ));
        } break;
        case EVEDB::invCategories::Ship: {
            return ShipItem::_LoadItem<ShipItem>(itemID, type, data);
        } break;
        case EVEDB::invCategories::Structure:
        case EVEDB::invCategories::Orbitals:
        case EVEDB::invCategories::SovereigntyStructure:
        case EVEDB::invCategories::StructureUpgrade: {
            return StructureItem::_LoadItem<StructureItem>(itemID, type, data);
        } break;
        case EVEDB::invCategories::Charge: {      // probes are charges.
            switch (type.groupID()) {
                case EVEDB::invGroups::Scanner_Probe:
                case EVEDB::invGroups::Survey_Probe:
                case EVEDB::invGroups::Warp_Disruption_Probe:
                case EVEDB::invGroups::Obsolete_Probes: {   // make error for these?
                    return ProbeItem::_LoadItem<ProbeItem>(itemID, type, data);
                } break;
                default: {
                    // create generic item for other charge types
                    return InventoryItemRef(new InventoryItem(itemID, type, data ));
                } break;
            }
        } break;
        case EVEDB::invCategories::Station: {
            // test for office first
            if (type.id() == 27) {
                return StationOffice::_LoadItem<StationOffice>(itemID, type, data);
            } else if (type.groupID() != EVEDB::invGroups::Station) {
                return StationItem::_LoadItem<StationItem>(itemID, type, data);
            }/* else if (type.groupID() != EVEDB::invGroups::Station_Services) {
               this isnt written yet...
            } */
        } break;
        case EVEDB::invCategories::Celestial: {
            switch (type.groupID()) {
                case EVEDB::invGroups::Solar_System: {
                    return SolarSystem::_LoadItem<SolarSystem>(itemID, type, data);
                } break;
                case EVEDB::invGroups::Secure_Cargo_Container:
                case EVEDB::invGroups::Cargo_Container:
                case EVEDB::invGroups::Freight_Container:
                case EVEDB::invGroups::Audit_Log_Secure_Container:
                case EVEDB::invGroups::Mission_Container: {
                    return CargoContainer::_LoadItem<CargoContainer>(itemID, type, data);
                } break;
                case EVEDB::invGroups::Wreck: {
                    return WreckContainer::_LoadItem<WreckContainer>(itemID, type, data);
                } break;
                case EVEDB::invGroups::Force_Field:
                default: {
                    // generic Celestial Object
                    return CelestialObject::_LoadItem<CelestialObject>(itemID, type, data);
                } break;
            }
        } break;
        // there are 216 groups in the entity category.  im not testing all of them.
        case EVEDB::invCategories::Entity: {
            switch (type.groupID()) {
                case EVEDB::invGroups::Spawn_Container: {
                    return CargoContainer::_LoadItem<CargoContainer>(itemID, type, data);
                } break;
                case EVEDB::invGroups::Temporary_Cloud: {
                    return CelestialObject::_LoadItem<CelestialObject>(itemID, type, data);
                } break;
                case EVEDB::invGroups::Billboard:
                // the rest are drones and npcs....i think
                // they are *somewhat* separated for eventual classification into their own itemtypes
                case EVEDB::invGroups::Convoy:
                case EVEDB::invGroups::Convoy_Drone:
                //  added checks for all npc's   -allan 26Dec14
                case EVEDB::invGroups::Drones:
                case EVEDB::invGroups::Police_Drone:
                case EVEDB::invGroups::Concord_Drone:
                case EVEDB::invGroups::Customs_Official:

                case EVEDB::invGroups::Sentry_Gun:
                case EVEDB::invGroups::Mobile_Sentry_Gun:
                case EVEDB::invGroups::Protective_Sentry_Gun:
                case EVEDB::invGroups::Destructible_Sentry_Gun:

                case EVEDB::invGroups::Pirate_Drone:
                case EVEDB::invGroups::Tutorial_Drone:

                case EVEDB::invGroups::Storyline_Frigate:
                case EVEDB::invGroups::Storyline_Cruiser:
                case EVEDB::invGroups::Storyline_Battleship:
                case EVEDB::invGroups::Storyline_Mission_Frigate:
                case EVEDB::invGroups::Storyline_Mission_Cruiser:
                case EVEDB::invGroups::Storyline_Mission_Battleship:

                case EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Sentinel:
                case EVEDB::invGroups::Deadspace_Sleeper_Awakened_Sentinel:
                case EVEDB::invGroups::Deadspace_Sleeper_Emergent_Sentinel:
                 /* start at EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Defender:
                end at EVEDB::invGroups::Deadspace_Sleeper_Emergent_Patroller:
                */
                case EVEDB::invGroups::Incursion_Sanshas_Nation_Industrial:
                case EVEDB::invGroups::Incursion_Sanshas_Nation_Capital:
                case EVEDB::invGroups::Incursion_Sanshas_Nation_Frigate:
                case EVEDB::invGroups::Incursion_Sanshas_Nation_Cruiser:
                case EVEDB::invGroups::Incursion_Sanshas_Nation_Battleship: {
                    _log(ITEM__WARNING, "item %u (type %u, group %u) defaulting to generic InventoryItem.", itemID, type.id(), type.groupID());
                } break;
                default: {
                    _log(ITEM__WARNING, "item %u (type %u, group %u,  cat %u) is not handled in II::_LoadItem::Entity.", itemID, type.id(), type.groupID(), type.categoryID());
                }
            }
        } break;
        default: {
            _log(ITEM__WARNING, "item %u (type %u, group %u,  cat %u) is not handled in II::_LoadItem.", itemID, type.id(), type.groupID(), type.categoryID());
        } break;
    }
    // Generic item, create one:
    return InventoryItemRef(new InventoryItem(itemID, type, data ));
}

InventoryItemRef InventoryItem::SpawnTemp(ItemData& data)
{
    /** @todo will need to update this to get 'proper' item creation for temps... */
    //  for now, return generic item
    // obtain type of new item
    const ItemType *iType = sItemFactory.GetType(data.typeID);
    if (iType == nullptr) {
        codelog(ITEM__ERROR, "II::SpawnTemp() - Invalid type returned for typeID %u", data.typeID);
        return InventoryItemRef(nullptr);
    }

    if (iType->groupID() == EVEDB::invGroups::Cargo_Container)
        return CargoContainer::SpawnTemp(data);

    return InventoryItem::SpawnItem(InventoryItem::CreateTempItemID(data), data);
}

// called from generic SpawnItem()
InventoryItemRef InventoryItem::Spawn(ItemData &data)
{
    // obtain type of new item
    const ItemType *iType = sItemFactory.GetType(data.typeID);
    if (iType == nullptr) {
        codelog(ITEM__ERROR, "II::Spawn() - Invalid type returned for typeID %u", data.typeID);
        return InventoryItemRef(nullptr);
    }

    switch(iType->categoryID() ) { //23
        case EVEDB::invCategories::Owner: {
            assert(0);  // this needs to make a serious error here....these CANNOT be called from here using the generic InventoryItem::Spawn() method
        } break;
        //! TODO not handled....use generic item
        case EVEDB::invCategories::Bonus:               // no clue what this is
        case EVEDB::invCategories::Implant:
        case EVEDB::invCategories::Trading:
        case EVEDB::invCategories::Asteroid:            // ore is "asteroid" also
        case EVEDB::invCategories::Material:            // includes minerals
        case EVEDB::invCategories::Reaction:
        case EVEDB::invCategories::Commodity:
        case EVEDB::invCategories::Accessories:         // this is for bookmark vouchers
        case EVEDB::invCategories::AncientRelics:       // t3 bpc from sleepers
        // these *may* need their own class
        case EVEDB::invCategories::Decryptors:
        case EVEDB::invCategories::StructureUpgrade:
        case EVEDB::invCategories::PlanetaryInteraction:
        case EVEDB::invCategories::PlanetaryResources:
        case EVEDB::invCategories::PlanetaryCommodities: {
            _log(ITEM__WARNING, "II::Spawn creating generic item for type %u, cat %u.", iType->id(), iType->categoryID());
            // Spawn generic item:
            uint32 itemID = InventoryItem::CreateItemID(data);
            return InventoryItem::SpawnItem(itemID, data);
        } break;
        case EVEDB::invCategories::Orbitals:
        case EVEDB::invCategories::Structure:
        case EVEDB::invCategories::SovereigntyStructure: {
            return StructureItem::Spawn(data);
        } break;
        case EVEDB::invCategories::Blueprint: {
            // this needs to distinguish between copy and orig
            EvERam::bpData bdata = EvERam::bpData();
                bdata.runs = -1;
            return Blueprint::Spawn(data, bdata);
        } break;
        case EVEDB::invCategories::Skill: {
            return Skill::Spawn(data);
        } break;
        case EVEDB::invCategories::Ship: {
            return ShipItem::Spawn(data);
        } break;
        case EVEDB::invCategories::Entity: {
            switch (iType->groupID()) {
                case EVEDB::invGroups::Spawn_Container: {
                    return CargoContainer::Spawn(data);
                } break;
                case EVEDB::invGroups::Billboard:
                case EVEDB::invGroups::Control_Bunker:
                case EVEDB::invGroups::Capture_Point: {
                    uint32 itemID = InventoryItem::CreateItemID(data);
                    return InventoryItem::SpawnItem(itemID, data);
                } break;
                case EVEDB::invGroups::Sentry_Gun:      // these are not saved
                case EVEDB::invGroups::Temporary_Cloud:
                default: {  // *should*  be all npcs
                    // use temp items and NOT save to db.
                    uint32 itemID = InventoryItem::CreateTempItemID(data);
                    return InventoryItem::SpawnItem(itemID, data);
                } break;
            }
        } break;
        case EVEDB::invCategories::Module:
        case EVEDB::invCategories::Subsystem: {
            return ModuleItem::Spawn(data);
        } break;
        case EVEDB::invCategories::Drone: {
            if (!sConfig.testing.EnableDrones)
                return InventoryItemRef(nullptr);
            //return DroneItem::Spawn(data);
        } // allow fallthru until DroneItem is written
        case EVEDB::invCategories::Deployable: {
            // Spawn generic item:
            uint32 itemID = InventoryItem::CreateItemID(data);
            InventoryItemRef itemRef = InventoryItem::SpawnItem(itemID, data);
            if (itemRef.get() == nullptr)
                return InventoryItemRef(nullptr);
            // THESE SHOULD BE MOVED INTO A _type::Spawn() function that does not exist yet
            itemRef->SetAttribute(AttrMass,           iType->mass(), false);           // Mass
            itemRef->SetAttribute(AttrRadius,         iType->radius(), false);       // Radius
            itemRef->SetAttribute(AttrVolume,         iType->volume(), false);       // Volume
            itemRef->SetAttribute(AttrCapacity,       iType->capacity(), false);   // Capacity
            return itemRef;
        } break;
        case EVEDB::invCategories::Charge: {
            uint32 itemID = 0;
            InventoryItemRef itemRef;
            switch (data.flag) {
                case EVEItemFlags::flagMissile: {
                    // Spawn launched missile item in MISSILE_ID range and does NOT save missile to db
                    itemID = InventoryItem::CreateTempItemID(data);
                    itemRef = InventoryItem::SpawnItem(itemID, data);
                } break;
                default: {
                    switch (iType->groupID()) {
                        case EVEDB::invGroups::Scanner_Probe:
                        case EVEDB::invGroups::Survey_Probe:
                        case EVEDB::invGroups::Warp_Disruption_Probe: {
                            return ProbeItem::Spawn(data);
                        } break;
                        case EVEDB::invGroups::Obsolete_Probes: {
                            // make error for these
                            _log(ITEM__ERROR, "II::Spawn - Obsolete Probe group called.");
                            return InventoryItemRef(nullptr);
                        }
                        default: {
                            // create generic item for other charge types
                            itemID = InventoryItem::CreateItemID(data);
                            itemRef = InventoryItem::SpawnItem(itemID, data);
                        } break;
                    }
                }
            }
            if (itemRef.get() == nullptr)
                return InventoryItemRef(nullptr);
            itemRef->SetAttribute(AttrMass,       iType->mass(), false);         // Mass
            itemRef->SetAttribute(AttrRadius,     iType->radius(), false);       // Radius
            return itemRef;
        } break;
        case EVEDB::invCategories::Station: {
            return StationItem::Spawn(data);
        } break;
        case EVEDB::invCategories::Celestial: {
            if ((iType->groupID() == EVEDB::invGroups::Secure_Cargo_Container)
            or (iType->groupID() == EVEDB::invGroups::Cargo_Container)
            or (iType->groupID() == EVEDB::invGroups::Freight_Container)
            or (iType->groupID() == EVEDB::invGroups::Audit_Log_Secure_Container)
            or (iType->groupID() == EVEDB::invGroups::Mission_Container)) {
                return CargoContainer::Spawn(data);
            } else if (iType->groupID() == EVEDB::invGroups::Wreck) {
                return WreckContainer::Spawn(data);
            } else if (iType->groupID() == EVEDB::invGroups::Force_Field) {
                // Spawn force field item in TEMP_ENTITY_ID range and does NOT save Force_Field to db
                uint32 itemID = InventoryItem::CreateTempItemID(data);
                return InventoryItem::SpawnItem(itemID, data);
            } else {
                // Spawn new Celestial Object
                return CelestialObject::Spawn(data);
            }
        } break;
    }

    // log error and return nullref for items not handled here
    _log(ITEM__WARNING, "II::Spawn item not handled - type %u, grp %u, cat %u.", iType->id(), iType->groupID(), iType->categoryID());
    return InventoryItemRef(nullptr);
}

// item manipulation methods
void InventoryItem::AddItem(InventoryItemRef iRef)
{
    // make error for invalid inventory?
    // only happens on char creation, when system isnt loaded yet, so we really dont need it.
    if (pInventory != nullptr)
        pInventory->AddItem(iRef);
}

void InventoryItem::RemoveItem(InventoryItemRef iRef)
{
    if (pInventory != nullptr)  // just in case
        pInventory->RemoveItem(iRef);
}

void InventoryItem::Delete()
{
    m_delete = true;

    // get out of client's sight.
    if (!IsNPCCorp(m_data.ownerID) and (m_data.ownerID > 1)) {
        Move(locJunkyard, flagNone, true);
    } else {
        // remove from current container's inventory
        if (IsValidLocationID(m_data.locationID)) {
            InventoryItemRef iRef = sItemFactory.GetItemRef(m_data.locationID);
            if (iRef.get() != nullptr) {
                iRef->GetMyInventory()->RemoveItem(InventoryItemRef(this));
            } else {
                _log(ITEM__ERROR, "II::Delete() - Cant find location %u containing %s.", m_data.locationID, m_data.name.c_str());
            }
        }
    }

    // remove from DB
    ItemDB::DeleteItem(m_itemID);
    // remove from factory cache
    sItemFactory.RemoveItem(m_itemID);
}

void InventoryItem::ToVirtual(uint32 locationID)
{
    InventoryItemRef iRef = sItemFactory.GetItemContainerRef(m_itemID, false);
    if (iRef.get() != nullptr) {
        // verify this gets inventory containing item before trying to manipulate
        //Inventory* pInv = iRef->GetMyInventory();
        if (pInventory != nullptr)  // this isnt right...this item wont be in it's own inventory
            pInventory->RemoveItem(InventoryItemRef(this));
    }
    if (pAttributeMap != nullptr)   // should never be null, but just in case
        pAttributeMap->Delete();

    //notify about the changes.
    std::map<int32, PyRep *> changes;
    changes[Inv::Update::Location] = new PyInt(m_data.locationID);
    SendItemChange(m_data.ownerID, changes);   //changes is consumed
    m_data.locationID = locationID;

    //take ourself out of the DB
    ItemDB::DeleteItem(m_itemID);
    //delete ourselves from factory cache
    sItemFactory.RemoveItem(m_itemID);
}

void InventoryItem::Rename(std::string name)
{
    m_data.name = name;
    SaveItem();

    PyList* list = new PyList();
        list->AddItem(new PyInt(m_itemID));
        list->AddItem(new PyString(name));
        list->AddItem(new PyFloat(0));
        list->AddItem(new PyFloat(0));
        list->AddItem(new PyFloat(0));
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString("evelocations"));
        tuple->SetItem(1, list);

    // get owner
    if (IsCharacterID(m_data.ownerID)) {
        // this will be the most-used case
        Client* pClient = sEntityList.FindClientByCharID(m_data.ownerID);
        if (pClient == nullptr)
            return;  //  make error here?
        if (pClient->IsDocked()) {
            pClient->SendNotification("OnCfgDataChanged", "charid", &tuple, false); //unsequenced.
        } else { // client in space.  sent update to all clients in bubble
            pClient->GetShipSE()->SysBubble()->BubblecastSendNotification("OnCfgDataChanged", "solarsystemid", &tuple, false);
        }
    } else if (IsPlayerCorp(m_data.ownerID)) {
        // bcast to all online corp members
        if (sDataMgr.IsStation(m_data.locationID)) {
            sEntityList.CorpNotify(m_data.ownerID, Notify::Types::ItemUpdateStation, "OnCfgDataChanged", "charid", tuple);
        } else {
            sEntityList.CorpNotify(m_data.ownerID, Notify::Types::ItemUpdateSystem, "OnCfgDataChanged", "solarsystemid", tuple);
        }
    }

    /** @todo  if renaming a POS or other space object, we'll need to BubblecastSendNotification instead of CorpNotify  */
}

void InventoryItem::Donate(uint32 new_owner/*ownerSystem*/, uint32 new_location/*locTemp*/, EVEItemFlags new_flag/*flagNone*/, bool notify/*true*/)
{
    if (!IsValidOwner(new_owner)) {
        _log(ITEM__ERROR, "II::Donate() - %u is invalid owner", new_owner);
        return;
    }

    if (!IsValidLocationID(new_location)) {
        _log(ITEM__ERROR, "II::Donate() - %u is invalid location", new_location);
        return;
    }

    if ((new_location == m_data.locationID) and (new_flag == m_data.flag) and (new_owner == m_data.ownerID) and !notify)
        return; //nothing to do...

    InventoryItemRef iRef(nullptr);
    uint32 old_location = m_data.locationID, old_owner = m_data.ownerID;
    EVEItemFlags old_flag = m_data.flag;

    if ((new_location != m_data.locationID) // diff container
    or ((new_location == m_data.locationID) // or same container
        and (new_flag != m_data.flag))) {   //  but different flag
        // remove from current location
        if (IsValidLocationID(m_data.locationID)) {
            iRef = sItemFactory.GetItemRef(m_data.locationID);
            if (iRef.get() != nullptr) {
                iRef->RemoveItem(InventoryItemRef(this));
            } else {
                _log(ITEM__ERROR, "II::Donate() - Cant find location %u containing %s.", m_data.locationID, m_data.name.c_str());
            }
        }
    }

    // update data
    m_data.flag = new_flag;
    m_data.ownerID = new_owner;
    m_data.locationID = new_location;

    if ((old_location != m_data.locationID) // diff container
    or ((old_location == m_data.locationID) // or same container
        and (old_flag != m_data.flag))) {   //  but different flag
        // add to new location
        iRef = sItemFactory.GetItemRef(m_data.locationID);
        if (iRef.get() != nullptr) {
            iRef->AddItem(InventoryItemRef(this));
        } else {
            _log(ITEM__ERROR, "II::Donate() - Cant find location %u containing %s.", m_data.locationID, m_data.name.c_str());
        }
    }

    SaveItem();
    //ItemDB::UpdateLocation(m_itemID, m_data.locationID, m_data.flag);

    // changes are cleared after sending, so make 2 sets to send to old owner and new owner
    if (notify) {
        std::map<int32, PyRep *> changes;
        if (new_flag != old_flag)
            changes[Inv::Update::Flag] = new PyInt(old_flag);
        if (new_owner != old_owner)
            changes[Inv::Update::Owner] = new PyInt(old_owner);
        if (new_location != old_location)
            changes[Inv::Update::Location] = new PyInt(old_location);
        SendItemChange(m_data.ownerID, changes);
        if (new_owner != old_owner) {
            if (new_flag != old_flag)
                changes[Inv::Update::Flag] = new PyInt(old_flag);
            if (new_owner != old_owner)
                changes[Inv::Update::Owner] = new PyInt(old_owner);
            if (new_location != old_location)
                changes[Inv::Update::Location] = new PyInt(old_location);
            SendItemChange(old_owner, changes);
        }
    }
}

void InventoryItem::Move(uint32 new_location/*locTemp*/, EVEItemFlags new_flag/*flagNone*/, bool notify/*false*/) {
    if ((new_location == m_data.locationID) and (new_flag == m_data.flag) and !notify)
        return; //nothing to do...

    InventoryItemRef iRef(nullptr);
    uint32 old_location = m_data.locationID;
    EVEItemFlags old_flag = m_data.flag;

    if ((new_location != m_data.locationID) // diff container
    or ((new_location == m_data.locationID) // or same container
        and (new_flag != m_data.flag))) {   //  but different flag
        // remove from current location
        if (IsValidLocationID(m_data.locationID)) {
            iRef = sItemFactory.GetItemRef(m_data.locationID);
            if (iRef.get() != nullptr) {
                iRef->RemoveItem(InventoryItemRef(this));
            } else {
                _log(ITEM__ERROR, "II::Move() - Cant find location %u containing %s.", m_data.locationID, m_data.name.c_str());
            }
        } else {
            _log(ITEM__WARNING, "II::Move()::Remove() - %u is invalid location.", m_data.locationID);
        }
    } else {
        _log(ITEM__TRACE, "II::Move()::Remove() - same same same.");
    }

    // update data
    m_data.flag = new_flag;
    m_data.locationID = new_location;

    if ((old_location != m_data.locationID) // diff container
    or ((old_location == m_data.locationID) // or same container
        and (old_flag != m_data.flag))) {   //  but different flag
        // add to new location
        if (IsValidLocationID(m_data.locationID)) {
            iRef = sItemFactory.GetItemRef(m_data.locationID);
            if (iRef.get() != nullptr) {
                iRef->AddItem(InventoryItemRef(this));
            } else {
                _log(ITEM__ERROR, "II::Move() - Cant find location %u to add %s.", m_data.locationID, m_data.name.c_str());
            }
        } else {
            _log(ITEM__WARNING, "II::Move()::Add() - %u is invalid location.", m_data.locationID);
        }
    } else {
        _log(ITEM__TRACE, "II::Move()::Add() - same same same.");
    }

    if (IsTempItem(m_itemID) or IsNPC(m_itemID))
        return;

    if ((m_data.locationID == locRAMItems || IsValidLocationID(m_data.locationID)) and (!m_delete))
        ItemDB::UpdateLocation(m_itemID, m_data.locationID, m_data.flag);

    //notify about the changes.
    if (notify) {
        std::map<int32, PyRep *> changes;
        if (m_data.locationID != old_location)
            changes[Inv::Update::Location] = new PyInt(old_location);
        if (m_data.flag != old_flag)
            changes[Inv::Update::Flag] = new PyInt(old_flag);
        SendItemChange(m_data.ownerID, changes);   //changes is consumed
    }
}

InventoryItemRef InventoryItem::Split(int32 qty/*0*/, bool notify/*true*/, bool silent/*false*/) {
    if (qty < 1) {
        _log(ITEM__ERROR, "II::Split() - %s(%u): Asked to split into a chunk of %i", m_data.name.c_str(), m_itemID, qty);
        return InventoryItemRef(nullptr);
    }
    if (!AlterQuantity(-qty, notify)) {
        _log(ITEM__ERROR, "II::Split() - %s(%u): Failed to remove quantity of %i during split.", m_data.name.c_str(), m_itemID, qty);
        return InventoryItemRef(nullptr);
    }

    ItemData idata(m_type.id(), m_data.ownerID, locTemp, m_data.flag, qty);
    InventoryItemRef iRef = sItemFactory.SpawnItem(idata);
    if (iRef.get() == nullptr)
        return InventoryItemRef(nullptr);  // couldnt spawn new item...we'll get over it.

    if (silent)
        return iRef;

    iRef->Move(m_data.locationID, m_data.flag, notify);
    return iRef;
}

void InventoryItem::Relocate(uint32 locID, EVEItemFlags flag) {
    if ((locID == m_data.locationID) and (flag == m_data.flag))
        return; //nothing to do...

    InventoryItemRef iRef(nullptr);
    uint32 old_location = m_data.locationID;
    EVEItemFlags old_flag = m_data.flag;

    // update data
    m_data.flag = flag;
    m_data.locationID = locID;

    if ((old_location != m_data.locationID) // diff container
    or ((old_location == m_data.locationID) // or same container
        and (old_flag != m_data.flag))) {   //  but different flag
        // add to new location
        if (IsValidLocationID(m_data.locationID)) {
            iRef = sItemFactory.GetItemRef(m_data.locationID);
            if (iRef.get() != nullptr) {
                iRef->AddItem(InventoryItemRef(this));
            } else {
                _log(ITEM__ERROR, "II::Relocate(): new location %u not found for %s.",
                        m_data.locationID, m_data.name.c_str());
            }
        }
    }

    if (IsValidLocationID(m_data.locationID))
        ItemDB::UpdateLocation(m_itemID, m_data.locationID, m_data.flag);

    //notify about the changes.
    std::map<int32, PyRep *> changes;
    if (m_data.locationID != old_location)
        changes[Inv::Update::Location] = new PyInt(old_location);
    if (m_data.flag != old_flag)
        changes[Inv::Update::Flag] = new PyInt(old_flag);
    SendItemChange(m_data.ownerID, changes);   //changes is consumed
}

bool InventoryItem::Merge(InventoryItemRef to_merge, int32 qty/*0*/, bool notify/*true*/) {
    if (to_merge.get() == nullptr)
        return false;

    if (m_data.singleton or to_merge->isSingleton())
        throw CustomError ("You cannot stack assembled items.");

    if (m_type.id() != to_merge->typeID()) {
        _log(ITEM__WARNING, "II::Merge() - %s (%u): Asked to merge with %s (%u).", m_data.name.c_str(), m_itemID, to_merge->name(), to_merge->itemID());
        return false;
    }

    if (qty < 1)
        qty = to_merge->quantity();

    // AlterQuantity will delete items with qty < 1
    if (!to_merge->AlterQuantity(-qty, notify)) {
        _log(ITEM__ERROR, "II::Merge() - %s (%u): Failed to remove quantity %u.", to_merge->name(), to_merge->itemID(), qty);
        if (IsCharacterID(m_data.ownerID)) {
            Client* pClient = sEntityList.FindClientByCharID(m_data.ownerID);
            if (pClient != nullptr)
                pClient->SendErrorMsg("Internal Server Error.  Ref: ServerError 63138");
        }
        return false;
    }

    if (!AlterQuantity(qty, notify)) {
        _log(ITEM__ERROR, "%s (%u): Failed to add quantity %u.", m_data.name.c_str(), m_itemID, qty);
        if (IsCharacterID(m_data.ownerID)) {
            Client* pClient = sEntityList.FindClientByCharID(m_data.ownerID);
            if (pClient != nullptr)
                pClient->SendErrorMsg("Internal Server Error.  Ref: ServerError 63238");
        }
        return false;
    }

    return true;
}

void InventoryItem::MergeTypesInCargo(ShipItem* pShip, EVEItemFlags flag/*flagNone*/)
{
    // get existing type in cargo
    InventoryItemRef iRef = pShip->GetMyInventory()->GetByTypeFlag(m_type.id(), flag);
    if (iRef.get() == nullptr) {
        Move(pShip->itemID(), flag, true);
        return;
    }
    // fix for elusive error when using IB::Add() to remove loaded modules (charge trying to add to module item)
    if (iRef->typeID() != m_type.id()) {
        Move(pShip->itemID(), flag, true);
        return;
    }

    // if either item is assembled, just move item (merge will throw on assembled items)
    if (iRef->isSingleton() or m_data.singleton) {
        Move(pShip->itemID(), flag, true);
        return;
    }
    // here we 'merge' with stack already in cargo
    //  if it dont work, just move item.
    if (!iRef->Merge(InventoryItemRef(this)))
        Move(pShip->itemID(), flag, true);
}

bool InventoryItem::AlterQuantity(int32 qty, bool notify/*false*/) {
    if (qty == 0) {
        _log(ITEM__WARNING, "II::AlterQuantity() - Sent qty=0 for %s(%u)", m_data.name.c_str(), m_itemID);
        EvE::traceStack();
        return false;
    }

    int32 new_qty = m_data.quantity + qty;
    if (new_qty < 0) {
        codelog(ITEM__ERROR, "II::AlterQuantity() - %s(%u): Tried to remove %i from stack of %i for ownerID %u.", \
                m_data.name.c_str(), m_itemID, qty, m_data.quantity, m_data.ownerID);
        // make player error msg here.....
        Delete();
        return false;
    }

    return SetQuantity(new_qty, notify);
}

bool InventoryItem::SetQuantity(int32 qty, bool notify/*false*/, bool deleteOnZero/*true*/) {
    //if an object is singleton, there is only one, and it shouldn't be able to alter qty
    if (m_data.singleton) {
        _log(ITEM__ERROR, "II::SetQuantity() - %s(%u): Failed to set quantity %i; the items singleton bit is set", m_data.name.c_str(), m_itemID, qty);
        // make player error msg here.....
        return false;
    }
    int32 old_qty = m_data.quantity;
    m_data.quantity = qty;

    /* this isnt needed.  quantity has hard limit.
    if (m_data.quantity > maxEveItem) {
        codelog(ITEM__ERROR, "II::SetQuantity() - %s(%u): quantity overflow", m_data.name.c_str(), m_itemID);
        m_data.quantity = maxEveItem -1;
        if (IsCharacterID(m_data.ownerID)) {
            Client* pClient = sEntityList.FindClientByCharID(m_data.ownerID);
            if (pClient != nullptr)
                pClient->SendInfoModalMsg("Your %s has reached quantity limits of this server.<br>If you try to add any more to this stack, you will lose items.  This is your only warning.", m_data.name.c_str());
        }
    } */

    if (notify or (IsCargoHoldFlag(m_data.flag) and (m_type.categoryID() == EVEDB::invCategories::Charge))) {
        std::map<int32, PyRep *> changes;
        changes[Inv::Update::StackSize] = new PyInt(old_qty);
        SendItemChange(m_data.ownerID, changes); //changes is consumed
    }

    // how are we gonna do modules owned by corp here???
    if (IsFittingSlot(m_data.flag) and (m_type.categoryID() == EVEDB::invCategories::Charge))
        if (IsCharacterID(m_data.ownerID)/* or IsPlayerCorp(m_data.ownerID)*/) {
            Client* pClient = sEntityList.FindClientByCharID(m_data.ownerID);
            SetAttribute(AttrQuantity, m_data.quantity, pClient == nullptr ? notify : pClient->IsInSpace());
        }

    if (m_data.quantity < 1) {
        if (deleteOnZero)
            Delete();
        return true;
    }

    if (sConfig.world.saveOnUpdate)
        SaveItem();

    return true;
}

bool InventoryItem::SetFlag(EVEItemFlags flag, bool notify/*false*/) {
    if (m_data.flag == flag)
        return true;

    EVEItemFlags old_flag = m_data.flag;
    m_data.flag = flag;

    ItemDB::UpdateLocation(m_itemID, m_data.locationID, m_data.flag);

    if (notify) {
        std::map<int32, PyRep *> changes;
        changes[Inv::Update::Flag] = new PyInt(old_flag);
        SendItemChange(m_data.ownerID, changes); //changes is consumed
    }

    return true;
}

bool InventoryItem::ChangeSingleton(bool singleton, bool notify/*false*/) {
    if (singleton == m_data.singleton)
        return true;    //nothing to do...

    bool old_singleton(m_data.singleton);
    m_data.singleton = singleton;

    //verify quantity is -1 for singletons
    if (m_data.singleton)
        if (m_data.quantity > 1) {
            _log(ITEM__WARNING, "%s(%u) is changing singleton to %s and qty is currently %i", \
                    m_data.name.c_str(), m_itemID, singleton?"On":"Off", m_data.quantity);
            m_data.quantity = -1;
        }

    if (sConfig.world.saveOnUpdate)
        SaveItem();

    if (notify) {
        std::map<int32, PyRep *> changes;
        changes[Inv::Update::Singleton] = new PyInt(old_singleton);
        SendItemChange(m_data.ownerID, changes); //changes is consumed
    }

    // must update volume when singleton (packaged state) changes for (mostly) ship items.
    SetAttribute(AttrVolume, GetPackagedVolume(), notify);
    return true;
}

void InventoryItem::ChangeOwner(uint32 new_owner, bool notify/*false*/) {
    if (new_owner == m_data.ownerID)
        return; //nothing to do...

    uint32 old_owner = m_data.ownerID;
    m_data.ownerID = new_owner;

    if (sConfig.world.saveOnUpdate)
        SaveItem();

    //notify about the changes.
    if (notify) {
        std::map<int32, PyRep *> changes;
        //send the notify to the new owner.
        changes[Inv::Update::Owner] = new PyInt(old_owner);
        SendItemChange(new_owner, changes); //changes is consumed
        //also send the notify to the old owner.
        changes[Inv::Update::Owner] = new PyInt(old_owner);
        SendItemChange(old_owner, changes); //changes is consumed
    }
}

//contents of changes are consumed and cleared
void InventoryItem::SendItemChange(uint32 toID, std::map<int32, PyRep *> &changes) {
    if (IsNPCCorp(toID) or (toID == 1) or IsFaction(toID))   //IsValidOwner()
        return;
    if (sConsole.IsShutdown())
        return;
    if (changes.empty())
        return;

    NotifyOnItemChange change;
        change.itemRow = GetItemRow();
        change.changes = changes;
    PyTuple *tmp = change.Encode();

    if (is_log_enabled(ITEM__CHANGE)) {
        _log(ITEM__CHANGE, "Sending Item changes for %s(%u)", m_data.name.c_str(), m_itemID);
        tmp->Dump(ITEM__CHANGE, "    ");
    }

    //TODO: figure out the appropriate list of interested people...
    if (IsCharacterID(toID)) {
        Client* pClient = sEntityList.FindClientByCharID(toID);
        if (pClient == nullptr)
            return;
        if (pClient->IsCharCreation())
            return;
        //if (IsShipItem()) //(pClient->IsBoard())
        //    pClient->SendNotification("OnItemsChanged", "charid", &tmp, false); //unsequenced.  <<--  this is called for multiple items
        //else
            pClient->SendNotification("OnItemChange", "clientID", &tmp, false); //unsequenced.  <<-- this is for single items
    } else if (IsPlayerCorp(toID)) {
        if (sDataMgr.IsStation(m_data.locationID)) {
            sEntityList.CorpNotify(toID, Notify::Types::ItemUpdateStation, "OnItemChange","*stationid&corpid", tmp);
        } else {
            sEntityList.CorpNotify(toID, Notify::Types::ItemUpdateSystem, "OnItemChange","corpid", tmp);
        }
    }
}

void InventoryItem::SaveItem()
{
    ItemData data(m_data.name.c_str(),
                  m_type.id(),
                  m_data.ownerID,
                  m_data.locationID,
                  m_data.flag,
                  m_data.contraband,
                  m_data.singleton,
                  m_data.quantity,
                  m_data.position,
                  customInfo().c_str()
                  );

    ItemDB::SaveItem(m_itemID, data);
    // item attributes are saved in ItemFactory.cpp:96  (save loop on shutdown for loaded items)
    // make call here for items saved after *some* change
    pAttributeMap->Save();
}

void InventoryItem::UpdateLocation() {
    ItemDB::UpdateLocation(m_itemID, m_data.locationID, m_data.flag);
}

PyPackedRow* InventoryItem::GetItemStatusRow() const {
    DBRowDescriptor* header = new DBRowDescriptor();
        header->AddColumn("instanceID",    DBTYPE_I8);
        header->AddColumn("online",        DBTYPE_BOOL);
        header->AddColumn("damage",        DBTYPE_R8);
        header->AddColumn("charge",        DBTYPE_R8);
        header->AddColumn("skillPoints",   DBTYPE_I4);
        header->AddColumn("armorDamage",   DBTYPE_R8);
        header->AddColumn("shieldCharge",  DBTYPE_R8);
        header->AddColumn("incapacitated", DBTYPE_BOOL);
    PyPackedRow* row = new PyPackedRow(header);
    GetItemStatusRow(row);
    return row;
}

void InventoryItem::GetItemStatusRow(PyPackedRow* into ) const {
    into->SetField("instanceID",    new PyLong(m_itemID ));
    into->SetField("online",        new PyBool((HasAttribute(AttrOnline) ? GetAttribute(AttrOnline).get_bool() : false) ));
    into->SetField("damage",        new PyFloat((HasAttribute(AttrDamage) ? GetAttribute(AttrDamage).get_float() : 0) ));
    into->SetField("charge",        new PyFloat((HasAttribute(AttrCapacitorCharge) ? GetAttribute(AttrCapacitorCharge).get_float() : 0) ));
    into->SetField("skillPoints",   new PyInt((HasAttribute(AttrSkillPoints) ? GetAttribute(AttrSkillPoints).get_uint32() : 0) ));
    into->SetField("armorDamage",   new PyFloat((HasAttribute(AttrArmorDamageAmount) ? GetAttribute(AttrArmorDamageAmount).get_float() : 0.0) ));
    into->SetField("shieldCharge",  new PyFloat((HasAttribute(AttrShieldCharge) ? GetAttribute(AttrShieldCharge).get_float() : 0.0) ));
    into->SetField("incapacitated", new PyBool((HasAttribute(AttrIsIncapacitated) ? GetAttribute(AttrIsIncapacitated).get_bool() : false) ));
}

/*  charge info for specific module  */
PyPackedRow* InventoryItem::GetChargeStatusRow(uint32 shipID) const {
    DBRowDescriptor* header = new DBRowDescriptor();
        header->AddColumn("instanceID", DBTYPE_I8);
        header->AddColumn("flagID",     DBTYPE_I2);
        header->AddColumn("typeID",     DBTYPE_I4);
        //header->AddColumn("quantity",   DBTYPE_I4);
    PyPackedRow* row = new PyPackedRow(header);
    GetChargeStatusRow(shipID, row);
    return row;
}

void InventoryItem::GetChargeStatusRow(uint32 shipID, PyPackedRow* into) const {
    into->SetField("instanceID",     new PyLong(shipID));  // locationID
    into->SetField("flagID",         new PyInt(m_data.flag));
    into->SetField("typeID",         new PyInt(m_type.id()));
}

PyPackedRow* InventoryItem::GetItemRow() const
{
    PyPackedRow* row = new PyPackedRow( sDataMgr.CreateHeader() );
    GetItemRow(row);
    return row;
}

void InventoryItem::GetItemRow(PyPackedRow* into) const
{
    int32 qty = (m_data.singleton ? -1 : m_data.quantity);
    if (m_type.categoryID() == EVEDB::invCategories::Blueprint)
        if (sItemFactory.GetBlueprintRef(m_itemID)->copy())
            qty = -2;

    into->SetField("itemID",       new PyLong(m_itemID));
    into->SetField("typeID",       new PyInt(m_type.id()));
    into->SetField("ownerID",      new PyInt(m_data.ownerID));
    into->SetField("locationID",   new PyInt(m_data.locationID));
    into->SetField("flagID",       new PyInt(m_data.flag));
    into->SetField("groupID",      new PyInt(type().groupID()));
    into->SetField("categoryID",   new PyInt(type().categoryID()));
    into->SetField("quantity",     new PyInt(qty));
    /*
    if (m_type.categoryID() == EVEDB::invCategories::Blueprint) {
        if (sItemFactory.GetBlueprintRef(m_itemID)->copy()) {
            into->SetField("stacksize",    new PyInt(1));
            into->SetField("singleton",    new PyInt(2));
        } else {
            into->SetField("stacksize",    new PyInt(m_data.singleton? -1 : m_data.quantity));
            into->SetField("singleton",    new PyInt(m_data.singleton?1:0));
        }
    } else {
        into->SetField("stacksize",    new PyInt(m_data.singleton? -1 : m_data.quantity));
        into->SetField("singleton",    new PyInt(m_data.singleton?1:0));
    }
    */
    // customInfo is actually used in client (but i dont think it's a string)
    //if const.ixLocationID in change and item.customInfo == logConst.eventUndock:
    into->SetField("customInfo",   new PyString(m_data.customInfo));
}

bool InventoryItem::Populate(Rsp_CommonGetInfo_Entry& result )
{
    /** @todo  this may need to be reworked once POS and Outposts are implemented. */

    //make sure trash data is removed from &result
    result.attributes.clear();
    PySafeDecRef(result.itemID);
    PySafeDecRef(result.invItem);
    result.time = GetFileTimeNow();
    // this is only to display item name in logs.  client ignores it
    result.description = m_data.name;

    // updated charge info...again  -allan 8Jan20
    if ((m_type.categoryID() == EVEDB::invCategories::Charge)
    and IsFittingSlot(m_data.flag)) {
        PyTuple* tuple = new PyTuple(3);
            tuple->SetItem(0, new PyInt(m_data.locationID));
            tuple->SetItem(1, new PyInt(m_data.flag));
            tuple->SetItem(2, new PyInt(m_type.id()));
        result.itemID = tuple;
        result.invItem = PyStatic.NewNone();
        for (AttrMapItr itr = pAttributeMap->begin(), end = pAttributeMap->end(); itr != end; ++itr)
            result.attributes[(*itr).first] = (*itr).second.GetPyObject();
        return true;
    }

    result.itemID = new PyInt(m_itemID);
    result.invItem = GetItemRow();

    if (m_type.categoryID() == EVEDB::invCategories::Skill) {
        result.attributes[AttrSkillTimeConstant] = GetAttribute(AttrSkillTimeConstant).GetPyObject();
        result.attributes[AttrSkillPoints] = GetAttribute(AttrSkillPoints).GetPyObject();
        result.attributes[AttrSkillLevel] = GetAttribute(AttrSkillLevel).GetPyObject();
        return true;
    }
    //} else if (m_type.id() == 51) { // for vouchers
    //    result.description = m_data.name;

    if (pAttributeMap->GetAttribute(AttrOnline).get_bool()) {
        EntityEffectState es;
            es.env_itemID = m_itemID;
            es.env_charID = m_data.ownerID;
            es.env_shipID = m_data.locationID;
            es.env_target = m_data.locationID;
            es.env_other = PyStatic.NewNone();
            es.env_area = PyStatic.NewNone();
            es.env_effectID = 16;
            /** @todo fix this once we start tracking effects */
            // on login, this is current time
            if (IsCharacterID(m_itemID)) {
                es.startTime = GetFileTimeNow() - EvE::Time::Minute; // m_timestamp
            } else {
                es.startTime = GetFileTimeNow() - EvE::Time::Minute; // GetAttribute(AttrStartTime).get_int();
            }
            es.duration = -1;
            es.repeat = 0;
            es.randomSeed = PyStatic.NewNone();
        result.activeEffects[es.env_effectID] = es.Encode();
    }

    for (AttrMapItr itr = pAttributeMap->begin(), end = pAttributeMap->end(); itr != end; ++itr) {
        //localization.GetByLabel('UI/Fitting/FittingWindow/WarpSpeed', distText=util.FmtDist(max(1.0, bws) * wsm * 3 * const.AU, 2))
        if ((*itr).first == AttrWarpSpeedMultiplier) {
            result.attributes[AttrWarpSpeedMultiplier] = new PyFloat((*itr).second.get_float() /3);
        } else {
            result.attributes[(*itr).first] = (*itr).second.GetPyObject();
        }
    }

    return true;
}

PyList* InventoryItem::GetItemInfo() const
{
    PyList* itemInfo = new PyList();
        itemInfo->AddItem(GetItemRow());
    return itemInfo;
}

PyObject* InventoryItem::ItemGetInfo()
{   // called from dogmaBound at least once (usually 2x) on every weapon update
    Rsp_ItemGetInfo result;
    if (!Populate(result.entry))
        return nullptr;
    return result.Encode();
}

void InventoryItem::SetCustomInfo(const char *ci) {
    if (ci != nullptr) {
        m_data.customInfo = ci;
    } else {
        m_data.customInfo = "";
    }

    if (sConfig.world.saveOnUpdate)
        SaveItem();
}

void InventoryItem::SetPosition(const GPoint& pos)
{
    if (m_data.position == pos)
        return;
    /*
    if (pos.isZero() and sDataMgr.IsSolarSystem(m_data.locationID)) {
        _log(DESTINY__TRACE, "II::SetPosition() - %s(%u) point is zero", m_data.name.c_str(), m_itemID);
        EvE::traceStack();
    } */

    m_data.position = pos;
    _log(ITEM__RELOCATE, "%s(%u) Relocating to %.2f, %.2f, %.2f.", m_data.name.c_str(), \
            m_itemID, m_data.position.x, m_data.position.y, m_data.position.z);
}

void InventoryItem::SetRadius(double radius)
{
    this->SetAttribute(AttrRadius, radius);
    _log(ITEM__RELOCATE, "%s(%u) Relocating to %.2f, %.2f, %.2f.", m_data.name.c_str(), \
            m_itemID, m_data.position.x, m_data.position.y, m_data.position.z);
}

void InventoryItem::SetAttribute(uint16 attrID, float num, bool notify/*true*/)
{
    EvilNumber eNum(num);
    pAttributeMap->SetAttribute(attrID, eNum, notify);
}

void InventoryItem::SetAttribute(uint16 attrID, double num, bool notify/*true*/)
{
    EvilNumber eNum(num);
    pAttributeMap->SetAttribute(attrID, eNum, notify);
}

void InventoryItem::SetAttribute(uint16 attrID, EvilNumber num, bool notify/*true*/)
{
    pAttributeMap->SetAttribute(attrID, num, notify);
}

void InventoryItem::SetAttribute(uint16 attrID, int num, bool notify/*true*/)
{
    EvilNumber eNum(num);
    pAttributeMap->SetAttribute(attrID, eNum, notify);
}

void InventoryItem::SetAttribute(uint16 attrID, int64 num, bool notify/*true*/)
{
    EvilNumber eNum(num);
    pAttributeMap->SetAttribute(attrID, eNum, notify);
}

void InventoryItem::SetAttribute(uint16 attrID, uint32 num, bool notify/*true*/)
{
    EvilNumber eNum(num);
    pAttributeMap->SetAttribute(attrID, eNum, notify);
}

void InventoryItem::MultiplyAttribute(uint16 attrID, EvilNumber num, bool notify/*false*/)
{
    pAttributeMap->MultiplyAttribute(attrID, num, notify);
}

double InventoryItem::GetPackagedVolume()
{
    if (m_data.singleton)
        return m_type.volume();

    // these volumes are hard-coded in client.
    switch (m_type.groupID()) {
        case 29:        //Capsule
        case 31:        //Shuttle
        case 1022: {    //Prototype Exploration Ship
            return 500;
        }
        case 12:        //Cargo Container
        case 306:       //Spawn Container
        case 340:       //Secure Cargo Container
        case 448:       //Audit Log Secure Container
        case 649:       //Freight Container
        case 952: {     //Mission Container
            return 1000;
        }
        case 25:        //Frigate
        case 324:       //Assault Ship
        case 830:       //Covert Ops
        case 893:       //Electronic Attack Ship
        case 831:       //Interceptor
        case 834: {     //Stealth Bomber
            return 2500;
        }
        case 543:       //Exhumer
        case 463: {     //Mining Barge
            return 3750;
        }
        case 541:       //Interdictor
        case 420:       //Destroyer
        case 963: {     //Strategic Cruiser
            return 5000;
        }
        case 906:       //Combat Recon Ship
        case 26:        //Cruiser
        case 833:       //Force Recon Ship
        case 358:       //Heavy Assault Ship
        case 894:       //Heavy Interdictor
        case 832: {     //Logistics
            return 10000;
        }
        case 419:       //Battlecruiser
        case 540: {     //Command Ship
            return 15000;
        }
        case 28:        //Industrial
        case 380: {     //Transport Ship
            return 20000;
        }
        case 27:        //Battleship
        case 900:       //Marauder
        case 898:       //Black Ops
        case 381: {     //Elite Battleship
            return 50000;
        }
        case 941: {     //Industrial Command Ship
            return 500000;
        }
        case 883:       //Capital Industrial Ship
        case 547:       //Carrier
        case 485:       //Dreadnought
        case 513:       //Freighter
        case 902:       //Jump Freighter
        case 659: {     //Supercarrier
            return 1000000;
        }
        case 30: {      //Titan
            return 10000000;
        }
    }

    // return basic volume for non-ship or non-container objects
    return m_type.volume();
}

bool InventoryItem::SkillCheck(InventoryItemRef refItem)
{
    EvilNumber need = 0, has = 0;
    uint16 attr = 182, skill = 277;
    for (int8 i = 0; i < 3; ++i, ++attr, ++skill) {
        if (refItem->HasAttribute(attr, need) and HasAttribute(skill, has)) {
            if (need > has)
                return false;
        }
    }
    if (refItem->HasAttribute(1285, need) and HasAttribute(1286, has)) {
        if (need > has)
            return false;
    }

    attr = 1289; skill = 1287;
    for (int8 i = 0; i < 2; ++i, ++attr, ++skill) {
        if (refItem->HasAttribute(attr, need) and HasAttribute(skill, has)) {
            if (need > has)
                return false;
        }
    }
    // all skill requirement checks passed.
    return true;
}

// new effects system  -allan 4Feb17
void InventoryItem::AddModifier(fxData &data)
{
    ModifierContainer* modifierContainer = static_cast<ModifierContainer*>(m_modifierContainer);
    modifierContainer->modifiers.emplace(data.math, data);
    if (is_log_enabled(EFFECTS__TRACE))
        _log(EFFECTS__TRACE, "II::AddModifier(): %s(%u) Added to map - <%s>, fxSrc:%s:%s(%u), targ:%s:%s(%u).", \
            m_data.name.c_str(), m_itemID, sFxProc.GetMathMethodName(data.math), \
            sFxProc.GetSourceName(data.fxSrc), sDataMgr.GetAttrName(data.srcAttr), data.srcAttr, \
            sFxProc.GetTargLocName(data.targLoc), sDataMgr.GetAttrName(data.targAttr), data.targAttr);
    // grpID, typeID, srcRef
}

void InventoryItem::RemoveModifier(fxData &data)
{
    switch (data.math) {
        case FX::Math::PreMul:         data.math = FX::Math::PreDiv;          break;
        case FX::Math::PreDiv:         data.math = FX::Math::PreMul;          break;
        case FX::Math::ModAdd:         data.math = FX::Math::ModSub;          break;
        case FX::Math::ModSub:         data.math = FX::Math::ModAdd;          break;
        case FX::Math::PostMul:        data.math = FX::Math::PostDiv;         break;
        case FX::Math::PostDiv:        data.math = FX::Math::PostMul;         break;
        case FX::Math::PostPercent:    data.math = FX::Math::RevPostPercent;  break;
        case FX::Math::PreAssignment:  data.math = FX::Math::PostAssignment;  break;
        case FX::Math::PostAssignment: data.math = FX::Math::PreAssignment;   break;
    }

    ModifierContainer* modifierContainer = static_cast<ModifierContainer*>(m_modifierContainer);
    modifierContainer->modifiers.emplace(data.math, data);
    if (is_log_enabled(EFFECTS__TRACE))
        _log(EFFECTS__TRACE, "II::RemoveModifier(): %s(%u) Removed from map - <%s>, fxSrc:%s:%s(%u), targ:%s:%s(%u).", \
        m_data.name.c_str(), m_itemID, sFxProc.GetMathMethodName(data.math), \
        sFxProc.GetSourceName(data.fxSrc), sDataMgr.GetAttrName(data.srcAttr), data.srcAttr, \
        sFxProc.GetTargLocName(data.targLoc), sDataMgr.GetAttrName(data.targAttr), data.targAttr);
    // grpID, typeID, srcRef
}

void InventoryItem::ClearModifiers()
{
    _log(EFFECTS__TRACE, "Clearing modifier map for %s", m_data.name.c_str());

    ModifierContainer* modifierContainer = static_cast<ModifierContainer*>(m_modifierContainer);
    modifierContainer->modifiers.clear();
}

void InventoryItem::ResetAttributes() {
    _log(EFFECTS__TRACE, "Resetting attrib map for %s", m_data.name.c_str());
    pAttributeMap->Load(true);
}

std::multimap<int8, fxData>& InventoryItem::GetModifiers()
{
    ModifierContainer* modifierContainer = static_cast<ModifierContainer*>(m_modifierContainer);
    return modifierContainer->modifiers;
}
