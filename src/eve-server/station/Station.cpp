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

#include "EVEServerPCH.h"

/*
 * StationTypeData
 */
StationTypeData::StationTypeData(
	uint32 _dockingBayGraphicID,
	uint32 _hangarGraphicID,
	const GPoint &_dockEntry,
	const GVector &_dockOrientation,
	uint32 _operationID,
	uint32 _officeSlots,
	double _reprocessingEfficiency,
	bool _conquerable)
: dockingBayGraphicID(_dockingBayGraphicID),
  hangarGraphicID(_hangarGraphicID),
  dockEntry(_dockEntry),
  dockOrientation(_dockOrientation),
  operationID(_operationID),
  officeSlots(_officeSlots),
  reprocessingEfficiency(_reprocessingEfficiency),
  conquerable(_conquerable)
{
}

/*
 * StationType
 */
StationType::StationType(
	uint32 _id,
	// ItemType stuff:
	const ItemGroup &_group,
	const TypeData &_data,
	// StationType stuff:
	const StationTypeData &_stData)
: ItemType(_id, _group, _data),
  m_dockingBayGraphicID(_stData.dockingBayGraphicID),
  m_hangarGraphicID(_stData.hangarGraphicID),
  m_dockEntry(_stData.dockEntry),
  m_dockOrientation(_stData.dockOrientation),
  m_operationID(_stData.operationID),
  m_officeSlots(_stData.officeSlots),
  m_reprocessingEfficiency(_stData.reprocessingEfficiency),
  m_conquerable(_stData.conquerable)
{
	// consistency check
	assert(_data.groupID == EVEDB::invGroups::Station);
}

StationType *StationType::Load(ItemFactory &factory, uint32 stationTypeID)
{
	return ItemType::Load<StationType>( factory, stationTypeID );
}

template<class _Ty>
_Ty *StationType::_LoadStationType(ItemFactory &factory, uint32 stationTypeID,
	// ItemType stuff:
	const ItemGroup &group, const TypeData &data,
	// StationType stuff:
	const StationTypeData &stData)
{
	// ready to create
	return new StationType( stationTypeID, group, data, stData );
}

/*
 * StationData
 */
StationData::StationData(
	uint32 _security,
	double _dockingCostPerVolume,
	double _maxShipVolumeDockable,
	uint32 _officeRentalCost,
	uint32 _operationID,
	double _reprocessingEfficiency,
	double _reprocessingStationsTake,
	EVEItemFlags _reprocessingHangarFlag)
: security(_security),
  dockingCostPerVolume(_dockingCostPerVolume),
  maxShipVolumeDockable(_maxShipVolumeDockable),
  officeRentalCost(_officeRentalCost),
  operationID(_operationID),
  reprocessingEfficiency(_reprocessingEfficiency),
  reprocessingStationsTake(_reprocessingStationsTake),
  reprocessingHangarFlag(_reprocessingHangarFlag)
{
}

/*
 * Station
 */
Station::Station(
	ItemFactory &_factory,
	uint32 _stationID,
	// InventoryItem stuff:
	const StationType &_type,
	const ItemData &_data,
	// CelestialObject stuff:
	const CelestialObjectData &_cData,
	// Station stuff:
	const StationData &_stData)
: CelestialObject(_factory, _stationID, _type, _data, _cData),
  m_security(_stData.security),
  m_dockingCostPerVolume(_stData.dockingCostPerVolume),
  m_maxShipVolumeDockable(_stData.maxShipVolumeDockable),
  m_officeRentalCost(_stData.officeRentalCost),
  m_operationID(_stData.operationID),
  m_reprocessingEfficiency(_stData.reprocessingEfficiency),
  m_reprocessingStationsTake(_stData.reprocessingStationsTake),
  m_reprocessingHangarFlag(_stData.reprocessingHangarFlag)
{
}

StationRef Station::Load(ItemFactory &factory, uint32 stationID)
{
	return InventoryItem::Load<Station>( factory, stationID );
}

template<class _Ty>
RefPtr<_Ty> Station::_LoadStation(ItemFactory &factory, uint32 stationID,
	// InventoryItem stuff:
	const StationType &type, const ItemData &data,
	// CelestialObject stuff:
	const CelestialObjectData &cData,
	// Station stuff:
	const StationData &stData)
{
	// ready to create
	return StationRef( new Station( factory, stationID, type, data, cData, stData ) );
}

bool Station::_Load()
{
	// load contents
	if( !LoadContents( m_factory ) )
		return false;

	return CelestialObject::_Load();
}


