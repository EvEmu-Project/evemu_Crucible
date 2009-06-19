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

#ifndef __SOLAR_SYSTEM__H__INCL__
#define __SOLAR_SYSTEM__H__INCL__

#include "../common/gpoint.h"

#include "Celestial.h"

/**
 * Data container for solarsystem data.
 */
class SolarSystemData {
public:
	SolarSystemData(
		const GPoint &_minPos = GPoint(0, 0, 0),
		const GPoint &_maxPos = GPoint(0, 0, 0),
		double _luminosity = 0.0,
		bool _border = false,
		bool _fringe = false,
		bool _corridor = false,
		bool _hub = false,
		bool _international = false,
		bool _regional = false,
		bool _constellation = false,
		double _security = 0.0,
		uint32 _factionID = 0,
		double _radius = 0.0,
		uint32 _sunTypeID = 0,
		const char *_securityClass = ""
	);

	// Data members:
	GPoint minPosition;
	GPoint maxPosition;
	double luminosity;

	// use bitfield to save some memory...
	bool border : 1;
	bool fringe : 1;
	bool corridor : 1;
	bool hub : 1;
	bool international : 1;
	bool regional : 1;
	bool constellation : 1;

	double security;
	uint32 factionID;
	double radius;
	uint32 sunTypeID;
	std::string securityClass;
};

/**
 * CelestialObject which represents solar system.
 */
class SolarSystem
: public CelestialObject
{
	friend class InventoryItem; // to let it construct us
	friend class CelestialObject; // to let it construct us
public:
	/**
	 * Loads solar system from DB.
	 *
	 * @param[in] factory
	 * @param[in] solarSystemID ID of solar system to load.
	 * @return Pointer to new solar system object; NULL if failed.
	 */
	static SolarSystem *Load(ItemFactory &factory, uint32 solarSystemID);

	/*
	 * Primary public interface:
	 */
	SolarSystem *IncRef() { return static_cast<SolarSystem *>( CelestialObject::IncRef() ); }

	/*
	 * Public Fields:
	 */
	const GPoint &      minPosition() const { return m_minPosition; }
	const GPoint &      maxPosition() const { return m_maxPosition; }
	double              luminosity() const { return m_luminosity; }

	bool                border() const { return m_border; }
	bool                fringe() const { return m_fringe; }
	bool                corridor() const { return m_corridor; }
	bool                hub() const { return m_hub; }
	bool                international() const { return m_international; }
	bool                regional() const { return m_regional; }
	bool                constellation() const { return m_constellation; }

	double              security() const { return m_security; }
	uint32              factionID() const { return m_factionID; }
	double              radius() const { return m_radius; }
	const Type &        sunType() const { return m_sunType; }
	const std::string & securityClass() const { return m_securityClass; }

protected:
	SolarSystem(
		ItemFactory &_factory,
		uint32 _solarSystemID,
		// InventoryItem stuff:
		const Type &_type,
		const ItemData &_data,
		// CelestialObject stuff:
		const CelestialObjectData &_cData,
		// SolarSystem stuff:
		const Type &_sunType,
		const SolarSystemData &_ssData
	);
	~SolarSystem();

	/*
	 * Member functions:
	 */
	// Template loader:
	template<class _Ty>
	static _Ty *_LoadCelestialObject(ItemFactory &factory, uint32 solarSystemID,
		// InventoryItem stuff:
		const Type &type, const ItemData &data,
		// CelestialObject stuff:
		const CelestialObjectData &cData)
	{
		// check it's a solar system
		if( type.groupID() != EVEDB::invGroups::Solar_System )
		{
			_log( ITEM__ERROR, "Trying to load %s %u as Solar system.", type.name().c_str(), solarSystemID );
			return NULL;
		}

		// load solar system data
		SolarSystemData ssData;
		if( !factory.db().GetSolarSystem( solarSystemID, ssData ) )
			return false;

		// get sun type
		const Type *sunType = factory.GetType( ssData.sunTypeID );
		if( sunType == NULL )
			return false;

		return _Ty::_LoadSolarSystem( factory, solarSystemID, type, data, cData, *sunType, ssData );
	}

	// Actual loading stuff:
	static SolarSystem *_Load(ItemFactory &factory, uint32 solarSystemID
	);
	static SolarSystem *_LoadItem(ItemFactory &factory, uint32 solarSystemID,
		// InventoryItem stuff:
		const Type &type, const ItemData &data
	);
	static SolarSystem *_LoadCelestialObject(ItemFactory &factory, uint32 solarSystemID,
		// InventoryItem stuff:
		const Type &type, const ItemData &data,
		// CelestialObject stuff:
		const CelestialObjectData &cData
	);
	static SolarSystem *_LoadSolarSystem(ItemFactory &factory, uint32 solarSystemID,
		// InventoryItem stuff:
		const Type &type, const ItemData &data,
		// CelestialObject stuff:
		const CelestialObjectData &cData,
		// SolarSystem stuff:
		const Type &sunType, const SolarSystemData &ssData
	);

	bool _Load();

	/*
	 * Data members:
	 */
	GPoint m_minPosition;
	GPoint m_maxPosition;
	double m_luminosity;

	// use bitfield to save some memory...
	bool m_border : 1;
	bool m_fringe : 1;
	bool m_corridor : 1;
	bool m_hub : 1;
	bool m_international : 1;
	bool m_regional : 1;
	bool m_constellation : 1;

	double m_security;
	uint32 m_factionID;
	double m_radius;
	const Type &m_sunType;
	std::string m_securityClass;
};

#endif /* !__SOLAR_SYSTEM__H__INCL__ */

