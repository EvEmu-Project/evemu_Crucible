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

#ifndef __STATION__H__INCL__
#define __STATION__H__INCL__

#include "inventory/Type.h"

/**
 * Station type data container.
 */
class StationTypeData {
public:
	StationTypeData(
		uint32 _dockingBayGraphicID = 0,
		uint32 _hangarGraphicID = 0,
		const GPoint &_dockEntry = GPoint(0, 0, 0),
		const GVector &_dockOrientation = GVector(0, 0, 0),
		uint32 _operationID = 0,
		uint32 _officeSlots = 0,
		double _reprocessingEfficiency = 0.0,
		bool _conquerable = false
	);

	// Data members:
	uint32 dockingBayGraphicID;
	uint32 hangarGraphicID;

	GPoint dockEntry;
	GVector dockOrientation;

	uint32 operationID;
	uint32 officeSlots;
	double reprocessingEfficiency;
	bool conquerable;
};

/**
 * Type of station.
 */
class StationType
: public Type
{
	friend class Type; // to let it construct us
public:
	/**
	 * Loads station type.
	 *
	 * @param[in] factory
	 * @param[in] stationTypeID ID of station type to load.
	 * @return Pointer to new StationType object; NULL if failed.
	 */
	static StationType *Load(ItemFactory &factory, uint32 stationTypeID) { return Type::_Load<StationType>(factory, stationTypeID); }

	/*
	 * Access methods:
	 */
	uint32      dockingBayGraphicID() const { return m_dockingBayGraphicID; }
	uint32      hangarGraphicID() const { return m_hangarGraphicID; }

	GPoint      dockEntry() const { return m_dockEntry; }
	GVector     dockOrientation() const { return m_dockOrientation; }

	uint32      operationID() const { return m_operationID; }
	uint32      officeSlots() const { return m_officeSlots; }
	double      reprocessingEfficiency() const { return m_reprocessingEfficiency; }
	bool        conquerable() const { return m_conquerable; }

protected:
	StationType(
		uint32 _id,
		// Type stuff:
		const Group &_group,
		const TypeData &_data,
		// StationType stuff:
		const StationTypeData &_stData
	);

	/*
	 * Member functions:
	 */
	static StationType *_Load(ItemFactory &factory, uint32 stationTypeID
	);
	static StationType *_Load(ItemFactory &factory, uint32 stationTypeID,
		// Type stuff:
		const Group &group, const TypeData &data
	);
	static StationType *_Load(ItemFactory &factory, uint32 stationTypeID,
		// Type stuff:
		const Group &group, const TypeData &data,
		// StationType stuff:
		const StationTypeData &stData
	);

	virtual bool _Load(ItemFactory &factory) { return Type::_Load(factory); }

	/*
	 * Data members:
	 */
	uint32 m_dockingBayGraphicID;
	uint32 m_hangarGraphicID;

	GPoint m_dockEntry;
	GVector m_dockOrientation;

	uint32 m_operationID;
	uint32 m_officeSlots;
	double m_reprocessingEfficiency;
	bool m_conquerable;
};

#endif /* !__STATION__H__INCL__ */


