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
	// Type stuff:
	const Group &_group,
	const TypeData &_data,
	// StationType stuff:
	const StationTypeData &_stData)
: Type(_id, _group, _data),
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

StationType *StationType::_Load(ItemFactory &factory, uint32 stationTypeID
) {
	// pull data
	TypeData data;
	if(!factory.db().GetType(stationTypeID, data))
		return NULL;

	// obtain group
	const Group *g = factory.GetGroup(data.groupID);
	if(g == NULL)
		return NULL;

	// verify it's a station type
	if(g->id() != EVEDB::invGroups::Station) {
		_log(ITEM__ERROR, "Trying to load %s as Station.", g->name().c_str());
		return NULL;
	}

	return(
		StationType::_Load(factory, stationTypeID, *g, data)
	);
}

StationType *StationType::_Load(ItemFactory &factory, uint32 stationTypeID,
	// Type stuff:
	const Group &group, const TypeData &data
) {
	// get station type data
	StationTypeData stData;
	if(!factory.db().GetStationType(stationTypeID, stData))
		return NULL;

	return(
		StationType::_Load(factory, stationTypeID, group, data, stData)
	);
}

StationType *StationType::_Load(ItemFactory &factory, uint32 stationTypeID,
	// Type stuff:
	const Group &group, const TypeData &data,
	// StationType stuff:
	const StationTypeData &stData
) {
	// ready to create
	return(new StationType(
		stationTypeID,
		group, data,
		stData
	));
}


