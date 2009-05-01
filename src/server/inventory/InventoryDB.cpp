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

#include "EvemuPCH.h"

InventoryDB::InventoryDB(DBcore *db)
: ServiceDB(db)
{
}

InventoryDB::~InventoryDB() {
}

bool InventoryDB::GetCategory(EVEItemCategories category, CategoryData &into) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" categoryName,"
		" description,"
		" published"
		" FROM invCategories"
		" WHERE categoryID=%u",
		uint32(category)))
	{
		_log(DATABASE__ERROR, "Error in query: %s.", res.error.c_str());
		return false;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Category %u not found.", uint32(category));
		return false;
	}

	into.name = row.GetText(0);
	into.description = row.GetText(1);
	into.published = row.GetInt(2) ? true : false;

	return true;
}

bool InventoryDB::GetGroup(uint32 groupID, GroupData &into) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" categoryID,"
		" groupName,"
		" description,"
		" useBasePrice,"
		" allowManufacture,"
		" allowRecycler,"
		" anchored,"
		" anchorable,"
		" fittableNonSingleton,"
		" published"
		" FROM invGroups"
		" WHERE groupID=%u",
		groupID))
	{
		_log(DATABASE__ERROR, "Failed to query group %u: %s.", groupID, res.error.c_str());
		return false;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Group %u not found.", groupID);
		return false;
	}

	into.category = EVEItemCategories(row.GetUInt(0));
	into.name = row.GetText(1);
	into.description = row.GetText(2);
	into.useBasePrice = row.GetInt(3) ? true : false;
	into.allowManufacture = row.GetInt(4) ? true : false;
	into.allowRecycler = row.GetInt(5) ? true : false;
	into.anchored = row.GetInt(6) ? true : false;
	into.anchorable = row.GetInt(7) ? true : false;
	into.fittableNonSingleton = row.GetInt(8) ? true : false;
	into.published = row.GetInt(9) ? true : false;

	return true;
}

bool InventoryDB::GetType(uint32 typeID, TypeData &into) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" groupID,"
		" typeName,"
		" description,"
		" radius,"
		" mass,"
		" volume,"
		" capacity,"
		" portionSize,"
		" raceID,"
		" basePrice,"
		" published,"
		" marketGroupID,"
		" chanceOfDuplicating"
		" FROM invTypes"
		" WHERE typeID=%u",
		typeID))
	{
		_log(DATABASE__ERROR, "Failed to query type %u: %s.", res.error.c_str());
		return false;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Type %u not found.", typeID);
		return false;
	}

	into.groupID = row.GetUInt(0);
	into.name = row.GetText(1);
	into.description = row.GetText(2);
	into.radius = row.GetDouble(3);
	into.mass = row.GetDouble(4);
	into.volume = row.GetDouble(5);
	into.capacity = row.GetDouble(6);
	into.portionSize = row.GetUInt(7);
	into.race = EVERace(row.IsNull(8) ? 0 : row.GetUInt(8));
	into.basePrice = row.GetDouble(9);
	into.published = row.GetInt(10) ? true : false;
	into.marketGroupID = row.IsNull(11) ? 0 : row.GetUInt(11);
	into.chanceOfDuplicating = row.GetDouble(12);

	return true;
}

bool InventoryDB::GetBlueprintType(uint32 blueprintTypeID, BlueprintTypeData &into) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" parentBlueprintTypeID,"
		" productTypeID,"
		" productionTime,"
		" techLevel,"
		" researchProductivityTime,"
		" researchMaterialTime,"
		" researchCopyTime,"
		" researchTechTime,"
		" productivityModifier,"
		" materialModifier,"
		" wasteFactor / 100,"	// we have it in db as percentage ...
		" chanceOfReverseEngineering,"
		" maxProductionLimit"
		" FROM invBlueprintTypes"
		" WHERE blueprintTypeID=%u",
		blueprintTypeID))
	{
		_log(DATABASE__ERROR, "Error in query: %s.", res.error.c_str());
		return false;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Blueprint type %u not found.", blueprintTypeID);
		return false;
	}

	into.parentBlueprintTypeID = row.IsNull(0) ? 0 : row.GetUInt(0);
	into.productTypeID = row.GetUInt(1);
	into.productionTime = row.GetUInt(2);
	into.techLevel = row.GetUInt(3);
	into.researchProductivityTime = row.GetUInt(4);
	into.researchMaterialTime = row.GetUInt(5);
	into.researchCopyTime = row.GetUInt(6);
	into.researchTechTime = row.GetUInt(7);
	into.productivityModifier = row.GetUInt(8);
	into.materialModifier = row.GetUInt(9);
	into.wasteFactor = row.GetDouble(10);
	into.chanceOfReverseEngineering = row.GetDouble(11);
	into.maxProductionLimit = row.GetUInt(12);

	return true;
}

bool InventoryDB::GetItem(uint32 itemID, ItemData &into) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" itemName,"
		" typeID,"
		" ownerID,"
		" locationID,"
		" flag,"
		" contraband,"
		" singleton,"
		" quantity,"
		" x, y, z,"
		" customInfo"
		" FROM entity"
		" WHERE itemID=%u",
		itemID))
	{
		codelog(SERVICE__ERROR, "Error in query for item %u: %s", itemID, res.error.c_str());
		return NULL;
	}
	
	DBResultRow row;
	if(!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Unable to load item %u", itemID);
		return false;
	}

	into.name = row.GetText(0);
	into.typeID = row.GetUInt(1);
	into.ownerID = row.GetUInt(2);
	into.locationID = row.GetUInt(3);
	into.flag = EVEItemFlags(row.GetUInt(4));
	into.contraband = row.GetInt(5) ? true : false;
	into.singleton = row.GetInt(6) ? true : false;
	into.quantity = row.GetUInt(7);
	into.position = GPoint(row.GetDouble(8), row.GetDouble(9), row.GetDouble(10));
	into.customInfo = row.IsNull(11) ? "" : row.GetText(11);

	return true;
}

uint32 InventoryDB::NewItem(const ItemData &data) {
	DBerror err;
	uint32 eid;

	std::string nameEsc;
	m_db->DoEscapeString(nameEsc, data.name);

	std::string customInfoEsc;
	m_db->DoEscapeString(customInfoEsc, data.customInfo);
	
	if(!m_db->RunQueryLID(err, eid,
		"INSERT INTO entity ("
		"	itemName, typeID, ownerID, locationID, flag,"
		"	contraband, singleton, quantity, x, y, z,"
		"	customInfo"
		" ) "
		"VALUES('%s', %u, %u, %u, %u,"
		"	%u, %u, %u, %f, %f, %f,"
		"	'%s' )",
		nameEsc.c_str(), data.typeID, data.ownerID, data.locationID, data.flag,
		data.contraband?1:0, data.singleton?1:0, data.quantity, data.position.x, data.position.y, data.position.z,
		customInfoEsc.c_str()
		)
	) {
		codelog(SERVICE__ERROR, "Failed to insert new entity: %s", err.c_str());
		return(0);
	}

	return(eid);
}

bool InventoryDB::SaveItem(uint32 itemID, const ItemData &data) {
	DBerror err;

	std::string nameEsc;
	m_db->DoEscapeString(nameEsc, data.name);

	std::string customInfoEsc;
	m_db->DoEscapeString(customInfoEsc, data.customInfo);

	if(!m_db->RunQuery(err,
		"UPDATE entity"
		" SET"
		" itemName = '%s',"
		" typeID = %u,"
		" ownerID = %u,"
		" locationID = %u,"
		" flag = %u,"
		" contraband = %u,"
		" singleton = %u,"
		" quantity = %u,"
		" x = %f, y = %f, z = %f,"
		" customInfo = '%s'"
		" WHERE itemID = %u",
		nameEsc.c_str(),
		data.typeID,
		data.ownerID,
		data.locationID,
		uint32(data.flag),
		uint32(data.contraband),
		uint32(data.singleton),
		data.quantity,
		data.position.x, data.position.y, data.position.z,
		customInfoEsc.c_str(),
		itemID))
	{
		_log(DATABASE__ERROR, "Error in query: %s.", err.c_str());
		return false;
	}

	return true;
}

bool InventoryDB::DeleteItem(uint32 itemID) {
	DBerror err;

	//NOTE: all child entities should be deleted by the caller first.
	
	if(!m_db->RunQuery(err,
		"DELETE"
		" FROM entity"
		" WHERE itemID=%u",
		itemID
	))
	{
		codelog(DATABASE__ERROR, "Failed to delete item %u: %s", itemID, err.c_str());
		return false;
	}
	return true;
}

bool InventoryDB::GetItemContents(uint32 itemID, std::vector<uint32> &items) {
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
		" WHERE locationID=%u",
		itemID))
	{
		codelog(SERVICE__ERROR, "Error in query for item %u: %s", itemID, res.error.c_str());
		return false;
	}
	
	DBResultRow row;
	while(res.GetRow(row)) {
		items.push_back(row.GetUInt(0));
	}

	return true;
}

bool InventoryDB::LoadTypeAttributes(uint32 typeID, EVEAttributeMgr &into) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" attributeID,"
		" valueInt,"
		" valueFloat"
		" FROM dgmTypeAttributes"
		" WHERE typeID=%u",
		typeID))
	{
		_log(DATABASE__ERROR, "Failed to query type attributes for type %u: %s.", typeID, res.error.c_str());
		return false;
	}

	DBResultRow row;
	EVEAttributeMgr::Attr attr;
	while(res.GetRow(row)) {
		if(row.IsNull(0)) {
			_log(DATABASE__ERROR, "Attribute row for type %u has attributeID NULL. Skipping.", typeID);
			continue;
		}
		attr = EVEAttributeMgr::Attr(row.GetUInt(0));
		if(row.IsNull(2)) {
			if(row.IsNull(1))
				_log(DATABASE__ERROR, "Attribute %u for type %u has both values NULL. Skipping.", attr, typeID);
			else
				into.SetInt(attr, row.GetInt(1));
		} else {
			if(!row.IsNull(1))
				_log(DATABASE__ERROR, "Attribute %u for type %u has both values non-NULL. Using float.", attr, typeID);
			into.SetReal(attr, row.GetDouble(2));
		}
	}
	return true;
}

bool InventoryDB::LoadItemAttributes(uint32 itemID, EVEAttributeMgr &into) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" attributeID,"
		" valueInt,"
		" valueFloat"
		" FROM entity_attributes"
		" WHERE itemID=%u",
		itemID))
	{
		_log(DATABASE__ERROR, "Failed to query item attributes for item %u: %s.", itemID, res.error.c_str());
		return false;
	}

	DBResultRow row;
	EVEAttributeMgr::Attr attr;
	while(res.GetRow(row)) {
		if(row.IsNull(0)) {
			_log(DATABASE__ERROR, "Attribute row for item %u has attributeID NULL. Skipping.", itemID);
			continue;
		}
		attr = EVEAttributeMgr::Attr(row.GetUInt(0));
		if(row.IsNull(2)) {
			if(row.IsNull(1))
				_log(DATABASE__ERROR, "Attribute %u for item %u has both values NULL. Skipping.", attr, itemID);
			else
				into.SetInt(attr, row.GetInt(1));
		} else {
			if(!row.IsNull(1))
				_log(DATABASE__ERROR, "Attribute %u for item %u has both values non-NULL. Using float.", attr, itemID);
			into.SetReal(attr, row.GetDouble(2));
		}
	}
	return true;
}

bool InventoryDB::UpdateAttribute_int(uint32 itemID, uint32 attributeID, int v) {
	DBerror err;
	if(!m_db->RunQuery(err,
		"REPLACE INTO entity_attributes"
		"	(itemID, attributeID, valueInt, valueFloat)"
		" VALUES"
		"	(%u, %u, %d, NULL)",
		itemID, attributeID, v)
	) {
		codelog(SERVICE__ERROR, "Failed to store attribute %d for item %u: %s", attributeID, itemID, err.c_str());
		return false;
	}
	return true;
}

bool InventoryDB::UpdateAttribute_double(uint32 itemID, uint32 attributeID, double v) {
	DBerror err;
	if(!m_db->RunQuery(err,
		"REPLACE INTO entity_attributes"
		"	(itemID, attributeID, valueInt, valueFloat)"
		" VALUES"
		"	(%u, %u, NULL, %f)",
		itemID, attributeID, v)
	) {
		codelog(SERVICE__ERROR, "Failed to store attribute %d for item %u: %s", attributeID, itemID, err.c_str());
		return false;
	}
	return true;
}
bool InventoryDB::EraseAttribute(uint32 itemID, uint32 attributeID) {
	DBerror err;
	if(!m_db->RunQuery(err,
		"DELETE FROM entity_attributes"
		" WHERE itemID=%u AND attributeID=%u",
		itemID, attributeID)
	) {
		codelog(SERVICE__ERROR, "Failed to erase attribute %d for item %u: %s", attributeID, itemID, err.c_str());
		return false;
	}
	return true;
}

bool InventoryDB::EraseAttributes(uint32 itemID) {
	DBerror err;
	if(!m_db->RunQuery(err,
		"DELETE"
		" FROM entity_attributes"
		" WHERE itemID=%u",
		itemID))
	{
		_log(DATABASE__ERROR, "Failed to erase attributes for item %u: %s", itemID, err.c_str());
		return false;
	}
	return true;
}

bool InventoryDB::GetBlueprint(uint32 blueprintID, BlueprintData &into) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" copy,"
		" materialLevel,"
		" productivityLevel,"
		" licensedProductionRunsRemaining"
		" FROM invBlueprints"
		" WHERE blueprintID=%u",
		blueprintID))
	{
		_log(DATABASE__ERROR, "Error in query: %s.", res.error.c_str());
		return false;
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Blueprint %u not found.", blueprintID);
		return false;
	}

	into.copy = row.GetInt(0) ? true : false;
	into.materialLevel = row.GetUInt(1);
	into.productivityLevel = row.GetUInt(2);
	into.licensedProductionRunsRemaining = row.GetInt(3);

	return true;
}

bool InventoryDB::NewBlueprint(uint32 blueprintID, const BlueprintData &data) {
	DBerror err;

	if(!m_db->RunQuery(err,
		"INSERT"
		" INTO invBlueprints"
		" (blueprintID, copy, materialLevel, productivityLevel, licensedProductionRunsRemaining)"
		" VALUES"
		" (%u, %u, %u, %u, %d)",
		blueprintID, data.copy, data.materialLevel, data.productivityLevel, data.licensedProductionRunsRemaining))
	{
		_log(DATABASE__ERROR, "Unable to create new blueprint entry for blueprint %u: %s.", blueprintID, err.c_str());
		return false;
	}

	return true;
}

bool InventoryDB::SaveBlueprint(uint32 blueprintID, const BlueprintData &data) {
	DBerror err;

	if(!m_db->RunQuery(err,
		"UPDATE invBlueprints"
		" SET"
		" copy = %u,"
		" materialLevel = %u,"
		" productivityLevel = %u,"
		" licensedProductionRunsRemaining = %d"
		" WHERE blueprintID = %u",
		uint32(data.copy),
		data.materialLevel,
		data.productivityLevel,
		data.licensedProductionRunsRemaining))
	{
		_log(DATABASE__ERROR, "Error in query: %s.", err.c_str());
		return false;
	}

	return true;
}

bool InventoryDB::DeleteBlueprint(uint32 blueprintID) {
	DBerror err;

	if(!m_db->RunQuery(err,
		"DELETE"
		" FROM invBlueprints"
		" WHERE blueprintID=%u",
		blueprintID))
	{
		codelog(DATABASE__ERROR, "Failed to delete blueprint %u: %s.", blueprintID, err.c_str());
		return false;
	}
	return true;
}






































