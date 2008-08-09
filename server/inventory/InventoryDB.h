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


#ifndef __INVENTORYDB_H_INCL__
#define __INVENTORYDB_H_INCL__

#include "../ServiceDB.h"
#include "../common/tables/invGroups.h"
#include "../common/tables/invCategories.h"

class InventoryItem;

class InventoryDB
: public ServiceDB {
public:
	InventoryDB(DBcore *db);
	virtual ~InventoryDB();

	bool RenameItem(uint32 itemID, const char *name);
	bool ChangeOwner(uint32 itemID, uint32 ownerID);
	bool ChangeQuantity(uint32 itemID, uint32 quantity);
	bool SetCustomInfo(uint32 itemID, const char *ci);
	bool RelocateEntity(uint32 itemID, double x, double y, double z);
	
	bool DeleteItem(InventoryItem *item);
	bool LoadEntityAttributes(InventoryItem *item);
	bool LoadItemAttributes(InventoryItem *item);
	bool LoadPersistentAttributes(InventoryItem *item);
	bool SaveAttributes(InventoryItem *item);
	bool EraseAttribute(uint32 itemID, uint32 attributeID);
	bool UpdateAttribute_int(uint32 itemID, uint32 attributeID, int v);
	bool UpdateAttribute_double(uint32 itemID, uint32 attributeID, double v);
	InventoryItem *LoadItem(uint32 itemID, ItemFactory *factory, bool recurse=true);
	bool GetItemContents(InventoryItem *item, std::vector<uint32> &items);
	InventoryItem *NewItemSingleton(ItemFactory *factory, uint32 typeID, uint32 ownerID, uint32 locationID, EVEItemFlags flag, const char *name, const GPoint &pos);
	InventoryItem *NewItem(ItemFactory *factory, uint32 typeID, uint32 quantity, uint32 ownerID, uint32 locationID, EVEItemFlags flag, const GPoint &pos);

	bool MoveEntity(uint32 itemID, uint32 newLocation, EVEItemFlags flag = flagAutoFit);
	bool ChangeSingletonEntity(uint32 itemID, bool singleton);

	EVEDB::invCategories::invCategories GetCatByGroup(EVEDB::invGroups::invGroups g);
protected:
	uint32 NewDBItem(uint32 typeID, uint32 ownerID, uint32 locationID, EVEItemFlags flag, bool singleton, uint32 quantity, const char *name, const GPoint &pos);
	InventoryItem * _RowToItem(ItemFactory *factory, DBResultRow &row, bool recurse);
	bool _NewBlueprintEntry(const uint32 itemID);

	class AttrCacheEntry {
	public:
		AttrCacheEntry(uint32 type) : typeID(type) { }
		uint32 typeID;
		std::map<uint16, int> int_attributes;
		std::map<uint16, double> double_attributes;
	};
	//making it static to share amongst all the children.
	static std::map<uint32, AttrCacheEntry *> m_attrCache;	//we own these items.

	void _LoadCatByGroup();
	static std::map<EVEDB::invGroups::invGroups, EVEDB::invCategories::invCategories> s_categoryByGroup;
};





#endif


