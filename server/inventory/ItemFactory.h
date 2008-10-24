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

class ItemFactory {
public:
	ItemFactory(DBcore *db, EntityList *el);
	~ItemFactory();
	
	EntityList *const entity_list;	//we do not own this.
	InventoryDB &db() { return(m_db); }

	InventoryItem *Load(uint32 itemID, bool recurse);
	
	//a somewhat specialized function to deal with item movement.
	InventoryItem *GetIfContentsLoaded(uint32 itemID);

	//create items from data obtained directly..
	InventoryItem *Create(
		uint32 _itemID, 
		const char *_itemName,
		uint32 _typeID, 
		uint32 _ownerID,
		uint32 _locationID,
		EVEItemFlags _flag,
		bool _contraband,
		bool _singleton,
		uint32 _quantity,
		const GPoint &_position,
		const char *_customInfo,
		uint32 _groupID, 
		EVEItemCategories _categoryID,
		bool _inDB = false);
	//and some useful overloads
	InventoryItem *Create(
		uint32 _itemID, 
		const char *_itemName,
		uint32 _typeID, 
		uint32 _ownerID,
		uint32 _locationID,
		EVEItemFlags _flag,
		bool _contraband,
		bool _singleton,
		uint32 _quantity,
		const char *_customInfo,
		uint32 _groupID, 
		EVEItemCategories _categoryID,
		bool _inDB = false);
	InventoryItem *Create(
		uint32 _itemID, 
		const char *_itemName,
		uint32 _typeID, 
		uint32 _ownerID, 
		uint32 _locationID,
		EVEItemFlags _flag,
		uint32 _groupID, 
		EVEItemCategories _categoryID,
		bool _inDB = false);
	InventoryItem *Create(
		uint32 _itemID, 
		const char *_itemName,
		uint32 _typeID, 
		uint32 _ownerID, 
		uint32 _locationID,
		EVEItemFlags _flag,
		const GPoint &_position,
		uint32 _groupID, 
		EVEItemCategories _categoryID,
		bool _inDB = false);
	
	//spawn a new item with the specified information, creating it in the DB as well.
	InventoryItem *SpawnSingleton(
		uint32 typeID,
		uint32 ownerID,
		uint32 locationID,
		EVEItemFlags flag,
		const char *name = NULL,
		const GPoint &pos = GPoint());
	InventoryItem *Spawn(
		uint32 typeID,
		uint32 quantity,
		uint32 ownerID,
		uint32 locationID,
		EVEItemFlags flag);

protected:
	friend class InventoryItem;	//only for access to _DeleteItem
	void _DeleteItem(InventoryItem *i);
	
private:
	bool _SpawnCommon(InventoryItem *i);
	
	InventoryDB m_db;
	
	std::map<uint32, InventoryItem *> m_items;	//we own a ref to these.
};


#endif

