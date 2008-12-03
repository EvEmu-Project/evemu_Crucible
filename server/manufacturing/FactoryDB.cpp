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

FactoryDB::FactoryDB(DBcore *db)
: ServiceDB(db)
{
}

FactoryDB::~FactoryDB() {
}

PyRep *FactoryDB::GetMaterialsForTypeWithActivity(const uint32 blueprintTypeID) const {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
				"SELECT requiredTypeID, quantity, damagePerJob, activityID"
				" FROM typeActivityMaterials"
				" WHERE typeID = %lu",
				blueprintTypeID))
	{
		_log(DATABASE__ERROR, "Could not retrieve materials for type with activity %lu : %s", blueprintTypeID, res.error.c_str());
		return NULL;
	}

	return(DBResultToRowset(res));
}

PyRep *FactoryDB::GetMaterialCompositionOfItemType(const uint32 typeID) const {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
				"SELECT requiredTypeID AS typeID, quantity"
				" FROM typeActivityMaterials"
				" WHERE typeID = (SELECT blueprintTypeID FROM invBlueprintTypes WHERE productTypeID = %lu)"
				" AND activityID = 1"
				" AND damagePerJob = 1",
				typeID))
	{
		_log(DATABASE__ERROR, "Could not retrieve materials for type with activity %lu : %s", typeID, res.error.c_str());
		return NULL;
	}

	return(DBResultToRowset(res));
}

