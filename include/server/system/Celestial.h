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

#ifndef __CELESTIAL__H__INCL__
#define __CELESTIAL__H__INCL__

#include "inventory/InventoryItem.h"

/**
 * Data container for celestial object.
 */
class CelestialObjectData {
public:
	CelestialObjectData(
		double _radius = 0.0,
		double _security = 0.0,
		uint8 _celestialIndex = 0,
		uint8 _orbitIndex = 0
	);

	// Data members:
	double radius;
	double security;
	uint8 celestialIndex;
	uint8 orbitIndex;
};

/**
 * InventoryItem for generic celestial object.
 */
class CelestialObject
: public InventoryItem
{
	friend class InventoryItem; // to let it construct us
public:
	/**
	 * Loads celestial object.
	 *
	 * @param[in] factory
	 * @param[in] celestialID ID of celestial object to load.
	 * @return Pointer to new CelestialObject; NULL if fails.
	 */
	static CelestialObjectRef Load(ItemFactory &factory, uint32 celestialID);

	/*
	 * Primary public interface:
	 */
	void Delete();

	/*
	 * Access methods:
	 */
	double      radius() const { return m_radius; }
	double      security() const { return m_security; }
	uint8       celestialIndex() const { return m_celestialIndex; }
	uint8       orbitIndex() const { return m_orbitIndex; }

protected:
	CelestialObject(
		ItemFactory &_factory,
		uint32 _celestialID,
		// InventoryItem stuff:
		const ItemType &_type,
		const ItemData &_data,
		// CelestialObject stuff:
		const CelestialObjectData &_cData
	);

	/*
	 * Member functions:
	 */
	using InventoryItem::_Load;

	// Template loader:
	template<class _Ty>
	static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 celestialID,
		// InventoryItem stuff:
		const ItemType &type, const ItemData &data)
	{
		// make sure it's celestial object or station
		if( type.categoryID() != EVEDB::invCategories::Celestial
			&& type.groupID() != EVEDB::invGroups::Station )
		{
			_log( ITEM__ERROR, "Trying to load %s as Celestial.", type.category().name().c_str() );
			return RefPtr<_Ty>();
		}

		// load celestial data
		CelestialObjectData cData;
		if( !factory.db().GetCelestialObject( celestialID, cData ) )
			return RefPtr<_Ty>();

		return _Ty::template _LoadCelestialObject<_Ty>( factory, celestialID, type, data, cData );
	}

	// Actual loading stuff:
	template<class _Ty>
	static RefPtr<_Ty> _LoadCelestialObject(ItemFactory &factory, uint32 celestialID,
		// InventoryItem stuff:
		const ItemType &type, const ItemData &data,
		// CelestialObject stuff:
		const CelestialObjectData &cData
	);

	/*
	 * Data members:
	 */
	double m_radius;
	double m_security;
	uint8 m_celestialIndex;
	uint8 m_orbitIndex;
};

#endif /* !__CELESTIAL__H__INCL__ */


