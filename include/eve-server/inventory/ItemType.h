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

#ifndef __ITEM_TYPE__H__INCL__
#define __ITEM_TYPE__H__INCL__

#include "inventory/EVEAttributeMgr.h"
#include "inventory/ItemFactory.h"

/*
 * LOADING INVOKATION EXPLANATION:
 * ItemCategory, ItemGroup, ItemType and InventoryItem classes and their children have special loading. Every such type has following methods:
 *
 *  static Load(ItemFactory &factory, <identifier>):
 *    Merges static and virtual loading trees.
 *    First calls static _Load() to create desired object and
 *    then calls its virtual _Load() (if the type has any).
 *
 *  static _Load(ItemFactory &factory, <identifier>[, <data-argument>, ...]):
 *    These functions gradually, one by one, load any data needed to create desired
 *    type and in the end they create the type object.
 *
 *  virtual _Load(ItemFactory &factory) (optional):
 *    Performs any post-construction loading.
 */

/*
 * Simple container for raw category data.
 */
class CategoryData {
public:
	CategoryData(
		const char *_name = "",
		const char *_desc = "",
		bool _published = false
	);

	// Content:
	std::string name;
	std::string description;
	bool published;
};

/*
 * Class which maintains category data.
 */
class ItemCategory {
public:
	/*
	 * Factory method:
	 */
	static ItemCategory *Load(ItemFactory &factory, EVEItemCategories category);

	/*
	 * Access methods
	 */
	EVEItemCategories id() const { return(m_id); }

	const std::string &name() const { return(m_name); }
	const std::string &description() const { return(m_description); }
	bool published() const { return(m_published); }

protected:
	ItemCategory(
		EVEItemCategories _id,
		// ItemCategory stuff:
		const CategoryData &_data
	);

	/*
	 * Member functions
	 */
	static ItemCategory *_Load(ItemFactory &factory, EVEItemCategories category
	);
	static ItemCategory *_Load(ItemFactory &factory, EVEItemCategories category,
		// ItemCategory stuff:
		const CategoryData &data
	);

	/*
	 * Data members
	 */
	const EVEItemCategories m_id;

	std::string m_name;
	std::string m_description;
	bool m_published;
};

/*
 * Simple container for raw group data.
 */
class GroupData {
public:
	GroupData(
		EVEItemCategories _category = (EVEItemCategories)0,
		const char *_name = "",
		const char *_desc = "",
		bool _useBasePrice = false,
		bool _allowManufacture = false,
		bool _allowRecycler = false,
		bool _anchored = false,
		bool _anchorable = false,
		bool _fittableNonSingleton = false,
		bool _published = false
	);

	// Content:
	EVEItemCategories category;
	std::string name;
	std::string description;
	// using a bitfield here saves
	// considerable amount of memory ...
	bool useBasePrice : 1;
	bool allowManufacture : 1;
	bool allowRecycler : 1;
	bool anchored : 1;
	bool anchorable : 1;
	bool fittableNonSingleton : 1;
	bool published : 1;
};

/*
 * Class which maintains group data.
 */
class ItemGroup {
public:
	/*
	 * Factory method:
	 */
	static ItemGroup *Load(ItemFactory &factory, uint32 groupID);

	/*
	 * Access methods:
	 */
	uint32 id() const { return(m_id); }

	const ItemCategory &category() const { return(*m_category); }
	EVEItemCategories categoryID() const { return(category().id()); }

	const std::string &name() const { return(m_name); }
	const std::string &description() const { return(m_description); }
	bool useBasePrice() const { return(m_useBasePrice); }
	bool allowManufacture() const { return(m_allowManufacture); }
	bool allowRecycler() const { return(m_allowRecycler); }
	bool anchored() const { return(m_anchored); }
	bool anchorable() const { return(m_anchorable); }
	bool fittableNonSingleton() const { return(m_fittableNonSingleton); }
	bool published() const { return(m_published); }

protected:
	ItemGroup(
		uint32 _id,
		// ItemGroup stuff:
		const ItemCategory &_category,
		const GroupData &_data
	);

	/*
	 * Member functions
	 */
	static ItemGroup *_Load(ItemFactory &factory, uint32 groupID
	);
	static ItemGroup *_Load(ItemFactory &factory, uint32 groupID,
		// ItemGroup stuff:
		const ItemCategory &category, const GroupData &data
	);

	/*
	 * Data members
	 */
	const uint32 m_id;

	const ItemCategory *m_category;

	std::string m_name;
	std::string m_description;
	// using a bitfield here saves
	// considerable amount of memory ...
	bool m_useBasePrice : 1;
	bool m_allowManufacture : 1;
	bool m_allowRecycler : 1;
	bool m_anchored : 1;
	bool m_anchorable : 1;
	bool m_fittableNonSingleton : 1;
	bool m_published : 1;
};

/*
 * Simple container for raw type data.
 */
class TypeData {
public:
	TypeData(
		uint32 _groupID = 0,
		const char *_name = "",
		const char *_desc = "",
		double _radius = 0.0,
		double _mass = 0.0,
		double _volume = 0.0,
		double _capacity = 0.0,
		uint32 _portionSize = 0,
		EVERace _race = (EVERace)0,
		double _basePrice = 0.0,
		bool _published = false,
		uint32 _marketGroupID = 0,
		double _chanceOfDuplicating = 0.0
	);

	// Content:
	uint32 groupID;
	std::string name;
	std::string description;
	double radius;
	double mass;
	double volume;
	double capacity;
	uint32 portionSize;
	EVERace race;
	double basePrice;
	bool published;
	uint32 marketGroupID;
	double chanceOfDuplicating;
};

/*
 * Class which maintains type data.
 */
class ItemType {
public:
	/**
	 * Loads type from DB.
	 *
	 * @param[in] factory
	 * @param[in] typeID ID of type to load.
	 * @return Pointer to new ItemType object; NULL if failed.
	 */
	static ItemType *Load(ItemFactory &factory, uint32 typeID);

	/*
	 * Attributes:
	 */
	TypeAttributeMgr attributes;

	/*
	 * Helper methods
	 */
	uint32 id() const { return(m_id); }

	const ItemGroup &group() const { return(*m_group); }
	uint32 groupID() const { return(group().id()); }

	const ItemCategory &category() const { return(group().category()); }
	EVEItemCategories categoryID() const { return(group().categoryID()); }

	const std::string &name() const { return(m_name); }
	const std::string &description() const  { return(m_description); }
	uint32 portionSize() const { return(m_portionSize); }
	double basePrice() const { return(m_basePrice); }
	bool published() const { return(m_published); }
	uint32 marketGroupID() const { return(m_marketGroupID); }
	double chanceOfDuplicating() const { return(m_chanceOfDuplicating); }

	double radius() const { return(attributes.radius()); }
	double mass() const { return(attributes.mass()); }
	double volume() const { return(attributes.volume()); }
	double capacity() const { return(attributes.capacity()); }
	EVERace race() const { return(static_cast<EVERace>(attributes.raceID())); }

protected:
	ItemType(
		uint32 _id,
		const ItemGroup &_group,
		const TypeData &_data
	);

	/*
	 * Member functions
	 */
	// Template helper:
	template<class _Ty>
	static _Ty *Load(ItemFactory &factory, uint32 typeID)
	{
		// static load
		_Ty *t = _Ty::template _Load<_Ty>( factory, typeID );
		if( t == NULL )
			return NULL;

		// dynamic load
		if( !t->_Load(factory) )
		{
			delete t;
			return NULL;
		}

		// return
		return t;
	}

	// Template loader:
	template<class _Ty>
	static _Ty *_Load(ItemFactory &factory, uint32 typeID)
	{
		// pull data
		TypeData data;
		if( !factory.db().GetType( typeID, data ) )
			return NULL;

		// obtain group
		const ItemGroup *g = factory.GetGroup( data.groupID );
		if( g == NULL )
			return NULL;

		return _Ty::template _LoadType<_Ty>( factory, typeID, *g, data );
	}

	// Actual loading stuff:
	template<class _Ty>
	static _Ty *_LoadType(ItemFactory &factory, uint32 typeID,
		// ItemType stuff:
		const ItemGroup &group, const TypeData &data
	);

	virtual bool _Load(ItemFactory &factory);

	/*
	 * Data members
	 */
	const uint32 m_id;

	const ItemGroup *m_group;

	std::string m_name;
	std::string m_description;
	uint32 m_portionSize;
	double m_basePrice;
	bool m_published;
	uint32 m_marketGroupID;
	double m_chanceOfDuplicating;
};

#endif /* __ITEM_TYPE__H__INCL__ */



