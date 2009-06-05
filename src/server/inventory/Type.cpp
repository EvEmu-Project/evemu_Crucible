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
	Author:		Bloody.Rabbit
*/

#include "EvemuPCH.h"

/*
 * CategoryData
 */
CategoryData::CategoryData(
	const char *_name,
	const char *_desc,
	bool _published)
: name(_name),
  description(_desc),
  published(_published)
{
}

/*
 * Category
 */
Category::Category(
	EVEItemCategories _id,
	// Category stuff:
	const CategoryData &_data)
: m_id(_id),
  m_name(_data.name),
  m_description(_data.description),
  m_published(_data.published)
{
	_log(ITEM__TRACE, "Created object %p for category %s (%u).", this, name().c_str(), (uint32)id());
}

Category *Category::Load(ItemFactory &factory, EVEItemCategories category) {
	// create category
	Category *c = Category::_Load(factory, category);
	if(c == NULL)
		return NULL;

	// Category has no virtual _Load()

	return(c);
}

Category *Category::_Load(ItemFactory &factory, EVEItemCategories category
) {
	// pull data
	CategoryData data;
	if(!factory.db().GetCategory(category, data))
		return NULL;

	return(
		Category::_Load(factory, category, data)
	);
}

Category *Category::_Load(ItemFactory &factory, EVEItemCategories category,
	// Category stuff:
	const CategoryData &data
) {
	// enough data for construction
	return(new Category(
		category, data
	));
}

/*
 * GroupData
 */
GroupData::GroupData(
	EVEItemCategories _category,
	const char *_name,
	const char *_desc,
	bool _useBasePrice,
	bool _allowManufacture,
	bool _allowRecycler,
	bool _anchored,
	bool _anchorable,
	bool _fittableNonSingleton,
	bool _published)
: category(_category),
  name(_name),
  description(_desc),
  useBasePrice(_useBasePrice),
  allowManufacture(_allowManufacture),
  allowRecycler(_allowRecycler),
  anchored(_anchored),
  anchorable(_anchorable),
  fittableNonSingleton(_fittableNonSingleton),
  published(_published)
{
}

/*
 * Group
 */
Group::Group(
	uint32 _id,
	// Group stuff:
	const Category &_category,
	const GroupData &_data)
: m_id(_id),
  m_category(&_category),
  m_name(_data.name),
  m_description(_data.description),
  m_useBasePrice(_data.useBasePrice),
  m_allowManufacture(_data.allowManufacture),
  m_allowRecycler(_data.allowRecycler),
  m_anchored(_data.anchored),
  m_anchorable(_data.anchorable),
  m_fittableNonSingleton(_data.fittableNonSingleton),
  m_published(_data.published)
{
	// assert for data consistency
	assert(_data.category == _category.id());

	_log(ITEM__TRACE, "Created object %p for group %s (%u).", this, name().c_str(), id());
}

Group *Group::Load(ItemFactory &factory, uint32 groupID) {
	// create group
	Group *g = Group::_Load(factory, groupID);
	if(g == NULL)
		return NULL;

	// Group has no virtual _Load()

	return(g);
}

Group *Group::_Load(ItemFactory &factory, uint32 groupID
) {
	// pull data
	GroupData data;
	if(!factory.db().GetGroup(groupID, data))
		return NULL;

	// retrieve category
	const Category *c = factory.GetCategory(data.category);
	if(c == NULL)
		return NULL;

	return(
		Group::_Load(factory, groupID, *c, data)
	);
}

Group *Group::_Load(ItemFactory &factory, uint32 groupID,
	// Group stuff:
	const Category &category, const GroupData &data
) {
	// enough data for construction
	return(new Group(
		groupID, category, data
	));
}

/*
 * TypeData
 */
TypeData::TypeData(
	uint32 _groupID,
	const char *_name,
	const char *_desc,
	double _radius,
	double _mass,
	double _volume,
	double _capacity,
	uint32 _portionSize,
	EVERace _race,
	double _basePrice,
	bool _published,
	uint32 _marketGroupID,
	double _chanceOfDuplicating)
: groupID(_groupID),
  name(_name),
  description(_desc),
  radius(_radius),
  mass(_mass),
  volume(_volume),
  capacity(_capacity),
  portionSize(_portionSize),
  race(_race),
  basePrice(_basePrice),
  published(_published),
  marketGroupID(_marketGroupID),
  chanceOfDuplicating(_chanceOfDuplicating)
{
}

/*
 * Type
 */
Type::Type(
	uint32 _id,
	const Group &_group,
	const TypeData &_data)
: attributes(*this),
  m_id(_id),
  m_group(&_group),
  m_name(_data.name),
  m_description(_data.description),
  m_portionSize(_data.portionSize),
  m_basePrice(_data.basePrice),
  m_published(_data.published),
  m_marketGroupID(_data.marketGroupID),
  m_chanceOfDuplicating(_data.chanceOfDuplicating)
{
	// assert for data consistency
	assert(_data.groupID == _group.id());

	// set some attributes
	attributes.Set_radius(_data.radius);
	attributes.Set_mass(_data.mass);
	attributes.Set_volume(_data.volume);
	attributes.Set_capacity(_data.capacity);
	attributes.Set_raceID(_data.race);

	_log(ITEM__TRACE, "Created object %p for type %s (%u).", this, name().c_str(), id());
}

Type *Type::Load(ItemFactory &factory, uint32 typeID)
{
	return Type::Load<Type>( factory, typeID );
}

Type *Type::_Load(ItemFactory &factory, uint32 typeID)
{
	return Type::_Load<Type>( factory, typeID );
}

Type *Type::_LoadType(ItemFactory &factory, uint32 typeID,
	// Type stuff:
	const Group &group, const TypeData &data)
{
	// See what to do next:
	switch( group.categoryID() ) {
		///////////////////////////////////////
		// Blueprint:
		///////////////////////////////////////
		case EVEDB::invCategories::Blueprint: {
			return BlueprintType::_LoadType( factory, typeID, group, data );
		}

		///////////////////////////////////////
		// Ship:
		///////////////////////////////////////
		case EVEDB::invCategories::Ship: {
			return ShipType::_LoadType( factory, typeID, group, data );
		}
	}

	// Category didn't do it, try Group:
	switch( group.id() ) {
		///////////////////////////////////////
		// Character:
		///////////////////////////////////////
		case EVEDB::invGroups::Character: {
			return CharacterType::_LoadType( factory, typeID, group, data );
		}

		///////////////////////////////////////
		// Station:
		///////////////////////////////////////
		case EVEDB::invGroups::Station: {
			return StationType::_LoadType( factory, typeID, group, data );
		}
	}

	// Generic one, create it:
	return new Type( typeID, group, data );
}

bool Type::_Load(ItemFactory &factory) {
	// load type attributes
	return(attributes.Load(factory.db()));
}



