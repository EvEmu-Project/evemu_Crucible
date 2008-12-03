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

/*
 * Category
 */
Category::Category(
	EVEItemCategories _id,
	const char *_name,
	const char *_description,
	bool _published)
: id(_id),
  name(_name),
  description(_description),
  published(_published)
{
}

Category *Category::LoadCategory(ItemFactory &factory, EVEItemCategories category) {
	std::string name, description;
	bool published;

	// pull data
	if(!factory.db().GetCategory(category,
		name, description, published))
	{
		return NULL;
	}

	// create category
	Category *c = new Category(
		category,
		name.c_str(),
		description.c_str(),
		published
	);

	// return
	return(c);
}

/*
 * Group
 */
Group::Group(
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
	bool _published)
: id(_id),
  category(_category),
  name(_name),
  description(_description),
  useBasePrice(_useBasePrice),
  allowManufacture(_allowManufacture),
  allowRecycler(_allowRecycler),
  anchored(_anchored),
  anchorable(_anchorable),
  fittableNonSingleton(_fittableNonSingleton),
  published(_published)
{
}

Group *Group::LoadGroup(ItemFactory &factory, uint32 groupID) {
	// pull data
	EVEItemCategories category;
	std::string name, description;
	bool useBasePrice, allowManufacture, allowRecycler, anchored, anchorable, fittableNonSingleton, published;

	if(!factory.db().GetGroup(groupID, category,
		name, description, useBasePrice, allowManufacture, allowRecycler, anchored, anchorable, fittableNonSingleton, published))
	{
		return NULL;
	}

	// retrieve category
	const Category *c = factory.category(category);
	if(c == NULL)
		return NULL;

	// create group
	Group *g = new Group(
		groupID,
		c,
		name.c_str(),
		description.c_str(),
		useBasePrice,
		allowManufacture,
		allowRecycler,
		anchored,
		anchorable,
		fittableNonSingleton,
		published
	);

	// return
	return(g);
}

/*
 * Type
 */
Type::Type(
	uint32 _id,
	const Group *_group,
	const char *_name,
	const char *_description,
	uint32 _portionSize,
	double _basePrice,
	bool _published,
	uint32 _marketGroupID,
	double _chanceOfDuplicating)
: id(_id),
  attributes(*this),
  group(_group),
  name(_name),
  description(_description),
  portionSize(_portionSize),
  basePrice(_basePrice),
  published(_published),
  marketGroupID(_marketGroupID),
  chanceOfDuplicating(_chanceOfDuplicating)
{
}

Type *Type::LoadType(ItemFactory &factory, uint32 typeID) {
	// This is a bit messy ...

	// pull data
	std::string name, description;
	uint32 groupID, portionSize, marketGroupID;
	double radius, mass, volume, capacity, basePrice, chanceOfDuplicating;
	EVERace race;
	bool published;

	if(!factory.db().GetType(typeID, groupID,
		name, description, radius, mass, volume, capacity, portionSize, race, basePrice, published, marketGroupID,
		chanceOfDuplicating))
	{
		return NULL;
	}

	// obtain group
	const Group *g = factory.group(groupID);
	if(g == NULL)
		return NULL;

	// create type
	Type *t;
	if(g->categoryID() == EVEDB::invCategories::Blueprint) {
		// we are blueprint

		// pull additional blueprint data
		uint32 parentBlueprintTypeID, productTypeID, productionTime, techLevel, researchProductivityTime, researchMaterialTime,
				researchCopyTime, researchTechTime, productivityModifier, materialModifier, maxProductionLimit;
		double wasteFactor, chanceOfReverseEngineering;

		if(!factory.db().GetBlueprintType(typeID, parentBlueprintTypeID, productTypeID,
			productionTime, techLevel, researchProductivityTime, researchMaterialTime, researchCopyTime, researchTechTime,
			productivityModifier, materialModifier, wasteFactor, chanceOfReverseEngineering, maxProductionLimit))
		{
			return NULL;
		}

		// obtain parent blueprint type
		const Type *parentBlueprintType;
		if(parentBlueprintTypeID != 0) {
			// we have parent type, get it
			parentBlueprintType = factory.type(parentBlueprintTypeID);
			if(parentBlueprintType == NULL)
				return NULL;
		} else
			// we have no parent type, set to NULL
			parentBlueprintType = NULL;

		// obtain product type
		const Type *productType;
		productType = factory.type(productTypeID);
		if(productType == NULL)
			return NULL;

		// create blueprint type
		t = new BlueprintType(
			typeID,
			g,
			name.c_str(),
			description.c_str(),
			portionSize,
			basePrice,
			published,
			marketGroupID,
			chanceOfDuplicating,
			parentBlueprintType,
			productType,
			productionTime,
			techLevel,
			researchProductivityTime,
			researchMaterialTime,
			researchCopyTime,
			researchTechTime,
			productivityModifier,
			materialModifier,
			wasteFactor,
			chanceOfReverseEngineering,
			maxProductionLimit
		);
	} else {
		// we are generic type

		// create type
		t = new Type(
			typeID,
			g,
			name.c_str(),
			description.c_str(),
			portionSize,
			basePrice,
			published,
			marketGroupID,
			chanceOfDuplicating
		);
	}

	if(!t->attributes.Load(factory.db())) {
		delete t;
		return NULL;
	}

	t->attributes.Set_radius(radius);
	t->attributes.Set_mass(mass);
	t->attributes.Set_volume(volume);
	t->attributes.Set_capacity(capacity);
	t->attributes.Set_raceID(race);

	return(t);
}

/*
 * BlueprintType
 */
BlueprintType::BlueprintType(
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
	uint32 _maxProductionLimit)
: Type(_id, _group, _name, _description, _portionSize, _basePrice, _published, _marketGroupID, _chanceOfDuplicating),
  parentBlueprintType(_parentBlueprintType),
  productType(_productType),
  productionTime(_productionTime),
  techLevel(_techLevel),
  researchProductivityTime(_researchProductivityTime),
  researchMaterialTime(_researchMaterialTime),
  researchCopyTime(_researchCopyTime),
  researchTechTime(_researchTechTime),
  productivityModifier(_productivityModifier),
  wasteFactor(_wasteFactor),
  chanceOfReverseEngineering(_chanceOfReverseEngineering),
  maxProductionLimit(_maxProductionLimit)
{
}



