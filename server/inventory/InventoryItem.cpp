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




InventoryItem::InventoryItem(
	ItemFactory *fact,
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
	bool inDB )
: m_refCount(1),
  factory(fact),
  m_itemID(_itemID),
  m_itemName(_itemName),
  m_typeID(_typeID),
  m_ownerID(_ownerID),
  m_locationID(_locationID),
  m_flag(_flag),
  m_contraband(_contraband),
  m_singleton(_singleton),
  m_quantity(_quantity),
  m_position(_position),
  m_customInfo(_customInfo),
  m_groupID(_groupID),
  m_categoryID(_categoryID),
  m_inDB(inDB),
  m_staticLoaded(false),
  m_attributesLoaded(inDB?false:true),	//non-DB items have no attributes stored in the DB...
  m_contentsLoaded(inDB?false:true)	//non-DB items have no DB contents...
{
}

InventoryItem::~InventoryItem() {
	if(m_refCount > 0) {
		_log(ITEM__ERROR, "Destructing an inventory item (%p) which has %d references!", this, m_refCount);
	}
	
	std::map<uint32, InventoryItem *>::iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++) {
		cur->second->Release();
	}
}

void InventoryItem::Release() {
	if(this == NULL) {
		_log(ITEM__ERROR, "Trying to release a NULL inventory item!");
		//TODO: log a stack.
		return;
	}
	if(m_refCount < 1) {
		_log(ITEM__ERROR, "Releasing an inventory item (%p) which has no references!", this);
		return;
	}
	m_refCount--;
	if(m_refCount == 0) {
		delete this;
	}
}

InventoryItem *InventoryItem::Ref() {
	if(this == NULL) {
		_log(ITEM__ERROR, "Trying to make a ref of a NULL inventory item!");
		//TODO: log a stack.
		return(NULL);
	}
	if(m_refCount < 1) {
		_log(ITEM__ERROR, "Attempting to make a ref of inventory item (%p) which has no references at all!", this);
		return(NULL);	//NULL is easier to debug than a bad pointer
	}
	m_refCount++;
	return(this);
}

void InventoryItem::Delete() {
	//first, get out of client's sight.
	//this also removes us from our container.
	Move(6);
	ChangeOwner(2);

	//we need to delete anything that we contain. This will be recursive.
	LoadContents(true);

	InventoryItem *i;
	std::map<uint32, InventoryItem *>::iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; ) {
		i = cur->second->Ref();
		//iterator will become invalid after calling
		//Delete(), so increment now.
		cur++;

		i->Delete();
	}
	//Delete() releases our ref on these.
	//so it should be empty anyway.
	m_contents.clear();

	//now we need to tell the factory to get rid of us from its cache.
	factory->_DeleteItem(this);
	
	//now we take ourself out of the DB
	factory->db().DeleteItem(this);
	
	//and now we destroy ourself.
	if(m_refCount != 1) {
		_log(ITEM__ERROR, "Delete() called on item %lu (%p) which has %d references! Invalidating as best as possible..", m_itemID, this, m_refCount);
		m_int_attributes.clear();
		m_double_attributes.clear();
		m_itemName = "BAD DELETED ITEM";
		m_quantity = 0;
		m_inDB = false;
		m_contentsLoaded = true;
		//TODO: mark the item for death so that if it does get Release()'d, it will properly die.
	}
	Release();
	//we should be deleted now.... so dont do anything!
}

void InventoryItem::Save(bool recursive) {
	factory->db().SaveAttributes(this);
	
	if(recursive) {
		InventoryItem *i;
		std::map<uint32, InventoryItem *>::iterator cur, end;
		cur = m_contents.begin();
		end = m_contents.end();
		for(; cur != end; cur++) {
			i = cur->second;
			i->Save(true);
		}
	}
}

bool InventoryItem::LoadStatic() {
	if(m_staticLoaded)
		return(true);
	
	m_int_attributes.clear();
	m_double_attributes.clear();
	
	if(!factory->db().LoadEntityAttributes(this))
		return(false);
	if(!factory->db().LoadItemAttributes(this))
		return(false);
	
	m_staticLoaded = true;
	return(true);
}

//this is intended for items loaded from the DB
bool InventoryItem::Load(bool recurse) {
	if(!LoadStatic())
		return(false);
	
	//load player/ship/other non-static attributes.
	if(!m_attributesLoaded) {
		if(m_inDB) {	//no bother checking the DB if the item has never been there..
			if(!factory->db().LoadPersistentAttributes(this))
				return(false);
		}
		m_attributesLoaded = true;
	}
	
	//now load contained items
	if(recurse) {
		if(!LoadContents(recurse))
			return(false);
	}
	
	return(true);
}

bool InventoryItem::LoadContents(bool recursive) {
	if(m_contentsLoaded)
		return(true);
	m_contentsLoaded = true;

	//load the list of items we need
	std::vector<uint32> m_itemIDs;
	if(!factory->db().GetItemContents(this, m_itemIDs))
		return(false);
	
	//Now get each one from the factory (possibly recursing)
	InventoryItem *i;
	std::vector<uint32>::iterator cur, end;
	cur = m_itemIDs.begin();
	end = m_itemIDs.end();
	for(; cur != end; cur++) {
		i = factory->Load(*cur, recursive);
		if(i == NULL) {
			_log(ITEM__ERROR, "Failed to load item %lu contained in %lu. Skipping.", *cur, m_itemID);
			continue;
		}
		//m_contents[i->m_itemID] = i;
		AddContainedItem(i);
		i->Release();
		// The problem was that Create() adds the item to container ... this gained item ref from factory and stick it
		// into contents before checking if item isn't already in m_contents, so we were having 2 or more refs at once
	}
	
	return(true);
}

PyRepDict *InventoryItem::GetEntityAttributes() const {
	PyRepDict *res = new PyRepDict();

	//TODO: combine this with BuildAttributesDict somehow.
	std::map<Attr, int>::const_iterator curi, endi;
	curi = m_int_attributes.begin();
	endi = m_int_attributes.end();
	for(; curi != endi; curi++) {
		res->add(new PyRepInteger(curi->first), new PyRepInteger(curi->second));
	}
	
	std::map<Attr, double>::const_iterator curd, endd;
	curd = m_double_attributes.begin();
	endd = m_double_attributes.end();
	for(; curd != endd; curd++) {
		res->add(new PyRepInteger(curd->first), new PyRepReal(curd->second));
	}

	return(res);
}

PyRepObject *InventoryItem::GetEntityRow() const {
	EntityRowObject ero;
	ero.itemID = m_itemID;
	ero.typeID = m_typeID;
	ero.ownerID = m_ownerID;
	ero.locationID = m_locationID;
	ero.flag = m_flag;
	ero.contraband = m_contraband?1:0;
	ero.singleton = m_singleton?1:0;
	ero.quantity = m_quantity;
	ero.groupID = m_groupID;
	ero.categoryID = m_categoryID;
	ero.customInfo = m_customInfo;
	return(ero.Encode());
}

PyRepObject *InventoryItem::GetInventoryRowset(EVEItemFlags _flag, uint32 forOwner) const {
	util_Rowset rowset;
	
	rowset.rowclass = "util.Row";
	
	rowset.header.push_back("itemID");
	rowset.header.push_back("typeID");
	rowset.header.push_back("ownerID");
	rowset.header.push_back("locationID");
	rowset.header.push_back("flag");
	rowset.header.push_back("contraband");
	rowset.header.push_back("singleton");
	rowset.header.push_back("quantity");
	rowset.header.push_back("groupID");
	rowset.header.push_back("categoryID");
	rowset.header.push_back("customInfo");
	
	//there has to be a better way to build this...
	InventoryItem *i;
	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++) {
		i = cur->second;
		if((_flag == flagAnywhere || i->m_flag == _flag) &&
			(forOwner == NULL || i->m_ownerID == forOwner)
			) {
			PyRepList *item = new PyRepList();
			item->addInt(i->m_itemID);
			item->addInt(i->m_typeID);
			item->addInt(i->m_ownerID);
			item->addInt(i->m_locationID);
			item->addInt(i->m_flag);
			item->addInt(i->m_contraband);
			item->addInt(i->m_singleton);
			item->addInt(i->m_quantity);
			item->addInt(i->m_groupID);
			item->addInt(i->m_categoryID);
			if(i->m_customInfo.empty()) {
				item->add(new PyRepNone());
			} else {
				item->addStr(i->m_customInfo.c_str());
			}
			rowset.lines.add(item);
		}
	}
	
	return(rowset.FastEncode());
}

void InventoryItem::BuildAttributesDict(std::map<uint32, PyRep *> &into) const {
	//NOTE: I suspect we will need to filter these some day..
	{
		std::map<Attr, int>::const_iterator curi, endi;
		curi = m_int_attributes.begin();
		endi = m_int_attributes.end();
		for(; curi != endi; curi++) {
			into[curi->first] = new PyRepInteger(curi->second);
		}
	}

	{
		std::map<Attr, double>::const_iterator curd, endd;
		curd = m_double_attributes.begin();
		endd = m_double_attributes.end();
		for(; curd != endd; curd++) {
			into[curd->first] = new PyRepReal(curd->second);
		}
	}
}

bool InventoryItem::Populate(Rsp_CommonGetInfo_Entry &result) const {
	//itemID:
	result.itemID = m_itemID;

	//invItem:
	result.invItem = GetEntityRow();
	if(result.invItem == NULL) {
		codelog(ITEM__ERROR, "%s (%lu): Unable to build item row for move", m_itemName.c_str(), m_itemID);
		return(false);
	}

	//hacky, but it dosent really hurt anything.
	if(isOnline() != 0) {
		//there is an effect that goes along with this. We should
		//prolly be properly tracking the effect due to some
		// timer things, but for now, were hackin it.
		EntityEffectState es;
		es.env_itemID = m_itemID;
		es.env_charID = m_ownerID;	//may not be quite right...
		es.env_shipID = m_locationID;
		es.env_target = m_locationID;	//this is what they do.
		es.env_other = 0;
		es.env_effectID = effectOnline;
		es.startTime = Win32TimeNow() - 0x60000;	//act like it happened a long time ago
		es.duration = INT_MAX;
		es.repeat = 0;
		result.activeEffects[es.env_effectID] = es.FastEncode();
	}
	
	//activeEffects:
	//result.entry.activeEffects[id] = List[11];
	
	//attributes:
	BuildAttributesDict(result.attributes);
	
	//no idea what time this is supposed to be
	codelog(SERVICE__WARNING, "%s (%lu): sending faked time", m_itemName.c_str(), m_itemID);
	result.time = Win32TimeNow();

	return(true);
}

PyRepObject *InventoryItem::ItemGetInfo() const {
	Rsp_ItemGetInfo result;

	if(!Populate(result.entry))
		return(NULL);	//print already done.
	
	return(result.FastEncode());
}

PyRepObject *InventoryItem::ShipGetInfo() {
	//TODO: verify that we are a ship?
	
	if(!LoadContents(true)) {
		codelog(ITEM__ERROR, "%s (%lu): Failed to load contents for ShipGetInfo", m_itemName.c_str(), m_itemID);
		return(NULL);
	}

	Rsp_CommonGetInfo result;
	Rsp_CommonGetInfo_Entry entry;

	//first populate the ship.
	if(!Populate(entry))
		return(NULL);	//print already done.
	
//hackin:
	//charge
		entry.attributes[Attr_charge] = new PyRepReal(100.000000);
	//shieldCharge
		entry.attributes[264] = new PyRepReal(63.000000);
	//maximumRangeCap
		entry.attributes[797] = new PyRepReal(250000.000000);
	
	result.items[m_itemID] = entry.FastEncode();


	//now encode contents...
	std::vector<InventoryItem *> equipped;
	//find all the equipped items
	FindByFlagRange(flagLowSlot0, flagFixedSlot, equipped, false);
	//encode an entry for each one.
	std::vector<InventoryItem *>::iterator cur, end;
	cur = equipped.begin();
	end = equipped.end();
	for(; cur != end; cur++) {
		if(!(*cur)->Populate(entry)) {
			codelog(ITEM__ERROR, "%s (%lu): Failed to load item %ld for ShipGetInfo", m_itemName.c_str(), m_itemID, (*cur)->m_itemID);
		} else {
			result.items[(*cur)->m_itemID] = entry.FastEncode();
		}
	}
	
	return(result.FastEncode());
}

PyRepObject *InventoryItem::CharGetInfo() {
	//TODO: verify that we are a char?
	
	if(!LoadContents(true)) {
		codelog(ITEM__ERROR, "%s (%lu): Failed to load contents for CharGetInfo", m_itemName.c_str(), m_itemID);
		return(NULL);
	}

	Rsp_CommonGetInfo result;
	Rsp_CommonGetInfo_Entry entry;

	//first encode self.
	if(!Populate(entry))
		return(NULL);	//print already done.
	
	result.items[m_itemID] = entry.FastEncode();
	

	//now encode skills...
	std::vector<InventoryItem *> skills;
	//find all the skills contained within ourself.
	FindByFlag(flagSkill, skills, false);
	//encode an entry for each one.
	std::vector<InventoryItem *>::iterator cur, end;
	cur = skills.begin();
	end = skills.end();
	for(; cur != end; cur++) {
		if(!(*cur)->Populate(entry)) {
			codelog(ITEM__ERROR, "%s (%lu): Failed to load skill item %ld for CharGetInfo", m_itemName.c_str(), m_itemID, (*cur)->m_itemID);
		} else {
			result.items[(*cur)->m_itemID] = entry.FastEncode();
		}
	}
	
	return(result.FastEncode());
}


InventoryItem *InventoryItem::FindFirstByFlag(EVEItemFlags _flag, bool newref) {
	InventoryItem *i;
	std::map<uint32, InventoryItem *>::iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++) {
		i = cur->second;
		if(i->m_flag == _flag) {
			if(newref)
				return(i->Ref());
			return(i);
		}
	}
	return(NULL);
}

InventoryItem *InventoryItem::GetByID(uint32 id, bool newref) {
	InventoryItem *i;
	std::map<uint32, InventoryItem *>::iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++) {
		i = cur->second;
		if(i->m_itemID == id) {
			if(newref)
				return(i->Ref());
			else
				return(i);
		}
	}
	return(NULL);
}

uint32 InventoryItem::FindByFlag(EVEItemFlags _flag, std::vector<InventoryItem *> &items, bool newref) {
	uint32 count = 0;
	InventoryItem *i;
	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++) {
		i = cur->second;
		if(i->m_flag == _flag) {
			if(newref)
				items.push_back(i->Ref());
			else
				items.push_back(i);
			count++;
		}
	}
	return(count);
}

uint32 InventoryItem::FindByFlagRange(EVEItemFlags low_flag, EVEItemFlags high_flag, std::vector<InventoryItem *> &items, bool newref) {
	uint32 count = 0;
	InventoryItem *i;
	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++) {
		i = cur->second;
		if(i->m_flag >= low_flag && i->m_flag <= high_flag) {
			if(newref)
				items.push_back(i->Ref());
			else
				items.push_back(i);
			count++;
		}
	}
	return(count);
}

//annoying duplication, but I am over it.
uint32 InventoryItem::FindByFlag(EVEItemFlags _flag, std::vector<const InventoryItem *> &items, bool newref) const {
	uint32 count = 0;
	InventoryItem *i;
	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++) {
		i = cur->second;
		if(i->m_flag == _flag) {
			if(newref)
				items.push_back(i->Ref());
			else
				items.push_back(i);
			count++;
		}
	}
	return(count);
}

uint32 InventoryItem::FindByFlagRange(EVEItemFlags low_flag, EVEItemFlags high_flag, std::vector<const InventoryItem *> &items, bool newref) const {
	uint32 count = 0;
	InventoryItem *i;
	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++) {
		i = cur->second;
		if(i->m_flag >= low_flag && i->m_flag <= high_flag) {
			if(newref)
				items.push_back(i->Ref());
			else
				items.push_back(i);
			count++;
		}
	}
	return(count);
}

uint32 InventoryItem::FindByFlagSet(std::set<EVEItemFlags> flags, std::vector<const InventoryItem *> &items, bool newref) const {
	uint32 count = 0;
	InventoryItem *i;
	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++) {
		i = cur->second;
		if(flags.count(i->flag())) {
			if(newref)
				items.push_back(i->Ref());
			else
				items.push_back(i);
			count++;
		}
	}
	return(count);

}

InventoryItem *InventoryItem::SpawnSingleton(
	uint32 _typeID,
	uint32 _ownerID,
	EVEItemFlags _flag,
	const char *name
) {
	//for now, just call into the factory, we might eventually make this
	//the only way to spawn items, in which case we can move some of the
	// "update your container" logic from the factory into here...
	return(factory->SpawnSingleton(_typeID, _ownerID, 
									m_itemID, //location
									_flag, name));
}
InventoryItem *InventoryItem::Spawn(
	uint32 _typeID,
	uint32 _quantity,
	uint32 _ownerID,
	EVEItemFlags _flag
) {
	//see note in other overload.
	return(factory->Spawn(_typeID, _quantity, _ownerID, 
						   m_itemID, //location
						   _flag));
}

void InventoryItem::AddContainedItem(InventoryItem *it) {
	std::map<uint32, InventoryItem *>::iterator res;
	res = m_contents.find(it->itemID());
	if(res == m_contents.end()) {
		m_contents[it->itemID()] = it->Ref();
	}
}

void InventoryItem::RemoveContainedItem(InventoryItem *it) {
	std::map<uint32, InventoryItem *>::iterator old_inst;
	old_inst = m_contents.find(it->itemID());
	if(old_inst != m_contents.end()) {
		old_inst->second->Release();
		m_contents.erase(old_inst);
	}
}

void InventoryItem::Rename(const char *to) {
	if(!factory->db().RenameItem(m_itemID, to)) {
		codelog(ITEM__ERROR, "%s (%lu): Failed to rename in the DB", m_itemName.c_str(), m_itemID);
		return;
	}
	m_itemName = to;
}

void InventoryItem::MoveInto(InventoryItem *new_home, EVEItemFlags _flag, bool notify) {
	Move(new_home->m_itemID, _flag, notify);
}

void InventoryItem::Move(uint32 location, EVEItemFlags new_flag, bool notify) {
	uint32 old_location = m_locationID;
	EVEItemFlags old_flag = m_flag;
	
	if(location == old_location && new_flag == old_flag)
		return;	//nothing to do...
	
	if(!factory->db().MoveEntity(m_itemID, location, new_flag)) {
		codelog(ITEM__ERROR, "%s (%lu): Failed to move in the DB to %lu", m_itemName.c_str(), m_itemID, location);
		return;
	}
	
	//first, take myself out of my old container, if its loaded.
	InventoryItem *old_container = factory->GetIfContentsLoaded(old_location);
	if(old_container != NULL) {
		old_container->RemoveContainedItem(this);	//releases its ref
		old_container->Release();
	}
	
	//then make sure that my new container is updated, if its loaded.
	InventoryItem *new_container = factory->GetIfContentsLoaded(location);
	if(new_container != NULL) {
		new_container->AddContainedItem(this);	//makes a new ref
		new_container->Release();
	}
	
	
	m_locationID = location;
	m_flag = new_flag;

	//notify about the changes.
	if(notify) {
		std::map<uint32, PyRep *> changes;
		changes[ixLocationID] = new PyRepInteger(old_location);
		if(new_flag != old_flag)
			changes[ixFlag] = new PyRepInteger(old_flag);
		SendItemChange(m_ownerID, changes);	//changes is consumed
	}
}

void InventoryItem::ChangeFlag(EVEItemFlags new_flag, bool notify) {
	EVEItemFlags old_flag = m_flag;

	if(new_flag == old_flag)
		return;	//nothing to do...

	if(!factory->db().MoveEntity(m_itemID, m_locationID, new_flag)) {
		codelog(ITEM__ERROR, "%s (%lu): Failed to change flag to %d", m_itemName.c_str(), m_itemID, new_flag);
		return;
	}

	m_flag = new_flag;

	//notify about the changes.
	if(notify) {
		std::map<uint32, PyRep *> changes;
		changes[ixFlag] = new PyRepInteger(old_flag);
		SendItemChange(m_ownerID, changes);	//changes is consumed
	}
}

bool InventoryItem::AlterQuantity(int32 qty_change, bool notify) {
	if(qty_change == 0)
		return(true);

	if((int32(m_quantity) + qty_change) < 0) {
		codelog(ITEM__ERROR, "%s (%lu): Tried to remove %ld quantity from stack of %lu", m_itemName.c_str(), m_itemID, -qty_change, m_quantity);
		return(false);
	}

	return(SetQuantity(uint32(m_quantity + qty_change), notify));
}

bool InventoryItem::SetQuantity(uint32 qty_new, bool notify) {
	//if an object has its singleton set then it shouldn't be able to add/remove qty
	if(m_singleton) {
		//Print error
		codelog(ITEM__ERROR, "%s (%lu): Failed to set quantity %lu , the items singleton bit is set", m_itemName.c_str(), m_itemID, qty_new);
		//return false
		return(false);
	}

	uint32 old_qty = m_quantity;
	uint32 new_qty = qty_new;
	
	if(!factory->db().ChangeQuantity(m_itemID, new_qty)) {
		codelog(ITEM__ERROR, "%s (%lu): Failed to change quantity in the DB to %lu", m_itemName.c_str(), m_itemID, new_qty);
		return(false);
	}

	m_quantity = new_qty;

	//notify about the changes.
	if(notify) {
		std::map<uint32, PyRep *> changes;

		//send the notify to the new owner.
		changes[ixQuantity] = new PyRepInteger(old_qty);
		SendItemChange(m_ownerID, changes);	//changes is consumed
	}
	
	return(true);
}

InventoryItem *InventoryItem::Split(int32 qty_to_take, bool notify) {
	if(qty_to_take <= 0) {
		_log(ITEM__ERROR, "%s (%lu): Asked to split into a chunk of %ld", itemName().c_str(), itemID(), qty_to_take);
		return(NULL);
	}
	if(!AlterQuantity(-qty_to_take, notify)) {
		_log(ITEM__ERROR, "%s (%lu): Failed to remove quantity %ld during split.", itemName().c_str(), itemID(), qty_to_take);
		return(NULL);
	}

	InventoryItem *res = factory->Spawn(
		typeID(), qty_to_take, ownerID(), (notify ? 1 : locationID()), flag()	//temp location to cause the spawn via update
		);
	if(notify)
		res->Move(locationID(), flag());

	if(res->categoryID() == EVEDB::invCategories::Blueprint) {
		// copy blueprint properties
		BlueprintProperties bp;
		factory->db().GetBlueprintProperties(itemID(), bp);
		factory->db().SetBlueprintProperties(res->itemID(), bp);
	}

	return( res );
}

bool InventoryItem::Merge(InventoryItem *to_merge, int32 qty, bool notify) {
	if(to_merge == NULL) {
		_log(ITEM__ERROR, "%s (%lu): Cannot merge with NULL item.", itemName().c_str(), itemID());
		return(false);
	}
	if(typeID() != to_merge->typeID()) {
		_log(ITEM__ERROR, "%s (%lu): Asked to merge with %s (%lu).", itemName().c_str(), itemID(), to_merge->itemName().c_str(), to_merge->itemID());
		return(false);
	}
	if(locationID() != to_merge->locationID() || flag() != to_merge->flag()) {
		_log(ITEM__ERROR, "%s (%lu) in location %lu, flag %lu: Asked to merge with item %lu in location %lu, flag %lu.", itemName().c_str(), itemID(), locationID(), flag(), to_merge->itemID(), to_merge->locationID(), to_merge->flag());
		return(false);
	}
	if(qty == 0)
		qty = to_merge->quantity();
	if(qty <= 0) {
		_log(ITEM__ERROR, "%s (%lu): Asked to merge with %ld units of item %lu.", itemName().c_str(), itemID(), qty, to_merge->itemID());
		return(false);
	}
	if(!AlterQuantity(qty, notify)) {
		_log(ITEM__ERROR, "%s (%lu): Failed to add quantity %ld.", itemName().c_str(), itemID(), qty);
		return(false);
	}

	if(qty == to_merge->quantity())
		to_merge->Delete();	//consumes ref
	else if(!to_merge->AlterQuantity(-qty, notify)) {
		_log(ITEM__ERROR, "%s (%lu): Failed to remove quantity %ld.", to_merge->itemName().c_str(), to_merge->itemID(), qty);
		return(false);
	} else
		to_merge->Release();	//consume ref

	return(true);
}

bool InventoryItem::ChangeSingleton(bool new_singleton, bool notify)
{
	bool old_singleton = m_singleton;
	
	if(new_singleton == old_singleton)
		return(true);	//nothing to do...
	
	if(!factory->db().ChangeSingletonEntity(m_itemID, new_singleton)) {
		codelog(ITEM__ERROR, "%s (%lu): Failed to change singleton to %d", m_itemName.c_str(), m_itemID, new_singleton);
		return(false);
	}
	
	m_singleton = new_singleton;

	//notify about the changes.
	if(notify) {
		std::map<uint32, PyRep *> changes;
		changes[ixSingleton] = new PyRepInteger(old_singleton);
		SendItemChange(m_ownerID, changes);	//changes is consumed
	}
	return(true);
}

void InventoryItem::ChangeOwner(uint32 new_owner, bool notify) {
	uint32 old_owner = m_ownerID;
	
	if(new_owner == old_owner)
		return;	//nothing to do...
	
	if(!factory->db().ChangeOwner(m_itemID, new_owner)) {
		codelog(ITEM__ERROR, "%s (%lu): Failed to change owner in the DB to %lu", m_itemName.c_str(), m_itemID, new_owner);
		return;
	}
	
	m_ownerID = new_owner;
	
	//notify about the changes.
	if(notify) {
		std::map<uint32, PyRep *> changes;

		//send the notify to the new owner.
		changes[ixOwnerID] = new PyRepInteger(old_owner);
		SendItemChange(new_owner, changes);	//changes is consumed

		//also send the notify to the old owner.
		changes[ixOwnerID] = new PyRepInteger(old_owner);
		SendItemChange(old_owner, changes);	//changes is consumed
	}
}

//contents of changes are consumed and cleared
void InventoryItem::SendItemChange(uint32 toID, std::map<uint32, PyRep *> &changes) const {
	//TODO: figure out the appropriate list of interested people...
	Client *c = factory->entity_list->FindCharacter(toID);
	if(c == NULL)
		return;	//not found or not online...
	
	
	NotifyOnItemChange change;
	change.itemRow = GetEntityRow();
	if(change.itemRow == NULL) {
		codelog(ITEM__ERROR, "%s (%lu): Unable to build item row for move", m_itemName.c_str(), m_itemID);
		return;
	}
	change.changes = changes;
	changes.clear();	//consume them.

	PyRepTuple *tmp = change.Encode();	//this is consumed below
	c->SendNotification("OnItemChange", "charid", &tmp, false);	//unsequenced.
}

void InventoryItem::PutOnline() {
	SetOnline(true);
}

void InventoryItem::PutOffline() {
	SetOnline(true);
}

void InventoryItem::SetOnline(bool newval) {
	//bool old = isOnline();
	Set_isOnline(newval);
	
	Client *c = factory->entity_list->FindCharacter(m_ownerID);
	if(c == NULL)
		return;	//not found or not online...
	
	Notify_OnModuleAttributeChange omac;
	omac.ownerID = m_ownerID;
	omac.itemKey = m_itemID;
	omac.attributeID = Attr_isOnline;
	omac.time = Win32TimeNow();
	omac.newValue = new PyRepInteger(newval?1:0);
	omac.oldValue = new PyRepInteger(newval?0:1);	//hack... should use old, but its not cooperating today.
	
	Notify_OnGodmaShipEffect ogf;
	ogf.itemID = m_itemID;
	ogf.effectID = effectOnline;
	ogf.when = Win32TimeNow();
	ogf.start = newval?1:0;
	ogf.active = newval?1:0;
	ogf.env_itemID = ogf.itemID;
	ogf.env_charID = m_ownerID;	//??
	ogf.env_shipID = m_locationID;
	ogf.env_target = m_locationID;
	ogf.env_effectID = ogf.effectID;
	ogf.startTime = ogf.when;
	ogf.duration = INT_MAX;	//I think this should be infinity (0x07 may be infinity?)
	ogf.repeat = 0;
	ogf.randomSeed = new PyRepNone();
	ogf.error = new PyRepNone();
	
	Notify_OnMultiEvent multi;
	multi.events.add(omac.FastEncode());
	multi.events.add(ogf.FastEncode());
	
	PyRepTuple *tmp = multi.FastEncode();	//this is consumed below
	c->SendNotification("OnMultiEvent", "clientID", &tmp);
	
	
}

void InventoryItem::SaveAttribute_int(Attr attr) const {
	std::map<Attr, int>::const_iterator res;
	res = m_int_attributes.find(attr);
	if(res == m_int_attributes.end()) {
		if(!factory->db().EraseAttribute(itemID(), attr)) {
			codelog(ITEM__ERROR, "%s (%lu): Failed to delete attribute %d", m_itemName.c_str(), m_itemID, attr);
		}
	} else {
		if(!factory->db().UpdateAttribute_int(itemID(), attr, res->second)) {
			codelog(ITEM__ERROR, "%s (%lu): Failed to save int attribute %d", m_itemName.c_str(), m_itemID, attr);
		}
	}
}

void InventoryItem::SaveAttribute_double(Attr attr) const {
	std::map<Attr, double>::const_iterator res;
	res = m_double_attributes.find(attr);
	if(res == m_double_attributes.end()) {
		if(!factory->db().EraseAttribute(itemID(), attr)) {
			codelog(ITEM__ERROR, "%s (%lu): Failed to delete attribute %d", m_itemName.c_str(), m_itemID, attr);
		}
	} else {
		if(!factory->db().UpdateAttribute_double(itemID(), attr, res->second)) {
			codelog(ITEM__ERROR, "%s (%lu): Failed to save double attribute %d", m_itemName.c_str(), m_itemID, attr);
		}
	}
}

void InventoryItem::SetCustomInfo(const char *ci) {
	if(ci == NULL)
		m_customInfo = "";
	else
		m_customInfo = ci;
	factory->db().SetCustomInfo(m_itemID, ci);
}

bool InventoryItem::Contains(InventoryItem *item, bool recursive) const {
	if(m_contents.find(item->itemID()) != m_contents.end())
		return(true);
	if(!recursive)
		return(false);
	InventoryItem *i;
	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; cur++) {
		i = cur->second;
		if(i->Contains(item, true))
			return(true);
	}
	return(false);
}

//I think I ultimately want this logic somewhere else...
//this dosent seem to be working either... for some reason, the client is
//getting 0 for its skill points per minute field and throwing a division
// exception.
void InventoryItem::TrainSkill(InventoryItem *skill) {
	if(m_flag != flagPilot) {
		codelog(ITEM__ERROR, "%s (%lu): Tried to train skill %lu on non-pilot object.", m_itemName.c_str(), m_itemID, skill->itemID());
		return;
	}
	
	if(skill->flag() == flagSkillInTraining) {
		_log(ITEM__TRACE, "%s (%lu): Requested to train skill %lu item %lu but it is already in training. Doing nothing.", m_itemName.c_str(), m_itemID, skill->typeID(), skill->itemID());
		return;
	}
	
	Client *c = factory->entity_list->FindCharacter(m_ownerID);
	
	//stop training our old skill...
	//search for all, just in case we screwed up
	std::vector<InventoryItem *> skills;
	//find all the skills contained within ourself.
	FindByFlag(flagSkillInTraining, skills, false);
	//encode an entry for each one.
	std::vector<InventoryItem *>::iterator cur, end;
	cur = skills.begin();
	end = skills.end();
	for(; cur != end; cur++) {
		InventoryItem *i = *cur;
		i->ChangeFlag(flagSkill);
		
		if(c != NULL) {
			NotifyOnSkillTrainingStopped osst;
			osst.itemID = skill->itemID();
			osst.endOfTraining = 0;
			PyRepTuple *tmp = osst.FastEncode();	//this is consumed below
			c->SendNotification("OnSkillTrainingStopped", "charid", &tmp);
		}
	}

	if(skill->flag() != flagSkill) {
		//this is a skill book being trained for the first time.
		_log(ITEM__TRACE, "%s (%lu): Initial training of skill %lu item %lu", m_itemName.c_str(), m_itemID, skill->typeID(), skill->itemID());
		skill->MoveInto(this, flagSkillInTraining);
		//set the initial attributes.
		skill->Set_skillLevel(0);
		skill->Set_skillPoints(0);
	} else if(!Contains(skill)) {
		//this must be a skill in another container...
		_log(ITEM__ERROR, "%s (%lu): Tried to train skill %lu item %lu which has the skill flag but is not contained within this item.", m_itemName.c_str(), m_itemID, skill->itemID());
		return;
	} else {
		//skill is within this item, change its flag.
		_log(ITEM__TRACE, "%s (%lu): Starting training of skill %lu item %lu", m_itemName.c_str(), m_itemID, skill->typeID(), skill->itemID());
		skill->ChangeFlag(flagSkillInTraining, true);
	}
	
	if(c != NULL) {
		NotifyOnSkillStartTraining osst;
		osst.itemID = skill->itemID();
		osst.endOfTraining = Win32TimeNow() + Win32Time_Month; //hack hack hack
		
		PyRepTuple *tmp = osst.FastEncode();	//this is consumed below
		c->SendNotification("OnSkillStartTraining", "charid", &tmp);
	}
}

void InventoryItem::Relocate(const GPoint &pos) {
	if(m_position == pos)
		return;

	m_position = pos;
	factory->db().RelocateEntity(m_itemID, m_position.x, m_position.y, m_position.z);
}

void InventoryItem::StackContainedItems(EVEItemFlags locFlag, uint32 forOwner) {
	std::map<uint32, InventoryItem *> types;

	InventoryItem *i;
	std::map<uint32, InventoryItem *>::iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for(; cur != end; ) {
		i = cur->second;
		cur++;

		if( !i->singleton() && (forOwner == i->ownerID() || forOwner == NULL) )
			if(types.find(i->typeID()) == types.end())
				types[i->typeID()] = i;
			else
				//dont forget to make ref
				types[i->typeID()]->Merge(i->Ref());
	}
}

double InventoryItem::GetRemainingCapacity(EVEItemFlags locationFlag) const {
	double remainingCargoSpace;
	//Get correct initial cargo value
	//TODO: Deal with cargo space bonuses
	switch (locationFlag) {
		case flagCargoHold: remainingCargoSpace = capacity(); break;
		case flagDroneBay:	remainingCargoSpace = droneCapacity(); break;
		default:			remainingCargoSpace = 0.0; break;
	}
	
	//TODO: And implement Sizes for packaged ships
 	std::map<uint32, InventoryItem *>::const_iterator cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	
	for(; cur != end; cur++) {
		if(cur->second->flag() == locationFlag)
			remainingCargoSpace -= (cur->second->quantity() * cur->second->volume());
	}

	return(remainingCargoSpace);
}


//this is a very convoluted way to deal with a non-continuous array.
class internalAttrDataKeeper {
private:
	enum {
		TypeInd_int,
		TypeInd_double
	};
public:
	internalAttrDataKeeper() {
		memset(persistent, false, sizeof(persistent));
		memset(is_int, false, sizeof(persistent));
		
		#define ATTR(ID, name, default_value, type, persistent_value) \
			persistent[ID] = persistent_value; \
			is_int[ID] = ( TypeInd_##type == TypeInd_int );
		#include "EVEAttributes.h"
	}
	bool persistent[InventoryItem::Invalid_Attr];
	bool is_int[InventoryItem::Invalid_Attr];
};

static internalAttrDataKeeper _internalAttrDataKeeper;

bool InventoryItem::IsPersistent(Attr attr) {
	return(_internalAttrDataKeeper.persistent[attr]);
}
bool InventoryItem::IsIntAttr(Attr attr) {
	return(_internalAttrDataKeeper.is_int[attr]);
}
bool InventoryItem::IsDoubleAttr(Attr attr) {
	return(!_internalAttrDataKeeper.is_int[attr]);
}






















