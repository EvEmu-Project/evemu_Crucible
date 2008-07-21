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

#include "MapDB.h"
#include "../common/dbcore.h"
#include "../common/logsys.h"
#include "../common/EVEDBUtils.h"

MapDB::MapDB(DBcore *db)
: ServiceDB(db)
{
}

MapDB::~MapDB() {
}

PyRepObject *MapDB::GetPseudoSecurities() {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		"	solarSystemID, security"
		" FROM mapSolarSystems"
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

PyRepObject *MapDB::GetStationExtraInfo() {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		"	stationID,solarSystemID,operationID,stationTypeID,corporationID AS ownerID"
		" FROM staStations"
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

PyRepObject *MapDB::GetStationOpServices() {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		"	operationID, serviceID"
		" FROM staOperationServices"
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

PyRepObject *MapDB::GetStationServiceInfo() {
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		"	serviceID,serviceName"
		" FROM staServices"
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}
























