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

class PyRep;
class PyRepDict;
class PyRepObject;

class ServiceDB;

class ItemFactory;
class Rsp_CommonGetInfo_Entry;

/*

NOTE: this object system should somehow be merged with the SystemEntity stuff
and class hierarchy built from it (Client, NPC, etc..) in the system manager...

however, the creation and destruction time logic is why it has not been done.


*/

static const uint32 SkillBasePoints = 250;

class InventoryItem {
	friend class ItemFactory;
	friend class InventoryDB;
private:
	uint16 m_refCount;
	//creation and destruction of this is protected for reference counting reasons.
	InventoryItem(
		ItemFactory *factory,
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
		bool inDB);
	virtual ~InventoryItem();
	
	//for use by ItemFactory:
	bool LoadStatic();	//load static type attributes
	bool Load(bool recurse=true);
	bool ContentsLoaded() const { return(m_contentsLoaded); }
	void AddContainedItem(InventoryItem *it);
	void RemoveContainedItem(InventoryItem *it);

public:
	void Release();
	InventoryItem *Ref();
	
	void Save(bool recursive=false);	//save the item to the DB.
	bool LoadContents(bool recursive=true);
	void Delete();	//remove the item from the DB, and Release() it. Consumes a ref!
	
	/*
     *   Primary public interface:
     */
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
     *   Helper routines:
     */
	InventoryItem *Split(int32 qty_to_take, bool notify=true);
	bool Merge(InventoryItem *to_merge, int32 qty=0, bool notify=true);	//consumes ref!
	
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
	InventoryItem *SpawnSingleton(
		uint32 typeID,
		uint32 ownerID,
		EVEItemFlags flag,
		const char *name = NULL);
	InventoryItem *Spawn(
		uint32 typeID,
		uint32 quantity,
		uint32 ownerID,
		EVEItemFlags flag);
	
	/*
     *  Primary public packet builders:
     */
	PyRepDict *GetEntityAttributes() const;
	PyRepObject *GetEntityRow() const;
	PyRepObject *GetInventoryRowset(EVEItemFlags flag, uint32 forOwner = NULL) const;
	PyRepObject *ItemGetInfo() const;
	PyRepObject *ShipGetInfo();
	PyRepObject *CharGetInfo();

	/*
     * Public Fields:
     */
	ItemFactory *const factory;	//for now, this needs to be public... I would like to change this eventually.
	uint32				itemID() const { return(m_itemID); }
	const std::string &	itemName() const { return(m_itemName); }
	uint32				typeID() const { return(m_typeID); }
	uint32				ownerID() const { return(m_ownerID); }
	uint32				locationID() const { return(m_locationID); }
	EVEItemFlags		flag() const { return(m_flag); }
	bool				contraband() const { return(m_contraband); }
	bool				singleton() const { return(m_singleton); }
	uint32				quantity() const { return(m_quantity); }
	const GPoint &		position() const { return(m_position); }
	const std::string &	customInfo() const { return(m_customInfo); }
	uint32				groupID() const { return(m_groupID); }
	EVEItemCategories	categoryID() const { return(m_categoryID); }

	/*
     * Attribute access:
     */
	//setup all the attribute access functions... what a nightmare...
	typedef enum {
		#define ATTR(ID, name, default_value, type, persistent) \
			Attr_##name = ID,
		#include "EVEAttributes.h"
		Invalid_Attr
	} Attr;
	
	#define ATTR(ID, name, default_value, type, persistent) \
		inline type name() const { \
			std::map<Attr, type>::const_iterator res; \
			res = m_##type##_attributes.find(Attr_##name); \
			if(res == m_##type##_attributes.end()) \
				return(default_value); \
			return(res->second); \
		} \
		inline void Set_##name(type value) { \
			m_##type##_attributes[Attr_##name] = value; \
			if(m_inDB && m_attributesLoaded && IsPersistent(Attr_##name)) { \
				SaveAttribute_##type(Attr_##name); \
			} \
		} \
		inline void Set_##name##_persist(type value) { \
			m_##type##_attributes[Attr_##name] = value; \
			if(m_inDB) { \
				SaveAttribute_##type(Attr_##name); \
			} \
		} \
		inline void Clear_##name(type value) { \
			m_##type##_attributes.erase(Attr_##name); \
			if(m_inDB && m_attributesLoaded && IsPersistent(Attr_##name)) { \
				SaveAttribute_##type(Attr_##name); \
			} \
		}
	#include "EVEAttributes.h"

	//access by index, be sure you know what your doing....
	void SetAttributeByIndexInt(uint32 index, int value) {	//I dont trust overloading with number types
		//should be checking index for validity, but it wont really hurt anything if its not in the enum right now.
		m_int_attributes[Attr(index)] = value;
	}
	void SetAttributeByIndexDouble(uint32 index, double value) {	//I dont trust overloading with number types
		//should be checking index for validity, but it wont really hurt anything if its not in the enum right now.
		m_double_attributes[Attr(index)] = value;
	}
	
	static bool IsPersistent(Attr attr);
	static bool IsIntAttr(Attr attr);
	static bool IsDoubleAttr(Attr attr);

protected:
	// our item data:
	const uint32	m_itemID;
	std::string		m_itemName;
	const uint32	m_typeID;
	uint32			m_ownerID;
	uint32			m_locationID;	//where is this item located
	EVEItemFlags	m_flag;
	bool			m_contraband;
	bool			m_singleton;
	uint32			m_quantity;
	GPoint			m_position;
	std::string		m_customInfo;
	//derived fields (based on typeID):
	const uint32	m_groupID;
	const EVEItemCategories	m_categoryID;
	
	/*
     * Internal helper routines:
     */
    void SendItemChange(uint32 toID, std::map<uint32, PyRep *> &changes) const;
	void BuildAttributesDict(std::map<uint32, PyRep *> &into) const;
	bool Populate(Rsp_CommonGetInfo_Entry &into) const;
	void SetOnline(bool newval);

	//attribute storage:
	std::map<Attr, int> m_int_attributes;
	std::map<Attr, double> m_double_attributes;
	void SaveAttribute_int(Attr attr) const;
	void SaveAttribute_double(Attr attr) const;
	
	bool m_inDB;	//true if this item is stored in the DB as well.
	bool m_staticLoaded;	//true if static type attributes have been loaded
	bool m_attributesLoaded;	//have persistent attributes from the DB been loaded?
	bool m_contentsLoaded;
	std::map<uint32, InventoryItem *> m_contents;	//maps item ID to its instance. we own a ref to all of these.
};


#endif

