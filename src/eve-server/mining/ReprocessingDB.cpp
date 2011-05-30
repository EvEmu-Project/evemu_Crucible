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

bool ReprocessingDB::IsRefinable(const uint32 typeID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
				"SELECT NULL"
				" FROM typeActivityMaterials"
				" WHERE typeID=%u"
				" AND recycle = 0"
				" LIMIT 1",
				typeID))
	{
		_log(DATABASE__ERROR, "Failed to check ore type ID: %s.", res.error.c_str());
		return false;
	}

	DBResultRow row;
	return(res.GetRow(row));
}

bool ReprocessingDB::IsRecyclable(const uint32 typeID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
				"SELECT NULL FROM typeActivityMaterials"
				" LEFT JOIN invBlueprintTypes ON typeID = blueprintTypeID"
				" WHERE damagePerJob = 1 AND ("
				"   (activityID = 6 AND typeID = %u)"
				"   OR"
				"	(activityID = 1 AND productTypeID = %u)"
				") AND recycle = 1"
				" LIMIT 1",
				typeID, typeID))
	{
		_log(DATABASE__ERROR, "Failed to check item type ID: %s.", res.error.c_str());
		return false;
	}

	DBResultRow row;
	return(res.GetRow(row));
}

bool ReprocessingDB::LoadStatic(const uint32 stationID, double &efficiency, double &tax) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
				"SELECT reprocessingEfficiency, reprocessingStationsTake"
				" FROM staStations"
				" WHERE stationID=%u",
				stationID))
	{
		_log(DATABASE__ERROR, "Failed to get reprocessing info for station %u: '%s'.", stationID, res.error.c_str());
		return false;
	}

	DBResultRow row;
	
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No data found for stationID %u.", stationID);
		return false;
	}

	efficiency = row.GetDouble(0);
	tax = row.GetDouble(1);

	return true;
}

bool ReprocessingDB::GetRecoverables(const uint32 typeID, std::vector<Recoverable> &into) {
	DBQueryResult res;
	DBResultRow row;

	if(!sDatabase.RunQuery(res,
				"SELECT requiredTypeID, MIN(quantity) FROM typeActivityMaterials"
				" LEFT JOIN invBlueprintTypes ON typeID = blueprintTypeID"
				" WHERE damagePerJob = 1 AND ("
				"   (activityID = 6 AND typeID = %u)"
				"   OR"
				"	(activityID = 1 AND productTypeID = %u))"
				" GROUP BY requiredTypeID",
				typeID, typeID, typeID))
	{
		_log(DATABASE__ERROR, "Unable to get recoverables for type ID %u: '%s'", typeID, res.error.c_str());
		return false;
	}

	Recoverable rec;

	while(res.GetRow(row)) {
		rec.typeID = row.GetInt(0);
		rec.amountPerBatch = row.GetInt(1);
		into.push_back(rec);
	}

	return true;
}
