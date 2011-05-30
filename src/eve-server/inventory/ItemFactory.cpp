/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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

#include "EVEServerPCH.h"

ItemFactory::ItemFactory(EntityList& el) : entity_list(el) {}

ItemFactory::~ItemFactory() {
    // items
    {
        std::map<uint32, InventoryItemRef>::const_iterator cur, end;
        cur = m_items.begin();
        end = m_items.end();
        for(; cur != end; cur++) {
            // save attributes of item (because of rechargable attributes)
            //cur->second->attributes.Save();
        }
    }
    // types
    {
        std::map<uint32, ItemType *>::const_iterator cur, end;
        cur = m_types.begin();
        end = m_types.end();
        for(; cur != end; cur++)
            delete cur->second;
    }
    // groups
    {
        std::map<uint32, ItemGroup *>::const_iterator cur, end;
        cur = m_groups.begin();
        end = m_groups.end();
        for(; cur != end; cur++)
            delete cur->second;
    }
    // categories
    {
        std::map<EVEItemCategories, ItemCategory *>::const_iterator cur, end;
        cur = m_categories.begin();
        end = m_categories.end();
        for(; cur != end; cur++)
            delete cur->second;
    }

    // Set Client pointer to NULL
    m_pClient = NULL;
}

const ItemCategory *ItemFactory::GetCategory(EVEItemCategories category) {
    std::map<EVEItemCategories, ItemCategory *>::iterator res = m_categories.find(category);
    if(res == m_categories.end()) {
        ItemCategory *cat = ItemCategory::Load(*this, category);
        if(cat == NULL)
            return NULL;

        // insert it into our cache
        res = m_categories.insert(
            std::make_pair(category, cat)
        ).first;
    }
    return(res->second);
}

const ItemGroup *ItemFactory::GetGroup(uint32 groupID) {
    std::map<uint32, ItemGroup *>::iterator res = m_groups.find(groupID);
    if(res == m_groups.end()) {
        ItemGroup *group = ItemGroup::Load(*this, groupID);
        if(group == NULL)
            return NULL;

        // insert it into cache
        res = m_groups.insert(
            std::make_pair(groupID, group)
        ).first;
    }
    return(res->second);
}

template<class _Ty>
const _Ty *ItemFactory::_GetType(uint32 typeID) {
    std::map<uint32, ItemType *>::iterator res = m_types.find(typeID);
    if(res == m_types.end()) {
        _Ty *type = _Ty::Load(*this, typeID);
        if(type == NULL)
            return NULL;

        // insert into cache
        res = m_types.insert(
            std::make_pair(typeID, type)
        ).first;
    }
    return static_cast<const _Ty *>(res->second);
}

const ItemType *ItemFactory::GetType(uint32 typeID) {
    return _GetType<ItemType>(typeID);
}

const BlueprintType *ItemFactory::GetBlueprintType(uint32 blueprintTypeID) {
    return _GetType<BlueprintType>(blueprintTypeID);
}

const CharacterType *ItemFactory::GetCharacterType(uint32 characterTypeID) {
    return _GetType<CharacterType>(characterTypeID);
}

const CharacterType *ItemFactory::GetCharacterTypeByBloodline(uint32 bloodlineID) {
    // Unfortunately, we have it indexed by typeID, so we must get it ...
    uint32 characterTypeID;
    if(!db().GetCharacterTypeByBloodline(bloodlineID, characterTypeID))
        return NULL;
    return GetCharacterType(characterTypeID);
}

const ShipType *ItemFactory::GetShipType(uint32 shipTypeID) {
    return _GetType<ShipType>(shipTypeID);
}

const StationType *ItemFactory::GetStationType(uint32 stationTypeID) {
    return _GetType<StationType>(stationTypeID);
}

template<class _Ty>
RefPtr<_Ty> ItemFactory::_GetItem(uint32 itemID)
{
    std::map<uint32, InventoryItemRef>::iterator res = m_items.find( itemID );
    if( res == m_items.end() )
    {
        // load the item
        RefPtr<_Ty> item = _Ty::Load( *this, itemID );
        if( !item )
            return RefPtr<_Ty>();

        //we keep the original ref.
        res = m_items.insert( std::make_pair( itemID, item ) ).first;
    }
    // return to the user.
    return RefPtr<_Ty>::StaticCast( res->second );
}

InventoryItemRef ItemFactory::GetItem(uint32 itemID)
{
    return _GetItem<InventoryItem>( itemID );
}

BlueprintRef ItemFactory::GetBlueprint(uint32 blueprintID)
{
    return _GetItem<Blueprint>( blueprintID );
}

CharacterRef ItemFactory::GetCharacter(uint32 characterID)
{
    return _GetItem<Character>( characterID );
}

ShipRef ItemFactory::GetShip(uint32 shipID)
{
    return _GetItem<Ship>( shipID );
}

CelestialObjectRef ItemFactory::GetCelestialObject(uint32 celestialID)
{
    return _GetItem<CelestialObject>( celestialID );
}

SolarSystemRef ItemFactory::GetSolarSystem(uint32 solarSystemID)
{
    return _GetItem<SolarSystem>( solarSystemID );
}

StationRef ItemFactory::GetStation(uint32 stationID)
{
    return _GetItem<Station>( stationID );
}

SkillRef ItemFactory::GetSkill(uint32 skillID)
{
    return _GetItem<Skill>( skillID );
}

OwnerRef ItemFactory::GetOwner(uint32 ownerID)
{
    return _GetItem<Owner>( ownerID );
}

StructureRef ItemFactory::GetStructure(uint32 structureID)
{
    return _GetItem<Structure>( structureID );
}

CargoContainerRef ItemFactory::GetCargoContainer(uint32 containerID)
{
    return _GetItem<CargoContainer>( containerID );
}

InventoryItemRef ItemFactory::SpawnItem(ItemData &data) {
    InventoryItemRef i = InventoryItem::Spawn(*this, data);
    if( !i )
        return InventoryItemRef();

    // spawn successful; store the ref
    m_items.insert( std::make_pair( i->itemID(), i ) );
    return i;
}

BlueprintRef ItemFactory::SpawnBlueprint(ItemData &data, BlueprintData &bpData) {
    BlueprintRef bi = Blueprint::Spawn(*this, data, bpData);
    if( !bi )
        return BlueprintRef();

    m_items.insert( std::make_pair( bi->itemID(), bi ) );
    return bi;
}

CharacterRef ItemFactory::SpawnCharacter(ItemData &data, CharacterData &charData, CharacterAppearance &appData, CorpMemberInfo &corpData) {
    CharacterRef c = Character::Spawn(*this, data, charData, appData, corpData);
    if( !c )
        return CharacterRef();

    m_items.insert( std::make_pair( c->itemID(), c ) );
    return c;
}

ShipRef ItemFactory::SpawnShip(ItemData &data) {
    ShipRef s = Ship::Spawn(*this, data);
    if( !s )
        return ShipRef();

    m_items.insert( std::make_pair( s->itemID(), s ) );
    return s;
}

SkillRef ItemFactory::SpawnSkill(ItemData &data)
{
    SkillRef s = Skill::Spawn( *this, data );
    if( !s )
        return SkillRef();

    m_items.insert( std::make_pair( s->itemID(), s ) );
    return s;
}

OwnerRef ItemFactory::SpawnOwner(ItemData &data)
{
    OwnerRef o = Owner::Spawn( *this, data );
    if( !o )
        return OwnerRef();

    m_items.insert( std::make_pair( o->itemID(), o ) );
    return o;
}

StructureRef ItemFactory::SpawnStructure(ItemData &data)
{
    StructureRef o = Structure::Spawn( *this, data );
    if( !o )
        return StructureRef();

    m_items.insert( std::make_pair( o->itemID(), o ) );
    return o;
}

CargoContainerRef ItemFactory::SpawnCargoContainer(ItemData &data)
{
    CargoContainerRef o = CargoContainer::Spawn( *this, data );
    if( !o )
        return CargoContainerRef();

    m_items.insert( std::make_pair( o->itemID(), o ) );
    return o;
}

Inventory *ItemFactory::GetInventory(uint32 inventoryID, bool load)
{
    InventoryItemRef item;

    if( load )
        item = GetItem( inventoryID );
    else
    {
        std::map<uint32, InventoryItemRef>::iterator res = m_items.find( inventoryID );
        if( res != m_items.end() )
            item = res->second;
    }

    return Inventory::Cast( item );
}

void ItemFactory::_DeleteItem(uint32 itemID)
{
    std::map<uint32, InventoryItemRef>::iterator res = m_items.find( itemID );
    if( res == m_items.end() )
    {
        sLog.Error("Item Factory", "Item ID %u not found when requesting deletion!", itemID );
    }
    else
    {
        m_items.erase( res );
    }
}

void ItemFactory::SetUsingClient(Client *pClient)
{
    m_pClient = pClient;
}

Client * ItemFactory::GetUsingClient()
{
    return m_pClient;
}
