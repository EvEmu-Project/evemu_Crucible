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

#include "EvemuPCH.h"

/*
 * CelestialObjectData
 */
CelestialObjectData::CelestialObjectData(
	double _radius,
	double _security,
	uint8 _celestialIndex,
	uint8 _orbitIndex)
: radius(_radius),
  security(_security),
  celestialIndex(_celestialIndex),
  orbitIndex(_orbitIndex)
{
}

/*
 * CelestialObject
 */
CelestialObject::CelestialObject(
	ItemFactory &_factory,
	uint32 _celestialID,
	// InventoryItem stuff:
	const Type &_type,
	const ItemData &_data,
	// CelestialObject stuff:
	const CelestialObjectData &_cData)
: InventoryItem(_factory, _celestialID, _type, _data),
  m_security(_cData.security),
  m_radius(_cData.radius),
  m_celestialIndex(_cData.celestialIndex),
  m_orbitIndex(_cData.orbitIndex)
{
}

CelestialObject *CelestialObject::Load(ItemFactory &factory, uint32 celestialID, bool recurse) {
	return InventoryItem::Load<CelestialObject>(factory, celestialID, recurse);
}

CelestialObject *CelestialObject::_Load(ItemFactory &factory, uint32 celestialID
) {
	return InventoryItem::_Load<CelestialObject>(
		factory, celestialID
	);
}

CelestialObject *CelestialObject::_Load(ItemFactory &factory, uint32 celestialID,
	// InventoryItem stuff:
	const Type &type, const ItemData &data
) {
	return CelestialObject::_Load<CelestialObject>(
		factory, celestialID, type, data
	);
}

CelestialObject *CelestialObject::_Load(ItemFactory &factory, uint32 celestialID,
	// InventoryItem stuff:
	const Type &type, const ItemData &data,
	// CelestialObject stuff:
	const CelestialObjectData &cData
) {
	// Our category is celestial; what to do next:
	switch(type.groupID()) {
		///////////////////////////////////////
		// Solar system:
		///////////////////////////////////////
		case EVEDB::invGroups::Solar_System: {
			// create solar system
			return(SolarSystem::_Load(
				factory, celestialID, type, data
			));
		};

		///////////////////////////////////////
		// Station:
		///////////////////////////////////////
		case EVEDB::invGroups::Station: {
			// cast the type into what it really is ...
			const StationType &stationType = static_cast<const StationType &>(type);

			return(Station::_Load(
				factory, celestialID, stationType, data
			));
		};

		///////////////////////////////////////
		// Default:
		///////////////////////////////////////
		default: {
			// let's create a generic one
			return(new CelestialObject(
				factory, celestialID,
				type, data,
				cData
			));
		};
	}
}

void CelestialObject::Save(bool recursive, bool saveAttributes) const {
	// For now disable any saving of Celestial objects.
}

void CelestialObject::Delete() {
	// We're definitely not going to remove Celetial object.
	// Just consume ref as supposed to.
	DecRef();
}



