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

ReprocessingDB::ReprocessingDB(DBcore *db)
: ServiceDB(db)
{
}

ReprocessingDB::~ReprocessingDB() {
}

bool ReprocessingDB::IsRefinable(const uint32 typeID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
				"SELECT NULL"
				" FROM typeActivityMaterials"
				" WHERE typeID=%lu"
				" AND recycle = 0"
				" LIMIT 1",
				typeID))
	{
		_log(DATABASE__ERROR, "Failed to check ore type ID: %s.", res.error.c_str());
		return(false);
	}

	DBResultRow row;
	return(res.GetRow(row));
}

bool ReprocessingDB::IsRecyclable(const uint32 typeID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
				"SELECT NULL FROM typeActivityMaterials"
				" LEFT JOIN invBlueprintTypes ON typeID = blueprintTypeID"
				" WHERE damagePerJob = 1 AND ("
				"   (activityID = 6 AND typeID = %lu)"
				"   OR"
				"	(activityID = 1 AND productTypeID = %lu))"
				" AND recycle = 1"
				" LIMIT 1",
				typeID, typeID))
	{
		_log(DATABASE__ERROR, "Failed to check item type ID: %s.", res.error.c_str());
		return(false);
	}

	DBResultRow row;
	return(res.GetRow(row));
}

bool ReprocessingDB::LoadStatic(const uint32 stationID, double &efficiency, double &tax) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
				"SELECT reprocessingEfficiency, reprocessingStationsTake"
				" FROM staStations"
				" WHERE stationID=%lu",
				stationID))
	{
		_log(DATABASE__ERROR, "Failed to get reprocessing info for station %lu: '%s'.", stationID, res.error.c_str());
		return(false);
	}

	DBResultRow row;
	
	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No data found for stationID %lu.", stationID);
		return(false);
	}

	efficiency = row.GetDouble(0);
	tax = row.GetDouble(1);

	return(true);
}

uint32 ReprocessingDB::GetPortionSize(const uint32 typeID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
				"SELECT portionSize"
				" FROM invTypes"
				" WHERE typeID=%lu",
				typeID
				))
	{
		_log(DATABASE__ERROR, "Unable to get portion size for type ID %lu: '%s'", typeID, res.error.c_str());
		return(0);
	}

	DBResultRow row;

	if(!res.GetRow(row)) {
		_log(DATABASE__ERROR, "No portion size found for type ID %lu.", typeID);
		return(0);
	}

	return(row.GetUInt(0));

}

bool ReprocessingDB::GetRecoverables(const uint32 typeID, std::vector<Recoverable> &into) {
	DBQueryResult res;
	DBResultRow row;

	if(!m_db->RunQuery(res,
				"SELECT requiredTypeID, MIN(quantity) FROM typeActivityMaterials"
				" LEFT JOIN invBlueprintTypes ON typeID = blueprintTypeID"
				" WHERE damagePerJob = 1 AND ("
				"   (activityID = 6 AND typeID = %lu)"
				"   OR"
				"	(activityID = 1 AND productTypeID = %lu))"
				" GROUP BY requiredTypeID",
				typeID, typeID, typeID))
	{
		_log(DATABASE__ERROR, "Unable to get recoverables for type ID %lu: '%s'", typeID, res.error.c_str());
		return(false);
	}

	Recoverable rec;

	while(res.GetRow(row)) {
		rec.typeID = row.GetInt(0);
		rec.amountPerBatch = row.GetInt(1);
		into.push_back(rec);
	}

	return(true);
}
