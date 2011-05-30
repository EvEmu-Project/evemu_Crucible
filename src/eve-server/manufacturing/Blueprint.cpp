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

#include "EVEServerPCH.h"

/*
 * BlueprintTypeData
 */
BlueprintTypeData::BlueprintTypeData(
	uint32 _parentBlueprintTypeID,
	uint32 _productTypeID,
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
: parentBlueprintTypeID(_parentBlueprintTypeID),
  productTypeID(_productTypeID),
  productionTime(_productionTime),
  techLevel(_techLevel),
  researchProductivityTime(_researchProductivityTime),
  researchMaterialTime(_researchMaterialTime),
  researchCopyTime(_researchCopyTime),
  researchTechTime(_researchTechTime),
  productivityModifier(_productivityModifier),
  materialModifier(_materialModifier),
  wasteFactor(_wasteFactor),
  chanceOfReverseEngineering(_chanceOfReverseEngineering),
  maxProductionLimit(_maxProductionLimit)
{
}

/*
 * BlueprintType
 */
BlueprintType::BlueprintType(
	uint32 _id,
	const ItemGroup &_group,
	const TypeData &_data,
	const BlueprintType *_parentBlueprintType,
	const ItemType &_productType,
	const BlueprintTypeData &_bpData)
: ItemType(_id, _group, _data),
  m_parentBlueprintType(_parentBlueprintType),
  m_productType(_productType),
  m_productionTime(_bpData.productionTime),
  m_techLevel(_bpData.techLevel),
  m_researchProductivityTime(_bpData.researchProductivityTime),
  m_researchMaterialTime(_bpData.researchMaterialTime),
  m_researchCopyTime(_bpData.researchCopyTime),
  m_researchTechTime(_bpData.researchTechTime),
  m_productivityModifier(_bpData.productivityModifier),
  m_wasteFactor(_bpData.wasteFactor),
  m_chanceOfReverseEngineering(_bpData.chanceOfReverseEngineering),
  m_maxProductionLimit(_bpData.maxProductionLimit)
{
	// asserts for data consistency
	assert(_bpData.productTypeID == _productType.id());
	if(_parentBlueprintType != NULL)
		assert(_bpData.parentBlueprintTypeID == _parentBlueprintType->id());
}

BlueprintType *BlueprintType::Load(ItemFactory &factory, uint32 typeID)
{
	return ItemType::Load<BlueprintType>( factory, typeID );
}

template<class _Ty>
_Ty *BlueprintType::_LoadBlueprintType(ItemFactory &factory, uint32 typeID,
	// ItemType stuff:
	const ItemGroup &group, const TypeData &data,
	// BlueprintType stuff:
	const BlueprintType *parentBlueprintType, const ItemType &productType, const BlueprintTypeData &bpData)
{
	return new BlueprintType(typeID, group, data, parentBlueprintType, productType, bpData );
}

/*
 * BlueprintData
 */
BlueprintData::BlueprintData(
	bool _copy,
	uint32 _materialLevel,
	uint32 _productivityLevel,
	int32 _licensedProductionRunsRemaining)
: copy(_copy),
  materialLevel(_materialLevel),
  productivityLevel(_productivityLevel),
  licensedProductionRunsRemaining(_licensedProductionRunsRemaining)
{
}

/*
 * Blueprint
 */
Blueprint::Blueprint(
	ItemFactory &_factory,
	uint32 _blueprintID,
	// InventoryItem stuff:
	const BlueprintType &_bpType,
	const ItemData &_data,
	// Blueprint stuff:
	const BlueprintData &_bpData)
: InventoryItem(_factory, _blueprintID, _bpType, _data),
  m_copy(_bpData.copy),
  m_materialLevel(_bpData.materialLevel),
  m_productivityLevel(_bpData.productivityLevel),
  m_licensedProductionRunsRemaining(_bpData.licensedProductionRunsRemaining)
{
	// data consistency asserts
	assert(_bpType.categoryID() == EVEDB::invCategories::Blueprint);
}

BlueprintRef Blueprint::Load(ItemFactory &factory, uint32 blueprintID)
{
	return InventoryItem::Load<Blueprint>( factory, blueprintID );
}

template<class _Ty>
RefPtr<_Ty> Blueprint::_LoadBlueprint(ItemFactory &factory, uint32 blueprintID,
	// InventoryItem stuff:
	const BlueprintType &bpType, const ItemData &data,
	// Blueprint stuff:
	const BlueprintData &bpData)
{
	// we have enough data, construct the item
	return BlueprintRef( new Blueprint( factory, blueprintID, bpType, data, bpData ) );
}

BlueprintRef Blueprint::Spawn(ItemFactory &factory, ItemData &data, BlueprintData &bpData) {
	uint32 blueprintID = Blueprint::_Spawn(factory, data, bpData);
	if(blueprintID == 0)
		return BlueprintRef();
	return Blueprint::Load(factory, blueprintID);
}

uint32 Blueprint::_Spawn(ItemFactory &factory,
	// InventoryItem stuff:
	ItemData &data,
	// Blueprint stuff:
	BlueprintData &bpData
) {
	// make sure it's a blueprint type
	const BlueprintType *bt = factory.GetBlueprintType(data.typeID);
	if(bt == NULL)
		return 0;

	// get the blueprintID
	uint32 blueprintID = InventoryItem::_Spawn(factory, data);
	if(blueprintID == 0)
		return 0;

	// insert blueprint entry into DB
	if(!factory.db().NewBlueprint(blueprintID, bpData)) {
		// delete item
		factory.db().DeleteItem(blueprintID);

		return 0;
	}

	return blueprintID;
}

void Blueprint::Delete() {
	// delete our blueprint record
	m_factory.db().DeleteBlueprint(m_itemID);
	// redirect to parent
	InventoryItem::Delete();
}

BlueprintRef Blueprint::SplitBlueprint(int32 qty_to_take, bool notify) {
	// split item
	BlueprintRef res = BlueprintRef::StaticCast( InventoryItem::Split( qty_to_take, notify ) );
	if( !res )
		return BlueprintRef();

	// copy our attributes
	res->SetCopy(m_copy);
	res->SetMaterialLevel(m_materialLevel);
	res->SetProductivityLevel(m_productivityLevel);
	res->SetLicensedProductionRunsRemaining(m_licensedProductionRunsRemaining);

	return res;
}

bool Blueprint::Merge(InventoryItemRef to_merge, int32 qty, bool notify) {
	if( !InventoryItem::Merge( to_merge, qty, notify ) )
		return false;
	// do something special? merge material level etc.?
	return true;
}

void Blueprint::SetCopy(bool copy) {
	m_copy = copy;

	SaveBlueprint();
}

void Blueprint::SetMaterialLevel(uint32 materialLevel) {
	m_materialLevel = materialLevel;

	SaveBlueprint();
}

bool Blueprint::AlterMaterialLevel(int32 materialLevelChange) {
	int32 new_material_level = m_materialLevel + materialLevelChange;

	if(new_material_level < 0) {
		_log(ITEM__ERROR, "%s (%u): Tried to remove %u material levels while having %u levels.", m_itemName.c_str(), m_itemID, -materialLevelChange, m_materialLevel);
		return false;
	}

	SetMaterialLevel(new_material_level);
	return true;
}

void Blueprint::SetProductivityLevel(uint32 productivityLevel) {
	m_productivityLevel = productivityLevel;

	SaveBlueprint();
}

bool Blueprint::AlterProductivityLevel(int32 producitvityLevelChange) {
	int32 new_productivity_level = m_productivityLevel + producitvityLevelChange;

	if(new_productivity_level < 0) {
		_log(ITEM__ERROR, "%s (%u): Tried to remove %u productivity levels while having %u levels.", m_itemName.c_str(), m_itemID, -producitvityLevelChange, m_productivityLevel);
		return false;
	}

	SetProductivityLevel(new_productivity_level);
	return true;
}

void Blueprint::SetLicensedProductionRunsRemaining(int32 licensedProductionRunsRemaining) {
	m_licensedProductionRunsRemaining = licensedProductionRunsRemaining;

	SaveBlueprint();
}

void Blueprint::AlterLicensedProductionRunsRemaining(int32 licensedProductionRunsRemainingChange) {
	int32 new_licensed_production_runs_remaining = m_licensedProductionRunsRemaining + licensedProductionRunsRemainingChange;

	SetLicensedProductionRunsRemaining(new_licensed_production_runs_remaining);
}

PyDict *Blueprint::GetBlueprintAttributes() const {
	Rsp_GetBlueprintAttributes rsp;

	// fill in our attribute info
	rsp.blueprintID = itemID();
	rsp.copy = copy() ? 1 : 0;
	rsp.productivityLevel = productivityLevel();
	rsp.materialLevel = materialLevel();
	rsp.licensedProductionRunsRemaining = licensedProductionRunsRemaining();
	rsp.wastageFactor = wasteFactor();

	rsp.productTypeID = productTypeID();
	rsp.manufacturingTime = type().productionTime();
	rsp.maxProductionLimit = type().maxProductionLimit();
	rsp.researchMaterialTime = type().researchMaterialTime();
	rsp.researchTechTime = type().researchTechTime();
	rsp.researchProductivityTime = type().researchProductivityTime();
	rsp.researchCopyTime = type().researchCopyTime();

	return(rsp.Encode());
}

void Blueprint::SaveBlueprint() const
{
	_log( ITEM__TRACE, "Saving blueprint %u.", itemID() );

	m_factory.db().SaveBlueprint(
		itemID(),
		BlueprintData(
			copy(),
			materialLevel(),
			productivityLevel(),
			licensedProductionRunsRemaining()
		)
	);
}


