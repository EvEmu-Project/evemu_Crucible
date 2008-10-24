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

#include "ItemFactory.h"
#include "InventoryItem.h"
#include "../common/logsys.h"

ItemFactory::ItemFactory(DBcore *db, EntityList *el)
: entity_list(el),
  m_db(db)
{
}

ItemFactory::~ItemFactory() {
	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_items.begin();
	end = m_items.end();
	for(; cur != end; cur++) {
		cur->second->Release();
	}
}
	
InventoryItem *ItemFactory::Load(uint32 itemID, bool recurse) {
	std::map<uint32, InventoryItem *>::const_iterator res;
	res = m_items.find(itemID);
	if(res != m_items.end()) {
		if(recurse) {
			_log(ITEM__TRACE, "Recursively loading contents of cached item %lu", itemID);
			if(!res->second->LoadContents(recurse)) {
				codelog(SERVICE__ERROR, "Failed to load recursive contents of item ID %d. Returning it unloaded.", itemID);
			}
		}
		return(res->second->Ref());
	}
	
	//item not cached, load it up. This calls back on our Create()
	InventoryItem *i = m_db.LoadItem(itemID, this);
	if(i == NULL)
		return(NULL);
	
	//load up any additional features of this item from the DB
	if(!i->Load(recurse)) {
		_log(ITEM__DEBUG, "Failed to load details for item %lu", i->itemID());
		i->Release();
		//TODO: clear this item out from its container....
		return(NULL);
	}
	
	//we give the ref which we were given by m_db.LoadItem to our caller.
	return(i);
}

InventoryItem *ItemFactory::GetIfContentsLoaded(uint32 itemID) {
	std::map<uint32, InventoryItem *>::const_iterator res;
	res = m_items.find(itemID);
	if(res != m_items.end()) {
		if(res->second->ContentsLoaded()) {
			return(res->second->Ref());
		}
		return(NULL);
	}
	return(NULL);
}
	
InventoryItem *ItemFactory::Create(
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
	bool _inDB
) {
	std::map<uint32, InventoryItem *>::iterator res;
	res = m_items.find(_itemID);
	if(res != m_items.end()) {
		codelog(SERVICE__ERROR, "Item ID %d already exixts, refusing to create it.", _itemID);
		return(NULL);
	}
	
	InventoryItem *i = new InventoryItem(
		this,
		_itemID, 
		_itemName,
		_typeID, 
		_ownerID,
		_locationID,
		_flag,
		_contraband,
		_singleton,
		_quantity,
		_position,
		_customInfo,
		_groupID, 
		_categoryID,
		_inDB
		);

	//load up attributes (even non-DB objects have item attributes)
	if(!i->LoadStatic()) {
		_log(ITEM__ERROR, "Failed to load static data for item %lu of type %lu", i->itemID(), i->typeID());
		i->Release();
		return(NULL);
	}
	
	_log(ITEM__TRACE, "Created object %p for item ID %lu of type %lu", i, i->itemID(), i->typeID());
	
	//if we have the item `locationID` loaded, we need to update it.
	std::map<uint32, InventoryItem *>::iterator parent_res;
	parent_res = m_items.find(i->locationID());
	if(parent_res != m_items.end()) {
		//we just created a new item, make sure its properly in the container
		_log(ITEM__TRACE, "   Updated loaded location %lu to contain created item %lu", i->locationID(), i->itemID());
		if(parent_res->second->ContentsLoaded()) {
			parent_res->second->AddContainedItem(i);
		}
	}

	//we keep the original ref.
	m_items[_itemID] = i;

	//we return an additional ref to the user.
	return(i->Ref());
}

InventoryItem *ItemFactory::Create(
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
	bool _inDB
) {
	GPoint z(0,0,0);
	return(Create(
		_itemID, 
		_itemName,
		_typeID, 
		_ownerID, 
		_locationID,
		_flag,
		_contraband,
		_singleton,
		_quantity,
		z,
		_customInfo,
		_groupID, 
		_categoryID,
		_inDB
		) );
}

InventoryItem *ItemFactory::Create(
	uint32 _itemID, 
	const char *_itemName,
	uint32 _typeID, 
	uint32 _ownerID, 
	uint32 _locationID,
	EVEItemFlags _flag,
	uint32 _groupID, 
	EVEItemCategories _categoryID,
	bool _inDB
) {
	GPoint z(0,0,0);
	return(Create(
		_itemID, 
		_itemName,
		_typeID, 
		_ownerID, 
		_locationID,
		_flag,
		false,
		true,
		1,
		z,
		"",
		_groupID, 
		_categoryID,
		_inDB
		) );
}

InventoryItem *ItemFactory::Create(
	uint32 _itemID, 
	const char *_itemName,
	uint32 _typeID, 
	uint32 _ownerID, 
	uint32 _locationID,
	EVEItemFlags _flag,
	const GPoint &_position,
	uint32 _groupID, 
	EVEItemCategories _categoryID,
	bool _inDB
) {
	return(Create(
		_itemID, 
		_itemName,
		_typeID, 
		_ownerID, 
		_locationID,
		_flag,
		false,
		true,
		1,
		_position,
		"",
		_groupID, 
		_categoryID,
		_inDB
		) );
}

InventoryItem *ItemFactory::SpawnSingleton(
		uint32 typeID,
		uint32 ownerID,
		uint32 locationID,
		EVEItemFlags flag,
		const char *name,
		const GPoint &pos
) {
	InventoryItem *new_item;

	//this will create the item in the DB, and calls back on
	//Create() above after loading it from the DB.
	new_item = m_db.NewItemSingleton(this, typeID, ownerID, locationID, flag, name, pos);
	if(new_item == NULL) {
		return(NULL);
	}
	
	if(!_SpawnCommon(new_item)) {
		new_item->Release();
		return(NULL);
	}
	
	_log(ITEM__TRACE, "Spawned new singleton item of type %lu for owner %lu with item ID %lu", typeID, ownerID, new_item->itemID());
	
	return(new_item);
}

InventoryItem *ItemFactory::Spawn(
		uint32 typeID,
		uint32 quantity,
		uint32 ownerID,
		uint32 locationID,
		EVEItemFlags flag
) {
	InventoryItem *new_item;

	//this will create the item in the DB, and calls back on
	//Create() above after loading it from the DB.
	GPoint pos(0, 0, 0);
	new_item = m_db.NewItem(this, typeID, quantity, ownerID, locationID, flag, pos);
	if(new_item == NULL) {
		codelog(ITEM__DEBUG, "Failed create new item of type %lu for owner %lu in the DB.", typeID, ownerID);
		return(NULL);
	}
	
	if(!_SpawnCommon(new_item)) {
		new_item->Release();
		return(NULL);
	}
	
	_log(ITEM__TRACE, "Spawned new item of type %lu quantity %lu for owner %lu with item ID %lu", typeID, quantity, ownerID, new_item->itemID());
	
	return(new_item);
}

bool ItemFactory::_SpawnCommon(InventoryItem *i) {
	if(!i->Load(true)) {
		codelog(ITEM__DEBUG, "Failed load new item of type %lu for owner %lu in the DB.", i->typeID(), i->ownerID());
		return(false);
	}
	
	return(true);
}

void ItemFactory::_DeleteItem(InventoryItem *i) {
	std::map<uint32, InventoryItem *>::iterator res;
	res = m_items.find(i->itemID());
	if(res == m_items.end()) {
		codelog(SERVICE__ERROR, "Item ID %d not found when requesting deletion!", i->itemID());
		return;
	}
	m_items.erase(res);
	i->Release();	//we own a ref, so release it
}




















