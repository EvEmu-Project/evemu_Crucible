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
 * ShipTypeData
 */
ShipTypeData::ShipTypeData(
	uint32 _weaponTypeID,
	uint32 _miningTypeID,
	uint32 _skillTypeID)
: weaponTypeID(_weaponTypeID),
  miningTypeID(_miningTypeID),
  skillTypeID(_skillTypeID)
{
}

/*
 * ShipType
 */
 ShipType::ShipType(
	uint32 _id,
	// Type stuff:
	const Group &_group,
	const TypeData &_data,
	// ShipType stuff:
	const Type *_weaponType,
	const Type *_miningType,
	const Type *_skillType,
	const ShipTypeData &stData)
: Type(_id, _group, _data),
  m_weaponType(_weaponType),
  m_miningType(_miningType),
  m_skillType(_skillType)
 {
 	// data consistency checks:
	if(_weaponType != NULL)
 		assert(_weaponType->id() == stData.weaponTypeID);
	if(_miningType != NULL) 
		assert(_miningType->id() == stData.miningTypeID);
	if(_skillType != NULL)
 		assert(_skillType->id() == stData.skillTypeID);
}

ShipType *ShipType::_Load(ItemFactory &factory, uint32 shipTypeID
) {
	// load type data
	TypeData data;
	if(!factory.db().GetType(shipTypeID, data))
		return NULL;

	// obtain group
	const Group *g = factory.GetGroup(data.groupID);
	if(g == NULL)
		return NULL;

	return(
		ShipType::_Load(factory, shipTypeID, *g, data)
	);
}

ShipType *ShipType::_Load(ItemFactory &factory, uint32 shipTypeID,
	// Type stuff:
	const Group &group, const TypeData &data
) {
	// verify it's a ship
	if(group.categoryID() != EVEDB::invCategories::Ship) {
		_log(ITEM__ERROR, "Tried to load %u (%s) as a Ship.", shipTypeID, group.category().name().c_str());
		return NULL;
	}

	// load additional ship type stuff
	ShipTypeData stData;
	if(!factory.db().GetShipType(shipTypeID, stData))
		return NULL;

	// try to load weapon type
	const Type *weaponType = NULL;
	if(stData.weaponTypeID != 0) {
		weaponType = factory.GetType(stData.weaponTypeID);
		if(weaponType == NULL)
			return NULL;
	}

	// try to load mining type
	const Type *miningType = NULL;
	if(stData.miningTypeID != 0) {
		miningType = factory.GetType(stData.miningTypeID);
		if(miningType == NULL)
			return NULL;
	}

	// try to load skill type
	const Type *skillType = NULL;
	if(stData.skillTypeID != 0) {
		skillType = factory.GetType(stData.skillTypeID);
		if(skillType == NULL)
			return NULL;
	}

	// continue with load
	return(
		ShipType::_Load(factory, shipTypeID, group, data, weaponType, miningType, skillType, stData)
	);
}

ShipType *ShipType::_Load(ItemFactory &factory, uint32 shipTypeID,
	// Type stuff:
	const Group &group, const TypeData &data,
	// ShipType stuff:
	const Type *weaponType, const Type *miningType, const Type *skillType, const ShipTypeData &stData
) {
	// we have all the data, let's create new object
	return(new ShipType(shipTypeID,
		group, data,
		weaponType, miningType, skillType, stData
	));
}

/*
 * Ship
 */
Ship::Ship(
	ItemFactory &_factory,
	uint32 _shipID,
	// InventoryItem stuff:
	const ShipType &_shipType,
	const ItemData &_data)
: InventoryItem(_factory, _shipID, _shipType, _data)
{
}

Ship *Ship::_Load(ItemFactory &factory, uint32 shipID
) {
	ItemData data;
	if(!factory.db().GetItem(shipID, data))
		return NULL;

	// get type
	const ShipType *st = factory.GetShipType(data.typeID);
	if(st == NULL)
		return NULL;

	return(
		Ship::_Load(factory, shipID, *st, data)
	);
}

Ship *Ship::_Load(ItemFactory &factory, uint32 shipID,
	// InventoryItem stuff:
	const ShipType &shipType, const ItemData &data
) {
	// we got shipType, so it's sure it's a ship

	// we don't need any additional stuff
	return(new Ship(
		factory, shipID,
		shipType, data
	));
}

Ship *Ship::Spawn(ItemFactory &factory,
	// InventoryItem stuff:
	ItemData &data
) {
	uint32 shipID = Ship::_Spawn(factory, data);
	if(shipID == 0)
		return 0;
	// item cannot contain anything yet - don't recurse
	return Ship::Load(factory, shipID, false);
}

uint32 Ship::_Spawn(ItemFactory &factory,
	// InventoryItem stuff:
	ItemData &data
) {
	// make sure it's a ship
	const ShipType *st = factory.GetShipType(data.typeID);
	if(st == NULL)
		return 0;

	// store item data
	uint32 shipID = InventoryItem::_Spawn(factory, data);
	if(shipID == 0)
		return 0;

	// nothing additional

	return shipID;
}