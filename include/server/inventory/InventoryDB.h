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


#ifndef __INVENTORYDB_H_INCL__
#define __INVENTORYDB_H_INCL__

#include "../ServiceDB.h"
#include "../common/tables/invGroups.h"
#include "../common/tables/invCategories.h"

class EVEAttributeMgr;

class CategoryData;
class GroupData;
class TypeData;
class BlueprintTypeData;

class ItemData;
class BlueprintData;
class CharacterData;

class InventoryDB
: public ServiceDB
{
public:
	/*
	 * Constructor & Destructor
	 */
	InventoryDB(DBcore *db);
	virtual ~InventoryDB();

	/*
	 * Type stuff
	 * (invCategories, invGroups, invTypes, invBlueprintTypes)
	 */
	bool GetCategory(EVEItemCategories category, CategoryData &into);
	bool GetGroup(uint32 groupID, GroupData &into);
	bool GetType(uint32 typeID, TypeData &into);
	bool GetBlueprintType(uint32 blueprintTypeID, BlueprintTypeData &into);

	/*
	 * Item stuff
	 * (entity)
	 */
	bool GetItem(uint32 itemID, ItemData &into);

	uint32 NewItem(const ItemData &data);
	bool SaveItem(uint32 itemID, const ItemData &data);
	bool DeleteItem(uint32 itemID);

	bool GetItemContents(uint32 itemID, std::vector<uint32> &items);

	/*
	 * Blueprint stuff
	 * (invBlueprints)
	 */
	bool GetBlueprint(uint32 blueprintID, BlueprintData &into);

	bool NewBlueprint(uint32 blueprintID, const BlueprintData &data);
	bool SaveBlueprint(uint32 blueprintID, const BlueprintData &data);
	bool DeleteBlueprint(uint32 blueprintID);

	/*
	 * Attribute stuff
	 * (entity_attributes)
	 */
	bool LoadTypeAttributes(uint32 typeID, EVEAttributeMgr &into);
	bool LoadItemAttributes(uint32 itemID, EVEAttributeMgr &into);

	bool UpdateAttribute_int(uint32 itemID, uint32 attributeID, int v);
	bool UpdateAttribute_double(uint32 itemID, uint32 attributeID, double v);
	bool EraseAttribute(uint32 itemID, uint32 attributeID);
	bool EraseAttributes(uint32 itemID);
};



#endif


