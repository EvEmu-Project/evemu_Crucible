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



#include "InventoryDB.h"
#include "../common/dbcore.h"
#include "../common/logsys.h"
#include "../common/EVEDBUtils.h"
#include "../inventory/InventoryItem.h"
#include "../inventory/ItemFactory.h"


std::map<uint32, InventoryDB::AttrCacheEntry *> InventoryDB::m_attrCache;
std::map<EVEDB::invGroups::invGroups, EVEDB::invCategories::invCategories> InventoryDB::s_categoryByGroup;

InventoryDB::InventoryDB(DBcore *db)
: ServiceDB(db)
{
}

InventoryDB::~InventoryDB() {
}

bool InventoryDB::RenameItem(uint32 itemID, const char *name) {
	DBerror err;
	
	std::string nameEsc;
	m_db->DoEscapeString(nameEsc, name);
	
	if(!m_db->RunQuery(err,
		"UPDATE entity"
		" SET itemName='%s'"
		" WHERE itemID=%lu",
			nameEsc.c_str(),
			itemID
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		return(false);
	}
	return(true);
}

bool InventoryDB::ChangeOwner(uint32 itemID, uint32 ownerID) {
	DBerror err;
	
	if(!m_db->RunQuery(err,
		"UPDATE entity"
		" SET ownerID=%lu"
		" WHERE itemID=%lu",
			ownerID,
			itemID
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		return(false);
	}
	return(true);
}

bool InventoryDB::ChangeQuantity(uint32 itemID, uint32 quantity) {
	DBerror err;
	
	if(!m_db->RunQuery(err,
		"UPDATE entity"
		" SET quantity=%lu"
		" WHERE itemID=%lu",
			quantity,
			itemID
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", err.c_str());
		return(false);
	}
	return(true);
}


//returns a ref to the item
InventoryItem *InventoryDB::LoadItem(uint32 itemID, ItemFactory *factory, bool recurse) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" entity.itemID,"
		" entity.itemName,"
		" entity.typeID,"
		" entity.ownerID,"
		" entity.locationID,"
		" entity.flag,"
		" entity.contraband,"
		" entity.singleton,"
		" entity.quantity,"
		" entity.x,"
		" entity.y,"
		" entity.z,"
		" entity.customInfo,"
		" invTypes.groupID,"
		" invGroups.categoryID"
		" FROM entity "
		"	LEFT JOIN invTypes ON entity.typeID=invTypes.typeID"
		"	LEFT JOIN invGroups ON invTypes.groupID=invGroups.groupID"
		" WHERE itemID=%lu",
			itemID))
	{
		codelog(SERVICE__ERROR, "Error in query for item %lu: %s", itemID, res.error.c_str());
		return(NULL);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Unable to load item %lu", itemID);
		return(false);
	}

	return(_RowToItem(factory, row, recurse));
}

bool InventoryDB::GetItemContents(InventoryItem *item, std::vector<uint32> &items) {

	//this could be optimized to load the full row of each
	//item which is to be loaded (and used to be), but it made
	//for some overly complex knowledge in the DB side which
	// really did not belong here, so weo go to the simpler
	// solution until it becomes a problem.
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" itemID"
		" FROM entity "
		" WHERE locationID=%lu",
			item->itemID()))
	{
		codelog(SERVICE__ERROR, "Error in query for item %lu: %s", item->itemID(), res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	while(res.GetRow(row)) {
		items.push_back(row.GetUInt(0));
	}
	return(true);
}

InventoryItem *InventoryDB::_RowToItem(ItemFactory *factory, DBResultRow &row, bool recurse) {
	int flag = row.GetUInt(5);
	if(flag > 255) {
		codelog(SERVICE__ERROR, "Invalid flag on item %s! Refusing to load.", row.GetText(0));
		return(NULL);
	}
	//TODO: check for a few of the holes in the flag space.
	//TODO: verify the category value.
	
	InventoryItem *result = factory->Create(
		row.GetUInt(0),
		row.GetText(1),
		row.GetUInt(2),
		row.GetUInt(3),
		row.GetUInt(4),
		EVEItemFlags(flag),
		(row.GetUInt(6)!=0),
		(row.GetUInt(7)!=0),
		row.GetUInt(8),
		GPoint(
			row.GetDouble(9),
			row.GetDouble(10),
			row.GetDouble(11)),
		(row.IsNull(12))?"":row.GetText(12),
		row.GetUInt(13),
		(EVEItemCategories)row.GetUInt(14),	//assume the DB dosent contain invalid category values... prolly not really a good assumption..
		true
		);
	if(result == NULL) {
		codelog(SERVICE__ERROR, "Unable to create new instance of item %lu through factory.", row.GetUInt(0));
		return(NULL);
	}
	
	if(!result->Load(recurse)) {
		codelog(SERVICE__ERROR, "Unable to finish load of item %lu", result->itemID());
		result->Release();
		return(NULL);
	}

	return(result);
}



bool InventoryDB::LoadEntityAttributes(InventoryItem *item) {
	std::map<uint32, AttrCacheEntry *>::const_iterator cache;
	cache = m_attrCache.find(item->typeID());
	if(cache == m_attrCache.end()) {
		//not found in the cache, we need to load it.
		DBQueryResult res;
	
		if(!m_db->RunQuery(res,
		"SELECT "
		"	attributeID,valueInt,valueFloat"
		" FROM dgmTypeAttributes"
		" WHERE typeID=%lu", item->typeID()))
		{
			codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
			return(false);
		}
	
		AttrCacheEntry *entry = new AttrCacheEntry(item->typeID());
		DBResultRow row;
		while(res.GetRow(row)) {
			if(row.IsNull(1)) {
				if(row.IsNull(2)) {
					codelog(SERVICE__ERROR, "dgmTypeAttributes row for type %d has both values NULL!", item->typeID());
					continue;
				}
				//ok, the row is a double, is the attribute?
				if(InventoryItem::IsDoubleAttr((InventoryItem::Attr) row.GetUInt(0))) {
					entry->double_attributes[row.GetInt(0)] = row.GetDouble(2);
				} else {
					//we keep it as an int internally.
					//this should prolly produce a warning... this is rarely good.
					entry->int_attributes[row.GetInt(0)] = row.GetInt(2);
				}
			} else {
				//TODO: detect 64 bit numbers and handle them... (is it even possible to store them in the current schema?)
				//ok, the row is an int, is the attribute?
				if(InventoryItem::IsIntAttr((InventoryItem::Attr) row.GetUInt(0))) {
					entry->int_attributes[row.GetInt(0)] = row.GetInt(1);
				} else {
					//stored in the DB as an int, but we treat it as a double. convert it.
					entry->double_attributes[row.GetInt(0)] = row.GetDouble(1);
				}
			}
		}
		
		//TODO: implement some sort of cache-cleaning system as to hold on to things forever.
		
		//store it in the cache...
		m_attrCache[item->typeID()] = entry;
		//this will find it.
		cache = m_attrCache.find(item->typeID());
	} //else, we found it in cache, hooray!

	//either way, cache points to the entry in the cache now.
	//these loops could be done more efficiently with direct assignments 
	// to the attribute arrays, but that would also overwrite any other 
	// attributes they had loaded... maybe its worth it some day, but not now.
	std::map<uint16, int>::const_iterator curi, endi;
	curi = cache->second->int_attributes.begin();
	endi = cache->second->int_attributes.end();
	for(; curi != endi; curi++) {
		item->SetAttributeByIndexInt(curi->first, curi->second);
	}
	std::map<uint16, double>::const_iterator curd, endd;
	curd = cache->second->double_attributes.begin();
	endd = cache->second->double_attributes.end();
	for(; curd != endd; curd++) {
		item->SetAttributeByIndexDouble(curd->first, curd->second);
	}
	
	return(true);
}


bool InventoryDB::LoadPersistentAttributes(InventoryItem *item) {
	//not found in the cache, we need to load it.
	DBQueryResult res;

	if(!m_db->RunQuery(res,
	"SELECT "
	"	attributeID,valueInt,valueFloat"
	" FROM entity_attributes"
	" WHERE itemID=%lu", item->itemID()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	while(res.GetRow(row)) {
		if(row.IsNull(1)) {
			if(row.IsNull(2)) {
				codelog(SERVICE__ERROR, "entity_attributes row for type %d, attr %s has both values NULL!", item->typeID(), row.GetText(0));
				continue;
			}
			item->SetAttributeByIndexDouble(row.GetInt(0), row.GetDouble(2));
		} else {
			//TODO: detect 64 bit numbers and handle them... (is it even possible to store them in the current schema?)
			item->SetAttributeByIndexInt(row.GetInt(0), row.GetInt(1));
		}
	}
	
	return(true);
}

bool InventoryDB::LoadItemAttributes(InventoryItem *item) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
	"SELECT "
	"	radius,mass,volume,capacity,raceID"
	" FROM invTypes"
	" WHERE typeID=%lu", item->typeID()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Error in query: no data for type %d", item->typeID());
		return(false);
	}

	if(!row.IsNull(0))	//radius
		item->Set_radius(row.GetDouble(0)/6500.0f);	//total crap...
	if(!row.IsNull(1))	//mass
		item->Set_mass(row.GetDouble(1));
	if(!row.IsNull(2))	//volume
		item->Set_volume(row.GetDouble(2));
	if(!row.IsNull(3))	//capacity
		item->Set_capacity(row.GetDouble(3));
	if(!row.IsNull(4))	//raceID
		item->Set_raceID(row.GetInt(4));

	return(true);
}
/* * * * * * * * * * * *
 * BLUEPRINTS:
 * - These 2 functions really got ugly just because we need them properly handle blueprints ... blueprint support should be really better
 *   than a simple hack ...
 * - Creation and deletion of blueprints is (hopefully) done, splitting as well (at least those redirected to InventoryItem::Split),
 *   but merging isn't (I think it will be a problem unless we make some system which will detect and handle singleton-only types).
 * - Right now if you try to merge 2 blueprints, quantity will be properly added, but properties of dragged blueprint will be lost and
 *   merged blueprints will have attributes of stationary (non-dragged) blueprint
 * - I know that the creation of blueprint entry should be more likely placed in NewDBItem, but we cannot easily get categoryID
 *   at that time ...
*/
InventoryItem *InventoryDB::NewItemSingleton(ItemFactory *factory, uint32 typeID, uint32 ownerID, uint32 locationID, EVEItemFlags flag, const char *name, const GPoint &pos) {
	uint32 id = NewDBItem(typeID, ownerID, locationID, flag, true, 1, name, pos);
	if(id == 0)
		return(NULL);
	InventoryItem *i = LoadItem(id, factory, false);
	if(i->categoryID() == EVEDB::invCategories::Blueprint && !_NewBlueprintEntry(i->itemID())) {
		i->Delete();
		return(NULL);
	}
	return(i);	//no need to recurse, we know its empty...
}

InventoryItem *InventoryDB::NewItem(ItemFactory *factory, uint32 typeID, uint32 quantity, uint32 ownerID, uint32 locationID, EVEItemFlags flag, const GPoint &pos) {
	uint32 id = NewDBItem(typeID, ownerID, locationID, flag, false, quantity, NULL, pos);
	if(id == 0)
		return(NULL);
	InventoryItem *i = LoadItem(id, factory, false);
	if(i->categoryID() == EVEDB::invCategories::Blueprint && !_NewBlueprintEntry(i->itemID())) {
		i->Delete();
		return(NULL);
	}
	return(i);	//no need to recurse, we know its empty...
}

uint32 InventoryDB::NewDBItem(uint32 typeID, uint32 ownerID, uint32 locationID, EVEItemFlags flag, bool singleton, uint32 quantity, const char *name, const GPoint &pos) {
	DBerror err;
	uint32 eid;

	std::string nameEsc;
	if(name == NULL) {
		//no name provided, pull one from the DB based on type.
		DBQueryResult res;
		
		if(!m_db->RunQuery(res,
		"SELECT "
		"	typeName"
		" FROM invTypes"
		" WHERE typeID=%lu", typeID))
		{
			codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
			return(0);
		}
		
		DBResultRow row;
		if(!res.GetRow(row)) {
			codelog(SERVICE__ERROR, "Error in query: no data for type %d", typeID);
			return(0);
		}
		
		m_db->DoEscapeString(nameEsc, row.GetText(0));
	} else {
		m_db->DoEscapeString(nameEsc, name);
	}
	
	if(!m_db->RunQueryLID(err, eid,
	"INSERT INTO entity ("
	"	itemName, typeID, ownerID, locationID, flag,"
	"	contraband, singleton, quantity, customInfo,"
	"	x, y, z"
	" ) "
	"VALUES('%s', %lu, %lu, %lu, %lu,"
	"	%lu, %lu, %lu, "
	"	NULL, %.2f, %.2f, %.2f )",
		nameEsc.c_str(), typeID, ownerID, locationID, flag,
		/*contraband*/0, singleton?1:0, quantity,
		pos.x, pos.y, pos.z
		)
	) {
		codelog(SERVICE__ERROR, "Failed to insert new entity: %s", err.c_str());
		return(0);
	}

	return(eid);
}

bool InventoryDB::_NewBlueprintEntry(const uint32 itemID) {
	DBerror err;

	if(!m_db->RunQuery(err,
		"INSERT INTO invBlueprints"
		" (blueprintID)"
		" VALUES (%lu)",
		itemID))
	{
		_log(DATABASE__ERROR, "Unable to create new blueprint entry for blueprint %lu: %s.", itemID, err.c_str());
		return(false);
	}

	return(true);
}

bool InventoryDB::MoveEntity(uint32 itemID, uint32 newLocation, EVEItemFlags flag) {
	DBerror err;

	if(flag == flagAutoFit) {
		//do not change the flag while moving.
		if(!m_db->RunQuery(err,
		"UPDATE entity SET locationID=%lu WHERE itemID=%lu",
			newLocation, itemID)
		) {
			_log(SERVICE__ERROR, "Failed to move entity %li: %s", itemID, err.c_str());
			return(false);
		}
	} else {
		if(!m_db->RunQuery(err,
		"UPDATE entity SET locationID=%lu,flag=%lu WHERE itemID=%lu",
			newLocation, flag, itemID)
		) {
			_log(SERVICE__ERROR, "Failed to move entity %li: %s", itemID, err.c_str());
			return(false);
		}
	}
	
	return(true);
}

bool InventoryDB::ChangeSingletonEntity(uint32 itemID, bool singleton) {
	DBerror err;

	if(!m_db->RunQuery(err,
		"UPDATE entity SET singleton=%lu WHERE itemID=%lu",
		singleton, itemID))
	{
		_log(SERVICE__ERROR, "Failed to set singleton for entity %li: %s", itemID, err.c_str());
		return(false);
	}
	
	return(true);
}

bool InventoryDB::DeleteItem(InventoryItem *item) {
	DBerror err;

	//delete all attributes first
	if(!m_db->RunQuery(err,
		"DELETE FROM entity_attributes"
		" WHERE itemID=%lu",
		item->itemID()))
	{
		codelog(DATABASE__ERROR, "Failed to delete attributes of item %lu: %s.", item->itemID(), err.c_str());
		return(false);
	}
	
	//note: all child entities should be deleted by the caller first.
	
	//delete standart entity entry and blueprint entry as well (if there is some)
	if(!m_db->RunQuery(err,
		"DELETE entity, invBlueprints"
		" FROM entity"
		" LEFT JOIN invBlueprints ON itemID = blueprintID"
		" WHERE itemID = %lu",
			item->itemID()
	))
	{
		codelog(DATABASE__ERROR, "Failed to delete item %lu: %s", item->itemID(), err.c_str());
		return(false);
	}
	return(true);
}

bool InventoryDB::SetCustomInfo(uint32 itemID, const char *ci) {
	DBerror err;

	if(ci == NULL) {
		if(!m_db->RunQuery(err,
		"UPDATE entity SET customInfo=NULL WHERE itemID=%lu",
			itemID)
		) {
			codelog(SERVICE__ERROR, "Failed query: %s", err.c_str());
			return(false);
		}
	} else {
		std::string ciEsc;
		m_db->DoEscapeString(ciEsc, ci);
	
		if(!m_db->RunQuery(err,
		"UPDATE entity SET customInfo='%s' WHERE itemID=%lu",
			ciEsc.c_str(), itemID)
		) {
			codelog(SERVICE__ERROR, "Failed query: %s", err.c_str());
			return(false);
		}
	}
	return(true);
}

bool InventoryDB::RelocateEntity(uint32 itemID, double x, double y, double z) {
	DBerror err;
	if(!m_db->RunQuery(err,
	"UPDATE entity SET x=%.13f,y=%.13f,z=%.13f WHERE itemID=%lu",
		x, y, z, itemID)
	) {
		codelog(SERVICE__ERROR, "Failed to relocate entity %li: %s", itemID, err.c_str());
		return(false);
	}
	return(true);
}

bool InventoryDB::SaveAttributes(InventoryItem *item) {
	//this func saves every persistent attribute
	DBerror err;

	//we used to clear all attributes for this item,
	//but as we can save non-persistent attribute only
	//by calling Set_persist, we can assume they know
	//what are they doing. so we dont clear anymore.
	{
		std::map<InventoryItem::Attr, int>::const_iterator curi, endi;
		curi = item->m_int_attributes.begin();
		endi = item->m_int_attributes.end();
		for(; curi != endi; curi++) {
			if(item->IsPersistent(curi->first))
				UpdateAttribute_int(item->itemID(), curi->first, curi->second);
		}
	}

	{
		std::map<InventoryItem::Attr, double>::const_iterator curd, endd;
		curd = item->m_double_attributes.begin();
		endd = item->m_double_attributes.end();
		for(; curd != endd; curd++) {
			if(item->IsPersistent(curd->first))
				UpdateAttribute_double(item->itemID(), curd->first, curd->second);
		}
	}
	return(true);
}

bool InventoryDB::EraseAttribute(uint32 itemID, uint32 attributeID) {
	DBerror err;
	if(!m_db->RunQuery(err,
		"DELETE FROM entity_attributes"
		" WHERE itemID=%lu AND attributeID=%lu",
		itemID, attributeID)
	) {
		codelog(SERVICE__ERROR, "Failed to erase attribute %d for item %lu: %s", attributeID, itemID, err.c_str());
		return(false);
	}
	return(true);
}

bool InventoryDB::UpdateAttribute_int(uint32 itemID, uint32 attributeID, int v) {
	DBerror err;
	if(!m_db->RunQuery(err,
		"REPLACE INTO entity_attributes"
		"	(itemID, attributeID, valueInt, valueFloat)"
		" VALUES"
		"	(%lu, %lu, %d, NULL)",
		itemID, attributeID, v)
	) {
		codelog(SERVICE__ERROR, "Failed to store attribute %d for item %lu: %s", attributeID, itemID, err.c_str());
		return(false);
	}
	return(true);
}

bool InventoryDB::UpdateAttribute_double(uint32 itemID, uint32 attributeID, double v) {
	DBerror err;
	if(!m_db->RunQuery(err,
		"REPLACE INTO entity_attributes"
		"	(itemID, attributeID, valueInt, valueFloat)"
		" VALUES"
		"	(%lu, %lu, NULL, %f)",
		itemID, attributeID, v)
	) {
		codelog(SERVICE__ERROR, "Failed to store attribute %d for item %lu: %s", attributeID, itemID, err.c_str());
		return(false);
	}
	return(true);
}

EVEDB::invCategories::invCategories InventoryDB::GetCatByGroup(EVEDB::invGroups::invGroups g) {
	if(s_categoryByGroup.empty()) {
		_LoadCatByGroup();
	}
	std::map<EVEDB::invGroups::invGroups, EVEDB::invCategories::invCategories>::const_iterator res;
	res = s_categoryByGroup.find(g);
	if(res == s_categoryByGroup.end()) {
		//not found!
		_log(ITEM__WARNING, "Unable to find category for item group %lu", g);
		return(EVEDB::invCategories::Owner);
	}
	return(res->second);
}

//populate the map cache.
void InventoryDB::_LoadCatByGroup() {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
	"SELECT "
	"	groupID,categoryID"
	" FROM invGroups"))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return;
	}
	
	DBResultRow row;
	while(res.GetRow(row)) {
		//assume validity for casting purposes.
		s_categoryByGroup[EVEDB::invGroups::invGroups(row.GetInt(0))]
			= EVEDB::invCategories::invCategories(row.GetInt(1));
	}
}






























