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
#ifndef EVE_INVENTORY_ITEM_H
#define EVE_INVENTORY_ITEM_H

#include <map>
#include <string>
#include <vector>
#include <set>

#include "../common/packet_types.h"
#include "../common/gpoint.h"
//#include "InventoryDB.h"
#include "EVEAttributeMgr.h"
#include "Type.h"

class PyRep;
class PyRepDict;
class PyRepObject;

class ServiceDB;

class ItemFactory;
class Rsp_CommonGetInfo_Entry;

/*
 * NOTE:
 * this object system should somehow be merged with the SystemEntity stuff
 * and class hierarchy built from it (Client, NPC, etc..) in the system manager...
 * however, the creation and destruction time logic is why it has not been done.
 */

extern const uint32 SKILL_BASE_POINTS;

/*
 * Simple container for raw item data.
 */
class ItemData {
public:
	// Full + default constructor:
	ItemData(
		const char *_name = "",
		uint32 _typeID = 0,
		uint32 _ownerID = 0,
		uint32 _locationID = 0,
		EVEItemFlags _flag = flagAutoFit,
		bool _contraband = false,
		bool _singleton = false,
		uint32 _quantity = 0,
		const GPoint &_position = GPoint(0, 0, 0),
		const char *_customInfo = "");
	// Usual-item friendly constructor:
	ItemData(
		uint32 _typeID,
		uint32 _ownerID,
		uint32 _locationID,
		EVEItemFlags _flag,
		uint32 _quantity,
		const char *_customInfo = "",
		bool _contraband = false);
	// Singleton friendly constructor:
	ItemData(
		uint32 _typeID,
		uint32 _ownerID,
		uint32 _locationID,
		EVEItemFlags _flag,
		const char *_name = "",
		const GPoint &_position = GPoint(0, 0, 0),
		const char *_customInfo = "",
		bool _contraband = false);

	// Content:
	std::string name;
	uint32 typeID;
	uint32 ownerID;
	uint32 locationID;
	EVEItemFlags flag;
	bool contraband;
	bool singleton;
	uint32 quantity;
	GPoint position;
	std::string customInfo;
};

/*
 * Class which maintains generic item.
 */
class InventoryItem {
public:
	/*
	 * Factory methods:
	 */
	static InventoryItem *Load(ItemFactory &factory, uint32 itemID, bool recurse=false);
	static InventoryItem *Spawn(ItemFactory &factory, ItemData &data);

	/*
	 * Primary public interface:
	 */
	void Release();
	InventoryItem *Ref();

	virtual void Save(bool recursive=false, bool saveAttributes=true) const;	//save the item to the DB.
	virtual void Delete();	//remove the item from the DB, and Release() it. Consumes a ref!

	bool ContentsLoaded() const { return(m_contentsLoaded); }
	bool LoadContents(bool recursive=true);

	void Rename(const char *to);
	void ChangeOwner(uint32 new_owner, bool notify=true);
	void Move(uint32 location, EVEItemFlags flag=flagAutoFit, bool notify=true);
	void MoveInto(InventoryItem *new_home, EVEItemFlags flag=flagAutoFit, bool notify=true);
	void ChangeFlag(EVEItemFlags flag, bool notify=true);
	void Relocate(const GPoint &pos);
	void SetCustomInfo(const char *ci);
	bool AlterQuantity(int32 qty_change, bool notify=true);
	bool SetQuantity(uint32 qty_new, bool notify=true);
	bool ChangeSingleton(bool singleton, bool notify=true);

	/*
	 * Helper routines:
	 */
	virtual InventoryItem *Split(int32 qty_to_take, bool notify=true);
	virtual bool Merge(InventoryItem *to_merge, int32 qty=0, bool notify=true);	//consumes ref!

	//do we want to impose recursive const?
	bool Contains(InventoryItem *item, bool recursive=false) const;
	InventoryItem *GetByID(uint32 id, bool newref = false);

	InventoryItem *FindFirstByFlag(EVEItemFlags flag, bool newref = false);
	uint32 FindByFlag(EVEItemFlags flag, std::vector<InventoryItem *> &items, bool newref = false);
	uint32 FindByFlagRange(EVEItemFlags low_flag, EVEItemFlags high_flag, std::vector<InventoryItem *> &items, bool newref = false);
	uint32 FindByFlagSet(std::set<EVEItemFlags> flags, std::vector<InventoryItem *> &items, bool newref = false);

	double GetRemainingCapacity( EVEItemFlags flag) const;
	void StackContainedItems( EVEItemFlags flag, uint32 forOwner = NULL);

	void PutOnline();
	void PutOffline();
	void TrainSkill(InventoryItem *skill);	//call on the character object.

	/*
	 * Primary public packet builders:
	 */
	PyRepObject *GetEntityRow() const;
	PyRepObject *GetInventoryRowset(EVEItemFlags flag, uint32 forOwner = NULL) const;
	PyRepObject *ItemGetInfo() const;
	PyRepObject *ShipGetInfo();
	PyRepObject *CharGetInfo();

	/*
	 * Public Fields:
	 */
	uint32                  itemID() const { return(m_itemID); }
	const std::string &     itemName() const { return(m_itemName); }
	const Type &            type() const { return(m_type); }
	uint32                  ownerID() const { return(m_ownerID); }
	uint32                  locationID() const { return(m_locationID); }
	EVEItemFlags            flag() const { return(m_flag); }
	bool                    contraband() const { return(m_contraband); }
	bool                    singleton() const { return(m_singleton); }
	uint32                  quantity() const { return(m_quantity); }
	const GPoint &          position() const { return(m_position); }
	const std::string &     customInfo() const { return(m_customInfo); }

	// helper type methods
	uint32                  typeID() const { return(type().id()); }
	const Group &           group() const { return(type().group()); }
	uint32                  groupID() const { return(type().groupID()); }
	const Category &        category() const { return(type().category()); }
	EVEItemCategories       categoryID() const { return(type().categoryID()); }

	/*
	 * Attribute access:
	 */
	ItemAttributeMgr attributes;

	/*
	 * Redirections
	 */
	#define ATTRFUNC(name, type) \
		inline type name() const { \
			return(attributes.name()); \
		} \
		inline void Set_##name(const type &value) { \
			attributes.Set_##name(value); \
		} \
		inline void Set_##name##_persist(const type &value) { \
			attributes.Set_##name##_persist(value); \
		} \
		inline void Clear_##name() { \
			attributes.Clear_##name(); \
		}
	#define ATTRI(ID, name, default_value, persistent) \
		ATTRFUNC(name, int)
	#define ATTRD(ID, name, default_value, persistent) \
		ATTRFUNC(name, double)
	#include "EVEAttributes.h"

protected:
	InventoryItem(
		ItemFactory &_factory,
		uint32 _itemID,
		// InventoryItem stuff:
		const Type &_type,
		const ItemData &_data);
	virtual ~InventoryItem();

	/*
	 * Internal helper routines:
	 */
	static InventoryItem *_Load(ItemFactory &factory, uint32 itemID
	);
	static InventoryItem *_Load(ItemFactory &factory, uint32 itemID,
		// InventoryItem stuff:
		const Type &type, const ItemData &data
	);

	virtual bool _Load(bool recurse=false);

	static InventoryItem *_Spawn(ItemFactory &factory,
		// InventoryItem stuff:
		const ItemData &data
	);

	void SendItemChange(uint32 toID, std::map<uint32, PyRep *> &changes) const;
	bool Populate(Rsp_CommonGetInfo_Entry &into) const;
	void SetOnline(bool newval);

	void AddContainedItem(InventoryItem *it);
	void RemoveContainedItem(InventoryItem *it);

	/*
	 * Member variables
	 */
	// our reference count:
	uint16          m_refCount;

	// our factory
	ItemFactory &   m_factory;

	// our item data:
	const uint32    m_itemID;
	std::string     m_itemName;
	const Type &    m_type;
	uint32          m_ownerID;
	uint32          m_locationID; //where is this item located
	EVEItemFlags    m_flag;
	bool            m_contraband;
	bool            m_singleton;
	uint32          m_quantity;
	GPoint          m_position;
	std::string     m_customInfo;

	bool m_contentsLoaded;
	std::map<uint32, InventoryItem *> m_contents;	//maps item ID to its instance. we own a ref to all of these.
};

/*
 * Basic container for raw blueprint data.
 */
class BlueprintData {
public:
	BlueprintData(
		bool _copy = false,
		uint32 _materialLevel = 0,
		uint32 _productivityLevel = 0,
		int32 _licensedProductionRunsRemaining = 0
	);

	// Content:
	bool copy;
	uint32 materialLevel;
	uint32 productivityLevel;
	int32 licensedProductionRunsRemaining;
};

/*
 * InventoryItem, which represents blueprint
 */
class BlueprintItem
: public InventoryItem
{
	friend class InventoryItem;	// to let it construct us
public:
	/*
	 * Factory methods:
	 */
	static BlueprintItem *Load(ItemFactory &factory, uint32 blueprintID, bool recurse=false);
	static BlueprintItem *Spawn(ItemFactory &factory, ItemData &data, BlueprintData &bpData);

	/*
	 * Public fields:
	 */
	const BlueprintType &   type() const { return(static_cast<const BlueprintType &>(InventoryItem::type())); }
	const BlueprintType *   parentBlueprintType() const { return(type().parentBlueprintType()); }
	uint32                  parentBlueprintTypeID() const { return(type().parentBlueprintTypeID()); }
	const Type &            productType() const { return(type().productType()); }
	uint32                  productTypeID() const { return(type().productTypeID()); }
	bool                    copy() const { return(m_copy); }
	uint32                  materialLevel() const { return(m_materialLevel); }
	uint32                  productivityLevel() const { return(m_productivityLevel); }
	int32                   licensedProductionRunsRemaining() const { return(m_licensedProductionRunsRemaining); }

	/*
	 * Primary public interface:
	 */
	BlueprintItem *Ref() { return static_cast<BlueprintItem *>(InventoryItem::Ref()); }

	void Save(bool recursive=false, bool saveAttributes=true) const;
	void Delete();

	// copy
	void SetCopy(bool copy);

	// material level
	void SetMaterialLevel(uint32 materialLevel);
	bool AlterMaterialLevel(int32 materialLevelChange);

	// productivity level
	void SetProductivityLevel(uint32 productivityLevel);
	bool AlterProductivityLevel(int32 producitvityLevelChange);

	// licensed production runs
	void SetLicensedProductionRunsRemaining(int32 licensedProductionRunsRemaining);
	void AlterLicensedProductionRunsRemaining(int32 licensedProductionRunsRemainingChange);

	// overload to split the blueprints properly
	InventoryItem *Split(int32 qty_to_take, bool notify=true) { return(SplitBlueprint(qty_to_take, notify)); }
	BlueprintItem *SplitBlueprint(int32 qty_to_take, bool notify=true);

	// overload to do proper merge
	bool Merge(InventoryItem *to_merge, int32 qty=0, bool notify=true);	//consumes ref!

	/*
	 * Helper routines:
	 */
	bool infinite() const { return(licensedProductionRunsRemaining() < 0); }
	double wasteFactor() const { return(type().wasteFactor() / (1 + materialLevel())); }
	// these are for manufacturing:
	double materialMultiplier() const { return(1.0 + wasteFactor()); }
	double timeMultiplier() const {
		return(1.0 - (timeSaved() / type().productionTime()));
	}
	double timeSaved() const {
		return((1.0 - (1.0 / (1 + productivityLevel()))) * type().productivityModifier());
	}

	/*
	 * Primary public packet builders:
	 */
	PyRepDict *GetBlueprintAttributes() const;

protected:
	BlueprintItem(
		ItemFactory &_factory,
		uint32 _blueprintID,
		// InventoryItem stuff:
		const BlueprintType &_bpType,
		const ItemData &_data,
		// BlueprintItem stuff:
		const BlueprintData &_bpData
	);

	/*
	 * Member functions
	 */
	static BlueprintItem *_Load(ItemFactory &factory, uint32 blueprintID
	);
	static BlueprintItem *_Load(ItemFactory &factory, uint32 blueprintID,
		// InventoryItem stuff:
		const BlueprintType &bpType, const ItemData &data
	);
	static BlueprintItem *_Load(ItemFactory &factory, uint32 blueprintID,
		// InventoryItem stuff:
		const BlueprintType &bpType, const ItemData &data,
		// BlueprintItem stuff:
		const BlueprintData &bpData
	);
	virtual bool _Load(bool recurse=false);

	static BlueprintItem *_Spawn(ItemFactory &factory,
		// InventoryItem stuff:
		const ItemData &data,
		// BlueprintItem stuff:
		const BlueprintData &bpData
	);


	/*
	 * Member variables
	 */
	bool m_copy;
	uint32 m_materialLevel;
	uint32 m_productivityLevel;
	int32 m_licensedProductionRunsRemaining;
};


#endif

