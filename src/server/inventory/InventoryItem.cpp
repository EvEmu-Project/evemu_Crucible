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

#ifndef INT_MAX
#	define INT_MAX 0x7FFFFFFF
#endif

const uint32 SKILL_BASE_POINTS = 250;

/*
 * ItemData
 */
ItemData::ItemData(
	const char *_name,
	uint32 _typeID,
	uint32 _ownerID,
	uint32 _locationID,
	EVEItemFlags _flag,
	bool _contraband,
	bool _singleton,
	uint32 _quantity,
	const GPoint &_position,
	const char *_customInfo)
: name(_name),
  typeID(_typeID),
  ownerID(_ownerID),
  locationID(_locationID),
  flag(_flag),
  contraband(_contraband),
  singleton(_singleton),
  quantity(_quantity),
  position(_position),
  customInfo(_customInfo)
{
}

ItemData::ItemData(
	uint32 _typeID,
	uint32 _ownerID,
	uint32 _locationID,
	EVEItemFlags _flag,
	uint32 _quantity,
	const char *_customInfo,
	bool _contraband)
: name(""),
  typeID(_typeID),
  ownerID(_ownerID),
  locationID(_locationID),
  flag(_flag),
  contraband(_contraband),
  singleton(false),
  quantity(_quantity),
  position(0, 0, 0),
  customInfo(_customInfo)
{
}

ItemData::ItemData(
	uint32 _typeID,
	uint32 _ownerID,
	uint32 _locationID,
	EVEItemFlags _flag,
	const char *_name,
	const GPoint &_position,
	const char *_customInfo,
	bool _contraband)
: name(_name),
  typeID(_typeID),
  ownerID(_ownerID),
  locationID(_locationID),
  flag(_flag),
  contraband(_contraband),
  singleton(true),
  quantity(1),
  position(_position),
  customInfo(_customInfo)
{
}

/*
 * InventoryItem
 */
InventoryItem::InventoryItem(
	ItemFactory &_factory,
	uint32 _itemID,
	const Type &_type,
	const ItemData &_data)
: attributes(_factory, *this, true, true),
  m_refCount(1),
  m_factory(_factory),
  m_itemID(_itemID),
  m_itemName(_data.name),
  m_type(_type),
  m_ownerID(_data.ownerID),
  m_locationID(_data.locationID),
  m_flag(_data.flag),
  m_contraband(_data.contraband),
  m_singleton(_data.singleton),
  m_quantity(_data.quantity),
  m_position(_data.position),
  m_customInfo(_data.customInfo)
{
	// assert for data consistency
	assert(_data.typeID == _type.id());

	_log(ITEM__TRACE, "Created object %p for item %s (%u).", this, itemName().c_str(), itemID());
}

InventoryItem::~InventoryItem() {
	if(m_refCount > 0)
	{
		_log(ITEM__ERROR, "Destructing an inventory item (%p) which has %d references!", this, m_refCount);
	}
}

void InventoryItem::DecRef() {
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
	if(m_refCount <= 0) {
		delete this;
	}
}

InventoryItem * InventoryItem::IncRef()
{
	if(this == NULL) {
		_log(ITEM__ERROR, "Trying to make a ref of a NULL inventory item!");
		//TODO: log a stack.
		return NULL;
	}
	if(m_refCount < 1) {
		_log(ITEM__ERROR, "Attempting to make a ref of inventory item (%p) which has no references at all!", this);
		return NULL;	//NULL is easier to debug than a bad pointer
	}
	m_refCount++;
	return(this);
}

InventoryItem *InventoryItem::Load(ItemFactory &factory, uint32 itemID)
{
	return InventoryItem::Load<InventoryItem>( factory, itemID );
}

template<class _Ty>
_Ty *InventoryItem::_LoadItem(ItemFactory &factory, uint32 itemID,
	// InventoryItem stuff:
	const Type &type, const ItemData &data)
{
	// See what to do next:
	switch( type.categoryID() ) {
		///////////////////////////////////////
		// Blueprint:
		///////////////////////////////////////
		case EVEDB::invCategories::Blueprint: {
			return Blueprint::_LoadItem<Blueprint>( factory, itemID, type, data );
		}

		///////////////////////////////////////
		// Celestial:
		///////////////////////////////////////
		case EVEDB::invCategories::Celestial: {
			return CelestialObject::_LoadItem<CelestialObject>( factory, itemID, type, data );
		}

		///////////////////////////////////////
		// Ship:
		///////////////////////////////////////
		case EVEDB::invCategories::Ship: {
			return Ship::_LoadItem<Ship>( factory, itemID, type, data );
		}

		///////////////////////////////////////
		// Skill:
		///////////////////////////////////////
		case EVEDB::invCategories::Skill: {
			return Skill::_LoadItem<Skill>( factory, itemID, type, data );
		}
	}

	// Category didn't do it, try Group:
	switch( type.groupID() ) {
		///////////////////////////////////////
		// Character:
		///////////////////////////////////////
		case EVEDB::invGroups::Character: {
			// create character
			return Character::_LoadItem<Character>( factory, itemID, type, data );
		}

		///////////////////////////////////////
		// Station:
		///////////////////////////////////////
		case EVEDB::invGroups::Station: {
			return Station::_LoadItem<Station>( factory, itemID, type, data );
		}
	}

	// Generic item, create one:
	return new InventoryItem( factory, itemID, type, data );
}

bool InventoryItem::_Load()
{
	// load attributes
	if(!attributes.Load())
		return false;

	// update container
	ItemContainer *container = m_factory.GetItemContainer( locationID(), false );
	if(container != NULL)
		container->AddContainedItem( *this );

	return true;
}

InventoryItem *InventoryItem::Spawn(ItemFactory &factory, ItemData &data)
{
	// obtain type of new item
	const Type *t = factory.GetType( data.typeID );
	if( t == NULL )
		return NULL;

	// See what to do next:
	switch( t->categoryID() ) {
		///////////////////////////////////////
		// Blueprint:
		///////////////////////////////////////
		case EVEDB::invCategories::Blueprint: {
			BlueprintData bdata; // use default blueprint attributes

			return Blueprint::Spawn( factory, data, bdata );
		}

		///////////////////////////////////////
		// Celestial:
		///////////////////////////////////////
		case EVEDB::invCategories::Celestial: {
			_log( ITEM__ERROR, "Refusing to spawn celestial object '%s'.", data.name.c_str() );

			return NULL;
		}

		///////////////////////////////////////
		// Ship:
		///////////////////////////////////////
		case EVEDB::invCategories::Ship: {
			return Ship::Spawn( factory, data );
		}

		///////////////////////////////////////
		// Skill:
		///////////////////////////////////////
		case EVEDB::invCategories::Skill: {
			return Skill::Spawn( factory, data );
		}
	}

	switch( t->groupID() ) {
		///////////////////////////////////////
		// Character:
		///////////////////////////////////////
		case EVEDB::invGroups::Character: {
			// we're not gonna create character from default attributes ...
			_log( ITEM__ERROR, "Refusing to create character '%s' from default attributes.", data.name.c_str() );

			return NULL;
		}

		///////////////////////////////////////
		// Station:
		///////////////////////////////////////
		case EVEDB::invGroups::Station: {
			_log( ITEM__ERROR, "Refusing to create station '%s'.", data.name.c_str() );

			return NULL;
		}
	}

	// Spawn generic item:
	uint32 itemID = InventoryItem::_Spawn( factory, data );
	if( itemID == 0 )
		return NULL;
	return InventoryItem::Load( factory, itemID );
}

uint32 InventoryItem::_Spawn(ItemFactory &factory,
	// InventoryItem stuff:
	ItemData &data
) {
	// obtain type of new item
	// this also checks that the type is valid
	const Type *t = factory.GetType(data.typeID);
	if(t == NULL)
		return 0;

	// fix the name (if empty)
	if(data.name.empty())
		data.name = t->name();

	// insert new entry into DB
	return factory.db().NewItem(data);
}

void InventoryItem::Delete() {
	//first, get out of client's sight.
	//this also removes us from our container.
	Move(6);
	ChangeOwner(2);

	//now we need to tell the factory to get rid of us from its cache.
	m_factory._DeleteItem(m_itemID);
	
	//now we take ourself out of the DB
	attributes.Delete();
	m_factory.db().DeleteItem(m_itemID);
	
	//and now we destroy ourself.
	if(m_refCount != 1) {
		_log(ITEM__ERROR, "Delete() called on item %u (%p) which has %u references! Invalidating as best as possible..", m_itemID, this, m_refCount);
		m_itemName = "BAD DELETED ITEM";
		m_quantity = 0;
		//TODO: mark the item for death so that if it does get DecRef()'d, it will properly die.
	}
	DecRef();
	//we should be deleted now.... so dont do anything!
}

PyRepPackedRow *InventoryItem::GetItemRow() const
{
	blue_DBRowDescriptor desc;

	ItemRow_Columns cols;
	desc.columns = cols.FastEncode();

	PyRepPackedRow *row = new PyRepPackedRow( desc.FastEncode(), true );
	GetItemRow( *row );
	return row;
}

void InventoryItem::GetItemRow(PyRepPackedRow &into) const
{
	// Content must be stacked by size:
	// 4-byte values:
	into.PushUInt32( itemID() );
	into.PushUInt32( ownerID() );
	into.PushUInt32( locationID() );
	into.PushUInt32( quantity() );

	// 2-byte values:
	into.PushUInt16( typeID() );
	into.PushUInt16( groupID() );

	// 1-byte values:
	into.PushUInt8( flag() );
	into.PushUInt8( categoryID() );

	// Bool values
	into.PushUInt8( (contraband() << 0)
	                | (singleton() << 1) );

	// Additional PyReps:
	into.PushPyRep( new PyRepString( customInfo() ) );
}

bool InventoryItem::Populate(Rsp_CommonGetInfo_Entry &result) const {
	//itemID:
	result.itemID = itemID();

	//invItem:
	result.invItem = GetItemRow();

	//hacky, but it doesn't really hurt anything.
	if(isOnline() != 0) {
		//there is an effect that goes along with this. We should
		//probably be properly tracking the effect due to some
		// timer things, but for now, were hacking it.
		EntityEffectState es;
		es.env_itemID = m_itemID;
		es.env_charID = m_ownerID;	//may not be quite right...
		es.env_shipID = m_locationID;
		es.env_target = m_locationID;	//this is what they do.
		es.env_other = new PyRepNone;
		es.env_effectID = effectOnline;
		es.startTime = Win32TimeNow() - Win32Time_Hour;	//act like it happened an hour ago
		es.duration = INT_MAX;
		es.repeat = 0;
		es.randomSeed = new PyRepNone;

		result.activeEffects[es.env_effectID] = es.FastEncode();
	}
	
	//activeEffects:
	//result.entry.activeEffects[id] = List[11];
	
	//attributes:
	attributes.EncodeAttributes(result.attributes);

	//no idea what time this is supposed to be
	result.time = Win32TimeNow();

	return true;
}

PyRepObject *InventoryItem::ItemGetInfo() const {
	Rsp_ItemGetInfo result;

	if(!Populate(result.entry))
		return NULL;	//print already done.
	
	return(result.FastEncode());
}


void InventoryItem::Rename(const char *to) {
	m_itemName = to;
	// TODO: send some kind of update?
	SaveItem();
}

void InventoryItem::MoveInto(ItemContainer &new_home, EVEItemFlags _flag, bool notify) {
	Move( new_home.containerID(), _flag, notify );
}

void InventoryItem::Move(uint32 new_location, EVEItemFlags new_flag, bool notify) {
	uint32 old_location = locationID();
	EVEItemFlags old_flag = flag();
	
	if( new_location == old_location && new_flag == old_flag )
		return;	//nothing to do...
	
	//first, take myself out of my old container, if its loaded.
	ItemContainer *old_container = m_factory.GetItemContainer( old_location, false );
	if(old_container != NULL)
		old_container->RemoveContainedItem( itemID() );	//releases its ref
	
	m_locationID = new_location;
	m_flag = new_flag;

	//then make sure that my new container is updated, if its loaded.
	ItemContainer *new_container = m_factory.GetItemContainer( new_location, false );
	if(new_container != NULL)
		new_container->AddContainedItem( *this );	//makes a new ref

	SaveItem();

	//notify about the changes.
	if( notify )
	{
		std::map<uint32, PyRep *> changes;

		if( new_location != old_location )
			changes[ixLocationID] = new PyRepInteger(old_location);
		if( new_flag != old_flag )
			changes[ixFlag] = new PyRepInteger(old_flag);

		SendItemChange( ownerID(), changes );	//changes is consumed
	}
}

bool InventoryItem::AlterQuantity(int32 qty_change, bool notify) {
	if(qty_change == 0)
		return true;

	int32 new_qty = m_quantity + qty_change;

	if(new_qty < 0) {
		codelog(ITEM__ERROR, "%s (%u): Tried to remove %d quantity from stack of %u", m_itemName.c_str(), m_itemID, -qty_change, m_quantity);
		return false;
	}

	return(SetQuantity(new_qty, notify));
}

bool InventoryItem::SetQuantity(uint32 qty_new, bool notify) {
	//if an object has its singleton set then it shouldn't be able to add/remove qty
	if(m_singleton) {
		//Print error
		codelog(ITEM__ERROR, "%s (%u): Failed to set quantity %u , the items singleton bit is set", m_itemName.c_str(), m_itemID, qty_new);
		//return false
		return false;
	}

	uint32 old_qty = m_quantity;
	
	m_quantity = qty_new;

	SaveItem();

	//notify about the changes.
	if(notify) {
		std::map<uint32, PyRep *> changes;

		//send the notify to the new owner.
		changes[ixQuantity] = new PyRepInteger(old_qty);
		SendItemChange(m_ownerID, changes);	//changes is consumed
	}
	
	return true;
}

InventoryItem *InventoryItem::Split(int32 qty_to_take, bool notify) {
	if(qty_to_take <= 0) {
		_log(ITEM__ERROR, "%s (%u): Asked to split into a chunk of %d", itemName().c_str(), itemID(), qty_to_take);
		return NULL;
	}
	if(!AlterQuantity(-qty_to_take, notify)) {
		_log(ITEM__ERROR, "%s (%u): Failed to remove quantity %d during split.", itemName().c_str(), itemID(), qty_to_take);
		return NULL;
	}

	ItemData idata(
		typeID(),
		ownerID(),
		(notify ? 1 : locationID()), //temp location to cause the spawn via update
		flag(),
		qty_to_take
	);

	InventoryItem *res = m_factory.SpawnItem(idata);
	if(notify)
		res->Move(locationID(), flag());

	return( res );
}

bool InventoryItem::Merge(InventoryItem *to_merge, int32 qty, bool notify) {
	if(to_merge == NULL) {
		_log(ITEM__ERROR, "%s (%u): Cannot merge with NULL item.", itemName().c_str(), itemID());
		return false;
	}
	if(typeID() != to_merge->typeID()) {
		_log(ITEM__ERROR, "%s (%u): Asked to merge with %s (%u).", itemName().c_str(), itemID(), to_merge->itemName().c_str(), to_merge->itemID());
		return false;
	}
	if(locationID() != to_merge->locationID() || flag() != to_merge->flag()) {
		_log(ITEM__ERROR, "%s (%u) in location %u, flag %u: Asked to merge with item %u in location %u, flag %u.", itemName().c_str(), itemID(), locationID(), flag(), to_merge->itemID(), to_merge->locationID(), to_merge->flag());
		return false;
	}
	if(qty == 0)
		qty = to_merge->quantity();
	if(qty <= 0) {
		_log(ITEM__ERROR, "%s (%u): Asked to merge with %d units of item %u.", itemName().c_str(), itemID(), qty, to_merge->itemID());
		return false;
	}
	if(!AlterQuantity(qty, notify)) {
		_log(ITEM__ERROR, "%s (%u): Failed to add quantity %d.", itemName().c_str(), itemID(), qty);
		return false;
	}

	if(qty == to_merge->quantity()) {
		to_merge->Delete();	//consumes ref
	} else if(!to_merge->AlterQuantity(-qty, notify)) {
		_log(ITEM__ERROR, "%s (%u): Failed to remove quantity %d.", to_merge->itemName().c_str(), to_merge->itemID(), qty);
		return false;
	} else
		to_merge->DecRef();	//consume ref

	return true;
}

bool InventoryItem::ChangeSingleton(bool new_singleton, bool notify) {
	bool old_singleton = m_singleton;
	
	if(new_singleton == old_singleton)
		return true;	//nothing to do...

	m_singleton = new_singleton;

	SaveItem();

	//notify about the changes.
	if(notify) {
		std::map<uint32, PyRep *> changes;
		changes[ixSingleton] = new PyRepInteger(old_singleton);
		SendItemChange(m_ownerID, changes);	//changes is consumed
	}

	return true;
}

void InventoryItem::ChangeOwner(uint32 new_owner, bool notify) {
	uint32 old_owner = m_ownerID;
	
	if(new_owner == old_owner)
		return;	//nothing to do...
	
	m_ownerID = new_owner;

	SaveItem();
	
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

void InventoryItem::SaveItem() const
{
	_log( ITEM__TRACE, "Saving item %u.", itemID() );

	m_factory.db().SaveItem(
		itemID(),
		ItemData(
			itemName().c_str(),
			typeID(),
			ownerID(),
			locationID(),
			flag(),
			contraband(),
			singleton(),
			quantity(),
			position(),
			customInfo().c_str()
		)
	);
}

//contents of changes are consumed and cleared
void InventoryItem::SendItemChange(uint32 toID, std::map<uint32, PyRep *> &changes) const {
	//TODO: figure out the appropriate list of interested people...
	Client *c = m_factory.entity_list.FindCharacter(toID);
	if(c == NULL)
		return;	//not found or not online...

	NotifyOnItemChange change;
	change.itemRow = GetItemRow();

	change.changes = changes;
	changes.clear();	//consume them.

	PyRepTuple *tmp = change.Encode();	//this is consumed below
	c->SendNotification("OnItemChange", "charid", &tmp, false);	//unsequenced.
}

void InventoryItem::SetOnline(bool newval) {
	//bool old = isOnline();
	Set_isOnline(newval);
	
	Client *c = m_factory.entity_list.FindCharacter(m_ownerID);
	if(c == NULL)
		return;	//not found or not online...
	
	Notify_OnModuleAttributeChange omac;
	omac.ownerID = m_ownerID;
	omac.itemKey = m_itemID;
	omac.attributeID = ItemAttributeMgr::Attr_isOnline;
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

void InventoryItem::SetCustomInfo(const char *ci) {
	if(ci == NULL)
		m_customInfo = "";
	else
		m_customInfo = ci;
	SaveItem();
}

void InventoryItem::Relocate(const GPoint &pos) {
	if(m_position == pos)
		return;

	m_position = pos;

	SaveItem();
}


