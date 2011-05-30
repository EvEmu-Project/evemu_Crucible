/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
	For the latest information visit http://evemu.org
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
	Author:		Bloody.Rabbit
*/

#ifndef __SHIP__H__INCL__
#define __SHIP__H__INCL__

#include "inventory/ItemType.h"
#include "inventory/Inventory.h"
#include "inventory/InventoryItem.h"
#include "system/SystemEntity.h"

/**
 * Basic container for raw ship type data.
 */
class ShipTypeData {
public:
	ShipTypeData(
		uint32 _weaponTypeID = 0,
		uint32 _miningTypeID = 0,
		uint32 _skillTypeID = 0
	);

	// Content:
	uint32 mWeaponTypeID;
	uint32 mMiningTypeID;
	uint32 mSkillTypeID;
};

/**
 * Class managing ship type data.
 */
class ShipType
: public ItemType
{
	friend class ItemType; // to let them construct us
public:
	/**
	 * Loads ship type.
	 *
	 * @param[in] factory
	 * @param[in] shipTypeID ID of ship type to load.
	 * @return Pointer to new ShipType object; NULL if failed.
	 */
	static ShipType *Load(ItemFactory &factory, uint32 shipTypeID);

	/*
	 * Access methods:
	 */
	const ItemType *weaponType() const { return m_weaponType; }
	const ItemType *miningType() const { return m_miningType; }
	const ItemType *skillType() const { return m_skillType; }

protected:
	ShipType(
		uint32 _id,
		// ItemType stuff:
		const ItemGroup &_group,
		const TypeData &_data,
		// ShipType stuff:
		const ItemType *_weaponType,
		const ItemType *_miningType,
		const ItemType *_skillType,
		const ShipTypeData &stData
	);

	/*
	 * Member functions:
	 */
	using ItemType::_Load;

	// Template loader:
	template<class _Ty>
	static _Ty *_LoadType(ItemFactory &factory, uint32 shipTypeID,
		// ItemType stuff:
		const ItemGroup &group, const TypeData &data)
	{
		// verify it's a ship
		if( group.categoryID() != EVEDB::invCategories::Ship ) {
			_log( ITEM__ERROR, "Tried to load %u (%s) as a Ship.", shipTypeID, group.category().name().c_str() );
			return NULL;
		}

		// load additional ship type stuff
		ShipTypeData stData;
		if( !factory.db().GetShipType(shipTypeID, stData) )
			return NULL;

		// try to load weapon type
		const ItemType *weaponType = NULL;
		if( stData.mWeaponTypeID != 0 ) {
			weaponType = factory.GetType( stData.mWeaponTypeID );
			if( weaponType == NULL )
				return NULL;
		}

		// try to load mining type
		const ItemType *miningType = NULL;
		if( stData.mMiningTypeID != 0 ) {
			miningType = factory.GetType( stData.mMiningTypeID );
			if( miningType == NULL )
				return NULL;
		}

		// try to load skill type
		const ItemType *skillType = NULL;
		if( stData.mSkillTypeID != 0 ) {
			skillType = factory.GetType( stData.mSkillTypeID );
			if( skillType == NULL )
				return NULL;
		}

		// continue with load
		return _Ty::template _LoadShipType<_Ty>( factory, shipTypeID, group, data, weaponType, miningType, skillType, stData );
	}

	// Actual loading stuff:
	template<class _Ty>
	static _Ty *_LoadShipType(ItemFactory &factory, uint32 shipTypeID,
		// ItemType stuff:
		const ItemGroup &group, const TypeData &data,
		// ShipType stuff:
		const ItemType *weaponType, const ItemType *miningType, const ItemType *skillType, const ShipTypeData &stData
	);

	/*
	 * Data content:
	 */
	const ItemType *m_weaponType;
	const ItemType *m_miningType;
	const ItemType *m_skillType;
};

/**
 * InventoryItem which represents ship.
 */
class Ship
: public InventoryItem,
  public InventoryEx
{
	friend class InventoryItem;	// to let it construct us
public:
	/**
	 * Loads ship from DB.
	 *
	 * @param[in] factory
	 * @param[in] shipID ID of ship to load.
	 * @return Pointer to Ship object; NULL if failed.
	 */
	static ShipRef Load(ItemFactory &factory, uint32 shipID);
	/**
	 * Spawns new ship.
	 *
	 * @param[in] factory
	 * @param[in] data Item data for ship.
	 * @return Pointer to new Ship object; NULL if failed.
	 */
	static ShipRef Spawn(ItemFactory &factory, ItemData &data);

	/*
	 * Primary public interface:
	 */
	void Delete();

	double GetCapacity(EVEItemFlags flag) const;
	/*
	 * _ExecAdd validation interface:
	 */
	static void ValidateAddItem(EVEItemFlags flag, InventoryItemRef item, Client *c);
	/*
	 * Checks for conflicts between ship and fitting
	 */
	static bool ValidateItemSpecifics(Client *c, InventoryItemRef equip);

	/*
	 * Public fields:
	 */
	const ShipType &    type() const { return static_cast<const ShipType &>(InventoryItem::type()); }

	/*
	 * Primary public packet builders:
	 */
	PyObject *ShipGetInfo();

	/*
	 * Validates boarding ship
	 */
	bool ValidateBoardShip(ShipRef ship, CharacterRef who);

    void SaveShip();

protected:
	Ship(
		ItemFactory &_factory,
		uint32 _shipID,
		// InventoryItem stuff:
		const ShipType &_shipType,
		const ItemData &_data
	);

	/*
	 * Member functions
	 */
	using InventoryItem::_Load;

	// Template loader:
	template<class _Ty>
	static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 shipID,
		// InventoryItem stuff:
		const ItemType &type, const ItemData &data)
	{
		// check it's a ship
		if( type.categoryID() != EVEDB::invCategories::Ship )
		{
			_log( ITEM__ERROR, "Trying to load %s as Ship.", type.category().name().c_str() );
			return RefPtr<_Ty>();
		}
		// cast the type
		const ShipType &shipType = static_cast<const ShipType &>( type );

		// no additional stuff

		return _Ty::template _LoadShip<_Ty>( factory, shipID, shipType, data );
	}

	// Actual loading stuff:
	template<class _Ty>
	static RefPtr<_Ty> _LoadShip(ItemFactory &factory, uint32 shipID,
		// InventoryItem stuff:
		const ShipType &shipType, const ItemData &data
	);

	bool _Load();

	static uint32 _Spawn(ItemFactory &factory,
		// InventoryItem stuff:
		ItemData &data
	);

	uint32 inventoryID() const { return itemID(); }
	PyRep *GetItem() const { return GetItemRow(); }

	void AddItem(InventoryItemRef item);
};

/**
 * DynamicSystemEntity which represents ship object in space
 */
class PyServiceMgr;
class InventoryItem;
class DestinyManager;
class SystemManager;
class ServiceDB;

class ShipEntity
: public DynamicSystemEntity
{
public:
	ShipEntity(
		ShipRef ship,
		SystemManager *system,
		PyServiceMgr &services,
		const GPoint &position);
    ~ShipEntity();

    /*
	 * Primary public interface:
	 */
    ShipRef GetShipObject() { return _shipRef; }
    DestinyManager * GetDestiny() { return m_destiny; }
    SystemManager * GetSystem() { return m_system; }

	/*
	 * Public fields:
	 */
	
	inline double x() const { return(GetPosition().x); }
	inline double y() const { return(GetPosition().y); }
	inline double z() const { return(GetPosition().z); }

	//SystemEntity interface:
	virtual EntityClass GetClass() const { return(ecShipEntity); }
	virtual bool IsShipEntity() const { return true; }
	virtual ShipEntity *CastToShipEntity() { return(this); }
	virtual const ShipEntity *CastToShipEntity() const { return(this); }
	virtual void Process();
	virtual void EncodeDestiny( Buffer& into ) const;
	virtual void TargetAdded(SystemEntity *who) {}
    virtual void TargetLost(SystemEntity *who) {}
    virtual void TargetedAdd(SystemEntity *who) {}
    virtual void TargetedLost(SystemEntity *who) {}
	virtual void TargetsCleared() {}
	virtual void QueueDestinyUpdate(PyTuple **du) {/* not required to consume */}
	virtual void QueueDestinyEvent(PyTuple **multiEvent) {/* not required to consume */}
	virtual uint32 GetCorporationID() const { return(1); }
	virtual uint32 GetAllianceID() const { return(0); }
	virtual void Killed(Damage &fatal_blow);
	virtual SystemManager *System() const { return(m_system); }
	
	void ForcedSetPosition(const GPoint &pt);
	
	virtual bool ApplyDamage(Damage &d);
	virtual void MakeDamageState(DoDestinyDamageState &into) const;

	void SendNotification(const PyAddress &dest, EVENotificationStream &noti, bool seq=true);
	void SendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq=true);

protected:
	/*
	 * Member functions
	 */
    void _ReduceDamage(Damage &d);
    void ApplyDamageModifiers(Damage &d, SystemEntity *target);
    void _DropLoot(SystemEntity *owner);

	/*
	 * Member fields:
	 */
	SystemManager *const m_system;	//we do not own this
	PyServiceMgr &m_services;	//we do not own this
    ShipRef _shipRef;   // We don't own this

	/* Used to calculate the damages on NPCs
	 * I don't know why, npc->Set_shieldCharge does not work
	 * calling npc->shieldCharge() return the complete shield
	 * So we get the values on creation and use then instead.
	*/
	double m_shieldCharge;
	double m_armorDamage;
	double m_hullDamage;
};

#endif /* !__SHIP__H__INCL__ */


