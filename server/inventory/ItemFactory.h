/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef EVE_ITEM_FACTORY_H
#define EVE_ITEM_FACTORY_H

#include <map>

#include "../common/packet_types.h"
#include "../common/gpoint.h"
#include "InventoryDB.h"

class InventoryItem;
class BlueprintItem;

class Type;
class Group;
class Category;

class ItemFactory {
	friend class InventoryItem;	//only for access to _DeleteItem
public:
	ItemFactory(DBcore *db, EntityList *el);
	~ItemFactory();
	
	EntityList *const entity_list;	//we do not own this.
	InventoryDB &db() { return(m_db); }

	/*
	 * Type stuff
	 */
	const Category *category(EVEItemCategories category);
	const Group *group(uint32 groupID);
	const Type *type(uint32 typeID);

	/*
	 * Item stuff
	 */
	InventoryItem *Load(uint32 itemID, bool recurse=true);
	BlueprintItem *LoadBlueprint(uint32 blueprintID, bool recurse=true);

	//a somewhat specialized function to deal with item movement.
	InventoryItem *GetIfContentsLoaded(uint32 itemID);

	//spawn a new item with the specified information, creating it in the DB as well.
	InventoryItem *Spawn(
		uint32 typeID,
		uint32 ownerID,
		uint32 locationID,
		EVEItemFlags flag,
		uint32 quantity);
	InventoryItem *SpawnSingleton(
		uint32 typeID,
		uint32 ownerID,
		uint32 locationID,
		EVEItemFlags flag,
		const char *name = NULL,
		const GPoint &pos = GPoint());
	BlueprintItem *SpawnBlueprint(
		uint32 typeID,
		uint32 ownerID,
		uint32 locationID,
		EVEItemFlags flag,
		uint32 quantity,
		bool copy,
		uint32 materialLevel,
		uint32 productivityLevel,
		int32 licensedProductionRunsRemaining);
	BlueprintItem *SpawnBlueprintSingleton(
		uint32 typeID,
		uint32 ownerID,
		uint32 locationID,
		EVEItemFlags flag,
		bool copy,
		uint32 materialLevel,
		uint32 productivityLevel,
		int32 licensedProductionRunsRemaining,
		const char *name = NULL,
		const GPoint &pos = GPoint());

protected:
	InventoryDB m_db;

	/*
	 * Type cache
	 */
	std::map<EVEItemCategories, Category *> m_categories;
	std::map<uint32, Group *> m_groups;
	std::map<uint32, Type *> m_types;

	/*
	 * Items
	 */
	void _DeleteItem(uint32 itemID);

	std::map<uint32, InventoryItem *> m_items;	//we own a ref to these.
};


#endif

