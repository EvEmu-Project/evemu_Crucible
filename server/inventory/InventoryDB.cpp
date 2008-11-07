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

#include "EvemuPCH.h"

InventoryDB::InventoryDB(DBcore *db)
: ServiceDB(db)
{
}

InventoryDB::~InventoryDB() {
}

bool InventoryDB::GetCategory(
	EVEItemCategories category,
	std::string &name,
	std::string &desc,
	bool &published)
{
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" categoryName,"
		" description,"
		" published"
		" FROM invCategories"
		" WHERE categoryID=%lu",
		uint32(category)))
	{
		_log(DATABASE__ERROR, "Error in query: %s.", res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Category %lu not found.", uint32(category));
		return(false);
	}

	name = row.GetText(0);
	desc = row.GetText(1);
	published = row.GetInt(2) ? true : false;

	return(true);
}

bool InventoryDB::GetGroup(
	uint32 groupID,
	EVEItemCategories &category,
	std::string &name,
	std::string &desc,
	bool &useBasePrice,
	bool &allowManufacture,
	bool &allowRecycler,
	bool &anchored,
	bool &anchorable,
	bool &fittableNonSingleton,
	bool &published)
{
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
		" WHERE groupID=%lu",
		groupID))
	{
		_log(DATABASE__ERROR, "Failed to query group %lu: %s.", groupID, res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Group %lu not found.", groupID);
		return(false);
	}

	category = EVEItemCategories(row.GetUInt(0));
	name = row.GetText(1);
	desc = row.GetText(2);
	useBasePrice = row.GetInt(3) ? true : false;
	allowManufacture = row.GetInt(4) ? true : false;
	allowRecycler = row.GetInt(5) ? true : false;
	anchored = row.GetInt(6) ? true : false;
	anchorable = row.GetInt(7) ? true : false;
	fittableNonSingleton = row.GetInt(8) ? true : false;
	published = row.GetInt(9) ? true : false;

	return(true);
}

bool InventoryDB::GetType(
	uint32 typeID,
	uint32 &groupID,
	std::string &name,
	std::string &desc,
	double &radius,
	double &mass,
	double &volume,
	double &capacity,
	uint32 &portionSize,
	EVERace &raceID,
	double &basePrice,
	bool &published,
	uint32 &marketGroupID,
	double &chanceOfDuplicating)
{
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
		" WHERE typeID=%lu",
		typeID))
	{
		_log(DATABASE__ERROR, "Failed to query type %lu: %s.", res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Type %lu not found.", typeID);
		return(false);
	}

	groupID = row.GetUInt(0);
	name = row.GetText(1);
	desc = row.GetText(2);
	radius = row.GetDouble(3);
	mass = row.GetDouble(4);
	volume = row.GetDouble(5);
	capacity = row.GetDouble(6);
	portionSize = row.GetUInt(7);
	raceID = EVERace(row.IsNull(8) ? 0 : row.GetUInt(8));
	basePrice = row.GetDouble(9);
	published = row.GetInt(10) ? true : false;
	marketGroupID = row.IsNull(11) ? 0 : row.GetUInt(11);
	chanceOfDuplicating = row.GetDouble(12);

	return(true);
}

bool InventoryDB::GetBlueprintType(
	uint32 blueprintTypeID,
	uint32 &parentBlueprintTypeID,
	uint32 &productTypeID,
	uint32 &productionTime,
	uint32 &techLevel,
	uint32 &researchProductivityTime,
	uint32 &researchMaterialTime,
	uint32 &researchCopyTime,
	uint32 &researchTechTime,
	uint32 &productivityModifier,
	uint32 &materialModifier,
	double &wasteFactor,
	double &chanceOfReverseEngineering,
	uint32 &maxProductionLimit)
{
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
		" WHERE blueprintTypeID=%lu",
		blueprintTypeID))
	{
		_log(DATABASE__ERROR, "Error in query: %s.", res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Blueprint type %lu not found.", blueprintTypeID);
		return(false);
	}

	parentBlueprintTypeID = row.IsNull(0) ? 0 : row.GetUInt(0);
	productTypeID = row.GetUInt(1);
	productionTime = row.GetUInt(2);
	techLevel = row.GetUInt(3);
	researchProductivityTime = row.GetUInt(4);
	researchMaterialTime = row.GetUInt(5);
	researchCopyTime = row.GetUInt(6);
	researchTechTime = row.GetUInt(7);
	productivityModifier = row.GetUInt(8);
	materialModifier = row.GetUInt(9);
	wasteFactor = row.GetDouble(10);
	chanceOfReverseEngineering = row.GetDouble(11);
	maxProductionLimit = row.GetUInt(12);

	return(true);
}

bool InventoryDB::LoadTypeAttributes(uint32 typeID, EVEAttributeMgr &into) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" attributeID,"
		" valueInt,"
		" valueFloat"
		" FROM dgmTypeAttributes"
		" WHERE typeID=%lu",
		typeID))
	{
		_log(DATABASE__ERROR, "Failed to query type attributes for type %lu: %s.", typeID, res.error.c_str());
		return(false);
	}

	DBResultRow row;
	EVEAttributeMgr::Attr attr;
	while(res.GetRow(row)) {
		if(row.IsNull(0)) {
			_log(DATABASE__ERROR, "Attribute row for type %lu has attributeID NULL. Skipping.", typeID);
			continue;
		}
		attr = EVEAttributeMgr::Attr(row.GetUInt(0));
		if(row.IsNull(2)) {
			if(row.IsNull(1))
				_log(DATABASE__ERROR, "Attribute %lu for type %lu has both values NULL. Skipping.", attr, typeID);
			else
				into.SetInt(attr, row.GetInt(1));
		} else {
			if(!row.IsNull(1))
				_log(DATABASE__ERROR, "Attribute %lu for type %lu has both values non-NULL. Using float.", attr, typeID);
			into.SetReal(attr, row.GetDouble(2));
		}
	}
	return(true);
}

bool InventoryDB::GetItem(
	uint32 itemID,
	std::string &name,
	uint32 &typeID,
	uint32 &ownerID,
	uint32 &locationID,
	EVEItemFlags &flag,
	bool &contraband,
	bool &singleton,
	uint32 &quantity,
	GPoint &position,
	std::string &customInfo)
{
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

	name = row.GetText(0);
	typeID = row.GetUInt(1);
	ownerID = row.GetUInt(2);
	locationID = row.GetUInt(3);
	flag = EVEItemFlags(row.GetUInt(4));
	contraband = row.GetInt(5) ? true : false;
	singleton = row.GetInt(6) ? true : false;
	quantity = row.GetUInt(7);
	position.x = row.GetDouble(8);
	position.y = row.GetDouble(9);
	position.z = row.GetDouble(10);
	customInfo = row.IsNull(11) ? "" : row.GetText(11);

	return(true);
}

uint32 InventoryDB::NewItem(const char *name, uint32 typeID, uint32 ownerID, uint32 locationID, EVEItemFlags flag, bool contraband, bool singleton, uint32 quantity, const GPoint &pos, const char *customInfo) {
	DBerror err;
	uint32 eid;

	std::string nameEsc;
	m_db->DoEscapeString(nameEsc, name);

	std::string customInfoEsc;
	m_db->DoEscapeString(customInfoEsc, customInfo);
	
	if(!m_db->RunQueryLID(err, eid,
		"INSERT INTO entity ("
		"	itemName, typeID, ownerID, locationID, flag,"
		"	contraband, singleton, quantity, x, y, z,"
		"	customInfo"
		" ) "
		"VALUES('%s', %lu, %lu, %lu, %lu,"
		"	%lu, %lu, %lu, %f, %f, %f,"
		"	'%s' )",
		nameEsc.c_str(), typeID, ownerID, locationID, flag,
		contraband?1:0, singleton?1:0, quantity, pos.x, pos.y, pos.z,
		customInfoEsc.c_str()
		)
	) {
		codelog(SERVICE__ERROR, "Failed to insert new entity: %s", err.c_str());
		return(0);
	}

	return(eid);
}

bool InventoryDB::SaveItem(uint32 itemID, const char *name, uint32 typeID, uint32 ownerID, uint32 locationID, EVEItemFlags flag, bool contraband, bool singleton, uint32 quantity, const GPoint &pos, const char *customInfo) {
	DBerror err;

	std::string nameEsc;
	m_db->DoEscapeString(nameEsc, name);

	std::string customInfoEsc;
	m_db->DoEscapeString(customInfoEsc, customInfo);

	if(!m_db->RunQuery(err,
		"UPDATE entity"
		" SET"
		" itemName = '%s',"
		" typeID = %lu,"
		" ownerID = %lu,"
		" locationID = %lu,"
		" flag = %ld,"
		" contraband = %lu,"
		" singleton = %lu,"
		" quantity = %lu,"
		" x = %f, y = %f, z = %f,"
		" customInfo = '%s'"
		" WHERE itemID = %lu",
		nameEsc.c_str(),
		typeID,
		ownerID,
		locationID,
		flag,
		uint32(contraband),
		uint32(singleton),
		quantity,
		pos.x, pos.y, pos.z,
		customInfoEsc.c_str(),
		itemID))
	{
		_log(DATABASE__ERROR, "Error in query: %s.", err.c_str());
		return(false);
	}

	return(true);
}

bool InventoryDB::DeleteItem(uint32 itemID) {
	DBerror err;

	//NOTE: all child entities should be deleted by the caller first.
	
	if(!m_db->RunQuery(err,
		"DELETE"
		" FROM entity"
		" WHERE itemID=%lu",
		itemID
	))
	{
		codelog(DATABASE__ERROR, "Failed to delete item %lu: %s", itemID, err.c_str());
		return(false);
	}
	return(true);
}

bool InventoryDB::GetBlueprint(
	uint32 blueprintID,
	bool &copy,
	uint32 &materialLevel,
	uint32 &productivityLevel,
	int32 &licensedProductionRunsRemaining)
{
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" copy,"
		" materialLevel,"
		" productivityLevel,"
		" licensedProductionRunsRemaining"
		" FROM invBlueprints"
		" WHERE blueprintID=%lu",
		blueprintID))
	{
		_log(DATABASE__ERROR, "Error in query: %s.", res.error.c_str());
		return(false);
	}

	DBResultRow row;
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "Blueprint %lu not found.", blueprintID);
		return(false);
	}

	copy = row.GetInt(0) ? true : false;
	materialLevel = row.GetUInt(1);
	productivityLevel = row.GetUInt(2);
	licensedProductionRunsRemaining = row.GetInt(3);

	return(true);
}

bool InventoryDB::NewBlueprint(uint32 blueprintID, bool copy, uint32 materialLevel, uint32 productivityLevel, int32 licensedProductionRunsRemaining) {
	DBerror err;

	if(!m_db->RunQuery(err,
		"INSERT"
		" INTO invBlueprints"
		" (blueprintID, copy, materialLevel, productivityLevel, licensedProductionRunsRemaining)"
		" VALUES"
		" (%lu, %lu, %lu, %lu, %ld)",
		blueprintID, copy, materialLevel, productivityLevel, licensedProductionRunsRemaining))
	{
		_log(DATABASE__ERROR, "Unable to create new blueprint entry for blueprint %lu: %s.", blueprintID, err.c_str());
		return(false);
	}

	return(true);
}

bool InventoryDB::SaveBlueprint(uint32 blueprintID, bool copy, uint32 materialLevel, uint32 productivityLevel, int32 licensedProductionRunsRemaining) {
	DBerror err;

	if(!m_db->RunQuery(err,
		"UPDATE invBlueprints"
		" SET"
		" copy = %lu,"
		" materialLevel = %lu,"
		" productivityLevel = %lu,"
		" licensedProductionRunsRemaining = %ld"
		" WHERE blueprintID = %lu",
		uint32(copy),
		materialLevel,
		productivityLevel,
		licensedProductionRunsRemaining))
	{
		_log(DATABASE__ERROR, "Error in query: %s.", err.c_str());
		return(false);
	}

	return(true);
}

bool InventoryDB::DeleteBlueprint(uint32 blueprintID) {
	DBerror err;

	if(!m_db->RunQuery(err,
		"DELETE"
		" FROM invBlueprints"
		" WHERE blueprintID=%lu",
		blueprintID))
	{
		codelog(DATABASE__ERROR, "Failed to delete blueprint %lu: %s.", blueprintID, err.c_str());
		return(false);
	}
	return(true);
}

bool InventoryDB::LoadItemAttributes(uint32 itemID, EVEAttributeMgr &into) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" attributeID,"
		" valueInt,"
		" valueFloat"
		" FROM entity_attributes"
		" WHERE itemID=%lu",
		itemID))
	{
		_log(DATABASE__ERROR, "Failed to query item attributes for item %lu: %s.", itemID, res.error.c_str());
		return(false);
	}

	DBResultRow row;
	EVEAttributeMgr::Attr attr;
	while(res.GetRow(row)) {
		if(row.IsNull(0)) {
			_log(DATABASE__ERROR, "Attribute row for item %lu has attributeID NULL. Skipping.", itemID);
			continue;
		}
		attr = EVEAttributeMgr::Attr(row.GetUInt(0));
		if(row.IsNull(2)) {
			if(row.IsNull(1))
				_log(DATABASE__ERROR, "Attribute %lu for item %lu has both values NULL. Skipping.", attr, itemID);
			else
				into.SetInt(attr, row.GetInt(1));
		} else {
			if(!row.IsNull(1))
				_log(DATABASE__ERROR, "Attribute %lu for item %lu has both values non-NULL. Using float.", attr, itemID);
			into.SetReal(attr, row.GetDouble(2));
		}
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

bool InventoryDB::EraseAttributes(uint32 itemID) {
	DBerror err;
	if(!m_db->RunQuery(err,
		"DELETE"
		" FROM entity_attributes"
		" WHERE itemID=%lu",
		itemID))
	{
		_log(DATABASE__ERROR, "Failed to erase attributes for item %lu: %s", itemID, err.c_str());
		return(false);
	}
	return(true);
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





































