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

#ifndef __BLUEPRINT_ITEM__H__INCL__
#define __BLUEPRINT_ITEM__H__INCL__

#include "inventory/Type.h"
#include "inventory/InventoryItem.h"

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
	/**
	 * Loads blueprint type from DB.
	 *
	 * @param[in] factory
	 * @param[in] typeID ID of blueprint type to load.
	 * @return Pointer to BlueprintType object; NULL if failed.
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
	// Template loader:
	template<class _Ty>
	static _Ty *_Load(ItemFactory &factory, uint32 typeID,
		// Type stuff:
		const Group &group, const TypeData &data
	) {
		// check if we are really loading a blueprint
		if(group.categoryID() != EVEDB::invCategories::Blueprint) {
			_log(ITEM__ERROR, "Load of blueprint type %u requested, but it's %s.", typeID, group.category().name().c_str());
			return NULL;
		}

		// pull additional blueprint data
		BlueprintTypeData bpData;
		if(!factory.db().GetBlueprintType(typeID, bpData))
			return NULL;

		// obtain parent blueprint type (might be NULL)
		const BlueprintType *parentBlueprintType = NULL;
		if(bpData.parentBlueprintTypeID != 0) {
			// we have parent type, get it
			parentBlueprintType = factory.GetBlueprintType(bpData.parentBlueprintTypeID);
			if(parentBlueprintType == NULL)
				return NULL;
		}

		// obtain product type
		const Type *productType = factory.GetType(bpData.productTypeID);
		if(productType == NULL)
			return NULL;

		// create blueprint type
		return(
			_Ty::_Load(factory, typeID, group, data, parentBlueprintType, *productType, bpData)
		);
	}

	// Actual loading stuff:
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
class Blueprint
: public InventoryItem
{
	friend class InventoryItem;	// to let it construct us
public:
	/**
	 * Loads blueprint from DB.
	 *
	 * @param[in] factory
	 * @param[in] blueprintID ID of blueprint to load.
	 * @param[in] recurse Whether all items contained within this item should be loaded.
	 * @return Pointer to new Blueprint object; NULL if failed.
	 */
	static Blueprint *Load(ItemFactory &factory, uint32 blueprintID, bool recurse=false);
	/**
	 * Spawns new blueprint.
	 *
	 * @param[in] factory
	 * @param[in] data Item data (for entity table).
	 * @param[in] bpData Blueprint-specific data.
	 * @return Pointer to new Blueprint object; NULL if failed.
	 */
	static Blueprint *Spawn(ItemFactory &factory, ItemData &data, BlueprintData &bpData);

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
	Blueprint *IncRef() { return static_cast<Blueprint *>(InventoryItem::IncRef()); }

	void Save(bool recursive=false, bool saveAttributes=true) const;
	void Delete();

	// Copy:
	void SetCopy(bool copy);

	// Material level:
	void SetMaterialLevel(uint32 materialLevel);
	bool AlterMaterialLevel(int32 materialLevelChange);

	// Productivity level:
	void SetProductivityLevel(uint32 productivityLevel);
	bool AlterProductivityLevel(int32 producitvityLevelChange);

	// Licensed production runs:
	void SetLicensedProductionRunsRemaining(int32 licensedProductionRunsRemaining);
	void AlterLicensedProductionRunsRemaining(int32 licensedProductionRunsRemainingChange);

	/*
	 * Helper routines:
	 */
	// overload to split the blueprints properly
	InventoryItem *Split(int32 qty_to_take, bool notify=true) { return(SplitBlueprint(qty_to_take, notify)); }
	Blueprint *SplitBlueprint(int32 qty_to_take, bool notify=true);

	// overload to do proper merge
	bool Merge(InventoryItem *to_merge, int32 qty=0, bool notify=true);	//consumes ref!

	// some blueprint-related stuff
	bool infinite() const                   { return(licensedProductionRunsRemaining() < 0); }
	double wasteFactor() const              { return(type().wasteFactor() / (1 + materialLevel())); }

	double materialMultiplier() const       { return(1.0 + wasteFactor()); }
	double timeMultiplier() const           { return(1.0 - (timeSaved() / type().productionTime())); }
	double timeSaved() const                { return((1.0 - (1.0 / (1 + productivityLevel()))) * type().productivityModifier()); }

	/*
	 * Primary public packet builders:
	 */
	PyRepDict *GetBlueprintAttributes() const;

protected:
	Blueprint(
		ItemFactory &_factory,
		uint32 _blueprintID,
		// InventoryItem stuff:
		const BlueprintType &_bpType,
		const ItemData &_data,
		// Blueprint stuff:
		const BlueprintData &_bpData
	);

	/*
	 * Member functions
	 */
	// Template loader:
	template<class _Ty>
	static _Ty *_Load(ItemFactory &factory, uint32 blueprintID,
		// InventoryItem stuff:
		const Type &type, const ItemData &data
	) {
		// check it's blueprint type
		if(type.categoryID() != EVEDB::invCategories::Blueprint) {
			_log(ITEM__ERROR, "Trying to load %s as Blueprint.", type.category().name().c_str());
			return NULL;
		}
		// cast the type
		const BlueprintType &bpType = static_cast<const BlueprintType &>(type);

		// we are blueprint; pull additional blueprint info
		BlueprintData bpData;
		if(!factory.db().GetBlueprint(blueprintID, bpData))
			return NULL;

		return(
			_Ty::_Load(factory, blueprintID, bpType, data, bpData)
		);
	}

	// Actual loading stuff:
	static Blueprint *_Load(ItemFactory &factory, uint32 blueprintID
	);
	static Blueprint *_Load(ItemFactory &factory, uint32 blueprintID,
		// InventoryItem stuff:
		const Type &type, const ItemData &data
	);
	static Blueprint *_Load(ItemFactory &factory, uint32 blueprintID,
		// InventoryItem stuff:
		const BlueprintType &bpType, const ItemData &data,
		// Blueprint stuff:
		const BlueprintData &bpData
	);
	virtual bool _Load(bool recurse=false) { return InventoryItem::_Load(recurse); }

	static uint32 _Spawn(ItemFactory &factory,
		// InventoryItem stuff:
		ItemData &data,
		// Blueprint stuff:
		BlueprintData &bpData
	);


	/*
	 * Member variables
	 */
	bool      m_copy;
	uint32    m_materialLevel;
	uint32    m_productivityLevel;
	int32     m_licensedProductionRunsRemaining;
};

#endif /* !__BLUEPRINT_ITEM__H__INCL__ */

