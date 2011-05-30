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
 * SolarSystemData
 */
SolarSystemData::SolarSystemData(
	const GPoint &_minPos,
	const GPoint &_maxPos,
	double _luminosity,
	bool _border,
	bool _fringe,
	bool _corridor,
	bool _hub,
	bool _international,
	bool _regional,
	bool _constellation,
	double _security,
	uint32 _factionID,
	double _radius,
	uint32 _sunTypeID,
	const char *_securityClass)
: minPosition(_minPos),
  maxPosition(_maxPos),
  luminosity(_luminosity),
  border(_border),
  fringe(_fringe),
  corridor(_corridor),
  hub(_hub),
  international(_international),
  regional(_regional),
  constellation(_constellation),
  security(_security),
  factionID(_factionID),
  radius(_radius),
  sunTypeID(_sunTypeID),
  securityClass(_securityClass)
{
}

/*
 * SolarSystem
 */
SolarSystem::SolarSystem(
	ItemFactory &_factory,
	uint32 _solarSystemID,
	// InventoryItem stuff:
	const ItemType &_type,
	const ItemData &_data,
	// CelestialObject stuff:
	const CelestialObjectData &_cData,
	// SolarSystem stuff:
	const ItemType &_sunType,
	const SolarSystemData &_ssData)
: CelestialObject(_factory, _solarSystemID, _type, _data, _cData),
  m_minPosition(_ssData.minPosition),
  m_maxPosition(_ssData.maxPosition),
  m_luminosity(_ssData.luminosity),
  m_border(_ssData.border),
  m_fringe(_ssData.fringe),
  m_corridor(_ssData.corridor),
  m_hub(_ssData.hub),
  m_international(_ssData.international),
  m_regional(_ssData.regional),
  m_constellation(_ssData.constellation),
  m_security(_ssData.security),
  m_factionID(_ssData.factionID),
  m_radius(_ssData.radius),
  m_sunType(_sunType),
  m_securityClass(_ssData.securityClass)
{
	// consistency check
	assert(_sunType.id() == _ssData.sunTypeID);
}

SolarSystem::~SolarSystem() {
}

SolarSystemRef SolarSystem::Load(ItemFactory &factory, uint32 solarSystemID)
{
	return InventoryItem::Load<SolarSystem>( factory, solarSystemID );
}

template<class _Ty>
RefPtr<_Ty> SolarSystem::_LoadSolarSystem(ItemFactory &factory, uint32 solarSystemID,
	// InventoryItem stuff:
	const ItemType &type, const ItemData &data,
	// CelestialObject stuff:
	const CelestialObjectData &cData,
	// SolarSystem stuff:
	const ItemType &sunType, const SolarSystemData &ssData)
{
	// we have it all
	return SolarSystemRef( new SolarSystem( factory, solarSystemID, type, data, cData, sunType, ssData ) );
}

bool SolarSystem::_Load()
{
	return CelestialObject::_Load();
}

void SolarSystem::AddItemToInventory(InventoryItemRef item)
{
    AddItem( item );
}

void SolarSystem::AddItem(InventoryItemRef item)
{
    Inventory::AddItem( item );
}

void SolarSystem::RemoveItemFromInventory(InventoryItemRef item)
{
    RemoveItem( item );
}

void SolarSystem::RemoveItem(InventoryItemRef item)
{
    Inventory::RemoveItem( item );
}
