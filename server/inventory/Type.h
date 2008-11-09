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

#ifndef __TYPE__H__INCL__
#define __TYPE__H__INCL__

#include <string>

#include "../common/packet_types.h"

#include "EVEAttributeMgr.h"
#include "ItemFactory.h"

/*
 * class which contain category data
 */
class Category {
public:
	Category(
		EVEItemCategories _id,
		const char *_name,
		const char *_description,
		bool _published);

	/*
	 * Factory method:
	 */
	static Category *LoadCategory(ItemFactory &factory, EVEItemCategories category);

	/*
	 * Members
	 */
	const EVEItemCategories id;

	std::string name;
	std::string description;
	bool published;
};

/*
 * class which contain group data
 */
class Group {
public:
	Group(
		uint32 _id,
		const Category *_category,
		const char *_name,
		const char *_description,
		bool _useBasePrice,
		bool _allowManufacture,
		bool _allowRecycler,
		bool _anchored,
		bool _anchorable,
		bool _fittableNonSingleton,
		bool _published);

	/*
	 * Factory method:
	 */
	static Group *LoadGroup(ItemFactory &factory, uint32 groupID);

	/*
	 * Helper methods
	 */
	EVEItemCategories categoryID() const { return(category->id); }

	/*
	 * Members
	 */
	const uint32 id;

	const Category *category;
	std::string name;
	std::string description;
	bool useBasePrice;
	bool allowManufacture;
	bool allowRecycler;
	bool anchored;
	bool anchorable;
	bool fittableNonSingleton;
	bool published;
};

/*
 * class which contain type data
 */
class Type {
public:
	Type(
		uint32 _id,
		const Group *_group,
		const char *_name,
		const char *_description,
		uint32 _portionSize,
		double _basePrice,
		bool _published,
		uint32 _marketGroupID,
		double _chanceOfDuplicating);

	/*
	 * Factory method:
	 */
	static Type *LoadType(ItemFactory &factory, uint32 typeID);

	/*
	 * Helper methods
	 */
	uint32 groupID() const { return(group->id); }
	const Category *category() const { return(group->category); }
	EVEItemCategories categoryID() const { return(category()->id); }

	double radius() const { return(attributes.radius()); }
	double mass() const { return(attributes.mass()); }
	double volume() const { return(attributes.volume()); }
	double capacity() const { return(attributes.capacity()); }
	EVERace race() const { return(EVERace(attributes.raceID())); }

	/*
	 * Members
	 */
	const uint32 id;

	TypeAttributeMgr attributes;

	const Group *group;
	std::string name;
	std::string description;
	uint32 portionSize;
	double basePrice;
	bool published;
	uint32 marketGroupID;
	double chanceOfDuplicating;
};

class BlueprintType
: public Type
{
public:
	BlueprintType(
		uint32 _id,
		const Group *_group,
		const char *_name,
		const char *_description,
		uint32 _portionSize,
		double _basePrice,
		bool _published,
		uint32 _marketGroupID,
		double _chanceOfDuplicating,
		const Type *_parentBlueprintType,
		const Type *_productType,
		uint32 _productionTime,
		uint32 _techLevel,
		uint32 _researchProductivityTime,
		uint32 _researchMaterialTime,
		uint32 _researchCopyTime,
		uint32 _researchTechTime,
		uint32 _productivityModifier,
		uint32 _materialModifier,
		double _wasteFactor,
		double _chanceOfReverseEngineering,
		uint32 _maxProductionLimit);

	/*
	 * Members
	 */
	const Type *parentBlueprintType;
	const Type *productType;
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

#endif /* __TYPE__H__INCL__ */



