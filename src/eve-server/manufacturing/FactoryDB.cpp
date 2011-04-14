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
	Author:		Zhur
*/

#include "EVEServerPCH.h"

PyRep *FactoryDB::GetMaterialsForTypeWithActivity(const uint32 blueprintTypeID) const {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
				"SELECT requiredTypeID, quantity, damagePerJob, activityID"
				" FROM typeActivityMaterials"
				" WHERE typeID = %u",
				blueprintTypeID))
	{
		_log(DATABASE__ERROR, "Could not retrieve materials for type with activity %u : %s", blueprintTypeID, res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}

PyRep *FactoryDB::GetMaterialCompositionOfItemType(const uint32 typeID) const {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
				"SELECT requiredTypeID AS typeID, quantity"
				" FROM typeActivityMaterials"
				" WHERE typeID = (SELECT blueprintTypeID FROM invBlueprintTypes WHERE productTypeID = %u)"
				" AND activityID = 1"
				" AND damagePerJob = 1",
				typeID))
	{
		_log(DATABASE__ERROR, "Could not retrieve materials for type with activity %u : %s", typeID, res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}

