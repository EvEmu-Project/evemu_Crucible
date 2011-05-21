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

bool SystemDB::LoadSystemEntities(uint32 systemID, std::vector<DBSystemEntity> &into) {
	DBQueryResult res;
	
	if(!sDatabase.RunQuery(res,
		"SELECT "
		" itemID,typeID,groupID,orbitID,"
		" x,y,z,radius,security,itemName"
		" FROM mapDenormalize"
		" WHERE solarSystemID=%u", systemID
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}
	
	DBResultRow row;
	DBSystemEntity entry;
	while(res.GetRow(row)) {
		entry.itemID = row.GetInt(0);
		entry.typeID = row.GetInt(1);
		entry.groupID = row.GetInt(2);
		entry.orbitID = (row.IsNull(3) ? 0 : row.GetInt(3));
		entry.position.x = row.GetDouble(4);
		entry.position.y = row.GetDouble(5);
		entry.position.z = row.GetDouble(6);
		entry.radius = row.GetDouble(7);
		entry.security = (row.IsNull(8) ? 0.0 : row.GetDouble(8));
		entry.itemName = row.GetText(9);
		into.push_back(entry);
	}

	return true;
}

bool SystemDB::LoadSystemDynamicEntities(uint32 systemID, std::vector<DBSystemDynamicEntity> &into) {
	DBQueryResult res;
	
    if(!sDatabase.RunQuery(res,
		"SELECT"
		"	entity.itemID,"
        "   entity.itemName,"
		"	entity.typeID,"
        "   entity.ownerID,"
        "   entity.locationID,"
        "   entity.flag,"
		"	invTypes.groupID,"
		"	invGroups.categoryID,"
        "   0,"//"   character_.corporationID,"
        "   0,"//"   corporation.allianceID,"
        "   x,"
        "   y,"
        "   z"
		" FROM entity, invTypes, invGroups"//, character_, corporation"
		" WHERE"
		"	    entity.typeID=invTypes.typeID"
		"	AND invTypes.groupID=invGroups.groupID"
		"	AND invGroups.categoryID NOT IN (%d,%d)"
        //"   AND character_.characterID = entity.ownerID"
        //"   AND corporation.corporationID = character_.corporationID"
		"	AND locationID=%u",
		//excluded categories:
			//celestials:
            EVEDB::invCategories::_System, EVEDB::invCategories::Station,
            //NPCs:
			//EVEDB::invCategories::Entity,
		systemID
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return false;
	}
	
	DBResultRow row;
	DBSystemDynamicEntity entry;
	while(res.GetRow(row)) {
		entry.itemID = row.GetInt(0);
        entry.itemName = row.GetText(1);
		entry.typeID = row.GetInt(2);
        entry.ownerID = row.GetInt(3);
        entry.locationID = row.GetInt(4);
        entry.flag = row.GetInt(5);
		entry.groupID = row.GetInt(6);
		entry.categoryID = row.GetInt(7);
        entry.corporationID = row.GetInt(8);
        entry.allianceID = row.GetInt(9);
        entry.x = row.GetDouble(10);
        entry.y = row.GetDouble(11);
        entry.z = row.GetDouble(12);
		into.push_back(entry);
	}

	return true;
}


PyObject *SystemDB::ListFactions() {
	DBQueryResult res;
	
	if(!sDatabase.RunQuery(res,
		"SELECT "
		" factionID"
		" FROM chrFactions "))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}
	
	return DBResultToRowset(res);
}

PyObject *SystemDB::ListJumps(uint32 stargateID) {
	DBQueryResult res;
	
	if(!sDatabase.RunQuery(res,
		"SELECT "
		" celestialID AS toCelestialID,"
		" solarSystemID AS locationID"
		" FROM mapJumps "
		"	LEFT JOIN mapDenormalize ON celestialID=itemID"
		" WHERE stargateID=%u", stargateID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}
	
	return DBResultToRowset(res);
}

uint32 SystemDB::GetObjectLocationID( uint32 itemID ) {
	
	//TODO: implement database logic and query

	return 0;
}






























