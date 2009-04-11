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
	Author:		Bloody.Rabbit
*/

#ifndef __TYPE__H__INCL__
#define __TYPE__H__INCL__

#include <string>

#include "../common/packet_types.h"

#include "EVEAttributeMgr.h"
#include "ItemFactory.h"

/*
 * LOADING INVOKATION EXPLANATION:
 * Category, Group, Type and InventoryItem classes and their children have special loading. Every such type has following methods:
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
class Category {
public:
	/*
	 * Factory method:
	 */
	static Category *Load(ItemFactory &factory, EVEItemCategories category);

	/*
	 * Access methods
	 */
	EVEItemCategories id() const { return(m_id); }

	const std::string &name() const { return(m_name); }
	const std::string &description() const { return(m_description); }
	bool published() const { return(m_published); }

protected:
	Category(
		EVEItemCategories _id,
		// Category stuff:
		const CategoryData &_data
	);

	/*
	 * Member functions
	 */
	static Category *_Load(ItemFactory &factory, EVEItemCategories category
	);
	static Category *_Load(ItemFactory &factory, EVEItemCategories category,
		// Category stuff:
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
class Group {
public:
	/*
	 * Factory method:
	 */
	static Group *Load(ItemFactory &factory, uint32 groupID);

	/*
	 * Access methods:
	 */
	uint32 id() const { return(m_id); }

	const Category &category() const { return(*m_category); }
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
	Group(
		uint32 _id,
		// Group stuff:
		const Category &_category,
		const GroupData &_data
	);

	/*
	 * Member functions
	 */
	static Group *_Load(ItemFactory &factory, uint32 groupID
	);
	static Group *_Load(ItemFactory &factory, uint32 groupID,
		// Group stuff:
		const Category &category, const GroupData &data
	);

	/*
	 * Data members
	 */
	const uint32 m_id;

	const Category *m_category;

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
class Type {
public:
	/*
	 * Factory method:
	 */
	static Type *Load(ItemFactory &factory, uint32 typeID);

	/*
	 * Attributes:
	 */
	TypeAttributeMgr attributes;

	/*
	 * Helper methods
	 */
	uint32 id() const { return(m_id); }

	const Group &group() const { return(*m_group); }
	uint32 groupID() const { return(group().id()); }

	const Category &category() const { return(group().category()); }
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
	Type(
		uint32 _id,
		const Group &_group,
		const TypeData &_data
	);

	/*
	 * Member functions
	 */
	static Type *_Load(ItemFactory &factory, uint32 typeID
	);
	static Type *_Load(ItemFactory &factory, uint32 typeID,
		// Type stuff:
		const Group &group, const TypeData &data
	);

	virtual bool _Load(ItemFactory &factory);

	/*
	 * Data members
	 */
	const uint32 m_id;

	const Group *m_group;

	std::string m_name;
	std::string m_description;
	uint32 m_portionSize;
	double m_basePrice;
	bool m_published;
	uint32 m_marketGroupID;
	double m_chanceOfDuplicating;
};

/*
 * Simple container for raw blueprint type data.
 */
class BlueprintTypeData {
public:
	BlueprintTypeData(
		uint32 _parentBlueprintTypeID = 0,
		uint32 _productTypeID = 0,
		uint32 _productionTime = 0,
		uint32 _techLevel = 0,
		uint32 _researchProductivityTime = 0,
		uint32 _researchMaterialTime = 0,
		uint32 _researchCopyTime = 0,
		uint32 _researchTechTime = 0,
		uint32 _productivityModifier = 0,
		uint32 _materialModifier = 0,
		double _wasteFactor = 0.0,
		double _chanceOfReverseEngineering = 0.0,
		uint32 _maxProductionLimit = 0
	);

	// Content:
	uint32 parentBlueprintTypeID;
	uint32 productTypeID;
	uint32 productionTime;
	uint32 techLevel;
	uint32 researchProductivityTime;
	uint32 researchMaterialTime;
	uint32 researchCopyTime;
	uint32 researchTechTime;
	uint32 productivityModifier;
	uint32 materialModifier;
	double wasteFactor;
	double chanceOfReverseEngineering;
	uint32 maxProductionLimit;
};

/*
 * Class which contains blueprint type data.
 */
class BlueprintType
: public Type
{
	friend class Type;	// To let our parent redirect construction to our _Load().
public:
	/*
	 * Factory method:
	 */
	static BlueprintType *Load(ItemFactory &factory, uint32 typeID);

	/*
	 * Access functions:
	 */
	const BlueprintType *parentBlueprintType() const { return(m_parentBlueprintType); }
	uint32 parentBlueprintTypeID() const { return(parentBlueprintType() == NULL ? 0 : parentBlueprintType()->id()); }

	const Type &productType() const { return(m_productType); }
	uint32 productTypeID() const { return(productType().id()); }

	uint32 productionTime() const { return(m_productionTime); }
	uint32 techLevel() const { return(m_techLevel); }
	uint32 researchProductivityTime() const { return(m_researchProductivityTime); }
	uint32 researchMaterialTime() const { return(m_researchMaterialTime); }
	uint32 researchCopyTime() const { return(m_researchCopyTime); }
	uint32 researchTechTime() const { return(m_researchTechTime); }
	uint32 productivityModifier() const { return(m_productivityModifier); }
	uint32 materialModifier() const { return(m_materialModifier); }
	double wasteFactor() const { return(m_wasteFactor); }
	double chanceOfReverseEngineering() const { return(m_chanceOfReverseEngineering); }
	uint32 maxProductionLimit() const { return(m_maxProductionLimit); }

protected:
	BlueprintType(
		uint32 _id,
		// Type stuff:
		const Group &_group,
		const TypeData &_data,
		// BlueprintType stuff:
		const BlueprintType *_parentBlueprintType,
		const Type &_productType,
		const BlueprintTypeData &_bpData
	);

	/*
	 * Member functions
	 */
	static BlueprintType *_Load(ItemFactory &factory, uint32 typeID
	);
	static BlueprintType *_Load(ItemFactory &factory, uint32 typeID,
		// Type stuff:
		const Group &group, const TypeData &data
	);
	static BlueprintType *_Load(ItemFactory &factory, uint32 typeID,
		// Type stuff:
		const Group &group, const TypeData &data,
		// BlueprintType stuff:
		const BlueprintType *parentBlueprintType, const Type &productType, const BlueprintTypeData &bpData
	);

	virtual bool _Load(ItemFactory &factory) { return Type::_Load(factory); }

	/*
	 * Data members
	 */
	const BlueprintType *m_parentBlueprintType;
	const Type &m_productType;

	uint32 m_productionTime;
	uint32 m_techLevel;
	uint32 m_researchProductivityTime;
	uint32 m_researchMaterialTime;
	uint32 m_researchCopyTime;
	uint32 m_researchTechTime;
	uint32 m_productivityModifier;
	uint32 m_materialModifier;
	double m_wasteFactor;
	double m_chanceOfReverseEngineering;
	uint32 m_maxProductionLimit;
};

#endif /* __TYPE__H__INCL__ */



