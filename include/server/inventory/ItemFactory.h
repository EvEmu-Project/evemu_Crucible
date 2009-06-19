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
	Author:		Zhur
*/
#ifndef EVE_ITEM_FACTORY_H
#define EVE_ITEM_FACTORY_H

#include <map>

#include "../common/packet_types.h"
#include "../common/gpoint.h"
#include "InventoryDB.h"

class Category;

class Group;

class Type;
class BlueprintType;
class CharacterType;
class ShipType;
class StationType;

class InventoryItem;
class Blueprint;
class Character;
class Ship;
class CelestialObject;
class SolarSystem;
class Station;
class Skill;

class ItemContainer;

class ItemFactory {
	friend class InventoryItem;	//only for access to _DeleteItem
public:
	ItemFactory(DBcore &db, EntityList &el);
	~ItemFactory();
	
	EntityList &entity_list;	//we do not own this.
	InventoryDB &db() { return(m_db); }

	/*
	 * Category stuff
	 */
	const Category *GetCategory(EVEItemCategories category);

	/*
	 * Group stuff
	 */
	const Group *GetGroup(uint32 groupID);

	/*
	 * Type stuff
	 */
	const Type *GetType(uint32 typeID);

	const BlueprintType *GetBlueprintType(uint32 blueprintTypeID);

	/**
	 * Loads character type, caches it and returns it.
	 *
	 * @param[in] characterTypeID Character type to be returned.
	 * @return Pointer to character type data container; NULL if fails.
	 */
	const CharacterType *GetCharacterType(uint32 characterTypeID);
	/**
	 * Loads character type, caches it and returns it.
	 *
	 * @param[in] characterTypeID Character type to be returned.
	 * @return Pointer to character type data container; NULL if fails.
	 */
	const CharacterType *GetCharacterTypeByBloodline(uint32 bloodlineID);

	/**
	 * Loads ship type, caches it and returns it.
	 *
	 * @param[in] shipTypeID ID of ship type.
	 * @return Pointer to ship type data container; NULL if fails.
	 */
	const ShipType *GetShipType(uint32 shipTypeID);

	/**
	 * Loads station type, caches it and returns it.
	 *
	 * @param[in] stationTypeID ID of station type to load.
	 * @return Pointer to StationType object; NULL if fails.
	 */
	const StationType *GetStationType(uint32 stationTypeID);

	/*
	 * Item stuff
	 */
	InventoryItem *GetItem(uint32 itemID);

	Blueprint *GetBlueprint(uint32 blueprintID);

	/**
	 * Loads character.
	 *
	 * @param[in] character ID of character to load.
	 * @return Pointer to Character object; NULL if load failed.
	 */
	Character *GetCharacter(uint32 characterID);

	/**
	 * Loads ship.
	 *
	 * @param[in] shipID ID of ship to load.
	 * @return Pointer to Ship object; NULL if failed.
	 */
	Ship *GetShip(uint32 shipID);

	/**
	 * Loads celestial object.
	 *
	 * @param[in] celestialID ID of celestial object to load.
	 * @return Pointer to CelestialObject; NULL if fails.
	 */
	CelestialObject *GetCelestialObject(uint32 celestialID);

	/**
	 * Loads solar system.
	 *
	 * @param[in] solarSystemID ID of solar system to load.
	 * @return Pointer to solar system object; NULL if failed.
	 */
	SolarSystem *GetSolarSystem(uint32 solarSystemID);

	/**
	 * Loads station.
	 *
	 * @param[in] stationID ID of station to load.
	 * @return Pointer to Station object; NULL if fails.
	 */
	Station *GetStation(uint32 stationID);

	/**
	 * Loads skill.
	 *
	 * @param[in] skillID ID of skill to load.
	 * @return Pointer to Skill object; NULL if fails.
	 */
	Skill *GetSkill(uint32 skillID);

	//spawn a new item with the specified information, creating it in the DB as well.
	InventoryItem *SpawnItem(ItemData &data);
	Blueprint *SpawnBlueprint(ItemData &data, BlueprintData &bpData);
	/**
	 * Spawns new character, caches it and returns it.
	 *
	 * @param[in] data Item data (for entity table).
	 * @param[in] charData Character data.
	 * @param[in] appData Character's appearance.
	 * @param[in] corpData Character's corporation-membership data.
	 * @return Pointer to new Character object; NULL if spawn failed.
	 */
	Character *SpawnCharacter(ItemData &data, CharacterData &charData, CharacterAppearance &appData, CorpMemberInfo &corpData);
	/**
	 * Spawns new ship.
	 *
	 * @param[in] data Item data for ship.
	 * @return Pointer to Ship object; NULL if failed.
	 */
	Ship *SpawnShip(ItemData &data);
	/**
	 * Spawns new skill.
	 *
	 * @param[in] data Item data for skill.
	 * @return Pointer to new Skill object; NULL if fails.
	 */
	Skill *SpawnSkill(ItemData &data);

	/*
	 * Container stuff
	 */
	ItemContainer *GetItemContainer(uint32 containerID, bool load=true);

protected:
	InventoryDB m_db;

	/*
	 * Member functions and variables:
	 */
	// Categories:
	std::map<EVEItemCategories, Category *> m_categories;

	// Groups:
	std::map<uint32, Group *> m_groups;

	// Types:
	template<class _Ty>
	const _Ty *_GetType(uint32 typeID);

	std::map<uint32, Type *> m_types;

	// Items:
	template<class _Ty>
	_Ty *_GetItem(uint32 itemID);

	void _DeleteItem(uint32 itemID);

	std::map<uint32, InventoryItem *> m_items;	//we own a ref to these.
};


#endif

