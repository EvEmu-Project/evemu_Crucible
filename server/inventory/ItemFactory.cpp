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

ItemFactory::ItemFactory(DBcore *db, EntityList &el)
: entity_list(el),
  m_db(db)
{
}

ItemFactory::~ItemFactory() {
	// items
	{
		std::map<uint32, InventoryItem *>::const_iterator cur, end;
		cur = m_items.begin();
		end = m_items.end();
		for(; cur != end; cur++) {
			// save item
			cur->second->Save(false);	// do not recurse
			cur->second->Release();
		}
	}
	// types
	{
		std::map<uint32, Type *>::const_iterator cur, end;
		cur = m_types.begin();
		end = m_types.end();
		for(; cur != end; cur++)
			delete cur->second;
	}
	// groups
	{
		std::map<uint32, Group *>::const_iterator cur, end;
		cur = m_groups.begin();
		end = m_groups.end();
		for(; cur != end; cur++)
			delete cur->second;
	}
	// categories
	{
		std::map<EVEItemCategories, Category *>::const_iterator cur, end;
		cur = m_categories.begin();
		end = m_categories.end();
		for(; cur != end; cur++)
			delete cur->second;
	}
}

const Category *ItemFactory::category(EVEItemCategories category) {
	std::map<EVEItemCategories, Category *>::iterator res = m_categories.find(category);
	if(res == m_categories.end()) {
		_log(ITEM__TRACE, "Loading category %lu.", category);

		Category *cat = Category::LoadCategory(*this, category);
		if(cat == NULL)
			return(NULL);

		// insert it into our cache
		res = m_categories.insert(
			std::pair<EVEItemCategories, Category *>(category, cat)
		).first;
	}
	return(res->second);
}

const Group *ItemFactory::group(uint32 groupID) {
	std::map<uint32, Group *>::iterator res = m_groups.find(groupID);
	if(res == m_groups.end()) {
		_log(ITEM__TRACE, "Loading group %lu.", groupID);

		Group *group = Group::LoadGroup(*this, groupID);
		if(group == NULL)
			return(NULL);

		// insert it into cache
		res = m_groups.insert(
			std::pair<uint32, Group *>(groupID, group)
		).first;
	}
	return(res->second);
}

const Type *ItemFactory::type(uint32 typeID) {
	std::map<uint32, Type *>::iterator res = m_types.find(typeID);
	if(res == m_types.end()) {
		_log(ITEM__TRACE, "Loading type %lu.", typeID);

		Type *type = Type::LoadType(*this, typeID);
		if(type == NULL)
			return(NULL);

		// insert into cache
		res = m_types.insert(
			std::pair<uint32, Type *>(typeID, type)
		).first;
	}
	return(res->second);
}
	
InventoryItem *ItemFactory::Load(uint32 itemID, bool recurse) {
	std::map<uint32, InventoryItem *>::iterator res = m_items.find(itemID);
	if(res == m_items.end()) {
		_log(ITEM__TRACE, "Loading item %lu.", itemID);

		InventoryItem *i = InventoryItem::LoadItem(*this, itemID, recurse);
		if(i == NULL)
			return(NULL);

		//we keep the original ref.
		res = m_items.insert(
			std::pair<uint32, InventoryItem *>(itemID, i)
		).first;
	}
	if(recurse)
		res->second->LoadContents(true);
	//we return new ref to the user.
	return(res->second->Ref());
}

BlueprintItem *ItemFactory::LoadBlueprint(uint32 blueprintID, bool recurse) {
	InventoryItem *i = Load(blueprintID, recurse);
	if(i == NULL)
		return(NULL);
	if(i->categoryID() != EVEDB::invCategories::Blueprint) {
		i->Release();
		return(NULL);
	}
	return((BlueprintItem *)(i));
}

InventoryItem *ItemFactory::GetIfContentsLoaded(uint32 itemID) {
	std::map<uint32, InventoryItem *>::const_iterator res = m_items.find(itemID);
	if(res != m_items.end()) {
		if(res->second->ContentsLoaded())
			return(res->second->Ref());
		else
			return(NULL);
	}
	return(NULL);
}
	
InventoryItem *ItemFactory::Spawn(
	uint32 typeID,
	uint32 ownerID,
	uint32 locationID,
	EVEItemFlags flag,
	uint32 quantity
) {
	const Type *t = type(typeID);
	if(t == NULL)
		return(NULL);

	if(t->categoryID() == EVEDB::invCategories::Blueprint)
		return(SpawnBlueprint(typeID, ownerID, locationID, flag, quantity, false, 0, 0, 0));

	GPoint pos(0, 0, 0);
	uint32 itemID = m_db.NewItem(
		t->name.c_str(),
		typeID,
		ownerID,
		locationID,
		flag,
		false,	// contraband
		false,
		quantity,
		pos,
		""	// customInfo
	);
	if(itemID == NULL)
		return(NULL);
	
	return(Load(itemID));
}

InventoryItem *ItemFactory::SpawnSingleton(
	uint32 typeID,
	uint32 ownerID,
	uint32 locationID,
	EVEItemFlags flag,
	const char *name,
	const GPoint &pos
) {
	const Type *t = type(typeID);
	if(t == NULL)
		return(NULL);

	if(t->categoryID() == EVEDB::invCategories::Blueprint)
		return(SpawnBlueprintSingleton(typeID, ownerID, locationID, flag, false, 0, 0, 0, name, pos));

	uint32 itemID = m_db.NewItem(
		name == NULL ? t->name.c_str() : name,
		typeID,
		ownerID,
		locationID,
		flag,
		false,	// contraband
		true,
		1,
		pos,
		""	// customInfo
	);
	if(itemID == NULL)
		return(NULL);
	
	return(Load(itemID));
}

BlueprintItem *ItemFactory::SpawnBlueprint(
	uint32 typeID,
	uint32 ownerID,
	uint32 locationID,
	EVEItemFlags flag,
	uint32 quantity,
	bool copy,
	uint32 materialLevel,
	uint32 productivityLevel,
	int32 licensedProductionRunsRemaining
) {
	const Type *t = type(typeID);
	if(t == NULL)
		return(NULL);

	if(t->categoryID() != EVEDB::invCategories::Blueprint) {
		_log(ITEM__ERROR, "Trying to spawn blueprint with non-blueprint type %lu.", typeID);
		return(NULL);
	}

	GPoint pos(0, 0, 0);
	uint32 itemID = m_db.NewItem(
		t->name.c_str(),
		typeID,
		ownerID,
		locationID,
		flag,
		false,	// contraband
		false,
		quantity,
		pos,
		""	// customInfo
	);
	if(itemID == NULL)
		return(NULL);

	if(!m_db.NewBlueprint(
		itemID,
		copy,
		materialLevel,
		productivityLevel,
		licensedProductionRunsRemaining))
	{
		m_db.DeleteItem(itemID);
		return(NULL);
	}

	return((BlueprintItem *)(Load(itemID)));
}

BlueprintItem *ItemFactory::SpawnBlueprintSingleton(
	uint32 typeID,
	uint32 ownerID,
	uint32 locationID,
	EVEItemFlags flag,
	bool copy,
	uint32 materialLevel,
	uint32 productivityLevel,
	int32 licensedProductionRunsRemaining,
	const char *name,
	const GPoint &pos
) {
	const Type *t = type(typeID);
	if(t == NULL)
		return(NULL);

	if(t->categoryID() != EVEDB::invCategories::Blueprint) {
		_log(ITEM__ERROR, "Trying to spawn blueprint with non-blueprint type %lu.", typeID);
		return(NULL);
	}

	uint32 itemID = m_db.NewItem(
		name == NULL ? t->name.c_str() : name,
		typeID,
		ownerID,
		locationID,
		flag,
		false,	// contraband
		true,
		1,
		pos,
		""	// customInfo
	);
	if(itemID == NULL)
		return(NULL);

	if(!m_db.NewBlueprint(
		itemID,
		copy,
		materialLevel,
		productivityLevel,
		licensedProductionRunsRemaining))
	{
		m_db.DeleteItem(itemID);
		return(NULL);
	}

	return((BlueprintItem *)(Load(itemID)));
}


void ItemFactory::_DeleteItem(uint32 itemID) {
	std::map<uint32, InventoryItem *>::iterator res;
	res = m_items.find(itemID);
	if(res == m_items.end()) {
		codelog(SERVICE__ERROR, "Item ID %d not found when requesting deletion!", itemID);
		return;
	}
	res->second->Release();
	m_items.erase(res);
}




















