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

static const uint32 SkillBasePoints = 250;

class InventoryItem {
	friend class ItemFactory;
public:
	InventoryItem(
		ItemFactory &_factory,
		uint32 _itemID,
		const char *_itemName,
		const Type *_type,
		uint32 _ownerID,
		uint32 _locationID,
		EVEItemFlags _flag,
		bool _contraband,
		bool _singleton,
		uint32 _quantity,
		const GPoint &_position,
		const char *_customInfo);
	virtual ~InventoryItem();

	virtual bool Load(bool recurse=false);

	// for use by ItemFactory:
	bool ContentsLoaded() const { return(m_contentsLoaded); }
	void AddContainedItem(InventoryItem *it);
	void RemoveContainedItem(InventoryItem *it);

	/*
	 * Factory method:
	 */
	static InventoryItem *LoadItem(ItemFactory &factory, uint32 itemID, bool recurse=false);

	/*
     * Primary public interface:
     */
	void Release();
	InventoryItem *Ref();

	virtual bool LoadContents(bool recursive=true);
	virtual void Save(bool recursive=false, bool saveAttributes=true) const;	//save the item to the DB.
	virtual void Delete();	//remove the item from the DB, and Release() it. Consumes a ref!

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
	InventoryItem *FindFirstByFlag(EVEItemFlags flag, bool newref = false);
	InventoryItem *GetByID(uint32 id, bool newref = false);
	uint32 FindByFlag(EVEItemFlags flag, std::vector<InventoryItem *> &items, bool newref = false);
	uint32 FindByFlag(EVEItemFlags flag, std::vector<const InventoryItem *> &items, bool newref = false) const;
	uint32 FindByFlagRange(EVEItemFlags low_flag, EVEItemFlags high_flag, std::vector<InventoryItem *> &items, bool newref = false);
	uint32 FindByFlagRange(EVEItemFlags low_flag, EVEItemFlags high_flag, std::vector<const InventoryItem *> &items, bool newref = false) const;
	uint32 FindByFlagSet(std::set<EVEItemFlags> flags, std::vector<InventoryItem *> &items, bool newref = false);
	uint32 FindByFlagSet(std::set<EVEItemFlags> flags, std::vector<const InventoryItem *> &items, bool newref = false) const;
	double GetRemainingCapacity( EVEItemFlags flag) const;
	void StackContainedItems( EVEItemFlags flag, uint32 forOwner = NULL);

	void PutOnline();
	void PutOffline();
	void TrainSkill(InventoryItem *skill);	//call on the character object.

	//spawn a new item with the specified information, creating it in the DB as well.
	InventoryItem *SpawnInto(
		uint32 typeID,
		uint32 ownerID,
		EVEItemFlags flag,
		uint32 quantity);
	InventoryItem *SpawnSingletonInto(
		uint32 typeID,
		uint32 ownerID,
		EVEItemFlags flag,
		const char *name = NULL);
	
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
	uint32				itemID() const { return(m_itemID); }
	const std::string &	itemName() const { return(m_itemName); }
	const Type *		type() const { return(m_type); }
	uint32				typeID() const { return(m_type->id); }
	uint32				ownerID() const { return(m_ownerID); }
	uint32				locationID() const { return(m_locationID); }
	EVEItemFlags		flag() const { return(m_flag); }
	bool				contraband() const { return(m_contraband); }
	bool				singleton() const { return(m_singleton); }
	uint32				quantity() const { return(m_quantity); }
	const GPoint &		position() const { return(m_position); }
	const std::string &	customInfo() const { return(m_customInfo); }

	// helper type methods
	const Group *		group() const { return(m_type->group); }
	uint32				groupID() const { return(m_type->groupID()); }
	const Category *	category() const { return(m_type->category()); }
	EVEItemCategories	categoryID() const { return(m_type->categoryID()); }

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
	uint16 m_refCount;

	// our factory
	ItemFactory &	m_factory;

	// our item data:
	const uint32	m_itemID;
	std::string		m_itemName;
	const Type *	m_type;
	uint32			m_ownerID;
	uint32			m_locationID;	//where is this item located
	EVEItemFlags	m_flag;
	bool			m_contraband;
	bool			m_singleton;
	uint32			m_quantity;
	GPoint			m_position;
	std::string		m_customInfo;
	
	/*
     * Internal helper routines:
     */
    void SendItemChange(uint32 toID, std::map<uint32, PyRep *> &changes) const;
	bool Populate(Rsp_CommonGetInfo_Entry &into) const;
	void SetOnline(bool newval);

	bool m_contentsLoaded;
	std::map<uint32, InventoryItem *> m_contents;	//maps item ID to its instance. we own a ref to all of these.
};

/*
 * InventoryItem, which represents blueprint
 */
class BlueprintItem
: public InventoryItem
{
public:
	BlueprintItem(
		ItemFactory &_factory,
		uint32 _itemID,
		const char *_itemName,
		const Type *_type,
		uint32 _ownerID,
		uint32 _locationID,
		EVEItemFlags _flag,
		bool _contraband,
		bool _singleton,
		uint32 _quantity,
		const GPoint &_position,
		const char *_customInfo,
		bool _copy,
		uint32 _materialLevel,
		uint32 _productivityLevel,
		int32 _licensedProductionRunsRemaining);

	/*
     * Primary public interface:
     */
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
     * Primary public packet builders:
     */
	PyRepDict *GetBlueprintAttributes() const;

	/*
	 * Public fields:
	 */
	const BlueprintType *	bptype() const { return((const BlueprintType *)(InventoryItem::type())); }
	bool					copy() const { return(m_copy); }
	uint32					materialLevel() const { return(m_materialLevel); }
	uint32					productivityLevel() const { return(m_productivityLevel); }
	int32					licensedProductionRunsRemaining() const { return(m_licensedProductionRunsRemaining); }

	/*
     * Helper routines:
     */
	const Type *parentBlueprintType() const { return(bptype()->parentBlueprintType); }
	const Type *productType() const { return(bptype()->productType); }

	bool infinite() const { return(licensedProductionRunsRemaining() < 0); }
	double wasteFactor() const { return(bptype()->wasteFactor / (1 + materialLevel())); }

	// these are for manufacturing!
	double materialMultiplier() const {
		return(1.0 + wasteFactor());
	}
	double timeMultiplier() const {
		return((bptype()->productionTime - (1.0 - (1.0 / (1 + productivityLevel()))) * bptype()->productivityModifier) / bptype()->productionTime);
	}

protected:
	// our blueprint data:
	bool m_copy;
	uint32 m_materialLevel;
	uint32 m_productivityLevel;
	int32 m_licensedProductionRunsRemaining;
};


#endif

