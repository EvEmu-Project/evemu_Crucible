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



#include "StandingDB.h"
#include "../common/dbcore.h"
#include "../common/logsys.h"
#include "../common/EVEDBUtils.h"

StandingDB::StandingDB(DBcore *db)
: ServiceDB(db)
{
}

StandingDB::~StandingDB() {
}


PyRepObject *StandingDB::GetNPCStandings() {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT "
		" fromID,toID,standing"
		" FROM npcStandings"
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}


PyRepObject *StandingDB::GetCharStandings(uint32 characterID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT "
		" toID, standing"
		" FROM chrStandings"
		" WHERE characterID=%lu", characterID
	))
	{
		_log(SERVICE__ERROR, "Error in GetCharStandings query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}


PyRepObject *StandingDB::GetCharPrimeStandings(uint32 characterID) {
	DBQueryResult res;

#ifndef WIN32
#warning hacking GetCharPrimeStandings until we know what it does
#endif
	if(!m_db->RunQuery(res,
		"SELECT "
		" itemID AS ownerID,"
		" itemName AS ownerName,"
		" typeID"
		" FROM entity"
		" WHERE itemID < 0"
	))
	{
		_log(SERVICE__ERROR, "Error in GetCharPrimeStandings query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}


PyRepObject *StandingDB::GetCharNPCStandings(uint32 characterID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT "
		" fromID, standing"
		" FROM chrNPCStandings"
		" WHERE characterID=%lu", characterID
	))
	{
		_log(SERVICE__ERROR, "Error in GetCharNPCStandings query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

PyRepObject *StandingDB::GetStandingTransactions(uint32 characterID) {
	DBQueryResult res;

	if (!m_db->RunQuery(res,
		" SELECT * FROM chrStandingChanges "
		" WHERE toID = %lu ", characterID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return (DBResultToRowset(res));
}





















