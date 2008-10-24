/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "EvemuPCH.h"

SystemDB::SystemDB(DBcore *db) : ServiceDB(db)
{
}

SystemDB::~SystemDB() {
}

bool SystemDB::LoadSystemEntities(uint32 systemID, std::vector<DBSystemEntity> &into) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" itemID,typeID,groupID,orbitID,"
		" x,y,z,radius,security,itemName"
		" FROM mapDenormalize"
		" WHERE solarSystemID=%lu", systemID
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	DBSystemEntity entry;
	while(res.GetRow(row)) {
		entry.itemID = row.GetInt(0);
		entry.typeID = row.GetInt(1);
		entry.groupID = row.GetInt(2);
		if(row.IsNull(3))
			entry.orbitID = 0;
		else
			entry.orbitID = row.GetInt(3);
		entry.position.x = row.GetDouble(4);
		entry.position.y = row.GetDouble(5);
		entry.position.z = row.GetDouble(6);
		entry.radius = row.GetDouble(7);
		entry.security = (row.IsNull(8) ? 0.0 : row.GetDouble(8));
		entry.itemName = row.GetText(9);
		into.push_back(entry);
	}

	return(true);
}

bool SystemDB::LoadSystemDynamicEntities(uint32 systemID, std::vector<DBSystemDynamicEntity> &into) {
	DBQueryResult res;
	using namespace EVEDB::invCategories;
	
	if(!m_db->RunQuery(res,
		"SELECT"
		"	entity.itemID,"
		"	entity.typeID,"
		"	invTypes.groupID,"
		"	invGroups.categoryID"
		" FROM entity, invTypes, invGroups"
		" WHERE"
		"	    entity.typeID=invTypes.typeID"
		"	AND invTypes.groupID=invGroups.groupID"
		"	AND invGroups.categoryID NOT IN (%d,%d,%d,%d)"
		"	AND locationID=%lu",
		//excluded categories:
			//celestials:
			_System, Celestial, Station,
			//NPCs:
			Entity,
		systemID
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(false);
	}
	
	DBResultRow row;
	DBSystemDynamicEntity entry;
	while(res.GetRow(row)) {
		entry.itemID = row.GetInt(0);
		entry.typeID = row.GetInt(1);
		entry.groupID = row.GetInt(2);
		entry.categoryID = row.GetInt(3);
		into.push_back(entry);
	}

	return(true);
}


PyRepObject *SystemDB::ListFactions() {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" factionID"
		" FROM chrFactions "))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

PyRepObject *SystemDB::ListJumps(uint32 stargateID) {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" celestialID AS toCelestialID,"
		" solarSystemID AS locationID"
		" FROM mapJumps "
		"	LEFT JOIN mapDenormalize ON celestialID=itemID"
		" WHERE stargateID=%lu", stargateID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}
std::string SystemDB::GetSystemSecurity(uint32 sysid) {
	DBQueryResult res;

	if (!m_db->RunQuery(res,
		" SELECT "
		" securityClass "
		" FROM mapSolarSystems "
		" WHERE solarsystemID = %lu ", sysid))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return "A";
	}

	DBResultRow row;
	res.GetRow(row);

	// easier handling
	std::string ret(row.GetText(0));
	return ret;
}






























