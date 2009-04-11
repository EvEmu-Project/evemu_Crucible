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

class InventoryItem;
class BlueprintItem;

class ItemFactory {
	friend class InventoryItem;	//only for access to _GetIfContentsLoaded and _DeleteItem
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

	/*
	 * Item stuff
	 */
	InventoryItem *GetItem(uint32 itemID, bool recurse=true);
	BlueprintItem *GetBlueprint(uint32 blueprintID, bool recurse=true);

	//spawn a new item with the specified information, creating it in the DB as well.
	InventoryItem *SpawnItem(
		ItemData &data);
	BlueprintItem *SpawnBlueprint(
		ItemData &data,
		BlueprintData &bpData);

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
	std::map<uint32, Type *> m_types;

	// Items:
	InventoryItem *_GetIfContentsLoaded(uint32 itemID); // specialized function to deal with item movement.
	void _DeleteItem(uint32 itemID);

	std::map<uint32, InventoryItem *> m_items;	//we own a ref to these.
};


#endif

